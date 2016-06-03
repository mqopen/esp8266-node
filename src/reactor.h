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

extern char *reactor_subscribe_topics[];

extern uint8_t reactor_subscribe_topics_count;

//#define reactor_subscribe_topics_count ((sizeof(reactor_subscribe_topics) / sizeof(reactor_subscribe_topics[0])) - 1)

/**
 * Initialize reactor hardware.
 */
extern void reactor_init(void);

/**
 * Notify reactor about MQTT data
 */
extern void reactor_on_data(char *topic, uint8_t *data, uint16_t data_len);

#define __reactor_subscribe_topics_count() \
    uint8_t reactor_subscribe_topics_count = (sizeof(reactor_subscribe_topics) / sizeof(reactor_subscribe_topics[0]));

#define __reactor_subscribe_topics(...) \
    char *reactor_subscribe_topics[] = { \
        __VA_ARGS__ \
    }; \
    __reactor_subscribe_topics_count()

#endif
