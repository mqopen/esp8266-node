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
#include "sensor.h"
#include "sensor_button.h"

/**
 * Topics.
 */
static struct sensor_str _sensor_button_topics = {
    .data = TOPIC("button"),
    .len = __sizeof_str("button"),
};

/**
 * Values.
 */
static struct sensor_str _sensor_button_data = {
    .data = "test",
    .len = __sizeof_str("test"),
};

const uint8_t sensor_topics_count = 1;

__sensor_get_topic_scalar(_sensor_button_topics)
__sensor_get_value_scalar(_sensor_button_data)
