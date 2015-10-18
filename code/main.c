/*
 * Copyright (C) Ivo Slanina <ivo.slanina@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <user_interface.h>
#include <gpio.h>
#include <osapi.h>
#include <espconn.h>
#include "user_config.h"
#include "uart.h"

#define user_procTaskQueueLen    1

const char ssid[32] = "my_home_ssid";
const char password[32] = "my_home_password";

static struct station_config station_config = {
    .ssid = "hostapd",
    .password = "password",
};

#if ! CONFIG_USE_DHCP
static struct ip_info ip_info;
#endif

os_event_t user_procTaskQueue[user_procTaskQueueLen];
struct rst_info rtc_info;

static void user_recv_callback(void *arg, char *pdata, unsigned short len) {
}

static void user_sent_callback(void *arg) {
}

static void user_espconn_connect_callback (void *arg) {
}

static void user_espconn_reconnect_callback (void *arg, sint8 err) {
}

void user_connect_callback(void *arg) {
}

void user_reconnect_callback(void *arg, sint8 err) {
}

void user_disconnect_callback(void *arg) {
}

void user_write_finish_fn(void *arg) {
}

struct _esp_tcp user_tcp = {
    .remote_port = 5555,
    .remote_ip = {192, 168, 10, 1},
    .connect_callback = user_connect_callback,
    .reconnect_callback = user_reconnect_callback,
    .disconnect_callback = user_disconnect_callback,
    .write_finish_fn = user_write_finish_fn,
};

struct espconn user_espconn = {
    .type = ESPCONN_TCP,
    .state = ESPCONN_NONE,
    .proto = &user_tcp,
    .recv_callback = user_recv_callback,
    .sent_callback = user_sent_callback,
};

static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events) {
}

void ICACHE_FLASH_ATTR user_init(void) {
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    //system_set_os_print(0);

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

    /* Connect to AP. */
    wifi_set_opmode_current(STATION_MODE);
    wifi_station_set_config_current(&station_config);
    wifi_station_connect();

    espconn_connect(&user_espconn);

    /* Start OS task. */
    system_os_task(user_procTask, USER_TASK_PRIO_0, user_procTaskQueue, user_procTaskQueueLen);
    system_os_post(USER_TASK_PRIO_0, 0, 0);
}
