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
    ETS_GPIO_INTR_DISABLE() ;
    PIN_FUNC_SELECT(ONEWIRE_DQ_MUX, ONEWIRE_DQ_FUNC);
    GPIO_REG_WRITE(
        GPIO_PIN_ADDR(
            GPIO_ID_PIN(ONEWIRE_DQ_GPIO)),
            GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(ONEWIRE_DQ_GPIO))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << ONEWIRE_DQ_GPIO));
    ETS_GPIO_INTR_ENABLE() ;
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
    gpio_output_set(1 << ONEWIRE_DQ_GPIO, 0, 1 << ONEWIRE_DQ_GPIO, 0);
}

static inline void _onewire_dq_low(void) {
    gpio_output_set(0, 1 << ONEWIRE_DQ_GPIO, 1 << ONEWIRE_DQ_GPIO, 0);
}

static inline void _onewire_dq_input(void) {
}

static inline void _onewire_dq_output(void) {
}

static inline uint8_t _onewire_dq_read(void) {
    return GPIO_INPUT_GET(GPIO_ID_PIN(ONEWIRE_DQ_GPIO));
}
