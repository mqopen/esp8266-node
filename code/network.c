#include <c_types.h>
#include <ip_addr.h>
#include <espconn.h>
#include <user_interface.h>
#include <gpio.h>
#include <osapi.h>
#include "user_config.h"
#include "node.h"
#include "network.h"

/**
 * Network timer.
 */
static os_timer_t network_timer;

#if ! CONFIG_USE_DHCP
/**
 * IP configuration.
 */
static struct ip_info ip_info;
#endif

/**
 * TCP connection.
 */
static struct _esp_tcp user_tcp;

/**
 * Network connection.
 */
static struct espconn user_espconn = {
    .proto.tcp = &user_tcp,
};

/**
 * Wireless association.
 */
static struct station_config station_config = {
    .ssid = CONFIG_WIFI_SSID,
    .password = CONFIG_WIFI_PASSWORD,
};

/* Static function prototypes. */
static void ICACHE_FLASH_ATTR _network_create_connection(void);
static void ICACHE_FLASH_ATTR _network_recv_callback(void *arg, char *pdata, unsigned short len);
static void ICACHE_FLASH_ATTR _network_sent_callback(void *arg);
static void ICACHE_FLASH_ATTR _network_connect_callback(void *arg);
static void ICACHE_FLASH_ATTR _network_reconnect_callback(void *arg, sint8 err);
static void ICACHE_FLASH_ATTR _network_disconnect_callback(void *arg);
static void ICACHE_FLASH_ATTR _network_write_finish_fn(void *arg);

/* Implementation. */

void ICACHE_FLASH_ATTR network_init(void) {
#if CONFIG_USE_DHCP
#error "Not implemented yet"
#else
    wifi_station_dhcpc_stop();
    IP4_ADDR(&ip_info.ip, CONFIG_CLIENT_IP_ADDRESS0,
                            CONFIG_CLIENT_IP_ADDRESS1,
                            CONFIG_CLIENT_IP_ADDRESS2,
                            CONFIG_CLIENT_IP_ADDRESS3);
    IP4_ADDR(&ip_info.gw, CONFIG_CLIENT_IP_GATEWAY0,
                            CONFIG_CLIENT_IP_GATEWAY1,
                            CONFIG_CLIENT_IP_GATEWAY2,
                            CONFIG_CLIENT_IP_GATEWAY3);
    IP4_ADDR(&ip_info.netmask, CONFIG_CLIENT_IP_NETMASK0,
                                CONFIG_CLIENT_IP_NETMASK1,
                                CONFIG_CLIENT_IP_NETMASK2,
                                CONFIG_CLIENT_IP_NETMASK3);
    wifi_set_ip_info(STATION_IF, &ip_info);
#endif

    /* IP address timer. */
    os_timer_disarm(&network_timer);
    os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
    os_timer_arm(&network_timer, 1000, 0);
}

void ICACHE_FLASH_ATTR network_connect(void) {
    wifi_set_opmode_current(STATION_MODE);
    wifi_station_set_config_current(&station_config);
    wifi_station_connect();
}

void ICACHE_FLASH_ATTR network_check_ip(void) {
    struct ip_info ipconfig;
    os_timer_disarm(&network_timer);
    wifi_get_ip_info(STATION_IF, &ipconfig);
    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
        node_update_state(NODE_STATE_BROKER_CONNECTING);
        char page_buffer[20];
        os_sprintf(page_buffer,"IP: %d.%d.%d.%d\r\n",IP2STR(&ipconfig.ip));
        uart0_tx_buffer(page_buffer,strlen(page_buffer));
        _network_create_connection();
    } else {
        os_printf("No ip found\r\n");
        os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
        os_timer_arm(&network_timer, 1000, 0);
    }
}

void ICACHE_FLASH_ATTR network_send(void *buf, uint16_t len) {
    espconn_send(&user_espconn, buf, len);
}

static void ICACHE_FLASH_ATTR _network_create_connection(void) {
    struct ip_addr ip;
    user_espconn.type = ESPCONN_TCP;
    user_espconn.state = ESPCONN_NONE;
    user_espconn.reverse = &user_espconn;
    user_espconn.proto.tcp->local_port = espconn_port();
    user_espconn.proto.tcp->remote_port = 5555;
    IP4_ADDR(&ip, 192, 168, 10, 1);
    os_memcpy(user_espconn.proto.tcp->remote_ip, &ip, 4);

    espconn_regist_connectcb(&user_espconn, _network_connect_callback);
    espconn_regist_disconcb(&user_espconn, _network_disconnect_callback);
    espconn_regist_reconcb(&user_espconn, _network_reconnect_callback);
    espconn_connect(&user_espconn);
}

static void ICACHE_FLASH_ATTR _network_recv_callback(void *arg, char *pdata, unsigned short len) {
}

static void ICACHE_FLASH_ATTR _network_sent_callback(void *arg) {
}

static void ICACHE_FLASH_ATTR _network_connect_callback(void *arg) {
    espconn_regist_sentcb(&user_espconn, _network_sent_callback);
    espconn_regist_recvcb(&user_espconn, _network_recv_callback);
    node_update_state(NODE_STATE_OPERATIONAL);
    system_os_post(CONFIG_SEND_TASK_PRIORITY, 0, 0 );
}

static void ICACHE_FLASH_ATTR _network_reconnect_callback(void *arg, sint8 err) {
    espconn_connect(&user_espconn);
}

static void ICACHE_FLASH_ATTR _network_disconnect_callback(void *arg) {
    espconn_connect(&user_espconn);
}

static void ICACHE_FLASH_ATTR _network_write_finish_fn(void *arg) {
    os_printf("__ Finished __\r\n");
}
