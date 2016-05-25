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
#include "bh1750fvi.h"
#include "sensor_bh1750fvi.h"

static char _sensor_bh1750fvi_data_temperature_str[SENSOR_VALUE_BUFFER_SIZE];

static struct sensor_str _sensor_bh1750fvi_topics = {
    .data = TOPIC(CONFIG_SENSOR_BH1750FVI_AMBIENTLIGHT_TOPIC),
    .len = __sizeof_str(TOPIC(CONFIG_SENSOR_BH1750FVI_AMBIENTLIGHT_TOPIC)),
};

static struct sensor_str _sensor_bh1750fvi_data = {
    .data = _sensor_bh1750fvi_data_temperature_str,
    .len = 0,
};

const uint8_t sensor_topics_count = 1;

enum sensor_io_result sensor_read(void) {
    uint8_t _len = 0;
    char _buf[SENSOR_VALUE_BUFFER_SIZE];
    enum bh1750fvi_io_result _io_result = bh1750fvi_read();
    switch (_io_result) {
        case BH1750FVI_IO_OK:
            _len = os_sprintf(
                _sensor_bh1750fvi_data.data,
                "%d",
                bh1750fvi_data);
            _sensor_bh1750fvi_data.len = _len;
            return SENSOR_IO_OK;
        case BH1750FVI_IO_WRITE_ADDRESS_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_ADDRESS");
            break;
        case BH1750FVI_IO_WRITE_REGISTER_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_REGISTER");
            break;
        case BH1750FVI_IO_WRITE_VALUE_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_VALUE");
            break;
        case BH1750FVI_IO_READ_ADDRESS_ERROR:
            _len = os_sprintf(_buf, "E_READ_ADDRESS");
            break;
        case BH1750FVI_IO_INVALID_DATA:
            _len = os_sprintf(_buf, "E_INVALID_DATA");
            break;
    }

    os_memcpy(_sensor_bh1750fvi_data.data, _buf, _len);
    _sensor_bh1750fvi_data.len = _len;
    return SENSOR_IO_ERROR;
}

__sensor_get_topic_scalar(_sensor_bh1750fvi_topics)
__sensor_get_value_scalar(_sensor_bh1750fvi_data)
