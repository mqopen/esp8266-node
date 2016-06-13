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

#if ENABLE_REACTOR_PINSTATE_INPUT_KEYWORD
/**
 * Compare data with a keyword.
 */
static bool _reactor_pinstate_cmp_keyword(const char *keyword, uint16_t keyword_len, const uint8_t *data, uint16_t data_len);
#endif

void reactor_on_data(const char *topic, const uint8_t *data, uint16_t data_len) {

/* Numeric input implementation. */
#if ENABLE_REACTOR_PINSTATE_INPUT_NUMERIC
    int32_t value;
    bool _is_criteria_met;
    if (!datautils_data_to_int32(&value, data, data_len, CONFIG_REACTOR_PINSTATE_INPUT_DECIMAL)) {
        _is_criteria_met = value REACTOR_PINSTATE_CMP_OP CONFIG_REACTOR_PINSTATE_TRESHOLD ? true : false;
        if (_is_criteria_met != pinstate_get()) {
            pinstate_set(_is_criteria_met);
        }
    }

/* Keyword input implementation. */
#elif ENABLE_REACTOR_PINSTATE_INPUT_KEYWORD
    bool _state = false;
    if (_reactor_pinstate_cmp_keyword(
            CONFIG_REACTOR_PINSTATE_KEYWORD_ENABLE,
            __sizeof_str(CONFIG_REACTOR_PINSTATE_KEYWORD_ENABLE),
            data,
            data_len)) {
        _state = true;
    } else if (_reactor_pinstate_cmp_keyword(
            CONFIG_REACTOR_PINSTATE_KEYWORD_DISABLE,
            __sizeof_str(CONFIG_REACTOR_PINSTATE_KEYWORD_DISABLE),
            data,
            data_len)) {
        _state = false;
    } else {
        /* Unknown keyword. Do nothing. */
        return;
    }

    /* Update state. */
    if (_state != pinstate_get()) {
        pinstate_set(_state);
    }
#else
  #error Unsupported pinstate input!
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
