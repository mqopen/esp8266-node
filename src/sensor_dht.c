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
#include "common.h"
#include "dht.h"
#include "sensor_dht.h"

/* Check that at least one sensor reading is enabled. */
#if ! ENABLE_SENSOR_DHT_TEMPERATURE && ! ENABLE_SENSOR_DHT_HUMIDITY
#error No sensor reading is enabled.
#endif

/**
 * Temperature data.
 */
#if ENABLE_SENSOR_DHT_TEMPERATURE
static char _sensor_dht_data_temperature_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

/**
 * humidity data.
 */
#if ENABLE_SENSOR_DHT_HUMIDITY
static char _sensor_dht_data_humidity_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

/**
 * Topics.
 */
static struct sensor_str _sensor_dht_topics[] = {
#if ENABLE_SENSOR_DHT_TEMPERATURE
    {
        .data = TOPIC(CONFIG_SENSOR_DHT_TEMPERATURE_TOPIC),
        .len = __sizeof_str(TOPIC(CONFIG_SENSOR_DHT_TEMPERATURE_TOPIC)),
    },
#endif
#if ENABLE_SENSOR_DHT_HUMIDITY
    {
        .data = TOPIC(CONFIG_SENSOR_DHT_HUMIDITY_TOPIC),
        .len = __sizeof_str(TOPIC(CONFIG_SENSOR_DHT_HUMIDITY_TOPIC)),
    },
#endif
};

/**
 * Values.
 */
static struct sensor_str _sensor_dht_data[] = {
#if ENABLE_SENSOR_DHT_TEMPERATURE
    {
        .data = _sensor_dht_data_temperature_str,
        .len = 0,
    },
#endif
#if ENABLE_SENSOR_DHT_HUMIDITY
    {
        .data = _sensor_dht_data_humidity_str,
        .len = 0,
    },
#endif
};

const uint8_t sensor_topics_count = sizeof(_sensor_dht_topics) / sizeof(_sensor_dht_topics[0]);

enum sensor_io_result sensor_read(void) {
    uint8_t i = 0;
    uint8_t _len = 0;
    char _buf[SENSOR_VALUE_BUFFER_SIZE];
    struct dht_data _data;
    enum dht_io_result _io_result = dht_read(&_data);
    switch (_io_result) {
        case DHT_IO_OK:
#if ENABLE_SENSOR_DHT_TEMPERATURE
            _len = os_sprintf(
                _sensor_dht_data[i].data,
                _data.temperature < 0 ? "-%d.%d" : "%d.%d",
                abs(_data.temperature) / 1000,
                abs(_data.temperature) % 1000);
            _sensor_dht_data[i++].len = _len;
#endif
#if ENABLE_SENSOR_DHT_HUMIDITY
            _len = os_sprintf(
                _sensor_dht_data[i].data,
                "%d.%d",
                _data.humidity / 1000, _data.humidity % 1000);
            _sensor_dht_data[i++].len = _len;
#endif
            return SENSOR_IO_OK;
        case DHT_IO_CHECKSUM_ERROR:
            _len = os_sprintf(_buf, "E_CHECKSUM");
            break;
        case DHT_IO_TIMEOUT_L_ERROR:
            _len = os_sprintf(_buf, "E_TIMEOUT_L");
            break;
        case DHT_IO_TIMEOUT_H_ERROR:
            _len = os_sprintf(_buf, "E_TIMEOUT_H");
            break;
        case DHT_IO_CONNECT_ERROR:
            _len = os_sprintf(_buf, "E_CONN");
            break;
        case DHT_IO_ACK_H_ERROR:
            _len = os_sprintf(_buf, "E_ACK_H");
            break;
        case DHT_IO_ACK_L_ERROR:
            _len = os_sprintf(_buf, "E_ACK_L");
            break;
    }
    for (i = 0; i < sensor_topics_count; i++) {
        os_memcpy(_sensor_dht_data[i].data, _buf, _len);
        _sensor_dht_data[i].len = _len;
    }

    return SENSOR_IO_ERROR;
}

__sensor_get_topic_array(_sensor_dht_topics)
__sensor_get_value_array(_sensor_dht_data)
