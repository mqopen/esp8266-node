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
#include "user_config.h"
#include "uart.h"
#include "node.h"
#include "network.h"

os_event_t user_proc_task_queue[CONFIG_PROC_TASK_QUEUE_LENGTH];

static void _send_task(os_event_t *events);

void user_rf_pre_init(void) {
}

void ICACHE_FLASH_ATTR user_init(void) {
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    network_init();
    system_init_done_cb(network_connect);
    system_os_task(_send_task,
                    CONFIG_SEND_TASK_PRIORITY,
                    user_proc_task_queue,
                    CONFIG_PROC_TASK_QUEUE_LENGTH);
}

static void _send_task(os_event_t *events) {
    if (node_current_state == NODE_STATE_OPERATIONAL) {
        network_send("test\n", 5);
        system_os_post(CONFIG_SEND_TASK_PRIORITY, 0, 0 );
    }
}
