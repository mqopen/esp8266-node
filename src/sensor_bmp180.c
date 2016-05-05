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
#include "sensor.h"
#include "bmp180.h"
#include "common.h"
#include "sensor_bmp180.h"

/* Oversampling setting */
#if ENABLE_SENSOR_BMP180_OVERSAMPLING_ULTRALOW
  #define SENSOR_BMP180_OSS BMP180_OSS_SINGLE
#elif ENABLE_SENSOR_BMP180_OVERSAMPLING_STANDARD
  #define SENSOR_BMP180_OSS BMP180_OSS_2
#elif ENABLE_SENSOR_BMP180_OVERSAMPLING_HIGH
  #define SENSOR_BMP180_OSS BMP180_OSS_4
#elif ENABLE_SENSOR_BMP180_OVERSAMPLING_ULTRAHIGH
  #define SENSOR_BMP180_OSS BMP180_OSS_8
#else
  #error Invalind oversampling settings!
#endif

/* Check that at least one sensor reading is enabled. */
#if ! ENABLE_SENSOR_BMP180_TEMPERATURE && ! CONFIG_SENSOR_BMP180_PRESSURE
#error No sensor reading is enabled.
#endif

#if ENABLE_SENSOR_BMP180_TEMPERATURE
static char _sensor_bmp180_data_temperature_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

#if CONFIG_SENSOR_BMP180_PRESSURE
static char _sensor_bmp180_data_pressure_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

/**
 * Topics.
 */
static struct sensor_str _sensor_bmp180_topics[] = {
#if ENABLE_SENSOR_BMP180_TEMPERATURE
    {
        .data = TOPIC(CONFIG_SENSOR_BMP180_TEMPERATURE_TOPIC),
        .len = sizeof(TOPIC(CONFIG_SENSOR_BMP180_TEMPERATURE_TOPIC)),
    },
#endif
#if CONFIG_SENSOR_BMP180_PRESSURE
    {
        .data = TOPIC(CONFIG_SENSOR_BMP180_PRESSURE_TOPIC),
        .len = sizeof(TOPIC(CONFIG_SENSOR_BMP180_PRESSURE_TOPIC)),
    },
#endif
};

/**
 * Values.
 */
static struct sensor_str _sensor_bmp180_data[] = {
#if ENABLE_SENSOR_BMP180_TEMPERATURE
    {
        .data = _sensor_bmp180_data_temperature_str,
        .len = 0,
    },
#endif
#if CONFIG_SENSOR_BMP180_PRESSURE
    {
        .data = _sensor_bmp180_data_pressure_str,
        .len = 0,
    },
#endif
};


const uint8_t sensor_topics_count = sizeof(_sensor_bmp180_topics) / sizeof(_sensor_bmp180_topics[0]);;

enum sensor_io_result sensor_read(void) {
    uint8_t i = 0;
    uint8_t _len = 0;
    char _buf[SENSOR_VALUE_BUFFER_SIZE];
    enum bmp180_io_result _io_result = bmp180_read(SENSOR_BMP180_OSS);
    switch (_io_result) {
        case BMP180_IO_OK:
#if ENABLE_SENSOR_BMP180_TEMPERATURE
            _len = os_sprintf(
                _sensor_bmp180_data[i].data,
                "%d.%d",
                bmp180_data.temperature / 1000, bmp180_data.temperature % 1000);
            _sensor_bmp180_data[i++].len = _len;
#endif
#if CONFIG_SENSOR_BMP180_PRESSURE
            _len = os_sprintf(
                _sensor_bmp180_data[i].data,
                "%d",
                bmp180_data.pressure);
            _sensor_bmp180_data[i++].len = _len;
#endif
            return SENSOR_IO_OK;
        case BMP180_IO_WRITE_ADDRESS_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_ADDRESS");
            break;
        case BMP180_IO_WRITE_REGISTER_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_REGISTER");
            break;
        case BMP180_IO_WRITE_VALUE_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_VALUE");
            break;
        case BMP180_IO_READ_ADDRESS_ERROR:
            _len = os_sprintf(_buf, "E_READ_ADDRESS");
            break;
        case BMP180_IO_INVALID_DATA:
            _len = os_sprintf(_buf, "E_INVALID_DATA");
            break;
    }

    for (i = 0; i < sensor_topics_count; i++) {
        os_memcpy(_sensor_bmp180_data[i].data, _buf, _len);
        _sensor_bmp180_data[i].len = _len;
    }
    return SENSOR_IO_ERROR;
}

__sensor_get_topic_array(_sensor_bmp180_topics)
__sensor_get_value_array(_sensor_bmp180_data)
