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
#include <eagle_soc.h>
#include <gpio.h>
#include <ets_sys.h>
#include "common.h"
#include "sensor.h"
#include "sensor_button.h"
#include "button.h"

#if ENABLE_SENSOR_BUTTON_1
  #define BUTTON_1_GPIO_PIN CONFIG_SENSOR_BUTTON_1_GPIO_PIN
  #if CONFIG_SENSOR_BUTTON_1_GPIO_PIN == 0
    #define BUTTON_1_MUX PERIPHS_IO_MUX_GPIO0_U
    #define BUTTON_1_FUNC FUNC_GPIO0
  #elif CONFIG_SENSOR_BUTTON_1_GPIO_PIN == 2
    #define BUTTON_1_MUX PERIPHS_IO_MUX_GPIO2_U
    #define BUTTON_1_FUNC FUNC_GPIO2
  #else
    #error Unsupported button GPIO pin number!
  #endif

  #if ENABLE_SENSOR_BUTTON_1_EVENTS_CHANGE
    #define BUTTON_1_EVENT GPIO_PIN_INTR_ANYEDGE
  #elif ENABLE_SENSOR_BUTTON_1_EVENTS_LOW
    #define BUTTON_1_EVENT GPIO_PIN_INTR_NEGEDGE
  #elif ENABLE_SENSOR_BUTTON_1_EVENTS_HIGH
    #define BUTTON_1_EVENT GPIO_PIN_INTR_POSEDGE
  #else
    #error Unsupported button 1 event!
  #endif
#endif

#if ENABLE_SENSOR_BUTTON_2
  #define BUTTON_2_GPIO_PIN CONFIG_SENSOR_BUTTON_2_GPIO_PIN
  #if CONFIG_SENSOR_BUTTON_2_GPIO_PIN == 0
    #define BUTTON_2_MUX PERIPHS_IO_MUX_GPIO0_U
    #define BUTTON_2_FUNC FUNC_GPIO0
  #elif CONFIG_SENSOR_BUTTON_2_GPIO_PIN == 2
    #define BUTTON_2_MUX PERIPHS_IO_MUX_GPIO2_U
    #define BUTTON_2_FUNC FUNC_GPIO2
  #else
    #error Unsupported button GPIO pin number!
  #endif

  #if ENABLE_SENSOR_BUTTON_2_EVENTS_CHANGE
    #define BUTTON_2_EVENT GPIO_PIN_INTR_ANYEDGE
  #elif ENABLE_SENSOR_BUTTON_2_EVENTS_LOW
    #define BUTTON_2_EVENT GPIO_PIN_INTR_NEGEDGE
  #elif ENABLE_SENSOR_BUTTON_2_EVENTS_HIGH
    #define BUTTON_2_EVENT GPIO_PIN_INTR_POSEDGE
  #else
    #error Unsupported button 2 event!
  #endif
#endif

struct _button_pin {
    const uint8_t pin;
    enum button_event_id id;
};

static void _button_interrupt_handler(uint32_t intr_mask, void *arg);

static struct _button_pin _button_enabled_pins[] = {
#if ENABLE_SENSOR_BUTTON_1
    {
        .pin = BUTTON_1_GPIO_PIN,
        .id = BUTTON_ID_1,
    },
#endif
#if ENABLE_SENSOR_BUTTON_2
    {
        .pin = BUTTON_2_GPIO_PIN,
        .id = BUTTON_ID_2,
    },
#endif
};

#define _button_pins_count (sizeof(_button_enabled_pins) / sizeof(_button_enabled_pins[0]))

void button_init(void) {
    gpio_init();

#if ENABLE_SENSOR_BUTTON_1
    PIN_FUNC_SELECT(BUTTON_1_MUX, BUTTON_1_FUNC);
    gpio_pin_intr_state_set(GPIO_ID_PIN(BUTTON_1_GPIO_PIN), BUTTON_1_EVENT);
#endif

#if ENABLE_SENSOR_BUTTON_2
    PIN_FUNC_SELECT(BUTTON_2_MUX, BUTTON_2_FUNC);
    gpio_pin_intr_state_set(GPIO_ID_PIN(BUTTON_2_GPIO_PIN), BUTTON_2_EVENT);
#endif

    ETS_GPIO_INTR_ATTACH(_button_interrupt_handler, NULL);
    ETS_GPIO_INTR_ENABLE();
}


static void _button_interrupt_handler(uint32_t intr_mask, void *arg) {
    uint8_t i;
    uint32_t gpio_status;
    gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    for (i = 0; i < _button_pins_count; i++) {
        if (gpio_status & _BV(_button_enabled_pins[i].pin)) {
            sensor_button_notify(
                _button_enabled_pins[i].id,
                GPIO_INPUT_GET(GPIO_ID_PIN(_button_enabled_pins[i].pin)));
        }
    }

    /* Debouncing delay. */
    os_delay_us(60000);

    /* Clear interrupt status. */
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
}
