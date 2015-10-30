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

#include <c_types.h>
#include <ip_addr.h>
#include <osapi.h>
#include <user_interface.h>
#include "user_config.h"
#include "mqttclient.h"
#include "user_config.h"
#include "network.h"

/**
 * Current network state.
 */
enum network_state network_state;

#if ! CONFIG_USE_DHCP
/**
 * IP configuration.
 */
static struct ip_info ip_info;
#endif

/**
 * Wireless association.
 */
static struct station_config station_config = {
    .ssid = CONFIG_WIFI_SSID,
    .password = CONFIG_WIFI_PASSWORD,
};

static os_timer_t _ip_check_timer;

/* Static function prototypes. */
static void ICACHE_FLASH_ATTR _network_config_address(void);
static void ICACHE_FLASH_ATTR _network_check_ip(void);
static void ICACHE_FLASH_ATTR _network_init_timer(void);

/**
 * Update network state.
 *
 * @param state New network state.
 */
static inline void _network_update_state(enum network_state state);

/* Implementation. */

void ICACHE_FLASH_ATTR network_init(void) {
    _network_update_state(NETWORK_STATE_INIT);
    _network_config_address();
    _network_init_timer();
}

static void ICACHE_FLASH_ATTR _network_config_address(void) {
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
}

static void ICACHE_FLASH_ATTR _network_init_timer(void) {
    /* IP address timer. */
    os_timer_disarm(&_ip_check_timer);
    os_timer_setfn(&_ip_check_timer, (os_timer_func_t *) _network_check_ip, NULL);
    os_timer_arm(&_ip_check_timer, 1000, 0);
}

void ICACHE_FLASH_ATTR network_connect(void) {
    wifi_set_opmode_current(STATION_MODE);
    wifi_station_set_config_current(&station_config);
    wifi_station_connect();
}

static void ICACHE_FLASH_ATTR _network_check_ip(void) {
    struct ip_info ipconfig;
    os_timer_disarm(&_ip_check_timer);
    wifi_get_ip_info(STATION_IF, &ipconfig);
    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
        _network_update_state(NETWORK_STATE_UP);
        system_os_post(CONFIG_PROCESS_TASK_PRIORITY, 0, 0);
    } else {
        os_timer_setfn(&_ip_check_timer, (os_timer_func_t *) _network_check_ip, NULL);
        os_timer_arm(&_ip_check_timer, 1000, 0);
    }
}

static inline void _network_update_state(enum network_state state) {
    network_state = state;
}
