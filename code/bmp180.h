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

#ifndef __BMP180_H__
#define __BMP180_H__

#include <c_types.h>

#define BMP180_ADDRESS_WRITE    0xee
#define BMP180_ADDRESS_READ     0xef
#define BMP180_CHIP_ID          0x55

#define BMP180_REGISTER_CTRL_MEAS   0xf4
#define BMP180_REGISTER_CHIP_ID     0xd0

#define BMP180_CALIBRATION_AC1_MSB  0xaa
#define BMP180_CALIBRATION_AC1_LSB  0xab
#define BMP180_CALIBRATION_AC2_MSB  0xac
#define BMP180_CALIBRATION_AC2_LSB  0xad
#define BMP180_CALIBRATION_AC3_MSB  0xae
#define BMP180_CALIBRATION_AC3_LSB  0xaf
#define BMP180_CALIBRATION_AC4_MSB  0xb0
#define BMP180_CALIBRATION_AC4_LSB  0xb1
#define BMP180_CALIBRATION_AC5_MSB  0xb2
#define BMP180_CALIBRATION_AC5_LSB  0xb3
#define BMP180_CALIBRATION_AC6_MSB  0xb4
#define BMP180_CALIBRATION_AC6_LSB  0xb5
#define BMP180_CALIBRATION_B1_MSB   0xb6
#define BMP180_CALIBRATION_B1_LSB   0xb7
#define BMP180_CALIBRATION_B2_MSB   0xb8
#define BMP180_CALIBRATION_B2_LSB   0xb9
#define BMP180_CALIBRATION_MB_MSB   0xba
#define BMP180_CALIBRATION_MB_LSB   0xbb
#define BMP180_CALIBRATION_MC_MSB   0xbc
#define BMP180_CALIBRATION_MC_LSB   0xbd
#define BMP180_CALIBRATION_MD_MSB   0xbe
#define BMP180_CALIBRATION_MD_LSB   0xbf

#define BMP180_OUT_MSB              0xf6
#define BMP180_OUT_LSB              0xf7
#define BMP180_OUT_XLSB             0xf8

#define BMP180_SCO          5
#define BMP180_OSS_SHIFT    6

/**
 * Calibration data readed at start-up.
 */
struct bmp180_calibration_data {
    int16_t ac1;
    int16_t ac2;
    int16_t ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t b1;
    int16_t b2;
    int16_t mb;
    int16_t mc;
    int16_t md;
};

/**
 * Sensor read status.
 */
enum bmp180_read_status {
    BMP180_READ_STATUS_OK,
    BMP180_READ_STATUS_WRITE_ADDRESS_ERROR,
    BMP180_READ_STATUS_WRITE_REGISTER_ERROR,
    BMP180_READ_STATUS_READ_ADDRESS_ERROR,
};

/**
 * Possible oss levels.
 */
enum bmp180_pressure_oss {
    BMP180_OSS_SINGLE = 0,
    BMP180_OSS_2 = 1,
    BMP180_OSS_4 = 2,
    BMP180_OSS_8 = 3,
};

/**
 * Sensor measurement.
 */
struct bmp180_data {
    int32_t temperature;            /**< Temperature in m degrees C. */
    uint32_t pressure;              /**< Pressure in Pa. */
};

extern struct bmp180_data bmp180_data;

void ICACHE_FLASH_ATTR bmp180_init(void);
uint8_t ICACHE_FLASH_ATTR bmp180_get_chip_id(void);
bool ICACHE_FLASH_ATTR bmp180_test(void);
enum bmp180_read_status ICACHE_FLASH_ATTR bmp180_read(enum bmp180_pressure_oss oss);

#endif
