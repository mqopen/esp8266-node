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

#ifndef __REACTOR_H__
#define __REACTOR_H__

#include <c_types.h>
#include "common.h"

#if ENABLE_REACTOR_PINSTATE
  #include "reactor_pinstate.h"
#else
  #error Unsupported reactor type!
#endif

/** Reactor string structure. */
struct reactor_str {
    char *data;         /**< String. */
    uint8_t len;        /**< String length. */
};

/** Array of subsribe topic strings. */
extern const char *reactor_subscribe_topics[];

/** Number of reactor subsribe topics. */
extern const uint8_t reactor_subscribe_topics_count;

/** Number of MQTT respond topics */
extern const uint8_t reactor_respond_topics_count;

/**
 * Initialize reactor hardware.
 */
extern void reactor_init(void);

/**
 * Notify reactor about MQTT data
 */
extern void reactor_on_data(const char *topic, const uint8_t *data, uint16_t data_len);

/**
 * Check if respond topic is updated.
 *
 * @param index Respond topic index.
 * @return True if topic is updated snd should be sent to network, False otherwise.
 */
extern bool reactor_respond_is_updated(uint8_t index);

/**
 * Get address of reactor MQTT respond topic indentified by it's index.
 *
 * @param index Index of MQTT respond topic.
 * @param buf_len Pointer to variable where to store string length.
 * @return Address of topic string.
 */
extern char *reactor_respond_get_topic(uint8_t index, uint8_t *buf_len);

/**
 * Get address of reactor respond value indentified by it's index.
 *
 * @param index Index of respond value.
 * @param buf_len Pointer to variable where to store string length.
 * @return Address of value string.
 */
extern char *reactor_respond_get_value(uint8_t index, uint8_t *buf_len);

/**
 * Get reactor respond topic MQTT flags indentified by it's index.
 *
 * @param index Respond topic index.
 * @return MQTT flags.
 */
extern uint8_t reactor_respond_get_flags(uint8_t index);

/**
 * Notify reactor that all respond topcis has been successfuly sent.
 */
extern void reactor_respond_commit(void);

#define __reactor_subscribe_topics_count() \
    const uint8_t reactor_subscribe_topics_count = (sizeof(reactor_subscribe_topics) / sizeof(reactor_subscribe_topics[0]));

/**
 * Helper macro to generate reactor_subscribe_topics array and
 * reactor_subscribe_topics_count variable.
 */
#define __reactor_subscribe_topics(...) \
    const char *reactor_subscribe_topics[] = { \
        __VA_ARGS__ \
    }; \
    __reactor_subscribe_topics_count()

#define __reactor_respond_get_topic(__topic_array) \
    char *reactor_respond_get_topic(uint8_t index, uint8_t *buf_len) { \
        *buf_len = __topic_array[index].len; \
        return __topic_array[index].data; \
    }

#define __reactor_respond_get_value(__value_array) \
    char *reactor_respond_get_value(uint8_t index, uint8_t *buf_len) { \
        *buf_len = __value_array[index].len; \
        return __value_array[index].data; \
    }

#define __reactor_respond_get_flags(__flags_array) \
    uint8_t reactor_respond_get_flags(uint8_t index) { \
        return __flags_array[index]; \
    }

#endif
