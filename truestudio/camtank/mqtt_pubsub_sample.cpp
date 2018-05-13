#include <kernel.h>
#include "kernel_cfg.h"
#include "app.h"
#include "mbed.h"
#include "app_config.h"

#include "Zumo.h"
#include "MQTTBP3595.h"
#include "MClient.h"

#define MQTT_SERV      "mqtt_server_name_here"
#define MQTT_PORT      1883

/*----------------------------------------------------------------------*/
/* prototype declarations */
static void mqtt_msg_handler(MQTT::MessageData& data);

/* global variables */
DigitalOut led1(LED1);

Serial pc(USBTX, USBRX);
Zumo zumo;

/*----------------------------------------------------------------------*/
void task_main(intptr_t exinf)
{
    int ret;
    int retry;

    pc.baud(19200);
    dly_tsk(500);

    /* establish WLAN connection */
    pc.printf("Associating with AP\r\n");
    MQTTBP3595 *mqtt_sock = new MQTTBP3595();
    retry = NET_RETRIES;
    ret = mqtt_sock->Connect(WLAN_SSID, WLAN_PSK);
    while ((ret != 0) && (retry > 0))
    {
        pc.printf("Cannot associate with AP: retry=%d\n", retry);
        mqtt_sock->disconnect();
        ret = mqtt_sock->Connect(WLAN_SSID, WLAN_PSK, WLAN_SECURITY);
        retry--;
    }
    if (ret != 0)
    {
        led1 = 1;
        pc.printf("Could not associate with AP\n");
        delete mqtt_sock;
        ext_tsk();
    }
    pc.printf("My IP address is %s\r\n", mqtt_sock->GetWlan()->getIPAddress());

    MClient *mclient = new MClient(mqtt_sock);
    /* TCP session to MQTT server  */
    retry = NET_RETRIES;
    ret = mclient->connect(MQTT_SERV, MQTT_PORT);
    while ((ret != 0) && (retry > 0))
    {
        pc.printf("Cannot establish TCP session with MQTT server: retry=%d\n", retry);
        ret = mclient->connect(MQTT_SERV, MQTT_PORT);
        retry--;
    }
    if (ret != 0)
    {
        led1 = 1;
        pc.printf("Could not establish TCP session with MQTT server\r\n");
        delete mclient;
        delete mqtt_sock;
        ext_tsk();
    }
    /* connect to MQTT server */
    retry = NET_RETRIES;
#ifdef USE_SPECIFIC_CLIENT_ID
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    char client_id[16];
    snprintf(client_id, sizeof(client_id), "zumo_%d", (int)((float)rand()/RAND_MAX*100000));
	data.keepAliveInterval = 20;
	data.cleansession = 1;
	data.MQTTVersion = 4;
	data.clientID.cstring = client_id;
	data.username.cstring = NULL;
	data.password.cstring = NULL;
    ret = mclient->connect(data);
    while ((ret != 0) && (retry > 0))
    {
        led1 = 1;
        pc.printf("Cannot connect to MQTT server: retry=%d\n", retry);
        ret = mclient->connect(data);
        retry--;
    }
#else  /* #ifdef USE_SPECIFIC_CLIENT_ID */
    ret = mclient->connect();
    while ((ret != 0) && (retry > 0))
    {
        led1 = 1;
        pc.printf("Cannot connect to MQTT server: retry=%d\n", retry);
        ret = mclient->connect();
        retry--;
    }
#endif /* #ifdef USE_SPECIFIC_CLIENT_ID */
    if (ret != 0)
    {
        led1 = 1;
        pc.printf("Could not connect to MQTT server\n");
        mclient->disconnect();
        delete mclient;
        delete mqtt_sock;
        ext_tsk();
    }

    pc.printf("Init complete\r\n");

    /*--------------------------------------------------*/

    /* subscribe to topic */
    char sub_topic[] = "/pbl1/teacher/sub_test";
    ret = mclient->subscribe(sub_topic, MQTT::QOS0, mqtt_msg_handler);
    if (ret != 0)
    {
        pc.printf("Warning: cannot subscribe to %s\n", sub_topic);
    }

    /* create MQTT message */
    char buf[256];
    char topic[]   = "/pbl1/teacher/test";
    unsigned char cnt;
    MQTT::Message msg;
    msg.qos        = MQTT::QOS0;
    msg.retained   = 0;
    msg.dup        = false;
    msg.payload    = (void*)buf;

    for (cnt=0; cnt < 30; cnt++)
    {
        led1 = 1;
        snprintf(buf, sizeof(buf), "Test: %d", cnt);
        msg.payloadlen = strlen(buf);
        ret = mclient->publish(topic, msg);
        if (ret != 0)
        {
            pc.printf("Cannot publish to %s: %d\r\n", topic, ret);
        }
        led1 = 0;
        mclient->yield(1000);
    }

    for (;;)
    {
        mclient->yield(200);
    }

    /* close connection */
    mclient->disconnect();
    delete mclient;
    delete mqtt_sock;

    ext_tsk();
    return;
}

/*----------------------------------------------------------------------*/
static void mqtt_msg_handler(MQTT::MessageData& data)
{
    MQTT::Message *msg = &data.message;
    char message[1024];
    char topic[1024];
    memcpy(message, msg->payload, msg->payloadlen);
    message[msg->payloadlen] = '\0';
    memcpy(topic, data.topicName.lenstring.data, data.topicName.lenstring.len);
    topic[data.topicName.lenstring.len] = '\0';

    pc.printf("topic=%s, message=%s\n", topic, message);

    return;
}
