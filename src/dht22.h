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

#ifndef __DHT22_H__
#define __DHT22_H__

#include <c_types.h>

/** GPIO DG pin. */
#define DHT22_GPIO_DQ   2

enum dht22_io_result {
    DHT22_IO_OK,                        /**< Communication is OK. */
    DHT22_IO_WRITE_ADDRESS_ERROR,       /**< Write address not acknowledged. */
    DHT22_IO_WRITE_REGISTER_ERROR,      /**< Write of destination register not acknowledged. */
    DHT22_IO_WRITE_VALUE_ERROR,         /**< Write of register value not acknowledged (Write operation only). */
    DHT22_IO_READ_ADDRESS_ERROR,        /**< Read address not acknowledged. */
    DHT22_IO_INVALID_DATA,              /**< Invalid data. */
};

struct dht22_data {
    uint16_t humidity;
    int16_t temperature;
};

extern struct dht22_data dht22_data;

void dht22_init(void);

enum dht22_io_result dht22_read(void);

#endif
