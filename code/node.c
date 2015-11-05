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
#include <os_type.h>
#include "user_config.h"
#include "mqttclient.h"
#include "node.h"

enum node_state node_current_state;

os_event_t node_proc_task_queue[CONFIG_PROC_TASK_QUEUE_LENGTH];

/* Static function prototypes. */

/**
 * Process new state.
 */
void ICACHE_FLASH_ATTR _node_process_state(os_event_t *events);

/* Implementation. */

void ICACHE_FLASH_ATTR node_init(void) {
    system_os_task(_node_process_state,
                    NODE_PROCESS_TASK_PRIORITY,
                    node_proc_task_queue,
                    CONFIG_PROC_TASK_QUEUE_LENGTH);
}

void ICACHE_FLASH_ATTR node_update_state(enum node_state state) {
    node_current_state = state;
    system_os_post(NODE_PROCESS_TASK_PRIORITY, 0, 0);
}

void ICACHE_FLASH_ATTR _node_process_state(os_event_t *events) {
    switch (node_current_state) {
        case NODE_STATE_NETWORK_DISCONNECTED:
            mqttclient_stop();
            break;
        case NODE_STATE_NETWORK_CONNECTED:
            mqttclient_start();
            break;
        default:
            break;
    }
}
