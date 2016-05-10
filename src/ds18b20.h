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

#ifndef __DS18B20_H__
#define __DS18B20_H__

#define DS18B20_CMD_CONVERTTEMP     0x44
#define DS18B20_CMD_RSCRATCHPAD     0xbe
#define DS18B20_CMD_WSCRATCHPAD     0x4e
#define DS18B20_CMD_CPYSCRATCHPAD   0x48
#define DS18B20_CMD_RECEEPROM       0xb8
#define DS18B20_CMD_RPWRSUPPLY      0xb4
#define DS18B20_CMD_SEARCHROM       0xf0
#define DS18B20_CMD_READROM         0x33
#define DS18B20_CMD_MATCHROM        0x55
#define DS18B20_CMD_SKIPROM         0xcc
#define DS18B20_CMD_ALARMSEARCH     0xec

enum ds18b20_io_result {
    DS18B20_IO_OK,                          /**< IO was OK. */
    DS18B20_IO_ERROR,
    DS18B20_IO_TEMP_CONVERSION_TIMEOUT,     /**< Temperature conversion timeouted. */
};

void ds18b20_init(void);

/**
 * Read temperature in 0.1 degree of celsius.
 *
 * @param value Address where to store temperatue.
 * @return Result of read operation.
 */
enum ds18b20_io_result ds18b20_read(double *value);

#endif
