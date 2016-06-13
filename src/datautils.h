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

#ifndef __DATAUTILS_H__
#define __DATAUTILS_H__

#include <c_types.h>

/**
 * Convert data buffer to int32_t value.
 *
 * @param val Pointer to memory where final value will be stored.
 * @param buf Pointer to data buffer.
 * @param len Length of data buffer.
 * @param decimal_digits Nuber of decimal digits.
 * @return Non-zero on success, zero on failure.
 */
uint8_t datautils_data_to_int32(int32_t *val, const uint8_t *buf, uint16_t len, uint8_t decimal_digits);

#endif
