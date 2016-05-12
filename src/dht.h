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

#ifndef __DHT_H__
#define __DHT_H__

#include <c_types.h>

#define DHT_GPIO_DQ CONFIG_SENSOR_DHT_GPIO_DQ
#if CONFIG_SENSOR_DHT_GPIO_DG == 0
  #define DHT_DQ_MUX PERIPHS_IO_MUX_GPIO0_U
  #define DHT_DQ_FUNC FUNC_GPIO0
#elif CONFIG_SENSOR_DHT_GPIO_DG == 2
  #define DHT_DQ_MUX PERIPHS_IO_MUX_GPIO2_U
  #define DHT_DQ_FUNC FUNC_GPIO2
#elif CONFIG_SENSOR_DHT_GPIO_DG == 14
  #define DHT_DQ_MUX PERIPHS_IO_MUX_MTMS_U
  #define DHT_DQ_FUNC FUNC_GPIO14
#else
  #error Unsupported DHT DQ pin number!
#endif


enum dht_io_result {
    DHT_IO_OK,                      /**< Communication is OK. */
    DHT_IO_WRITE_ADDRESS_ERROR,     /**< Write address not acknowledged. */
    DHT_IO_WRITE_REGISTER_ERROR,    /**< Write of destination register not acknowledged. */
    DHT_IO_WRITE_VALUE_ERROR,       /**< Write of register value not acknowledged (Write operation only). */
    DHT_IO_READ_ADDRESS_ERROR,      /**< Read address not acknowledged. */
    DHT_IO_INVALID_DATA,            /**< Invalid data. */
};

struct dht_data {
    uint16_t humidity;
    int16_t temperature;
};

#if ENABLE_SENSOR_DHT22
  #include "dht22.h"
  #define dht_init dht22_init
  #define dht_read dht22_read
#elif ENABLE_SENSOR_DHT11
  #include "dht11.h"
  #define dht_init dht11_init
  #define dht_read dht11_read
#else
  #error Unsupported DHT variant.
#endif

#endif
