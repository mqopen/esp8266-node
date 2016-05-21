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

#ifndef __SENSOR_BUTTON_H__
#define __SENSOR_BUTTON_H__

#include "button.h"

#define sensor_init button_init

#define sensor_register_notify_callback button_register_event_callback

#define SENSOR_VALUE_BUFFER_SIZE    10

/* Declare asynchrounous sensor type. */
#define SENSOR_TYPE_ASYNCHRONOUS    1

#endif
