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
#include "actsig.h"
#include "commsig.h"

/** Signal network activity. */
static struct actsig_signal _commsig_signal;

void commsig_init(void) {
    actsig_init(&_commsig_signal, CONFIG_MQTT_ACTIVITY_LED_BLINK_TRANSMITT_DELAY);
    actsig_set_normal_off(&_commsig_signal);
}

void commsig_connection_status(bool status) {
    if (status) {
        actsig_set_normal_on(&_commsig_signal);
    } else {
        actsig_set_normal_off(&_commsig_signal);
    }
}

void commsig_notify(void)   {
    actsig_notify(&_commsig_signal);
}
