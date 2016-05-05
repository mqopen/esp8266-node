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

#ifndef __SENSOR__DS18B20_H__
#define __SENSOR__DS18B20_H__

#define sensor_init ds18b20_init

/**
 * Possible values are:
 *  - 'xxx.xxx'             : len = 7 (temperature, humidity)
 *  - 'E_WRITE_ADDRESS'     : len = 15
 *  - 'E_WRITE_REGISTER'    : len = 16
 *  - 'E_WRITE_VALUE'       : len = 13
 *  - 'E_READ_ADDRESS'      : len = 14
 *  - 'E_INVALID_DATA'      : len = 14
 *
 * maxium possible length: 16 Bytes
 */
#define SENSOR_VALUE_BUFFER_SIZE    16

#endif
