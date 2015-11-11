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

#include <osapi.h>
#include "common.h"
#include "i2c_master.h"
#include "bmp180.h"

/**
 * Sensor calibration.
 */
struct bmp180_calibration_data _bmp180_calibration;

struct bmp180_data bmp180_data;

/* Static function prototypes. */

/**
 * Read uncalibrated temperature.
 *
 * @param ut Pointer to memory where UT will be stored.
 * @return Result of read operation.
 */
static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_read_ut(int32_t *ut);

/**
 * Read uncalibrated pressure.
 *
 * @param up Pointer to memory where UP will be stored.
 * @return Result of read operation.
 */
static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_read_up(int32_t *up, enum bmp180_pressure_oss oss);

/**
 * Read value from the register.
 *
 * @return Result of read operation.
 *      BMP180_IO_OK if communication is OK.
 *      BMP180_IO_WRITE_ADDRESS_ERROR when I2C write address is not acknowledged.
 *      BMP180_IO_WRITE_REGISTER_ERROR when device read register address is not acknowledged.
 *      BMP180_IO_READ_ADDRESS_ERROR when I2C read address is not acknowledged.
 */
static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_read(uint8_t reg, uint8_t *value);

/**
 * Loop until measurement conversion is complete.
 *
 * @return Status of read operation.
 */
static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_loop_sco(void);

/**
 * Read two byte value from two reisters.
 *
 * @param msb Address of MSB register.
 * @param lsb Address of LSB register.
 * @return Result of read operation.
 */
static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_read_short(uint8_t msb, uint8_t lsb, uint16_t *value);

/**
 * Write to chip.
 *
 * @param address
 * @param value
 * @return Result of write operation.
 *      BMP180_IO_OK if communication is OK.
 *      BMP180_IO_WRITE_ADDRESS_ERROR if I2C write address is not acknowledged.
 *      BMP180_IO_WRITE_REGISTER_ERROR if device write register address is not acknowledged.
 *      BMP180_IO_WRITE_VALUE_ERROR if device register value is not acknowledged.
 */
static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_write(uint8_t address, uint8_t value);

/**
 * Init calibration.
 */
static void ICACHE_FLASH_ATTR _bmp180_init_calibration(void);

void ICACHE_FLASH_ATTR bmp180_init(void) {
    _bmp180_init_calibration();
}

uint8_t ICACHE_FLASH_ATTR bmp180_get_chip_id(void) {
    uint8_t chip_id;
    _bmp180_read(BMP180_REGISTER_CHIP_ID, &chip_id);
    return chip_id;
}

bool ICACHE_FLASH_ATTR bmp180_test(void) {
    return bmp180_get_chip_id() == BMP180_CHIP_ID;
}

enum bmp180_io_result ICACHE_FLASH_ATTR bmp180_read(enum bmp180_pressure_oss oss) {
    /* Read temperature. */
    int32_t _ut = 0;
    int32_t _up = 0;
    int32_t _x1;
    int32_t _x2;
    int32_t _x3;
    int32_t _b3;
   uint32_t _b4;
    int32_t _b5;
    int32_t _b6;
   uint32_t _b7;
    int32_t _t;
    int32_t _p;

    enum bmp180_io_result _io_result;
    _io_result = _bmp180_read_ut(&_ut);
    if (_io_result == BMP180_IO_OK) {
        _x1 = ((_ut - _bmp180_calibration.ac6) * _bmp180_calibration.ac5) >> 15;
        _x2 = (_bmp180_calibration.mc << 11) / (_x1 + _bmp180_calibration.md);
        _b5 = _x1 + _x2;
        _t = (_b5 + 8) >> 4;
        bmp180_data.temperature = _t * 100;
    } else {
        return _io_result;
    }

    /* Read pressure. */
    _io_result = _bmp180_read_up(&_up, oss);
    if (_io_result == BMP180_IO_OK) {
        os_printf("UP: %d\r\n", _up);
        _b6 = _b5 - 4000;
        os_printf("B6: %d\r\n", _b6);
        _x1 = (_bmp180_calibration.b2 * ((_b6 * _b6) >> 12)) >> 11;
        os_printf("X1: %d\r\n", _x1);
        _x2 = (_bmp180_calibration.ac2 * _b6) >> 11;
        os_printf("X2: %d\r\n", _x2);
        _x3 = _x1 + _x2;
        os_printf("X3: %d\r\n", _x3);
        _b3 = (((_bmp180_calibration.ac1 * 4 + _x3) << oss) + 2) >> 2;
        os_printf("B3: %d\r\n", _b3);
        _x1 = (_bmp180_calibration.ac3 * _b6) >> 13;
        os_printf("X1: %d\r\n", _x1);
        _x2 = (_bmp180_calibration.b1 * ((_b6 * _b6) >> 12)) >> 16;
        os_printf("X2: %d\r\n", _x2);
        _x3 = ((_x1 + _x2) + 2) >> 2;
        os_printf("X3: %d\r\n", _x3);
        _b4 = (_bmp180_calibration.ac4 * ((uint32_t) (_x3 + 32768))) >> 15;
        os_printf("B4: %u\r\n", _b4);
        _b7 = ((uint32_t) _up - _b3) * (50000 >> oss);
        os_printf("B7: %u\r\n", _b7);
        if (_b7 < 0x80000000) {
            _p = (_b7 << 1) / _b4;
        } else {
            _p = (_b7 / _b4) << 1;
        }
        os_printf("P: %d\r\n", _p);
        _x1 = (_p >> 8) * (_p >> 8);
        os_printf("X1: %d\r\n", _x1);
        _x1 = (_x1 * 3038) >> 16;
        os_printf("X1: %d\r\n", _x1);
        _x2 = (-7357 * _p) >> 16;
        os_printf("X2: %d\r\n", _x2);
        _p = (_p + (_x1 + _x2 + 3791)) >> 4;
        os_printf("P: %d\r\n", _p);
        os_printf("  ----------------------  \r\n");
        bmp180_data.pressure = _p;
    }
    return _io_result;
}

static void ICACHE_FLASH_ATTR _bmp180_init_calibration(void) {
    _bmp180_read_short(BMP180_CALIBRATION_AC1_MSB, BMP180_CALIBRATION_AC1_LSB, (uint16_t *) &_bmp180_calibration.ac1);
    _bmp180_read_short(BMP180_CALIBRATION_AC2_MSB, BMP180_CALIBRATION_AC2_LSB, (uint16_t *) &_bmp180_calibration.ac2);
    _bmp180_read_short(BMP180_CALIBRATION_AC3_MSB, BMP180_CALIBRATION_AC3_LSB, (uint16_t *) &_bmp180_calibration.ac3);
    _bmp180_read_short(BMP180_CALIBRATION_AC4_MSB, BMP180_CALIBRATION_AC4_LSB, &_bmp180_calibration.ac4);
    _bmp180_read_short(BMP180_CALIBRATION_AC5_MSB, BMP180_CALIBRATION_AC5_LSB, &_bmp180_calibration.ac5);
    _bmp180_read_short(BMP180_CALIBRATION_AC6_MSB, BMP180_CALIBRATION_AC6_LSB, &_bmp180_calibration.ac6);
    _bmp180_read_short(BMP180_CALIBRATION_B1_MSB, BMP180_CALIBRATION_B1_LSB, (uint16_t *) &_bmp180_calibration.b1);
    _bmp180_read_short(BMP180_CALIBRATION_B2_MSB, BMP180_CALIBRATION_B2_LSB, (uint16_t *) &_bmp180_calibration.b2);
    _bmp180_read_short(BMP180_CALIBRATION_MB_MSB, BMP180_CALIBRATION_MB_LSB, (uint16_t *) &_bmp180_calibration.mb);
    _bmp180_read_short(BMP180_CALIBRATION_MC_MSB, BMP180_CALIBRATION_MC_LSB, (uint16_t *) &_bmp180_calibration.mc);
    _bmp180_read_short(BMP180_CALIBRATION_MD_MSB, BMP180_CALIBRATION_MB_LSB, (uint16_t *) &_bmp180_calibration.md);

    os_printf("AC1: %d\r\n", _bmp180_calibration.ac1);
    os_printf("AC2: %d\r\n", _bmp180_calibration.ac2);
    os_printf("AC3: %d\r\n", _bmp180_calibration.ac3);
    os_printf("AC4: %u\r\n", _bmp180_calibration.ac4);
    os_printf("AC5: %u\r\n", _bmp180_calibration.ac5);
    os_printf("AC6: %u\r\n", _bmp180_calibration.ac6);
    os_printf("B1: %d\r\n", _bmp180_calibration.b1);
    os_printf("B2: %d\r\n", _bmp180_calibration.b2);
    os_printf("MB: %d\r\n", _bmp180_calibration.mb);
    os_printf("MC: %d\r\n", _bmp180_calibration.mc);
    os_printf("MD: %d\r\n", _bmp180_calibration.md);
}

static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_read_ut(int32_t *ut) {
    uint16_t _ut16;
    enum bmp180_io_result _io_result;
    _io_result = _bmp180_write(BMP180_REGISTER_CTRL_MEAS, _BV(BMP180_SCO) | 0x0e);
    if (_io_result != BMP180_IO_OK)
        return _io_result;
    _io_result = _bmp180_loop_sco();
    if (_io_result != BMP180_IO_OK)
        return _io_result;
    _io_result =_bmp180_read_short(BMP180_OUT_MSB, BMP180_OUT_LSB, &_ut16);
    if (_io_result == BMP180_IO_OK)
        *ut = _ut16;
    return _io_result;
}

static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_read_up(int32_t *up, enum bmp180_pressure_oss oss) {
    uint8_t _up_msb;
    uint8_t _up_lsb;
    uint8_t _up_xlsb;
    enum bmp180_io_result _io_result;
    _io_result = _bmp180_write(BMP180_REGISTER_CTRL_MEAS, (uint8_t) (0x34 | (oss << BMP180_OSS_SHIFT)));
    if (_io_result != BMP180_IO_OK)
        return _io_result;
    _io_result = _bmp180_loop_sco();
    if (_io_result != BMP180_IO_OK)
        return _io_result;
    _io_result =_bmp180_read(BMP180_OUT_MSB, &_up_msb);
    if (_io_result != BMP180_IO_OK)
        return _io_result;
    _io_result =_bmp180_read(BMP180_OUT_LSB, &_up_lsb);
    if (_io_result != BMP180_IO_OK)
        return _io_result;
    _io_result = _bmp180_read(BMP180_OUT_XLSB, &_up_xlsb);
    if (_io_result != BMP180_IO_OK)
        return _io_result;
    *up = ((_up_msb << 16) | (_up_lsb << 8) | _up_xlsb) >> (8 - oss);
    return _io_result;
}

static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_read_short(uint8_t msb, uint8_t lsb, uint16_t *value) {
    uint8_t _msb;
    uint8_t _lsb;
    enum bmp180_io_result _io_result;
    _io_result = _bmp180_read(msb, &_msb);
    if (_io_result != BMP180_IO_OK)
        return _io_result;
    _io_result = _bmp180_read(lsb, &_lsb);
    if (_io_result != BMP180_IO_OK)
        return _io_result;
    *value = (_msb << 8) | _lsb;
    return _io_result;
}

static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_read(uint8_t reg, uint8_t *value) {
    i2c_master_start();
    i2c_master_writeByte(BMP180_ADDRESS_WRITE);
    if (i2c_master_getAck()) {
        return BMP180_IO_WRITE_ADDRESS_ERROR;
    }

    i2c_master_writeByte(reg);
    if (i2c_master_getAck()) {
        return BMP180_IO_WRITE_REGISTER_ERROR;
    }
    i2c_master_stop();

    i2c_master_start();
    i2c_master_writeByte(BMP180_ADDRESS_READ);
    if (i2c_master_getAck()) {
        return BMP180_IO_READ_ADDRESS_ERROR;
    }

    *value = i2c_master_readByte();
    i2c_master_setAck(1);

    i2c_master_stop();
    return BMP180_IO_OK;
}

static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_write(uint8_t address, uint8_t value) {
    i2c_master_start();
    i2c_master_writeByte(BMP180_ADDRESS_WRITE);
    if (i2c_master_getAck()) {
        return BMP180_IO_WRITE_ADDRESS_ERROR;
    }

    /* Control register. */
    i2c_master_writeByte(address);
    if (i2c_master_getAck()) {
        return BMP180_IO_WRITE_REGISTER_ERROR;
    }

    /* Set register value. */
    i2c_master_writeByte(value);
    if (i2c_master_getAck()) {
        return BMP180_IO_WRITE_VALUE_ERROR;
    }
    i2c_master_stop();
    return BMP180_IO_OK;
}

static enum bmp180_io_result ICACHE_FLASH_ATTR _bmp180_loop_sco(void) {
    enum bmp180_io_result _io_result;
    uint8_t _ctl_meas;
    do {
        _io_result = _bmp180_read(BMP180_REGISTER_CTRL_MEAS, &_ctl_meas);
        if (_io_result != BMP180_IO_OK)
            return _io_result;
    } while (_ctl_meas & _BV(BMP180_SCO));
    return _io_result;
}
