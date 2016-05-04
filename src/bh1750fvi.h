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
#include <os_type.h>

#ifndef __BH1750FVI_H__
#define __BH1750FVI_H__

/* I2C address configuration. */
#if ENABLE_SENSOR_BH1750FVI_I2C_ADDRESS_0X23
  #define BH1750FVI_ADDRESS               0x23
#elif ENABLE_SENSOR_BH1750FVI_I2C_ADDRESS_0X5C
  #define BH1750FVI_ADDRESS               0x5c
#else
  #error Unsupported I2C address!
#endif
#define BH1750FVI_ADDRESS_WRITE         (BH1750FVI_ADDRESS << 1)
#define BH1750FVI_ADDRESS_READ          ((BH1750FVI_ADDRESS << 1) | 1)

/* bh1750fvi instructions. */
#define BH1750FVI_CMD_PWRDOWN           0x00
#define BH1750FVI_CMD_PWRUP             0x01
#define BH1750FVI_CMD_RESET             0x07
#define BH1750FVI_CMD_HRES1             0x10
#define BH1750FVI_CMD_HRES2             0x11
#define BH1750FVI_CMD_LRES              0x13
#define BH1750FVI_CMD_ONETIME_HRES1     0x20
#define BH1750FVI_CMD_ONETIME_HRES2     0x21
#define BH1750FVI_CMD_ONETIME_LRES      0x23

extern uint16_t bh1750fvi_data;

/**
 * Result of IO operation with sensor.
 */
enum bh1750fvi_io_result {
    BH1750FVI_IO_OK,                       /**< Communication is OK. */
    BH1750FVI_IO_WRITE_ADDRESS_ERROR,      /**< Write address not acknowledged. */
    BH1750FVI_IO_WRITE_REGISTER_ERROR,     /**< Write of destination register not acknowledged. */
    BH1750FVI_IO_WRITE_VALUE_ERROR,        /**< Write of register value not acknowledged (Write operation only). */
    BH1750FVI_IO_READ_ADDRESS_ERROR,       /**< Read address not acknowledged. */
    BH1750FVI_IO_INVALID_DATA,             /**< Invalid data. */
};

void bh1750fvi_init(void);

enum bh1750fvi_io_result bh1750fvi_read(void);

#endif
