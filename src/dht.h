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
#if CONFIG_SENSOR_DHT_GPIO_DQ == 0
  #define DHT_DQ_MUX PERIPHS_IO_MUX_GPIO0_U
  #define DHT_DQ_FUNC FUNC_GPIO0
#elif CONFIG_SENSOR_DHT_GPIO_DQ == 2
  #define DHT_DQ_MUX PERIPHS_IO_MUX_GPIO2_U
  #define DHT_DQ_FUNC FUNC_GPIO2
#elif CONFIG_SENSOR_DHT_GPIO_DQ == 13
  #define DHT_DQ_MUX PERIPHS_IO_MUX_MTCK_U
  #define DHT_DQ_FUNC FUNC_GPIO13
#elif CONFIG_SENSOR_DHT_GPIO_DQ == 14
  #define DHT_DQ_MUX PERIPHS_IO_MUX_MTMS_U
  #define DHT_DQ_FUNC FUNC_GPIO14
#else
  #error Unsupported DHT DQ pin number!
#endif

/** Number of bytes to read. */
#define DHT_DATA_BYTE_LEN   5

#define DHT_INIT_PULLUP_DELAY 40

enum dht_io_result {
    DHT_IO_OK,                      /**< Communication was successful. */
    DHT_IO_CHECKSUM_ERROR,          /**< Checksum doesn't match */
    DHT_IO_TIMEOUT_L_ERROR,         /**< Communication timeouted on low voltage phase. */
    DHT_IO_TIMEOUT_H_ERROR,         /**< Communication timeouted on high voltage phase. */
    DHT_IO_CONNECT_ERROR,           /**< Initial response signal not received. */
    DHT_IO_ACK_H_ERROR,             /**< Ack pull-up not received. */
    DHT_IO_ACK_L_ERROR,             /**< Ack pull-down not received. */
};

struct dht_data {
    uint32_t humidity;
    int32_t temperature;
};

#if ENABLE_SENSOR_DHT22
  #include "dht22.h"
  #define dht_read dht22_read
#elif ENABLE_SENSOR_DHT11
  #include "dht11.h"
  #define dht_read dht11_read
#else
  #error Unsupported DHT variant.
#endif

/**
 * Initialize DHT sensor.
 */
void dht_init(void);

/**
 * Read data from DHT sensor.
 *
 * @param buf Pointer to data buffer. It must be 5 bytes long.
 * @return Result of read operation.
 */
enum dht_io_result dht_read_data(uint8_t *buf);

#endif
