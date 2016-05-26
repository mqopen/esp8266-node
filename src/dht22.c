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
#include "dht.h"
#include "dht22.h"

enum dht_io_result dht22_read(struct dht_data *data) {
    uint8_t buf[DHT_DATA_BYTE_LEN];
    enum dht_io_result _io_result = dht_read_data(buf);
    if (_io_result == DHT_IO_OK) {
        data->humidity = ((buf[0] << 8) + buf[1]) * 100;
        data->temperature = (((buf[2] & 0x7F) << 8) + buf[3]) * 100;
        if (buf[2] & 0x80) {
            data->temperature = -data->temperature;
        }
    }
    return _io_result;
}
