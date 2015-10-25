#include <osapi.h>
#include "i2c_master.h"
#include "bmp180.h"

struct bmp180_calibration_data _bmp180_calibration;

int16_t bmp180_temperature;

/* Static function prototypes. */

static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_read(uint8_t reg, uint8_t *buf, uint32_t len);
static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_init_calibration(void);

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
    int32_t _x1, _x2;
    enum bmp180_read_status status = _bmp180_read(0xf6, (uint8_t *) &_raw_temperature, sizeof(_raw_temperature));
    if (status == BMP180_READ_STATUS_OK) {
        _x1 = (_raw_temperature - _bmp180_calibration.ac6) * (_bmp180_calibration.ac5 / (2<<15));
        _x2 = (_bmp180_calibration.mc * (2<<11)) / (_x1 + _bmp180_calibration.md);
        bmp180_temperature = ((_x1 + _x2) + 8) / (2<<4);
    }

    if (status == BMP180_READ_STATUS_OK) {
        os_printf("Temp: %d\r\n", bmp180_temperature);
    } else {
        os_printf("Temp read failed\r\n");
    }

    return status;
}

static enum bmp180_read_status ICACHE_FLASH_ATTR _bmp180_init_calibration(void) {
    return _bmp180_read(BMP180_REGISTER_CALIBRATION, (uint8_t *) &_bmp180_calibration, sizeof(_bmp180_calibration));
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
