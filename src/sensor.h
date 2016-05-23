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

#if ENABLE_SENSOR_DHT22 || ENABLE_SENSOR_DHT11
  #include "sensor_dht.h"
#elif ENABLE_SENSOR_BMP180
  #include "sensor_bmp180.h"
#elif ENABLE_SENSOR_BH1750FVI
  #include "sensor_bh1750fvi.h"
#elif ENABLE_SENSOR_DS18B20
  #include "sensor_ds18b20.h"
#elif ENABLE_SENSOR_BUTTON
  #include "sensor_button.h"
#else
  #error Unsupported sensor.
#endif

/* Default sensor type is synchronous. */
#ifndef SENSOR_TYPE_ASYNCHRONOUS
  #define SENSOR_TYPE_ASYNCHRONOUS  0
#endif

#if SENSOR_TYPE_ASYNCHRONOUS
  #define SENSOR_TYPE_SYNCHRONOUS   0
#else
  #define SENSOR_TYPE_SYNCHRONOUS   1
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

#if SENSOR_TYPE_SYNCHRONOUS
/**
 * Read new data from the sensor.
 *
 * @return Result of IO operation.
 */
extern enum sensor_io_result sensor_read(void);
#else
extern void sensor_register_notify_callback(sensor_notify_callback_t callback);
#endif

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

#define __sensor_get_topic_array(__topic_array) \
    char *sensor_get_topic(uint8_t index, uint8_t *buf_len) { \
        *buf_len = __topic_array[index].len; \
        return __topic_array[index].data; \
    }

#define __sensor_get_value_array(__value_array) \
    char *sensor_get_value(uint8_t index, uint8_t *buf_len) { \
        *buf_len = __value_array[index].len; \
        return __value_array[index].data; \
    }

#define __sensor_get_topic_scalar(__topic_struct) \
    char *sensor_get_topic(uint8_t index, uint8_t *buf_len) { \
        *buf_len = __topic_struct.len; \
        return __topic_struct.data; \
    }

#define __sensor_get_value_scalar(__value_struct) \
    char *sensor_get_value(uint8_t index, uint8_t *buf_len) { \
        *buf_len = __value_struct.len; \
        return __value_struct.data; \
    }

#endif
