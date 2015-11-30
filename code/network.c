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
#include "node.h"
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

/* Static function prototypes. */
static void ICACHE_FLASH_ATTR _network_config_address(void);
static void ICACHE_FLASH_ATTR _network_wifi_event_callback(System_Event_t *event);

/**
 * Update network state.
 *
 * @param state New network state.
 */
static inline void _network_update_state(enum network_state state);

/* Implementation. */

void ICACHE_FLASH_ATTR network_init(void) {
    wifi_set_event_handler_cb(_network_wifi_event_callback);
    _network_config_address();
    _network_update_state(NETWORK_STATE_INIT);
}

static void ICACHE_FLASH_ATTR _network_config_address(void) {
#if CONFIG_USE_DHCP
#error "Not implemented yet"
#else
    wifi_set_opmode(STATIONAP_MODE);
    if (!wifi_station_dhcpc_stop())
        os_printf("dhcpc stop failed\r\n");
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

void ICACHE_FLASH_ATTR network_connect(void) {
    wifi_set_opmode_current(STATION_MODE);
    wifi_station_set_config_current(&station_config);
    wifi_station_connect();
}

static inline void _network_update_state(enum network_state state) {
    enum network_state previous_state = network_state;
    network_state = state;
    if (network_state == NETWORK_STATE_UP) {
        node_update_state(NODE_STATE_NETWORK_CONNECTED);
    } else if (previous_state == NETWORK_STATE_UP) {
        node_update_state(NODE_STATE_NETWORK_DISCONNECTED);
    }
}

static void ICACHE_FLASH_ATTR _network_wifi_event_callback(System_Event_t *event) {
    switch (event->event) {
        case EVENT_STAMODE_CONNECTED:
#if CONFIG_USE_DHCP
#error "Not implemented yet"
#else
            _network_update_state(NETWORK_STATE_UP);
#endif
            break;
        case EVENT_STAMODE_DISCONNECTED:
            _network_update_state(NETWORK_STATE_AP_ASSOCIATING);
            break;
    }
}
