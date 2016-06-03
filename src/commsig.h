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

/**
 * Signalization of network communication.
 */

#ifndef __COMMSIG_H__
#define __COMMSIG_H__

#include <c_types.h>

/**
 * Initialize communication signalization.
 */
void commsig_init(void);

/**
 * Notify about connection status.
 *
 * @param status Connection status. True for established connection, False otherwise.
 */
void commsig_connection_status(bool status);

/**
 * Notify communication.
 */
void commsig_notify(void);

#endif
