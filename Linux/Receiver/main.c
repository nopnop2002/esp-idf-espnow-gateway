/*
Florenc Caminade
Thomas FLayols
Etienne Arlaud

Receive raw 802.11 packet and filter ESP-NOW vendor specific action frame using BPF filters.
https://hackaday.io/project/161896
https://github.com/thomasfla/Linux-ESPNOW

Adapted from :
https://stackoverflow.com/questions/10824827/raw-sockets-communication-over-wifi-receiver-not-able-to-receive-packets

1/Find your wifi interface:
$ iwconfig

2/Setup your interface in monitor mode :
$ sudo ifconfig wlp5s0 down
$ sudo iwconfig wlp5s0 mode monitor
$ sudo ifconfig wlp5s0 up
$ sudo iwconfig wlp5s0 channel 11

3/Run this code as root
*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <assert.h>
#include <linux/filter.h>
#include <esp_rom_crc.h>

#define MAX_PACKET_LEN 1512 // For version v2.0, x = 1512(1470 + 6*7), for version v1.0, x = 257(250 + 7)

//filter action frame packets
  //Equivalent for tcp dump :
	//type 0 subtype 0xd0 and wlan[24:4]=0x7f18fe34 and wlan[32]=221 and wlan[33:4]&0xffffff = 0x18fe34 and wlan[37]=0x4
//NB : There is no filter on source or destination addresses, so this code will 'receive' the action frames sent by this computer...
#define FILTER_LENGTH 20
static struct sock_filter bpfcode[FILTER_LENGTH] = {
  { 0x30, 0, 0, 0x00000003 },	// ldb [3]	// radiotap header length : MS byte
  { 0x64, 0, 0, 0x00000008 },	// lsh #8	// left shift it
  { 0x7, 0, 0, 0x00000000 },	// tax		// 'store' it in X register
  { 0x30, 0, 0, 0x00000002 },	// ldb [2]	// radiotap header length : LS byte
  { 0x4c, 0, 0, 0x00000000 },	// or  x	// combine A & X to get radiotap header length in A
  { 0x7, 0, 0, 0x00000000 },	// tax		// 'store' it in X
  { 0x50, 0, 0, 0x00000000 },	// ldb [x + 0]		// right after radiotap header is the type and subtype
  { 0x54, 0, 0, 0x000000fc },	// and #0xfc		// mask the interesting bits, a.k.a 0b1111 1100
  { 0x15, 0, 10, 0x000000d0 },	// jeq #0xd0 jt 9 jf 19	// compare the types (0) and subtypes (0xd)
  { 0x40, 0, 0, 0x00000018 },	// Ld  [x + 24]			// 24 bytes after radiotap header is the end of MAC header, so it is category and OUI (for action frame layer)
  { 0x15, 0, 8, 0x7f18fe34 },	// jeq #0x7f18fe34 jt 11 jf 19	// Compare with category = 127 (Vendor specific) and OUI 18:fe:34
  { 0x50, 0, 0, 0x00000020 },	// ldb [x + 32]				// Begining of Vendor specific content + 4 ?random? bytes : element id
  { 0x15, 0, 6, 0x000000dd },	// jeq #0xdd jt 13 jf 19		// element id should be 221 (according to the doc)
  { 0x40, 0, 0, 0x00000021 },	// Ld  [x + 33]				// OUI (again!) on 3 LS bytes
  { 0x54, 0, 0, 0x00ffffff },	// and #0xffffff			// Mask the 3 LS bytes
  { 0x15, 0, 3, 0x0018fe34 },	// jeq #0x18fe34 jt 16 jf 19		// Compare with OUI 18:fe:34
  { 0x50, 0, 0, 0x00000025 },	// ldb [x + 37]				// Type
  { 0x15, 0, 1, 0x00000004 },	// jeq #0x4 jt 18 jf 19			// Compare type with type 0x4 (corresponding to ESP_NOW)
  { 0x6, 0, 0, 0x00040000 },	// ret #262144	// return 'True'
  { 0x6, 0, 0, 0x00000000 },	// ret #0	// return 'False'
};

typedef struct {
    uint8_t type;                         //Broadcast or unicast ESPNOW data.
    uint8_t state;                        //Indicate that if has received broadcast ESPNOW data or not.
    uint16_t seq_num;                     //Sequence number of ESPNOW data.
    uint16_t crc;                         //CRC16 value of ESPNOW data.
    uint32_t magic;                       //Magic number which is used to determine which device to send unicast ESPNOW data.
    uint8_t payload[0];                   //Real payload of ESPNOW data.
} __attribute__((packed)) example_espnow_data_t;

void print_packet(char * title, uint8_t *data, int len)
{
	printf("\n------------------- %s len:%d -------------------\n", title, len);
	int i;
	for (i = 0; i < len; i++) {
		if (i % 16 == 0) printf("\n");
		printf("0x%02x, ", data[i]);
	}
	printf("\n\n");

	int offset = 24;
	printf("MAC Header:");
	for (i = 0; i < 24; i++) {
		if (i % 16 == 0) printf("\n");
		printf("0x%02x, ", data[offset]);
		offset++;
	}
	printf("\n");

	printf("Category Code: 0x%.02x\n", data[offset]);
	printf("Organization Identifier: 0x%.02x-0x%.02x-0x%.02x\n", data[offset+1], data[offset+2], data[offset+3]);
	printf("Random Values: 0x%.02x-0x%.02x-0x%.02x-0x%.02x\n", data[offset+4], data[offset+5], data[offset+6], data[offset+7]);

	offset = 56;
	int totalBodyLength = 0;
	uint8_t *totalBody = malloc(1);

	while(1) {
		printf("\n");
		printf("Element ID: 0x%.02x\n", data[offset]);
		printf("Length: 0x%.02x\n", data[offset+1]);
		printf("Organization Identifier: 0x%.02x-0x%.02x-0x%.02x\n", data[offset+2], data[offset+3], data[offset+4]);
		printf("Type: 0x%.02x\n", data[offset+5]);
		unsigned int moreData = data[offset+6] & 0x10;
		unsigned int version = data[offset+6] & 0x0F;
		printf("More data: 0x%.02x\n", moreData);
		printf("Version: 0x%.02x\n", version);
		int bodyLength = data[offset+1] - 5;
		printf("Body Length:  %d\n", bodyLength);
		for (i = 0; i < bodyLength; i++) {
			if (i % 16 == 0) printf("\n");
			printf("0x%02x, ", data[i+offset+7]);
		}
		printf("\n\n");

		int _totalBodyLength = totalBodyLength;
		totalBodyLength = totalBodyLength + bodyLength;
		int *_totalBody = realloc(totalBody, totalBodyLength);
		if (_totalBody == NULL) {
			printf("realloc fail\n");
			free(totalBody);
			return;
		} 
		*totalBody = *_totalBody;
		memcpy(&totalBody[_totalBodyLength], &data[offset+7], bodyLength);

		if (moreData == 0x00) break;
		offset = offset + 257;
	}

	offset = 56;
	example_espnow_data_t example_espnow_data;
	memcpy(&example_espnow_data, &data[offset+7], sizeof(example_espnow_data_t));
	printf("example_espnow_data.type: %d\n", example_espnow_data.type);
	printf("example_espnow_data.state: %d\n", example_espnow_data.state);
	printf("example_espnow_data.seq_num: %d\n", example_espnow_data.seq_num);
	printf("example_espnow_data.crc: 0x%04x\n", example_espnow_data.crc);
	printf("example_espnow_data.magic: 0x%08x\n", example_espnow_data.magic);

	example_espnow_data.crc = 0;
	memcpy(totalBody, &example_espnow_data, sizeof(example_espnow_data_t));
	uint16_t crc = esp_rom_crc16_le(UINT16_MAX, totalBody, totalBodyLength);
	printf("calculated crc: 0x%04x\n", crc);
	free(totalBody);
}

int create_raw_socket(char *dev, struct sock_fprog *bpf)
{
	struct sockaddr_ll s_dest_addr;
	struct ifreq ifr;
	int fd, ifi, rb, attach_filter;

	bzero(&s_dest_addr, sizeof(s_dest_addr));
	bzero(&ifr, sizeof(ifr));

	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	assert(fd != -1);

	strncpy((char *)ifr.ifr_name, dev, IFNAMSIZ);
	ifi = ioctl(fd, SIOCGIFINDEX, &ifr);
	assert(ifi != -1);

	s_dest_addr.sll_protocol = htons(ETH_P_ALL);
	s_dest_addr.sll_family = PF_PACKET;
	s_dest_addr.sll_ifindex = ifr.ifr_ifindex;
	s_dest_addr.sll_pkttype = PACKET_OTHERHOST;

	rb = bind(fd, (struct sockaddr *)&s_dest_addr, sizeof(s_dest_addr));
	assert(rb != -1);

	attach_filter = setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, bpf, sizeof(*bpf));
	assert(attach_filter != -1);

	return fd;
}

int main(int argc, char **argv)
{
	assert(argc == 2);

	uint8_t recv_data[MAX_PACKET_LEN] = {0};
	int sock_fd;
	char *dev = argv[1];
	struct sock_fprog bpf = {FILTER_LENGTH, bpfcode};

	sock_fd = create_raw_socket(dev, &bpf); /* Creating the raw socket */
	if (sock_fd == -1) {
		perror("Could not create the socket");
		return 1;
	}

	printf("\nWaiting to receive packets ........ \n");

	while (1)
	{
		int recv_len = recvfrom(sock_fd, recv_data, MAX_PACKET_LEN, MSG_TRUNC, NULL, 0);

		if (recv_len < 0) {
			perror("Socket receive failed or error");
			break;
		} else {
			print_packet("new packet", recv_data, recv_len);
		}
	}
	close(sock_fd);
	return 0;
}
