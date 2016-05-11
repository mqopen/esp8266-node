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
#include "common.h"
#include "sensor.h"
#include "ds18b20.h"
#include "sensor_ds18b20.h"

const uint8_t sensor_topics_count = 1;

static char _sensor_ds18b20_data_temperature_str[SENSOR_VALUE_BUFFER_SIZE];

static struct sensor_str _sensor_ds18b20_topics = {
    .data = TOPIC(CONFIG_SENSOR_DS18B20_TEMPERATURE_TOPIC),
    .len = sizeof(TOPIC(CONFIG_SENSOR_DS18B20_TEMPERATURE_TOPIC)),
};

static struct sensor_str _sensor_ds18b20_data = {
    .data = _sensor_ds18b20_data_temperature_str,
    .len = 0,
};

enum sensor_io_result sensor_read(void) {
    uint8_t _len = 0;
    char _buf[SENSOR_VALUE_BUFFER_SIZE];
    int16_t _temperature;
    enum ds18b20_io_result _io_result = ds18b20_read(&_temperature);
    switch (_io_result) {
        case DS18B20_IO_OK:
            _temperature += CONFIG_SENSOR_DS18B20_TEMPERATURE_OFFSET;
            _len = os_sprintf(
                _sensor_ds18b20_data.data,
                _temperature < 0 ? "-%d.%d" : "%d.%d",
                abs(_temperature) / 10,
                abs(_temperature) % 10);
            _sensor_ds18b20_data.len = _len;
            return SENSOR_IO_OK;
        case DS18B20_IO_ERROR:
        case DS18B20_IO_TEMP_CONVERSION_TIMEOUT:
            _len = os_sprintf(_buf, "E_READ");
            break;
    }

    os_memcpy(_sensor_ds18b20_data.data, _buf, _len);
    _sensor_ds18b20_data.len = _len;
    return SENSOR_IO_ERROR;
}

__sensor_get_topic_scalar(_sensor_ds18b20_topics)
__sensor_get_value_scalar(_sensor_ds18b20_data)
