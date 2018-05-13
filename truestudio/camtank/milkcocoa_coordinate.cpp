#include <kernel.h>
#include "kernel_cfg.h"
#include "milkcocoa_coordinate.h"

#include "mbed.h"

#include "app_config.h"

#include "Zumo.h"
#include "MQTTBP3595.h"
#include "MClient.h"
#include "Milkcocoa.h"

/*======================================================================
 * global variables, macros
 *======================================================================*/
#define LED_NONE        0
#define LED_R           0x01
#define LED_G           0x02
#define LED_B           0x04
#define ZUMO_SPEED      80
#ifndef NODEID
# error Undefined NODEID
#endif  /* #ifndef NODEID */
#define MLK_DSTORE_BASE "zumo"
#define MLK_DSTORE_MAX  128
#define START_ACC_THR   1000    /* zumo start acceleration threshold */

/*------------------------------
 * private
 *------------------------------*/
DigitalOut led_r(LED1);
DigitalOut led_g(LED2);
DigitalOut led_b(LED3);
Serial pc(USBTX, USBRX);
Zumo zumo;
MQTTBP3595 *mqtt_sock;
MClient *mclient;
Milkcocoa *milkc;
static uint8_t blink_target;        /* led blink target */
static state_t state;               /* current state */
static event_t event;               /* event */
static char dstore[MLK_DSTORE_MAX]; /* milkcocoa datastore name */
static state_t state_trans_tbl[][ST_END] = {
    /*  INIT        ERR         STOP        FORWARD */
    {   ST_STOP,    ST_NONE,    ST_NONE,    ST_NONE}, /* INIT_DONE */
    {   ST_NONE,    ST_NONE,    ST_FORWARD, ST_STOP}, /* ACC_DETECT */
    {   ST_ERR,     ST_NONE,    ST_ERR,     ST_ERR},  /* FATAL_ERR */
    {   ST_NONE,    ST_NONE,    ST_NONE,    ST_STOP}, /* FORCE_STOP */
    {   ST_NONE,    ST_NONE,    ST_FORWARD, ST_NONE}  /* EV_CTRL_FWD */
};

/*======================================================================
 * prototype declarations for private functions
*======================================================================*/
static void led_control(uint8_t rgb);
static void on_mlk_push(MQTT::MessageData& data);
static int entry_init(void);
static int entry_err(void);
static int entry_stop(void);
static int entry_forward(void);

/*------------------------------*/
static tran_tbl_t tran_func[ST_END] = {
    {&entry_init},
    {&entry_err},
    {&entry_stop},
    {&entry_forward}
};

/*----------------------------------------------------------------------*/
static void led_control(uint8_t rgb)
{
    led_r = rgb & 0x01;
    led_g = (rgb & 0x02) >> 1;
    led_b = (rgb & 0x04) >> 2;
    return;
}

/*----------------------------------------------------------------------*/
static void on_mlk_push(MQTT::MessageData& md)
{
    MQTT::Message &msg = md.message;
    DataElement data = DataElement((char*)msg.payload);
    char *ctrl = data.getString("control");

    /* control message pushed */
    if (ctrl)
    {
        pc.printf("onpush:control\r\n");
        event = EV_CTRL_FWD;
        wup_tsk(TASKID_ST_TRAN);
    }
    return;
}

/*----------------------------------------------------------------------*/
static int entry_init(void)
{
    int ret;
    int retry;

    mqtt_sock = NULL;
    mclient = NULL;
    milkc = NULL;
    blink_target = LED_NONE;
    state = ST_INIT;
    event = EV_END;
    memset(dstore, 0, sizeof(dstore));

    led_control(LED_R|LED_G|LED_B);

    pc.baud(57600);
    dly_tsk(1000);
    pc.printf("\r\nReady\r\n");

    /* establish WLAN connection */
    pc.printf("Associating with AP\r\n");
    mqtt_sock = new MQTTBP3595();
    retry = NET_RETRIES;
    ret = mqtt_sock->Connect(WLAN_SSID, WLAN_PSK);
    while ((ret != 0) && (retry > 0))
    {
        pc.printf("Cannot associate with AP: retry=%d\r\n", retry);
        mqtt_sock->disconnect();
        ret = mqtt_sock->Connect(WLAN_SSID, WLAN_PSK, WLAN_SECURITY);
        retry--;
    }
    if (ret != 0)
    {
        pc.printf("Could not associate with AP\r\n");
        return -1;
    }
    pc.printf("My IP address is %s\r\n", mqtt_sock->GetWlan()->getIPAddress());

    led_control(LED_G|LED_B);

    mclient = new MClient(mqtt_sock);
    milkc = new Milkcocoa(mclient, MQTT_SERVER, MILKCOCOA_SERVERPORT, MILKCOCOA_APP_ID, MQTT_CLIENTID);
    /* Connect to Milkcocoa */
    retry = NET_RETRIES;
    ret = milkc->connect();
    while ((ret != 0) && (retry > 0))
    {
        pc.printf("Cannot establish TCP session with MQTT server: retry=%d\r\n", retry);
        ret = milkc->connect();
        retry--;
    }
    if (ret != 0)
    {
        pc.printf("Could not establish TCP session with MQTT server\r\n");
        return -1;
    }
    /* generate datastore name */
    snprintf(dstore, sizeof(dstore)-1, "%s/%X", MLK_DSTORE_BASE, NODEID);
    pc.printf("Connected to milkcocoa for publish: datastore=%s\r\n", dstore);

    /* notify IP address */
    DataElement data = DataElement();
    data.setValue("ip_address", mqtt_sock->GetWlan()->getIPAddress());
    milkc->push(dstore, data);

    /* subscribe to datastore */
    ret = milkc->on(dstore, "push", on_mlk_push);
    if (!ret)
    {
        pc.printf("Could not subscribe to %s\r\n", dstore);
        return -1;
    }
    pc.printf("Subscribe to %s\r\n", dstore);

    return 0;
}

/*----------------------------------------------------------------------*/
static int entry_err(void)
{
    blink_target = LED_RED;
    sta_cyc(CYCID_LED_BLK);

    if (milkc)
    {
        delete milkc;
    }
    if (mclient)
    {
        delete mclient;
    }
    if (mqtt_sock)
    {
        delete mqtt_sock;
    }

    return 0;
}

/*----------------------------------------------------------------------*/
static int entry_stop(void)
{
    led_control(LED_NONE);
    sta_cyc(CYCID_TIM);
    pc.printf("zumo.stop\r\n");
    zumo.driveTank(0,0);
    return 0;
}

/*----------------------------------------------------------------------*/
static int entry_forward(void)
{
    led_control(LED_G);
    zumo.driveTank(ZUMO_SPEED, ZUMO_SPEED);
    pc.printf("zumo.start\r\n");
    sta_alm(ALMID_STOP, 800);
    return 0;
}

/*======================================================================*/
void task_main(intptr_t exinf)
{
    if (entry_init() < 0)
    {
        event = EV_FATAL_ERR;
        wup_tsk(TASKID_ST_TRAN);
        ext_tsk();
    }

    event = EV_INIT_DONE;
    wup_tsk(TASKID_ST_TRAN);

    for (;;)
    {
        milkc->loop();
        dly_tsk(500);
    }

    ext_tsk();
}

/*----------------------------------------------------------------------*/
void task_st_tran(intptr_t exinf)
{
    state_t pre_state;
    state_t new_state;
    int (*func)();

    for (;;)
    {
        slp_tsk();

        func = NULL;
        pre_state = state;

        new_state = state_trans_tbl[event][pre_state];
        if (new_state != ST_NONE)
        {
            state = new_state;
            pc.printf("state=%x>%x\r\n", pre_state, state);
            func = tran_func[new_state].func;
        }

        /* call state-entry function */
        if (func)
        {
            (*func)();
            act_tsk(TASKID_NTF_MLK);
        }
    }

    ext_tsk();
}

/*----------------------------------------------------------------------*/
void task_acc_mon(intptr_t exinf)
{
    static int diffsum_x = 0;
    static int prev_x = 0;
    static uint8_t cnt = 0;
    short x, y, z;

    for (;;)
    {
        slp_tsk();

        zumo.getAcceleration(&x, &y, &z);
        diffsum_x += abs(prev_x - x);
        prev_x = x;

        cnt++;
        if ((cnt & 0x03) == 0)
        {
            diffsum_x = diffsum_x >> 2;
            if (diffsum_x > START_ACC_THR)
            {
                event = EV_ACC_DETECT;
                wup_tsk(TASKID_ST_TRAN);
            }

            diffsum_x = 0;
            cnt = 0;
        }
    }

    ext_tsk();
}

/*----------------------------------------------------------------------*/
/* notify mlikcocoa state transision */
void task_ntf_mlk(intptr_t exinf)
{
    DataElement data = DataElement();
    data.setValue("state", state);
    milkc->push(dstore, data);

    ext_tsk();
}

/*----------------------------------------------------------------------*/
void cyc_timer(intptr_t exinf)
{
    iwup_tsk(TASKID_ACC_MON);
}

/*----------------------------------------------------------------------*/
void cyc_led_blink(intptr_t exinf)
{
    static uint8_t led_stat = LED_NONE;
    led_stat ^= blink_target;
    led_control(led_stat);
}

/*----------------------------------------------------------------------*/
void alm_stop(intptr_t exinf)
{
    event = EV_FORCE_STOP;
    iwup_tsk(TASKID_ST_TRAN);
}
