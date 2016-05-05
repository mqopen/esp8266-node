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
#include "onewire.h"

/* Static functions prototypes. */

/**
 * Set DQ ping HIGH.
 */
static inline void _onewire_dq_high(void);

/**
 * Set DQ pin LOW.
 */
static inline void _onewire_dq_low(void);

/**
 * Set DQ pin as input.
 */
static inline void _onewire_dq_input(void);

/**
 * Set DQ pin as output.
 */
static inline void _onewire_dq_output(void);

/**
 * Read DQ pin.
 *
 * @return 0 if DQ in LOW, 1 if DQ is HIGH.
 */
static inline uint8_t _onewire_dq_read(void);

void onewire_init() {
    _onewire_dq_high();
    _onewire_dq_output();
}

uint8_t onewire_reset(void) {
    uint8_t presence;
    _onewire_dq_low();
    os_delay_us(480);
    _onewire_dq_high();
    os_delay_us(70);
    _onewire_dq_input();

    presence = !_onewire_dq_read();
    os_delay_us(410);
    _onewire_dq_output();
    return presence;
}

void onewire_write(uint8_t v) {
    uint8_t i = 8;
    while (i--) {
        onewire_write_bit(v & 1);
        v >>= 1;
    }
}

uint8_t onewire_read(void) {
    uint8_t ret = 0;
    uint8_t i = 8;
    while (i--) {
        ret >>= 1;
        ret |= (onewire_read_bit() << 7);
    }
    return ret;
}

void onewire_write_bit(uint8_t v) {
    _onewire_dq_low();
    os_delay_us(1);
    if (v)
        _onewire_dq_high();
    os_delay_us(60);
    _onewire_dq_high();
}

uint8_t onewire_read_bit(void) {
    uint8_t bit = 0;
    _onewire_dq_low();
    os_delay_us(1);
    _onewire_dq_high();
    os_delay_us(14);
    _onewire_dq_input();
    if (_onewire_dq_read())
        bit = 1;
    os_delay_us(45);
    _onewire_dq_output();
    _onewire_dq_high();
    return bit;
}

static inline void _onewire_dq_high(void) {
    GPIO_OUTPUT_SET(ONEWIRE_DQ_GPIO, 1);
}

static inline void _onewire_dq_low(void) {
    GPIO_OUTPUT_SET(ONEWIRE_DQ_GPIO, 0);
}

static inline void _onewire_dq_input(void) {
    GPIO_DIS_OUTPUT(ONEWIRE_DQ_GPIO);
    PIN_PULLUP_EN(ONEWIRE_DQ_MUX);
}

static inline void _onewire_dq_output(void) {
}

static inline uint8_t _onewire_dq_read(void) {
    return GPIO_INPUT_GET(ONEWIRE_DQ_GPIO);
}
