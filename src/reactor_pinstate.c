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
#include <osapi.h>
#include "common.h"
#include "datautils.h"
#include "reactor.h"
#include "reactor_pinstate.h"

__reactor_subscribe_topics(
    CONFIG_REACTOR_PINSTATE_TOPIC
);

extern void reactor_on_data(char *topic, uint8_t *data, uint16_t data_len) {
    int32_t value;
    if (!datautils_data_to_int32(&value, data, data_len)) {
        pinstate_update(value);
    }
}
