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

#ifndef __MQTTCLIENT_DATA_H__
#define __MQTTCLIENT_DATA_H__

#include <c_types.h>

/**
 * Initial PUBLISH item.
 */
struct mqttclient_data_init_seq_item {
    char *topic;                            /**< Service topic. */
    uint8_t *value;                         /**< Topic value. */
    uint16_t value_len;                     /**< Value length. */
    uint8_t flags;                          /**< Publish flags. */
};

/** Array of initial sequence PUBLISH messages. Terminated with NULL element. */
extern const struct mqttclient_data_init_seq_item mqttclient_data_init_seq_items[];

/** Number of initial sequence PUBLISH messages. */
extern const uint8_t mqttclient_data_init_seq_items_count;

/** Array of device sunscribe topics. */
extern char *mqttclient_data_subscribe_topics[];

/** Number of device subscribe topics. */
extern const uint8_t mqttclient_data_subscribe_topics_count;

#endif
