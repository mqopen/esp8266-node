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
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include "common.h"
#include "dht.h"

/* Static function protypes. */
static inline enum dht_io_result _dht_read_data(uint8_t *buf);
static inline void _dht_dq_output(void);
static inline void _dht_dq_input(void);
static inline void _dht_dq_high(void);
static inline void _dht_dq_low(void);
static inline uint8_t _dht_dq_read(void);
static inline void _dht_reset_dq(void);

void dht_init(void) {
    PIN_FUNC_SELECT(DHT_DQ_MUX, DHT_DQ_FUNC);
    GPIO_REG_WRITE(
        GPIO_PIN_ADDR(
            GPIO_ID_PIN(DHT_GPIO_DQ)),
            GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(DHT_GPIO_DQ))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << DHT_GPIO_DQ));
    _dht_reset_dq();
}


enum dht_io_result dht_read_data(uint8_t *buf) {
    enum dht_io_result _io_result = _dht_read_data(buf);
    _dht_reset_dq();
    return _io_result;
}

static inline enum dht_io_result _dht_read_data(uint8_t *buf) {
    uint16_t _timeout;
    uint8_t _result;
    uint8_t _sum;
    uint8_t i;
    uint8_t j;


    _dht_dq_low();
    os_delay_us(DHT_INIT_PULLDOWN_DELAY);

    _dht_dq_high();
    os_delay_us(DHT_INIT_PULLUP_DELAY);

    _dht_dq_input();

    /* Wait for pull-down response signal. */
    _timeout = 9;
    while (_dht_dq_read()) {
        os_delay_us(10);
        if (!_timeout--) {
            return DHT_IO_CONNECT_ERROR;
        }
    }

    /* Wait rest of the period of pull-down response signal. */
    if (_timeout)
        os_delay_us((_timeout - 1) * 10);

    /* Wait for pull-up ACK signal */
    _timeout = 9;
    while (!_dht_dq_read()) {
        os_delay_us(10);
        if (!_timeout--) {
            return DHT_IO_ACK_L_ERROR;
        }
    }

    /* Wait for pull-down of the previous ACK and begining of the transmission. */
    if (_timeout) {
        while (_dht_dq_read()) {
            os_delay_us(10);
            if (!_timeout--) {
                return DHT_IO_ACK_H_ERROR;
            }
        }
    }

    for (j = 0; j < DHT_DATA_BYTE_LEN; j++) {
        _result = 0;
        for (i = 0; i < 8; i++) {

            /* Wait for pull-up that signalizes begining oth the transmitted bit.  */
            _timeout = 6;
            while (!_dht_dq_read()) {
                os_delay_us(10);
                if (!_timeout--) {
                    return DHT_IO_TIMEOUT_L_ERROR;
                }
            }

            os_delay_us(35);
            if (_dht_dq_read()) {
                _result |= _BV(7 - i);

                /* We are receiving one, that should take 70 us. */
                _timeout = 5;
                while (_dht_dq_read()) {
                    os_delay_us(10);
                    if (!_timeout--) {
                        return DHT_IO_TIMEOUT_H_ERROR;
                    }
                }
            }
        }
        *(buf + j) = _result;
    }

    _sum = buf[0] + buf[1] + buf[2] + buf[3];
    if (_sum != buf[4]) {
        return DHT_IO_CHECKSUM_ERROR;
    } else {
        return DHT_IO_OK;
    }
}

static inline void _dht_dq_output(void) {
}

static inline void _dht_dq_input(void) {
    GPIO_DIS_OUTPUT(DHT_GPIO_DQ);
}

static inline void _dht_dq_high(void) {
    gpio_output_set(1 << DHT_GPIO_DQ, 0, 1 << DHT_GPIO_DQ, 0);
}

static inline void _dht_dq_low(void) {
     gpio_output_set(0, 1 << DHT_GPIO_DQ, 1 << DHT_GPIO_DQ, 0);
}

static inline uint8_t _dht_dq_read(void) {
    return GPIO_INPUT_GET(GPIO_ID_PIN(DHT_GPIO_DQ));
}

static inline void _dht_reset_dq(void) {
    _dht_dq_output();
    _dht_dq_high();
}
