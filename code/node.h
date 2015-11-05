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

#define NODE_PROCESS_TASK_PRIORITY  0

/**
 * Possible node states.
 */
enum node_state {
    NODE_STATE_INIT,                    /** Node init phase. */
    NODE_STATE_NETWORK_CONNECTED,       /** Node is connected to AP. */
    NODE_STATE_NETWORK_DISCONNECTED,    /** Node is disconnected from AP. */
    NODE_STATE_BROKER_OPERATIONAL,      /** MQTT client is up and running. */
};

/**
 * Current node state.
 */
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

#endif
