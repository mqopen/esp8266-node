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

#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <c_types.h>

/**
 * Network states.
 */
enum network_state {
    NETWORK_STATE_INIT,             /**< Network init phase. */
    NETWORK_STATE_AP_ASSOCIATING,   /**< Associating with access point. */
    NETWORK_STATE_IP_CONFIGURING,   /**< Configuring device IP address. */
    NETWORK_STATE_UP,               /**< Network connectivity is up. */
};

/**
 * Current network state.
 */
extern enum network_state network_state;

/**
 * Module responsible for IP setup for future connection with MQTT broker.
 */

/**
 * Init network.
 */
void ICACHE_FLASH_ATTR network_init(void);

/**
 * Connect to AP.
 */
void ICACHE_FLASH_ATTR network_connect(void);
#endif
