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
#include "uart.h"

#define user_procTaskQueueLen    1

const char ssid[32] = "my_home_ssid";
const char password[32] = "my_home_password";

static struct station_config station_config = {
    .ssid = "hostapd",
    .password = "password",
};

static struct ip_info ip_info;

os_event_t user_procTaskQueue[user_procTaskQueueLen];
struct rst_info rtc_info;

static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events) {
}

void ICACHE_FLASH_ATTR user_init(void) {
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    //system_set_os_print(0);

    wifi_station_dhcpc_stop();
    IP4_ADDR(&ip_info.ip, 192, 168, 10, 200);
    IP4_ADDR(&ip_info.gw, 192, 168, 10, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    wifi_set_ip_info(STATION_IF, &ip_info);

    wifi_set_opmode_current(STATION_MODE);
    wifi_station_set_config_current(&station_config);
    wifi_station_connect();

    //Start os task
    system_os_task(user_procTask, USER_TASK_PRIO_0, user_procTaskQueue, user_procTaskQueueLen);
    system_os_post(USER_TASK_PRIO_0, 0, 0);
}
