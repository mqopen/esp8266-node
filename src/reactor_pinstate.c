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
#include "datautils.h"
#include "reactor.h"
#include "reactor_pinstate.h"

/* Comparison criteria. */
#if ENABLE_REACTOR_PINSTATE_INPUT_NUMERIC
  #if ENABLE_REACTOR_PINSTATE_CRITERIA_LT
    #define REACTOR_PINSTATE_CMP_OP     <
  #elif ENABLE_REACTOR_PINSTATE_CRITERIA_GT
    #define REACTOR_PINSTATE_CMP_OP     >
  #elif ENABLE_REACTOR_PINSTATE_CRITERIA_EQ
    #define REACTOR_PINSTATE_CMP_OP     ==
  #else
    #error Unsupported comparison criteria!
  #endif
#endif

/* Check for number of decimal digits. */
#if CONFIG_REACTOR_PINSTATE_INPUT_DECIMAL < 0
  #error Negative decimal configuration is not allowed!
#endif

__reactor_subscribe_topics(
    CONFIG_REACTOR_PINSTATE_TOPIC
);

/** Respond value buffer */
#if ENABLE_REACTOR_PINSTATE_RESPOND
static char _reactor_pinstate_respond_vaue_str[max(
    __sizeof_str(CONFIG_REACTOR_PINSTATE_RESPOND_MESSAGE_ENABLE),
    __sizeof_str(CONFIG_REACTOR_PINSTATE_RESPOND_MESSAGE_DISABLE))];
#endif

/** Respond MQTT topics. */
static struct reactor_str _reactor_pinstate_respond_topics[] = {
#if ENABLE_REACTOR_PINSTATE_RESPOND
    {
        .data = TOPIC(CONFIG_REACTOR_PINSTATE_RESPOND_TOPIC),
        .len = __sizeof_str(TOPIC(CONFIG_REACTOR_PINSTATE_RESPOND_TOPIC)),
    },
#endif
};

/** Respond values. */
static struct reactor_str _reactor_pinstate_respond_data[] = {
#if ENABLE_REACTOR_PINSTATE_RESPOND
    {
        .data = _reactor_pinstate_respond_vaue_str,
        .len = 0,
    },
#endif
};

/** Respond topic flags. */
static uint8_t _reactor_pinstate_respond_flags[] = {
#if ENABLE_REACTOR_PINSTATE_RESPOND
    _BV(UMQTT_OPT_RETAIN),
#endif
};

/** Respond dirty flag. */
#if ENABLE_REACTOR_PINSTATE_RESPOND
static uint8_t _reactor_pinstate_respond_dirty_flags = 0;
#endif

/** Count of MQTT respond topics.  */
#if ENABLE_REACTOR_PINSTATE_RESPOND
const uint8_t reactor_respond_topics_count = 1;
#else
const uint8_t reactor_respond_topics_count = 0;
#endif

#if ENABLE_REACTOR_PINSTATE_INPUT_KEYWORD
/**
 * Compare data with a keyword.
 */
static bool _reactor_pinstate_cmp_keyword(const char *keyword, uint16_t keyword_len, const uint8_t *data, uint16_t data_len);
#endif

void reactor_init(void) {
#if ENABLE_REACTOR_PINSTATE_RESPOND
    bool _enabled;
    uint8_t _len;
#endif
    pinstate_init();
#if ENABLE_REACTOR_PINSTATE_RESPOND
    _enabled = pinstate_get();
    if (_enabled) {
        _len = os_sprintf(
            _reactor_pinstate_respond_data[0].data,
            CONFIG_REACTOR_PINSTATE_RESPOND_MESSAGE_ENABLE);
    } else {
        _len = os_sprintf(
            _reactor_pinstate_respond_data[0].data,
            CONFIG_REACTOR_PINSTATE_RESPOND_MESSAGE_DISABLE);
    }
    _reactor_pinstate_respond_data[0].len = _len;
#endif
}

void reactor_on_data(const char *topic, const uint8_t *data, uint16_t data_len) {
    bool _enabled = false;
#if ENABLE_REACTOR_PINSTATE_RESPOND
    bool _pinstate_changed = false;
    uint8_t _len;
#endif

/* Numeric input implementation. */
#if ENABLE_REACTOR_PINSTATE_INPUT_NUMERIC
    int32_t value;
    if (!datautils_data_to_int32(&value, data, data_len, CONFIG_REACTOR_PINSTATE_INPUT_DECIMAL)) {
        _enabled = value REACTOR_PINSTATE_CMP_OP CONFIG_REACTOR_PINSTATE_TRESHOLD ? true : false;
        if (_enabled != pinstate_get()) {
            pinstate_set(_enabled);
  #if ENABLE_REACTOR_PINSTATE_RESPOND
            _pinstate_changed = true;
  #endif
        }
    }

/* Keyword input implementation. */
#elif ENABLE_REACTOR_PINSTATE_INPUT_KEYWORD
    if (_reactor_pinstate_cmp_keyword(
            CONFIG_REACTOR_PINSTATE_KEYWORD_ENABLE,
            __sizeof_str(CONFIG_REACTOR_PINSTATE_KEYWORD_ENABLE),
            data,
            data_len)) {
        _enabled = true;
    } else if (_reactor_pinstate_cmp_keyword(
            CONFIG_REACTOR_PINSTATE_KEYWORD_DISABLE,
            __sizeof_str(CONFIG_REACTOR_PINSTATE_KEYWORD_DISABLE),
            data,
            data_len)) {
        _enabled = false;
    } else {
        /* Unknown keyword. Do nothing. */
        return;
    }

    /* Update state. */
    if (_enabled != pinstate_get()) {
        pinstate_set(_enabled);
  #if ENABLE_REACTOR_PINSTATE_RESPOND
        _pinstate_changed = true;
  #endif
    }
#else
  #error Unsupported pinstate input!
#endif

#if ENABLE_REACTOR_PINSTATE_RESPOND
    if (_pinstate_changed) {
        if (_enabled) {
            _len = os_sprintf(
                _reactor_pinstate_respond_data[0].data,
                CONFIG_REACTOR_PINSTATE_RESPOND_MESSAGE_ENABLE);
        } else {
            _len = os_sprintf(
                _reactor_pinstate_respond_data[0].data,
                CONFIG_REACTOR_PINSTATE_RESPOND_MESSAGE_DISABLE);
        }
        _reactor_pinstate_respond_data[0].len = _len;
        _reactor_pinstate_respond_dirty_flags = 1;
    }
#endif
}

bool reactor_respond_is_updated(uint8_t index) {
#if ENABLE_REACTOR_PINSTATE_RESPOND
    return _reactor_pinstate_respond_dirty_flags & _BV(index);
#else
    return false;
#endif
}

__reactor_respond_get_topic(_reactor_pinstate_respond_topics);
__reactor_respond_get_value(_reactor_pinstate_respond_data);
__reactor_respond_get_flags(_reactor_pinstate_respond_flags);

void reactor_respond_commit(void) {
#if ENABLE_REACTOR_PINSTATE_RESPOND
    _reactor_pinstate_respond_dirty_flags = 0;
#endif
}

#if ENABLE_REACTOR_PINSTATE_INPUT_KEYWORD
static bool _reactor_pinstate_cmp_keyword(const char *keyword, uint16_t keyword_len, const uint8_t *data, uint16_t data_len) {
    if (data_len == keyword_len && os_memcmp(keyword, data, data_len) == 0) {
        return true;
    } else {
        return false;
    }
}
#endif
