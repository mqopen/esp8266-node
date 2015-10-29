#include <osapi.h>
#include "common.h"
#include "i2c_master.h"
#include "bmp180.h"

struct bmp180_calibration_data _bmp180_calibration;

struct bmp180_data bmp180_data;

/* Static function prototypes. */

static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read_ut(int32_t *ut);
static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read_up(int32_t *up, enum bmp180_pressure_oss oss);
static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read(uint8_t reg, uint8_t *buf, uint32_t len);
static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read_short(uint8_t msb, uint8_t lsb, uint16_t *value);
static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_write(uint8_t address, uint8_t value);
static void ICACHE_FLASH_ATTR _bmp180_init_calibration(void);

void ICACHE_FLASH_ATTR bmp180_init(void) {
    _bmp180_init_calibration();
}

uint8_t ICACHE_FLASH_ATTR bmp180_get_chip_id(void) {
    uint8_t chip_id;
    _bmp180_read(BMP180_REGISTER_CHIP_ID, &chip_id, sizeof(chip_id));
    return chip_id;
}

bool ICACHE_FLASH_ATTR bmp180_test(void) {
    return bmp180_get_chip_id() == BMP180_CHIP_ID;
}

enum bmp180_read_status ICACHE_FLASH_ATTR bmp180_read(enum bmp180_pressure_oss oss) {
    /* Read temperature. */
    int32_t _ut;
    int32_t _up;
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

    enum bmp180_read_status _status;
    _status = _bmp180_read_ut(&_ut);
    if (_status == BMP180_READ_STATUS_OK) {
        _x1 = (_ut - _bmp180_calibration.ac6) * _bmp180_calibration.ac5 / (2<<14);
        _x2 = _bmp180_calibration.mc * (2<<10) / (_x1 + _bmp180_calibration.md);
        _b5 = _x1 + _x2;
        _t = (_b5 + 8) / (2<<3);
        bmp180_data.temperature = _t * 100;
    }

    /* Read pressure. */
    _status = _bmp180_read_up(&_up, oss);
    if (_status == BMP180_READ_STATUS_OK) {
        _b6 = _b5 - 4000;
        _x1 = (_bmp180_calibration.b2 * (_b6 * _b6 / (2<<11))) / (2<<10);
        _x2 = _bmp180_calibration.ac2 * _b6 / (2<<10);
        _x3 = _x1 + _x2;
        _b3 = (((_bmp180_calibration.ac1 * 4 + _x3) << oss) + 2) / 4;
        _x1 = _bmp180_calibration.ac3 * _b6 / (2<<12);
        _x2 = (_bmp180_calibration.b1 * (_b6 * _b6 / (2<<11))) / (2<<15);
        _x3 = ((_x1 + _x2) + 2) / (2<<1);
        _b4 = _bmp180_calibration.ac4 * ((uint32_t) (_x3 + 32768)) / (2<<14);
        _b7 = ((uint32_t) (_up - _b3)) * (50000 >> oss);
        if (_b7 < 0x80000000) {
            _p = (_b7 * 2) / _b4;
        } else {
            _p = (_b7 / _b4) * 2;
        }
        _x1 = (_p / (2<<7)) * (_p / (2<<7));
        _x1 = (_x1 * 3038) / (2<<16);
        _x2 = (-7357 * _p) / (2<<15);
        _p = _p + (_x1 + _x2 + 3791) / (2<<3);
        bmp180_data.pressure = _p;
    }
    return _status;
}

static void ICACHE_FLASH_ATTR _bmp180_init_calibration(void) {
    _bmp180_read_short(BMP180_CALIBRATION_AC1_MSB, BMP180_CALIBRATION_AC1_LSB, &_bmp180_calibration.ac1);
    _bmp180_read_short(BMP180_CALIBRATION_AC2_MSB, BMP180_CALIBRATION_AC2_LSB, &_bmp180_calibration.ac2);
    _bmp180_read_short(BMP180_CALIBRATION_AC3_MSB, BMP180_CALIBRATION_AC3_LSB, &_bmp180_calibration.ac3);
    _bmp180_read_short(BMP180_CALIBRATION_AC4_MSB, BMP180_CALIBRATION_AC4_LSB, &_bmp180_calibration.ac4);
    _bmp180_read_short(BMP180_CALIBRATION_AC5_MSB, BMP180_CALIBRATION_AC5_LSB, &_bmp180_calibration.ac5);
    _bmp180_read_short(BMP180_CALIBRATION_AC6_MSB, BMP180_CALIBRATION_AC6_LSB, &_bmp180_calibration.ac6);
    _bmp180_read_short(BMP180_CALIBRATION_B1_MSB, BMP180_CALIBRATION_B1_LSB, &_bmp180_calibration.b1);
    _bmp180_read_short(BMP180_CALIBRATION_B2_MSB, BMP180_CALIBRATION_B2_LSB, &_bmp180_calibration.b2);
    _bmp180_read_short(BMP180_CALIBRATION_MB_MSB, BMP180_CALIBRATION_MB_LSB, &_bmp180_calibration.mb);
    _bmp180_read_short(BMP180_CALIBRATION_MC_MSB, BMP180_CALIBRATION_MC_LSB, &_bmp180_calibration.mc);
    _bmp180_read_short(BMP180_CALIBRATION_MD_MSB, BMP180_CALIBRATION_MB_LSB, &_bmp180_calibration.md);
}

static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read_ut(int32_t *ut) {
    uint16_t _ut16;
    _bmp180_write(BMP180_REGISTER_CTRL_MEAS, _BV(BMP180_SCO) | 0x0e);
    enum bmp180_read_status _status =_bmp180_read_short(BMP180_OUT_MSB, BMP180_OUT_LSB, &_ut16);
    if (_status == BMP180_READ_STATUS_OK) {
        *ut = _ut16;
    }
    return _status;
}

static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read_up(int32_t *up, enum bmp180_pressure_oss oss) {
    uint16_t _up;
    uint8_t _up_xlsb;
    _bmp180_write(BMP180_REGISTER_CTRL_MEAS, (uint8_t) (0x34 | (oss << BMP180_OSS_SHIFT)));
    enum bmp180_read_status _status =_bmp180_read_short(BMP180_OUT_MSB, BMP180_OUT_LSB, &_up);
    if (_status != BMP180_READ_STATUS_OK)
        return _status;
    _status = _bmp180_read(BMP180_OUT_XLSB, &_up_xlsb, sizeof(_up_xlsb));
    if (_status != BMP180_READ_STATUS_OK)
        return _status;
    *up = ((_up << 8) | _up_xlsb) >> (8 - oss);
    return _status;
}

static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read_short(uint8_t msb, uint8_t lsb, uint16_t *value) {
    uint8_t _msb;
    uint8_t _lsb;
    uint8_t _ctl_meas;
    enum bmp180_read_status status;

    /* Wait until measurement is done. */
    do {
        status = _bmp180_read(BMP180_REGISTER_CTRL_MEAS, &_ctl_meas, sizeof(_ctl_meas));
        if (status != BMP180_READ_STATUS_OK)
            return status;
    } while (_ctl_meas & _BV(BMP180_SCO));

    status = _bmp180_read(msb, &_msb, sizeof(_msb));
    if (status != BMP180_READ_STATUS_OK)
        return status;
    status = _bmp180_read(lsb, &_lsb, sizeof(_lsb));
    if (status != BMP180_READ_STATUS_OK)
        return status;
    *value = (_msb << 8) | _lsb;
    return status;
}

static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read(uint8_t reg, uint8_t *buf, uint32_t len) {
    i2c_master_start();
    i2c_master_writeByte(BMP180_ADDRESS_WRITE);
    if (i2c_master_getAck()) {
        return BMP180_READ_STATUS_WRITE_ADDRESS_ERROR;
    }

    i2c_master_writeByte(reg);
    if (i2c_master_getAck()) {
        return BMP180_READ_STATUS_WRITE_REGISTER_ERROR;
    }
    i2c_master_stop();

    i2c_master_start();
    i2c_master_writeByte(BMP180_ADDRESS_READ);
    if (i2c_master_getAck()) {
        return BMP180_READ_STATUS_READ_ADDRESS_ERROR;
    }

    while (len--) {
        *buf++ = i2c_master_readByte();
        i2c_master_setAck(1);
    }
    i2c_master_stop();
    return BMP180_READ_STATUS_OK;
}

static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_write(uint8_t address, uint8_t value) {
    i2c_master_start();
    i2c_master_writeByte(BMP180_ADDRESS_WRITE);
    if (i2c_master_getAck()) {
        return BMP180_READ_STATUS_WRITE_ADDRESS_ERROR;
    }

    /* Control register. */
    i2c_master_writeByte(address);
    if (i2c_master_getAck()) {
        return BMP180_READ_STATUS_WRITE_REGISTER_ERROR;
    }

    /* Set register value. */
    i2c_master_writeByte(value);
    if (i2c_master_getAck()) {
        return BMP180_READ_STATUS_WRITE_REGISTER_ERROR;
    }
    i2c_master_stop();
    return BMP180_READ_STATUS_OK;
}
