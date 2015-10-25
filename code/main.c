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
#include "user_config.h"
#include "uart.h"
#include "i2c_master.h"
#include "bmp180.h"
#include "network.h"
#include "mqttclient.h"

os_event_t user_proc_task_queue[CONFIG_PROC_TASK_QUEUE_LENGTH];

static void ICACHE_FLASH_ATTR _process(os_event_t *events);
static inline void _process_network_up(os_event_t *events);

void ICACHE_FLASH_ATTR user_rf_pre_init(void) {
}

void ICACHE_FLASH_ATTR user_init(void) {
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    i2c_master_gpio_init();
    i2c_master_init();
    bmp180_init();
    network_init();
    mqttclient_init();
    system_init_done_cb(network_connect);
    system_os_task(_process,
                    CONFIG_PROCESS_TASK_PRIORITY,
                    user_proc_task_queue,
                    CONFIG_PROC_TASK_QUEUE_LENGTH);
}

static void ICACHE_FLASH_ATTR _process(os_event_t *events) {
    if (network_state == NETWORK_STATE_UP) {
        _process_network_up(events);
    }
}

static inline void _process_network_up(os_event_t *events) {
    mqttclient_start();
}
