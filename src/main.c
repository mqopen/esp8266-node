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

#include <os_type.h>
#include <osapi.h>
#include <user_interface.h>
#include "user_config.h"
#include "uart.h"
#include "node.h"
#include "bus.h"
#include "sensor.h"
#include "network.h"
#include "mqttclient.h"


void ICACHE_FLASH_ATTR user_rf_pre_init(void) {
}

void ICACHE_FLASH_ATTR user_init(void) {
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    network_init();
    node_init();
    bus_init();
    sensor_init();
    mqttclient_init();
    system_init_done_cb(network_connect);
    node_update_state(NODE_STATE_INIT);
}
