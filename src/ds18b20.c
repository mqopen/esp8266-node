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

#include "onewire.h"
#include "ds18b20.h"

void ds18b20_init(void) {
}

enum ds18b20_io_result ds18b20_read(double *value) {
    uint8_t _temperature_l;
    uint8_t _temperature_h;

    if (!onewire_reset())
        return DS18B20_ERROR;
    onewire_write(DS18B20_CMD_SKIPROM);
    onewire_write(DS18B20_CMD_CONVERTTEMP);

    while (!onewire_read_bit());

    if (!onewire_reset())
        return DS18B20_ERROR;
    onewire_write(DS18B20_CMD_SKIPROM);
    onewire_write(DS18B20_CMD_RSCRATCHPAD);

    /* Read 2 bytes from scratchpad. */
    _temperature_l = onewire_read();
    _temperature_h = onewire_read();

    *value = ((_temperature_h << 8) | _temperature_l) * 0.0625;
    return DS18B20_OK;
}
