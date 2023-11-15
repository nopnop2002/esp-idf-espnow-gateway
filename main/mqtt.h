typedef enum {PUBLISH, SUBSCRIBE, STOP} TOPIC_TYPE;

typedef struct {
    int topic_type;
    int topic_len;
    char topic[64];
    int data_len;
    char data[64];
} MQTT_t;

