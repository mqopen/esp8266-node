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
#include <gpio.h>
#include <eagle_soc.h>
#include "pinstate.h"

#include <osapi.h>

/**
 * Pinstate GPIO pin.
 */
#define PINSTATE_PIN_GPIO CONFIG_REACTOR_PINSTATE_GPIO_PIN
#if CONFIG_REACTOR_PINSTATE_GPIO_PIN == 0
  #define PINSTATE_PIN_MUX PERIPHS_IO_MUX_GPIO0_U
  #define PINSTATE_PIN_FUNC FUNC_GPIO0
#elif CONFIG_REACTOR_PINSTATE_GPIO_PIN == 2
  #define PINSTATE_PIN_MUX PERIPHS_IO_MUX_GPIO2_U
  #define PINSTATE_PIN_FUNC FUNC_GPIO2
#elif CONFIG_REACTOR_PINSTATE_GPIO_PIN == 13
  #define PINSTATE_PIN_MUX PERIPHS_IO_MUX_MTCK_U
  #define PINSTATE_PIN_FUNC FUNC_GPIO13
#elif CONFIG_REACTOR_PINSTATE_GPIO_PIN == 14
  #define PINSTATE_PIN_MUX PERIPHS_IO_MUX_MTMS_U
  #define PINSTATE_PIN_FUNC FUNC_GPIO14
#else
  #error Unsupported pinstate pin number!
#endif

/* Logic inversion */
#if ENABLE_REACTOR_PINSTATE_LOGIC
  #define PINSTATE_INVERT !
#else
  #define PINSTATE_INVERT
#endif

static bool _pinstate_is_enabled = false;

/* Static functions prototypes. */
inline void _pinstate_high(void);
inline void _pinstate_low(void);

void pinstate_init(void) {
    gpio_init();
    PIN_FUNC_SELECT(PINSTATE_PIN_MUX, PINSTATE_PIN_FUNC);
    PIN_PULLUP_DIS(PINSTATE_PIN_MUX);
    pinstate_set(false);
}

void pinstate_set(bool enable) {
    if (PINSTATE_INVERT enable) {
        _pinstate_high();
    } else {
        _pinstate_low();
    }
    _pinstate_is_enabled = enable;
}

inline bool pinstate_get(void) {
    return _pinstate_is_enabled;
}

inline void _pinstate_high(void) {
    gpio_output_set(1 << PINSTATE_PIN_GPIO, 0, 1 << PINSTATE_PIN_GPIO, 0);
}

inline void _pinstate_low(void) {
    gpio_output_set(0, 1 << PINSTATE_PIN_GPIO, 1 << PINSTATE_PIN_GPIO, 0);
}
