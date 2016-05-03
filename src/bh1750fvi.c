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

#include <os_type.h>
#include "bh1750fvi.h"

uint16_t bh1750fvi_data;

/* Static function prototypes. */
static enum bh1750fvi_io_result ICACHE_FLASH_ATTR _bh1750fvi_power_up(void);
static enum bh1750fvi_io_result ICACHE_FLASH_ATTR _bh1750fvi_write(uint8_t cmd);
static enum bh1750fvi_io_result ICACHE_FLASH_ATTR _bh1750fvi_read(uint16_t *value);

void bh1750fvi_init(void) {
    _bh1750fvi_power_up();
    _bh1750fvi_write(BH1750FVI_CMD_HRES1);
}

enum bh1750fvi_io_result bh1750fvi_read(void) {
    return _bh1750fvi_read(&bh1750fvi_data);
}

static enum bh1750fvi_io_result ICACHE_FLASH_ATTR _bh1750fvi_power_up(void) {
    return _bh1750fvi_write(BH1750FVI_CMD_PWRUP);
}

static enum bh1750fvi_io_result ICACHE_FLASH_ATTR _bh1750fvi_write(uint8_t cmd) {
    i2c_master_start();
    i2c_master_writeByte(BH1750FVI_ADDRESS_WRITE);
    if (i2c_master_getAck()) {
        return BH1750FVI_IO_WRITE_ADDRESS_ERROR;
    }
    i2c_master_writeByte(cmd);
    if (i2c_master_getAck()) {
        return BH1750FVI_IO_WRITE_REGISTER_ERROR;
    }
    i2c_master_stop();
    return BH1750FVI_IO_OK;
}

static enum bh1750fvi_io_result ICACHE_FLASH_ATTR _bh1750fvi_read(uint16_t *value) {
    uint8_t value_l;
    uint8_t value_h;
    i2c_master_start();
    i2c_master_writeByte(BH1750FVI_ADDRESS_READ);
    if (i2c_master_getAck()) {
        return BH1750FVI_IO_READ_ADDRESS_ERROR;
    }
    value_h = i2c_master_readByte();
    i2c_master_send_ack();
    value_l = i2c_master_readByte();
    i2c_master_send_nack();
    i2c_master_stop();
    *value = ((value_h << 8) | value_l) / 1.2;
    return BH1750FVI_IO_OK;
}
