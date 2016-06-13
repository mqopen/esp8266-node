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

#ifndef __PINSTATE_H__
#define __PINSTATE_H__

#include <c_types.h>

/**
 * Initialize pinstate hardware.
 */
void pinstate_init(void);

/**
 * Set state.
 *
 * @param enable True when pinstate configured condition is met, False otherwise.
 */
void pinstate_set(bool enable);

/**
 * Get Current pinstate state.
 *
 * @return True if pinstate if condition is currently met, False otherwise.
 */
inline bool pinstate_get(void);

#endif
