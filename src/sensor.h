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

#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <c_types.h>

/**
 * Sensor abstraction layer.
 */

#if ENABLE_SENSOR_DHT22
  #include "sensor_dht22.h"
#elif ENABLE_SENSOR_BMP180
  #include "sensor_bmp180.h"
#elif ENABLE_SENSOR_BH1750FVI
  #include "sensor_bh1750fvi.h"
#else
  #error Unsupported sensor.
#endif

enum sensor_io_result {
    SENSOR_IO_OK,
    SENSOR_IO_ERROR
};

struct sensor_str {
    char *data;
    uint8_t len;
};

/**
 * Num of enables sensor topics.
 */
extern const uint8_t sensor_topics_count;

/**
 * Initialize sensor hardware.
 */
extern void sensor_init(void);

/**
 * Read new data from the sensor.
 *
 * @return Result of IO operation.
 */
extern enum sensor_io_result sensor_read(void);

/**
 * Get address of MQTT topic indentified by it's index.
 *
 * @param index Index of MQTT topic.
 * @param buf_len Pointer to variable where to store string length.
 * @return Address of topic string.
 */
extern char *sensor_get_topic(uint8_t index, uint8_t *buf_len);

/**
 * Get address of measured value indentified by it's index.
 *
 * @param index Index of measired value.
 * @param buf_len Pointer to variable where to store string length.
 * @return Address of value string.
 */
extern char *sensor_get_value(uint8_t index, uint8_t *buf_len);

#endif
