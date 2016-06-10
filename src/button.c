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
#include <user_interface.h>
#include "common.h"
#include "sensor.h"
#include "sensor_button.h"
#include "button.h"

/**
 * Buttin event types.
 */
enum _button_event {
    _BUTTON_HIGH,               /**< Rising edge event. */
    _BUTTON_LOW,                /**< Falling edge event. */
    _BUTTON_CHANGE,             /**< Logic change event. */
};

/**
 * Internal data about button pin.
 */
struct _button_pin {
    const uint8_t pin;          /**< GPIO pin number. */
    enum button_event_id id;    /**< Assigned button ID. */
    uint8_t last_state;         /**< Last pin state. */
    uint8_t debounce_counter;   /**< Debounce counter value. */
    enum _button_event event;   /**< Button event configuration. */
};

#if ENABLE_SENSOR_BUTTON_1
  #define BUTTON_1_GPIO_PIN CONFIG_SENSOR_BUTTON_1_GPIO_PIN
  #if CONFIG_SENSOR_BUTTON_1_GPIO_PIN == 0
    #define BUTTON_1_MUX PERIPHS_IO_MUX_GPIO0_U
    #define BUTTON_1_FUNC FUNC_GPIO0
  #elif CONFIG_SENSOR_BUTTON_1_GPIO_PIN == 2
    #define BUTTON_1_MUX PERIPHS_IO_MUX_GPIO2_U
    #define BUTTON_1_FUNC FUNC_GPIO2
  #elif CONFIG_SENSOR_BUTTON_1_GPIO_PIN == 13
    #define BUTTON_1_MUX PERIPHS_IO_MUX_MTCK_U
    #define BUTTON_1_FUNC FUNC_GPIO13
  #elif CONFIG_SENSOR_BUTTON_1_GPIO_PIN == 14
    #define BUTTON_1_MUX PERIPHS_IO_MUX_MTMS_U
    #define BUTTON_1_FUNC FUNC_GPIO14
  #else
    #error Unsupported button 1 GPIO pin number!
  #endif

  #if ENABLE_SENSOR_BUTTON_1_EVENTS_CHANGE
    #define BUTTON_1_EVENT _BUTTON_CHANGE
  #elif ENABLE_SENSOR_BUTTON_1_EVENTS_LOW
    #define BUTTON_1_EVENT _BUTTON_LOW
  #elif ENABLE_SENSOR_BUTTON_1_EVENTS_HIGH
    #define BUTTON_1_EVENT _BUTTON_HIGH
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
  #elif CONFIG_SENSOR_BUTTON_2_GPIO_PIN == 13
    #define BUTTON_2_MUX PERIPHS_IO_MUX_MTCK_U
    #define BUTTON_2_FUNC FUNC_GPIO13
  #elif CONFIG_SENSOR_BUTTON_2_GPIO_PIN == 14
    #define BUTTON_2_MUX PERIPHS_IO_MUX_MTMS_U
    #define BUTTON_2_FUNC FUNC_GPIO14
  #else
    #error Unsupported button 2 GPIO pin number!
  #endif

  #if ENABLE_SENSOR_BUTTON_2_EVENTS_CHANGE
    #define BUTTON_2_EVENT _BUTTON_CHANGE
  #elif ENABLE_SENSOR_BUTTON_2_EVENTS_LOW
    #define BUTTON_2_EVENT _BUTTON_LOW
  #elif ENABLE_SENSOR_BUTTON_2_EVENTS_HIGH
    #define BUTTON_2_EVENT _BUTTON_HIGH
  #else
    #error Unsupported button 2 event!
  #endif
#endif

/** Array of enabled button pins.  */
static struct _button_pin _button_enabled_pins[] = {
#if ENABLE_SENSOR_BUTTON_1
    {
        .pin = BUTTON_1_GPIO_PIN,
        .id = BUTTON_ID_1,
        .event = BUTTON_1_EVENT,
    },
#endif
#if ENABLE_SENSOR_BUTTON_2
    {
        .pin = BUTTON_2_GPIO_PIN,
        .id = BUTTON_ID_2,
        .event = BUTTON_2_EVENT,
    },
#endif
};

/** Helper marco to get enable button pins count. */
#define _button_pins_count (sizeof(_button_enabled_pins) / sizeof(_button_enabled_pins[0]))

/** Number of button reads to ensure that its level stabilizes. */
#define _button_debounce_check_count 4

/** Check period every 10 ms */
#define _button_debounce_check_period 10

/** Button debounce timer structure. */
static os_timer_t _button_debounce_timer;

/** Mask of unresolved button events. */
static uint32_t _button_gpio_status = 0;

/**
 * Button event interrupt handler.
 *
 * @param intr_mask Internal interrupt mask.
 * @param arg User defined argument. Not used.
 */
static void _button_interrupt_handler(uint32_t intr_mask, void *arg);

/**
 * Initialize button state. Called when initializing a hardware.
 */
static inline void _button_init_state(void);

/**
 * Debouncing timer callback.
 */
static void _button_debounce_callback(void);

/**
 * Trigger an even, if configured.
 */
static void _button_process_event(uint8_t index, uint8_t state);

void button_init(void) {
    os_timer_disarm(&_button_debounce_timer);
    os_timer_setfn(&_button_debounce_timer, (os_timer_func_t *) _button_debounce_callback, NULL);

    gpio_init();

#if ENABLE_SENSOR_BUTTON_1
    PIN_FUNC_SELECT(BUTTON_1_MUX, BUTTON_1_FUNC);
#if ENABLE_SENSOR_BUTTON_1_PULLUP
    PIN_PULLUP_EN(BUTTON_1_MUX);
#endif
    gpio_pin_intr_state_set(GPIO_ID_PIN(BUTTON_1_GPIO_PIN), GPIO_PIN_INTR_ANYEDGE);
#endif

#if ENABLE_SENSOR_BUTTON_2
    PIN_FUNC_SELECT(BUTTON_2_MUX, BUTTON_2_FUNC);
#if ENABLE_SENSOR_BUTTON_2_PULLUP
    PIN_PULLUP_EN(BUTTON_2_MUX);
#endif
    gpio_pin_intr_state_set(GPIO_ID_PIN(BUTTON_2_GPIO_PIN), GPIO_PIN_INTR_ANYEDGE);
#endif

    _button_init_state();

    ETS_GPIO_INTR_ATTACH(_button_interrupt_handler, NULL);
    //ETS_GPIO_INTR_ENABLE();
}

uint8_t button_is_active(enum button_event_id id) {
    uint8_t i;
    uint8_t _state;

    /* Get button index. */
    for (i = 0; i < _button_pins_count; i++) {
        if (_button_enabled_pins[i].id == id)
            break;
    }

    /* Check if button is active. */
    _state = _button_enabled_pins[i].last_state;
    switch (_button_enabled_pins[i].event) {
        case _BUTTON_HIGH:
            if (_state) {
                return 1;
            } else {
                break;
            }
        case _BUTTON_LOW:
            if (!_state) {
                return 1;
            } else {
                break;
            }
        case _BUTTON_CHANGE:
            return 1;
    }
    return 0;
}

inline void button_notify_lock(void) {
    ETS_GPIO_INTR_DISABLE();
}

inline void button_notify_release(void) {
    ETS_GPIO_INTR_ENABLE();
}

static inline void _button_init_state(void) {
    uint8_t i;
    uint8_t _state;
    for (i = 0; i < _button_pins_count; i++) {
        _state = GPIO_INPUT_GET(GPIO_ID_PIN(_button_enabled_pins[i].pin));
        _button_enabled_pins[i].last_state = _state;
        _button_enabled_pins[i].debounce_counter = 0;
        _button_process_event(i, _state);
    }
}

static void _button_interrupt_handler(uint32_t intr_mask, void *arg) {
    uint32_t _gpio_status;
    _gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    _button_gpio_status |= _gpio_status;

    /* Disable GPIO interrupts and wait for debouncing callback to enable it. */
    ETS_GPIO_INTR_DISABLE();

    os_timer_arm(&_button_debounce_timer, _button_debounce_check_period, 0);

    /* Clear interrupt status. */
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, _gpio_status);
}

static void _button_debounce_callback(void) {
    uint8_t i;
    uint8_t _pin_state;
    for (i = 0; i < _button_pins_count; i++) {

        /* Check is pin has pending unresolved event. */
        if (_button_gpio_status & _BV(_button_enabled_pins[i].pin)) {

            /* Get current pin state. */
            _pin_state = GPIO_INPUT_GET(GPIO_ID_PIN(_button_enabled_pins[i].pin));

            /* Check if current pin state is same as last pin state. */
            if (_pin_state != _button_enabled_pins[i].last_state) {

                /* Pin state changed. Increment deboucing counter. */
                _button_enabled_pins[i].debounce_counter++;

                /* Check if deboucing completes. */
                if (_button_enabled_pins[i].debounce_counter == _button_debounce_check_count) {

                    /* Debouncing is complete. Generate an event. */
                    _button_process_event(i, _pin_state);

                    /*  Reset tracking variables */
                    _button_gpio_status &= ~(_BV(_button_enabled_pins[i].pin));
                    _button_enabled_pins[i].last_state = _pin_state;
                    _button_enabled_pins[i].debounce_counter = 0;
                } else {

                    /* Debouncing is not complete. Shedule next check. */
                    os_timer_arm(&_button_debounce_timer, _button_debounce_check_period, 0);
                }
            } else {

                /* Pin has same state as previous debouced one. Reset counter and try again. */
                _button_enabled_pins[i].debounce_counter = 0;
                os_timer_arm(&_button_debounce_timer, _button_debounce_check_period, 0);
            }
        }
    }
    ETS_GPIO_INTR_ENABLE();
}

static void _button_process_event(uint8_t index, uint8_t state) {
    switch (_button_enabled_pins[index].event) {
        case _BUTTON_HIGH:
            if (!state) {
                return;
            } else {
                break;
            }
        case _BUTTON_LOW:
            if (state) {
                return;
            } else {
                break;
            }
        case _BUTTON_CHANGE:
            break;
    }
    sensor_button_notify(_button_enabled_pins[index].id, state);
}
