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
#include "common.h"
#include "version.h"
#include "umqtt.h"
#if ENABLE_DEVICE_CLASS_SENSOR
  #include "sensor.h"
#endif
#if ENABLE_DEVICE_CLASS_REACTOR
  #include "reactor.h"
#endif
#include "mqttclient_data.h"

const struct mqttclient_data_init_seq_item mqttclient_data_init_seq_items[] = {
    {
        .topic = __topic_presence,
        .value = (uint8_t *) CONFIG_MQTT_PRESENCE_ONLINE,
        .value_len = __sizeof_str(CONFIG_MQTT_PRESENCE_ONLINE),
        .flags = _BV(UMQTT_OPT_RETAIN),
    },
    {
        .topic = __topic_fwversion,
        .value = (uint8_t *) VERSION,
        .value_len = __sizeof_str(VERSION),
        .flags = _BV(UMQTT_OPT_RETAIN),
    },
    {
        .topic = __topic_hwversion,
        .value = (uint8_t *) CONFIG_GENERAL_HW_VERSION,
        .value_len = __sizeof_str(CONFIG_GENERAL_HW_VERSION),
        .flags = _BV(UMQTT_OPT_RETAIN),
    },
    {
        .topic = __topic_arch,
        .value = (uint8_t *) "esp",
        .value_len = __sizeof_str("esp"),
        .flags = _BV(UMQTT_OPT_RETAIN),
    },
    {
        .topic = __topic_variant,
        .value = (uint8_t *) "esp8266",
        .value_len = __sizeof_str("esp8266"),
        .flags = _BV(UMQTT_OPT_RETAIN),
    },
    {
        .topic = __topic_link,
        .value = (uint8_t *) "wifi",
        .value_len = __sizeof_str("wifi"),
        .flags = _BV(UMQTT_OPT_RETAIN),
    },
    {
        .topic = __topic_ip,
        .value = (uint8_t *) CONFIG_NETWORK_IP_ADDRESS,
        .value_len = __sizeof_str(CONFIG_NETWORK_IP_ADDRESS),
        .flags = _BV(UMQTT_OPT_RETAIN),
    },
    {
        .topic = __topic_class,
#if ENABLE_DEVICE_CLASS_SENSOR
        .value = (uint8_t *) "sensor",
        .value_len = __sizeof_str("sensor"),
#elif ENABLE_DEVICE_CLASS_REACTOR
        .value = (uint8_t *) "reactor",
        .value_len = __sizeof_str("reactor"),
#else
  #error Unsupported sensor class!
#endif
        .flags = _BV(UMQTT_OPT_RETAIN),
    },

#if ENABLE_DEVICE_CLASS_SENSOR
    {
        .topic = __topic_sensor,
        .value = (uint8_t *) SENSOR_NAME,
        .value_len = __sizeof_str(SENSOR_NAME),
        .flags = _BV(UMQTT_OPT_RETAIN),
    },
#endif

#if ENABLE_DEVICE_CLASS_REACTOR
    {
        .topic = __topic_reactor,
        .value = (uint8_t *) REACTOR_NAME,
        .value_len = __sizeof_str(REACTOR_NAME),
        .flags = _BV(UMQTT_OPT_RETAIN),
    },
#endif

#if ENABLE_DEVICE_CLASS_SENSOR
    {
        .topic = __topic_sync,
  #if SENSOR_TYPE_SYNCHRONOUS
        .value = (uint8_t *) "sync",
        .value_len = __sizeof_str("sync"),
  #else
        .value = (uint8_t *) "async",
        .value_len = __sizeof_str("async"),
  #endif
        .flags = _BV(UMQTT_OPT_RETAIN),
    },
#endif

    /* NULL element. */
    {
        .topic = NULL,
        .value = NULL,
        .value_len = 0,
        .flags = 0,
    },
};

#define __mqttclient_data_init_seq_items_count \
    ((sizeof(mqttclient_data_init_seq_items) / sizeof(mqttclient_data_init_seq_items[0])) - 1)

const uint8_t mqttclient_data_init_seq_items_count = __mqttclient_data_init_seq_items_count;
