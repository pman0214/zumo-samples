/*----------------------------------------------------------------------*/
/* network setting */
#define WLAN_SSID               "ssid_here"
#define WLAN_PSK                "password_here"
/* specify if you use DHCP instead of static IP */
#define USE_DHCP
/* retry number of network connection */
#define NET_RETRIES             5

/* WLAN security:
 *   NSAPI_SECURITY_NONE,
 *   NSAPI_SECURITY_WEP,
 *   NSAPI_SECURITY_WPA,
 *   or NSAPI_SECURITY_WPA2
 */
#define WLAN_SECURITY           NSAPI_SECURITY_WPA2

#ifndef USE_DHCP
# define IP_ADDRESS             "192.168.10.200"   /* IP address */
# define SUBNET_MASK            "255.255.255.0"    /* subnet mask */
# define DEFAULT_GATEWAY        "192.168.10.1"     /* default gateway */
#endif

/*----------------------------------------------------------------------*/
/* camera */
/* camera input: VIDEO_CMOS_CAMERA or VIDEO_CVBS */
# define VIDEO_INPUT_METHOD     VIDEO_CMOS_CAMERA
/* Video format: VIDEO_YCBCR422, VIDEO_RGB888, or VIDEO_RGB565 */
#define VIDEO_INPUT_FORMAT      VIDEO_YCBCR422
/* 0 or 1, use 0 for VIDEO_CMOS_CAMERA */
#define USE_VIDEO_CH            0
/* 0 (NTSC) or 1(PAL), ignored for VIDEO_CVBS */
#define VIDEO_PAL               1

/*----------------------------------------------------------------------*/
/* SD card spi channel setting */
#define SD_SPICH            (2)

/*----------------------------------------------------------------------*/
/* Milkcocoa */
#define MILKCOCOA_APP_ID        "app_id_here"
#define MILKCOCOA_SERVERPORT    1883
const char MQTT_SERVER[] = MILKCOCOA_APP_ID ".mlkcca.com";
const char MQTT_CLIENTID[] = __TIME__ MILKCOCOA_APP_ID;
