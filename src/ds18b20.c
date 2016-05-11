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
#include "onewire.h"
#include "ds18b20.h"

/** For temperature conversion timeouting. Allow some more miliseconds than datasheet defines. */
#define DS18B20_TEMP_CONVERSION_TIMEOUT_TOLERATION 10

#if ENABLE_SENSOR_DS18B20_TEMPRESOLUTION_9BIT
  #define DS18B20_TEMP_CONVERSION_TIMEOUT   (94 + DS18B20_TEMP_CONVERSION_TIMEOUT_TOLERATION)
#elif ENABLE_SENSOR_DS18B20_TEMPRESOLUTION_10BIT
  #define DS18B20_TEMP_CONVERSION_TIMEOUT   (185 + DS18B20_TEMP_CONVERSION_TIMEOUT_TOLERATION)
#elif ENABLE_SENSOR_DS18B20_TEMPRESOLUTION_11BIT
  #define DS18B20_TEMP_CONVERSION_TIMEOUT   (375 + DS18B20_TEMP_CONVERSION_TIMEOUT_TOLERATION)
#elif ENABLE_SENSOR_DS18B20_TEMPRESOLUTION_12BIT
  #define DS18B20_TEMP_CONVERSION_TIMEOUT   (750 + DS18B20_TEMP_CONVERSION_TIMEOUT_TOLERATION)
#else
  #error Unsupported bit resolution!
#endif

void ds18b20_init(void) {
}

enum ds18b20_io_result ds18b20_read(int16_t *value) {
    uint8_t _temperature_h;
    uint8_t _temperature_l;
    int16_t _temperature_integral;
    int8_t _temperature_decimal;

    if (!onewire_reset())
        return DS18B20_IO_ERROR;
    onewire_write(DS18B20_CMD_SKIPROM);
    onewire_write(DS18B20_CMD_CONVERTTEMP);

    uint16_t _timeout = DS18B20_TEMP_CONVERSION_TIMEOUT;
    while (!onewire_read_bit()) {
        os_delay_us(1000);
        _timeout--;
        if (_timeout == 0) {
            return DS18B20_IO_TEMP_CONVERSION_TIMEOUT;
        }
    }

    if (!onewire_reset())
        return DS18B20_IO_ERROR;
    onewire_write(DS18B20_CMD_SKIPROM);
    onewire_write(DS18B20_CMD_RSCRATCHPAD);

    /* Read 2 bytes from scratchpad. */
    _temperature_l = onewire_read();
    _temperature_h = onewire_read();

    _temperature_integral = ((_temperature_h << 8) | _temperature_l) >> 4;
    //_temperature_integral = _temperature_h;
    //_temperature_integral <<= 8;
    //_temperature_integral |= (_temperature_l & 0xf0);
    //_temperature_integral >>= 4;

    _temperature_decimal = (_temperature_l & 0x0f) * 0.625;

    *value = (_temperature_integral * 10) + _temperature_decimal;
    return DS18B20_IO_OK;
}
