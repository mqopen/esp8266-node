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

#ifndef __NODE_H__
#define __NODE_H__

#include <c_types.h>
#include <os_type.h>

#define NODE_PROCESS_TASK_PRIORITY  0

/**
 * Possible node states.
 */
enum node_state {
    NODE_STATE_INIT,                    /** */
    NODE_STATE_NETWORK_CONNECTED,
    NODE_STATE_NETWORK_DISCONNECTED,
    NODE_STATE_BROKER_OPERATIONAL,
    //NODE_STATE_AP_ASSOCIATING,      /**< Node is associating with the access point. */
    //NODE_STATE_BROKER_CONNECTING,   /**< Node is connecting to broker. */
    //NODE_STATE_OPERATIONAL,         /**< Node is connected to MQTT broker and ready to transmit data. */
};

extern enum node_state node_current_state;

/**
 * Initiate node.
 */
void ICACHE_FLASH_ATTR node_init(void);

/**
 * Update node state.
 *
 * @param state
 */
void ICACHE_FLASH_ATTR node_update_state(enum node_state state);

/**
 * Process new state.
 */
void ICACHE_FLASH_ATTR node_process_state(os_event_t *events);

#endif
