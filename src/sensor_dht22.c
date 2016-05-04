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
#include "dht22.h"
#include "common.h"
#include "sensor_dht22.h"

/* Check that at least one sensor reading is enabled. */
#if ! ENABLE_SENSOR_DHT22_TEMPERATURE && ! ENABLE_SENSOR_DHT22_HUMIDITY
#error No sensor reading is enabled.
#endif

/**
 * Temperature data.
 */
#if ENABLE_SENSOR_DHT22_TEMPERATURE
static char _sensor_dht22_data_temperature_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

/**
 * humidity data.
 */
#if ENABLE_SENSOR_DHT22_HUMIDITY
static char _sensor_dht22_data_humidity_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

/**
 * Topics.
 */
static struct sensor_str _sensor_dht22_topics[] = {
#if ENABLE_SENSOR_DHT22_TEMPERATURE
    {
        .data = TOPIC(CONFIG_SENSOR_DHT22_TEMPERATURE_TOPIC),
        .len = sizeof(TOPIC(CONFIG_SENSOR_DHT22_TEMPERATURE_TOPIC)),
    },
#endif
#if ENABLE_SENSOR_DHT22_HUMIDITY
    {
        .data = TOPIC(CONFIG_SENSOR_DHT22_HUMIDITY_TOPIC),
        .len = sizeof(TOPIC(CONFIG_SENSOR_DHT22_HUMIDITY_TOPIC)),
    },
#endif
};

/**
 * Values.
 */
static struct sensor_str _sensor_dht22_data[] = {
#if ENABLE_SENSOR_DHT22_TEMPERATURE
    {
        .data = _sensor_dht22_data_temperature_str,
        .len = 0,
    },
#endif
#if ENABLE_SENSOR_DHT22_HUMIDITY
    {
        .data = _sensor_dht22_data_humidity_str,
        .len = 0,
    },
#endif
};

const uint8_t sensor_topics_count = sizeof(_sensor_dht22_topics) / sizeof(_sensor_dht22_topics[0]);

enum sensor_io_result sensor_read(void) {
    uint8_t i = 0;
    uint8_t _len = 0;
    char _buf[SENSOR_VALUE_BUFFER_SIZE];
    enum dht22_io_result _io_result = dht22_read();
    switch (_io_result) {
        case DHT22_IO_OK:
#if ENABLE_SENSOR_DHT22_TEMPERATURE
            _len = os_sprintf(
                _sensor_dht22_data[i].data,
                "%d.%d",
                dht22_data.temperature / 1000, dht22_data.temperature % 1000);
            _sensor_dht22_data[i++].len = _len;
#endif
#if ENABLE_SENSOR_DHT22_HUMIDITY
            _len = os_sprintf(
                _sensor_dht22_data[i].data,
                "%d.%d",
                dht22_data.humidity / 1000, dht22_data.humidity % 1000);
            _sensor_dht22_data[i++].len = _len;
#endif
            return SENSOR_IO_OK;
        case DHT22_IO_WRITE_ADDRESS_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_ADDRESS");
            break;
        case DHT22_IO_WRITE_REGISTER_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_REGISTER");
            break;
        case DHT22_IO_WRITE_VALUE_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_VALUE");
            break;
        case DHT22_IO_READ_ADDRESS_ERROR:
            _len = os_sprintf(_buf, "E_READ_ADDRESS");
            break;
        case DHT22_IO_INVALID_DATA:
            _len = os_sprintf(_buf, "E_INVALID_DATA");
            break;
    }
    for (i = 0; i < sensor_topics_count; i++) {
        os_memcpy(_sensor_dht22_data[i].data, _buf, _len);
        _sensor_dht22_data[i].len = _len;
    }

    return SENSOR_IO_ERROR;
}

char *sensor_get_topic(uint8_t index, uint8_t *buf_len) {
    *buf_len = _sensor_dht22_topics[index].len;
    return _sensor_dht22_topics[index].data;
}

char *sensor_get_value(uint8_t index, uint8_t *buf_len) {
    *buf_len = _sensor_dht22_data[index].len;
    return _sensor_dht22_data[index].data;
}
