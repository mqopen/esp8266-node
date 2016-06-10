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

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <c_types.h>

enum button_event_id {
#if ENABLE_SENSOR_BUTTON_1
    BUTTON_ID_1,
#endif
#if ENABLE_SENSOR_BUTTON_2
    BUTTON_ID_2,
#endif
};

/**
 * Initialize button hardware.
 */
void button_init(void);

/**
 * Check if button is in active state.
 *
 * @param id Button ID.
 * @return Non zero if button is in active state, zero otherwise.
 */
uint8_t button_is_active(enum button_event_id id);

inline void button_notify_lock(void);

inline void button_notify_release(void);

#endif
