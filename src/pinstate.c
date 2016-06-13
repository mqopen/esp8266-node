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

#define PINSTATE_PIN        14
#define PINSTATE_MUX        PERIPHS_IO_MUX_MTMS_U
#define PINSTATE_FUNC       FUNC_GPIO14

static _pinstate_state;

void _pinstate_set(bool state);

bool _pinstate_get(void);

void pinstate_init(void) {
    _pinstate_state = false;
    PIN_FUNC_SELECT(PINSTATE_MUX, PINSTATE_FUNC);
    gpio_output_set(0, 1 << PINSTATE_PIN, 1 << PINSTATE_PIN, 0);
}

void pinstate_update(int32_t value) {
}

void _pinstate_set(bool state) {
    _pinstate_state = state;
}

bool _pinstate_get(void) {
    return _pinstate_state;
}
