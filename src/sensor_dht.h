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

#ifndef __SENSOR_DHT_H__
#define __SENSOR_DHT_H__

#include "dht.h"

#if ENABLE_SENSOR_DHT22
  #define SENSOR_NAME "dht22"
#elif ENABLE_SENSOR_DHT11
  #define SENSOR_NAME "dht11"
#else
  #error Unsupported DHT variant.
#endif

#define sensor_init dht_init

/**
 * Possible values are:
 *  - 'xxx.xxx'             : len = 7 (temperature, humidity)
 *  - 'E_CONN'              : len = 6
 *  - 'E_ACK_H'             : len = 7
 *  - 'E_ACK_L'             : len = 7
 *  - 'E_TIMEOUT_L'         : len = 11
 *  - 'E_TIMEOUT_H'         : len = 11
 *  - 'E_CHECKSUM'          : len = 10
 *
 * maxium possible length: 16 Bytes
 */
#define SENSOR_VALUE_BUFFER_SIZE    11

#endif
