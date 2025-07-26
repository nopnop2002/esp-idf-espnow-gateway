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
#include <libgen.h>
#include <time.h>
#include "MQTTClient.h"

#define ADDRESS     "tcp://broker.emqx.io"
//#define ADDRESS     "tcp://broker.hivemq.com:1883"
#define QOS	    1
#define RETAINED    0
#define TIMEOUT     10000L

#define PACKET_LENGTH 400 //Approximate
#define MYDATA 18	  //0x12
#define MAX_PACKET_LEN 1000

//#define LOGGING

/*our MAC address*/
//{0xF8, 0x1A, 0x67, 0xB7, 0xeB, 0x0B};

/*ESP8266 host MAC address*/
//{0x84,0xF3,0xEB,0x73,0x55,0x0D};


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

// Must match the receiver structure
typedef struct struct_message {
  char topic[64];
  char payload[64];
} struct_message;

// Create a struct_message called myData
struct_message myData;

void print_packet(uint8_t *data, int len)
{
    printf("----------------------------new packet-----------------------------------\n");
    int i;
    for (i = 0; i < len; i++)
    {
	if (i % 16 == 0)
	    printf("\n");
	printf("0x%02x, ", data[i]);
    }
    printf("\n\n");
}

int create_raw_socket(char *dev, struct sock_fprog *bpf)
{
    struct sockaddr_ll sll;
    struct ifreq ifr;
    int fd, ifi, rb, attach_filter;

    bzero(&sll, sizeof(sll));
    bzero(&ifr, sizeof(ifr));

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    assert(fd != -1);

    strncpy((char *)ifr.ifr_name, dev, IFNAMSIZ);
    ifi = ioctl(fd, SIOCGIFINDEX, &ifr);
    assert(ifi != -1);

    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_family = PF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_pkttype = PACKET_OTHERHOST;

    rb = bind(fd, (struct sockaddr *)&sll, sizeof(sll));
    assert(rb != -1);

    attach_filter = setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, bpf, sizeof(*bpf));
    assert(attach_filter != -1);

    return fd;
}

int main(int argc, char **argv)
{
    assert(argc == 2);

    uint8_t buff[MAX_PACKET_LEN] = {0};
    int sock_fd;
    char *dev = argv[1];
    struct sock_fprog bpf = {FILTER_LENGTH, bpfcode};

    sock_fd = create_raw_socket(dev, &bpf); /* Creating the raw socket */

#ifdef LOGGING
    // log file name
    char szTmp[32];
    char exePath[1024];
    char logPath[1024];
    sprintf(szTmp, "/proc/%d/exe", getpid());
    int bytes = readlink(szTmp, exePath, sizeof(exePath));
    //printf("bytes=%d\n",bytes);
    if(bytes >= 0) exePath[bytes] = '\0';
    printf("exePath=%s\n",exePath);
    char * dname = dirname(exePath);
    printf("dname=%s\n",dname);
    sprintf(logPath, "%s/espnow.txt", dname);
    printf("logPath=%s\n",logPath);
#endif

    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    char clientId[40];
    pid_t c_pid = getpid();
    sprintf(clientId, "CLIENT-%d", c_pid);
    //printf("clientId=%s\n", clientId);
    printf("Connecting to %s\n", ADDRESS);
    MQTTClient_create(&client, ADDRESS, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
	printf("Failed to connect, return code %d\n", rc);
	exit(-1);
    }

    printf("\nWaiting to receive packets ........ \n");

    while (1)
    {
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock_fd, &fds);
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	int received = select(sock_fd+1, &fds, NULL, NULL, &tv);
	printf("received=%d\n",received);
	if (received == 0) {
	    MQTTClient_yield();
	    continue;
	}

	int len = recvfrom(sock_fd, buff, MAX_PACKET_LEN, MSG_TRUNC, NULL, 0);

	if (len < 0)
	{
	    perror("Socket receive failed or error");
	    break;
	}

	//printf("len:%d\n", len);
	//print_packet(buff, len);
	//print_packet(&buff[63], len-63);
	memcpy(myData.topic, &buff[63], sizeof(myData.topic));
	memcpy(myData.payload, &buff[127], sizeof(myData.payload));
	printf("myData.topic=[%s]\n",myData.topic);
	printf("myData.payload=[%s]\n",myData.payload);

#ifdef LOGGING
	FILE *fp = fopen(logPath, "a+");
	if (fp) {
	    time_t timer = time(NULL); 
	    struct tm *local; 
	    local = localtime(&timer);
	    int year = local->tm_year + 1900;
	    int month = local->tm_mon + 1;
	    int day = local->tm_mday;
	    int hour = local->tm_hour;
	    int minute = local->tm_min;
	    int second = local->tm_sec;
	    
	    fprintf(fp, "%d/%02d/%02d %02d:%02d:%02d %s %s\n", 
	    year,month,day,hour,minute,second,myData.topic, myData.payload);
	    fclose(fp);
	}
#endif

	pubmsg.payload = &myData.payload;
	pubmsg.payloadlen = strlen(myData.payload);
	pubmsg.qos = QOS;
	pubmsg.retained = RETAINED;
	MQTTClient_publishMessage(client, myData.topic, &pubmsg, &token);
	printf("Waiting for up to %d seconds for publication\n"
	"on topic %s for client with ClientID: %s\n", (int)(TIMEOUT/1000), myData.topic, clientId);
	rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
	printf("Message with delivery token %d delivered\n", token);
	//sleep(1);
    } // end while

    // never reach here
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    close(sock_fd);
    return 0;
}
