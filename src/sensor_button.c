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
#include "common.h"
#include "sensor.h"
#include "sensor_button.h"

/* Check that at least one button is enabled. */
#if ! ENABLE_SENSOR_BUTTON_1 && ! ENABLE_SENSOR_BUTTON_2
  #error At least one button must be enabled!
#endif

/** Button 1 value buffer. */
#if ENABLE_SENSOR_BUTTON_1 && ENABLE_SENSOR_BUTTON_1_EVENTS_CHANGE
static char _sensor_button_1_data_str[max(
        __sizeof_str(CONFIG_SENSOR_BUTTON_1_EVENTS_LOW_MESSAGE),
        __sizeof_str(CONFIG_SENSOR_BUTTON_1_EVENTS_HIGH_MESSAGE))];
#endif

/** Button 2 value buffer. */
#if ENABLE_SENSOR_BUTTON_2 && ENABLE_SENSOR_BUTTON_2_EVENTS_CHANGE
static char _sensor_button_2_data_str[max(
        __sizeof_str(CONFIG_SENSOR_BUTTON_2_EVENTS_LOW_MESSAGE),
        __sizeof_str(CONFIG_SENSOR_BUTTON_2_EVENTS_HIGH_MESSAGE))];
#endif

/**
 * Topics.
 */
static struct sensor_str _sensor_button_topics[] = {
#if ENABLE_SENSOR_BUTTON_1
    {
        .data = TOPIC(CONFIG_SENSOR_BUTTON_1_TOPIC),
        .len = __sizeof_str(CONFIG_SENSOR_BUTTON_1_TOPIC),
    },
#endif
#if ENABLE_SENSOR_BUTTON_2
    {
        .data = TOPIC(CONFIG_SENSOR_BUTTON_2_TOPIC),
        .len = __sizeof_str(CONFIG_SENSOR_BUTTON_2_TOPIC),
    },
#endif
};

/**
 * Values.
 */
static struct sensor_str _sensor_button_data[] = {
#if ENABLE_SENSOR_BUTTON_1
    {
  #if ENABLE_SENSOR_BUTTON_1_EVENTS_LOW
    .data = CONFIG_SENSOR_BUTTON_1_EVENTS_LOW_MESSAGE,
    .len = __sizeof_str(CONFIG_SENSOR_BUTTON_1_EVENTS_LOW_MESSAGE),
  #elif ENABLE_SENSOR_BUTTON_1_EVENTS_HIGH
    .data = CONFIG_SENSOR_BUTTON_1_EVENTS_HIGH_MESSAGE,
    .len = __sizeof_str(CONFIG_SENSOR_BUTTON_1_EVENTS_HIGH_MESSAGE),
  #elif ENABLE_SENSOR_BUTTON_1_EVENTS_CHANGE
    .data = _sensor_button_1_data_str,
    .len = 0,
  #else
    #error Unsupported button 1 events config!
  #endif
    },
#endif
#if ENABLE_SENSOR_BUTTON_2
    {
  #if ENABLE_SENSOR_BUTTON_2_EVENTS_LOW
    .data = CONFIG_SENSOR_BUTTON_2_EVENTS_LOW_MESSAGE,
    .len = __sizeof_str(CONFIG_SENSOR_BUTTON_2_EVENTS_LOW_MESSAGE),
  #elif ENABLE_SENSOR_BUTTON_2_EVENTS_HIGH
    .data = CONFIG_SENSOR_BUTTON_2_EVENTS_HIGH_MESSAGE,
    .len = __sizeof_str(CONFIG_SENSOR_BUTTON_2_EVENTS_HIGH_MESSAGE),
  #elif ENABLE_SENSOR_BUTTON_2_EVENTS_CHANGE
    .data = _sensor_button_2_data_str,
    .len = 0,
  #else
    #error Unsupported button 2 events config!
  #endif
    },
#endif
};

static sensor_notify_callback_t _sensor_notify_callback = NULL;

static enum button_event_id _sensor_button_index[] = {
#if ENABLE_SENSOR_BUTTON_1
    BUTTON_ID_1,
#endif
#if ENABLE_SENSOR_BUTTON_2
    BUTTON_ID_2,
#endif
};

const uint8_t sensor_topics_count = sizeof(_sensor_button_topics) / sizeof(_sensor_button_topics[0]);

uint8_t sensor_get_initial_value(uint8_t index, char **buf, uint8_t *buf_len) {
    *buf = _sensor_button_data[index].data;
    *buf_len = _sensor_button_data[index].len;
    return button_is_active(_sensor_button_index[index]);
}

void sensor_register_notify_callback(sensor_notify_callback_t callback) {
    _sensor_notify_callback = callback;
}

void sensor_notify_lock(void) {
}

void sensor_notify_release(void) {
}

void sensor_button_notify(enum button_event_id id, uint8_t state) {
    uint8_t i = 0;

/* This code will be used only if at least one button is configured as on change. */
#if (ENABLE_SENSOR_BUTTON_1 && ENABLE_SENSOR_BUTTON_1_EVENTS_CHANGE) || \
    (ENABLE_SENSOR_BUTTON_2 && ENABLE_SENSOR_BUTTON_2_EVENTS_CHANGE)
    uint8_t _len = 0;
#endif

#if ENABLE_SENSOR_BUTTON_1 && ENABLE_SENSOR_BUTTON_2
    /* Get data and topic index. */
    for (i = 0; i < (sizeof(_sensor_button_index) / sizeof(_sensor_button_index[0])); i++) {
        if (_sensor_button_index[i] == id) break;
    }
#endif

    switch (id) {
#if ENABLE_SENSOR_BUTTON_1
        case BUTTON_ID_1:

  /* MQTT topic changes only when button is configured as on change. */
  #if ENABLE_SENSOR_BUTTON_1_EVENTS_CHANGE
            if (state) {

                /* High state message. */
                os_memcpy(
                    _sensor_button_data[i].data,
                    CONFIG_SENSOR_BUTTON_1_EVENTS_HIGH_MESSAGE,
                    __sizeof_str(CONFIG_SENSOR_BUTTON_1_EVENTS_HIGH_MESSAGE));
                _len = __sizeof_str(CONFIG_SENSOR_BUTTON_1_EVENTS_HIGH_MESSAGE);
            } else {

                /* Low state message. */
                os_memcpy(
                    _sensor_button_data[i].data,
                    CONFIG_SENSOR_BUTTON_1_EVENTS_LOW_MESSAGE,
                    __sizeof_str(CONFIG_SENSOR_BUTTON_1_EVENTS_LOW_MESSAGE));
                _len = __sizeof_str(CONFIG_SENSOR_BUTTON_1_EVENTS_LOW_MESSAGE);
            }
            _sensor_button_data[i].len = _len;
  #endif
            break;
#endif
#if ENABLE_SENSOR_BUTTON_2
        case BUTTON_ID_2:

  /* MQTT topic changes only when button is configured as on change. */
  #if ENABLE_SENSOR_BUTTON_2_EVENTS_CHANGE
            if (state) {

                /* High state message. */
                os_memcpy(
                    _sensor_button_data[i].data,
                    CONFIG_SENSOR_BUTTON_2_EVENTS_HIGH_MESSAGE,
                    __sizeof_str(CONFIG_SENSOR_BUTTON_2_EVENTS_HIGH_MESSAGE));
                _len = __sizeof_str(CONFIG_SENSOR_BUTTON_2_EVENTS_HIGH_MESSAGE);
            } else {

                /* Low state message. */
                os_memcpy(
                    _sensor_button_data[i].data,
                    CONFIG_SENSOR_BUTTON_2_EVENTS_LOW_MESSAGE,
                    __sizeof_str(CONFIG_SENSOR_BUTTON_2_EVENTS_LOW_MESSAGE));
                _len = __sizeof_str(CONFIG_SENSOR_BUTTON_2_EVENTS_LOW_MESSAGE);
            }
            _sensor_button_data[i].len = _len;
  #endif
            break;
#endif
        default:
            /* unknown button event. Do nothing. */
            return;
    }


    if (_sensor_notify_callback != NULL) {
        _sensor_notify_callback(i);
    }
}

__sensor_get_topic_array(_sensor_button_topics)
__sensor_get_value_array(_sensor_button_data)
