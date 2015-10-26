#include <osapi.h>
#include "i2c_master.h"
#include "bmp180.h"

struct bmp180_calibration_data _bmp180_calibration;

int16_t bmp180_temperature;

/* Static function prototypes. */

static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read(uint8_t reg, uint8_t *buf, uint32_t len);
static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read_short(uint8_t reg, uint16_t *value);
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

enum bmp180_read_status ICACHE_FLASH_ATTR bmp180_read_temperature(void) {
    i2c_master_start();
    i2c_master_writeByte(BMP180_ADDRESS_WRITE);
    if (i2c_master_getAck()) {
        return BMP180_READ_STATUS_WRITE_ADDRESS_ERROR;
    }

    /* Control register. */
    i2c_master_writeByte(0xf4);
    if (i2c_master_getAck()) {
        return BMP180_READ_STATUS_WRITE_REGISTER_ERROR;
    }

    /* Set register to read temeparture */
    i2c_master_writeByte(0x2e);
    if (i2c_master_getAck()) {
        return BMP180_READ_STATUS_WRITE_REGISTER_ERROR;
    }
    i2c_master_stop();

    /* Read temperature. */
    int16_t _raw_temperature;
    int32_t _x1, _x2, _b5;
    //enum bmp180_read_status status = _bmp180_read(0xf6, (uint8_t *) &_raw_temperature, sizeof(_raw_temperature));
    enum bmp180_read_status status = _bmp180_read_short(0xf6, &_raw_temperature);
    if (status == BMP180_READ_STATUS_OK) {
        //_raw_temperature = (_raw_temperature >> 8) | (_raw_temperature << 8);
        _x1 = (_raw_temperature - _bmp180_calibration.ac6) * (_bmp180_calibration.ac5 / (2<<14));
        _x2 = (_bmp180_calibration.mc * (2<<10)) / (_x1 + _bmp180_calibration.md);
        _b5 = _x1 + _x2;
        bmp180_temperature = (_b5 + 8) / (2<<3);
    }

    if (status == BMP180_READ_STATUS_OK) {
        //os_printf("X1: %d, X2: %d, B5: %d\r\n", _x1, _x2, _b5);
        //os_printf("Temp: %d, (%d)\r\n", bmp180_temperature, _raw_temperature);
    } else {
        //os_printf("Temp read failed\r\n");
    }

    return status;
}

static void ICACHE_FLASH_ATTR _bmp180_init_calibration(void) {
    _bmp180_read_short(0xaa, &_bmp180_calibration.ac1);
    _bmp180_read_short(0xac, &_bmp180_calibration.ac2);
    _bmp180_read_short(0xae, &_bmp180_calibration.ac3);
    _bmp180_read_short(0xb0, &_bmp180_calibration.ac4);
    _bmp180_read_short(0xb2, &_bmp180_calibration.ac5);
    _bmp180_read_short(0xb4, &_bmp180_calibration.ac6);

    _bmp180_read_short(0xb6, &_bmp180_calibration.b1);
    _bmp180_read_short(0xb8, &_bmp180_calibration.b2);

    _bmp180_read_short(0xba, &_bmp180_calibration.mb);
    _bmp180_read_short(0xbc, &_bmp180_calibration.mc);
    _bmp180_read_short(0xbe, &_bmp180_calibration.md);

    os_printf("AC1: %d\r\n", _bmp180_calibration.ac1);
    os_printf("AC2: %d\r\n", _bmp180_calibration.ac2);
    os_printf("AC3: %d\r\n", _bmp180_calibration.ac3);
    os_printf("AC4: %d\r\n", _bmp180_calibration.ac4);
    os_printf("AC5: %d\r\n", _bmp180_calibration.ac5);
    os_printf("AC6: %d\r\n", _bmp180_calibration.ac6);

    os_printf("B1: %d\r\n", _bmp180_calibration.b1);
    os_printf("B2: %d\r\n", _bmp180_calibration.b2);

    os_printf("MB: %d\r\n", _bmp180_calibration.mb);
    os_printf("MC: %d\r\n", _bmp180_calibration.mc);
    os_printf("MD: %d\r\n", _bmp180_calibration.md);
}

static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read_short(uint8_t reg, uint16_t *value) {
    uint8_t _raw_data[2];
    enum bmp180_read_status status = _bmp180_read(reg, _raw_data, sizeof(_raw_data));
    if (status == BMP180_READ_STATUS_OK) {
        *value = (_raw_data[0] << 8) | _raw_data[1];
    }
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
