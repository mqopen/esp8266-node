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

#include <ets_sys.h>
#include <os_type.h>
#include <ip_addr.h>
#include <espconn.h>
#include <user_interface.h>
#include <osapi.h>
#include "user_config.h"
#include "umqtt.h"
#include "commsig.h"
#if ENABLE_DEVICE_CLASS_SENSOR
  #include "sensor.h"
#endif
#if ENABLE_DEVICE_CLASS_REACTOR
  #include "reactor.h"
#endif
#include "version.h"
#include "common.h"
#include "mqttclient.h"

/** Timer for ending MQTT Keep Alive messages. */
static os_timer_t _mqttclient_keep_alive_timer;

#if ENABLE_DEVICE_CLASS_SENSOR
  #if SENSOR_TYPE_SYNCHRONOUS
/** Timer for sending DHT measurements. */
static os_timer_t  _mqttclient_publish_timer;
  #endif
#endif

/** Timer for limit reconnect attempts. */
static os_timer_t _mqttclient_reconnect_timer;

/** Keep track if MQTT client is running. */
static bool _mqttclient_is_running = false;

/** TCP connection. */
static struct _esp_tcp _mqttclient_tcp;

/** Network connection. */
static struct espconn _mqttclient_espconn = {
    .proto.tcp = &_mqttclient_tcp,
};

/**
 * Current link state
 */
//static enum mqttclient_state _mqttclient_state = MQTTCLIENT_BROKER_DISCONNECTED;

/**
 * Current communication stete
 */
static enum mqttclient_comm_state _mqttclient_comm_state = MQTTCLIENT_COMM_INIT;

static uint8_t _mqttclient_tx_buffer[200];
static uint8_t _mqttclient_rx_buffer[200];
static uint8_t _rx_buf[200];

/**
 * Handle PUBLISH message from broker.
 */
static void ICACHE_FLASH_ATTR _mqttclient_on_publish(struct umqtt_connection *_m, char *topic, uint8_t *data, uint16_t len);

/** MQTT connection structure instance. */
static struct umqtt_connection _mqttclient_mqtt = {
    .txbuff = {
        .start = _mqttclient_tx_buffer,
        .length = sizeof(_mqttclient_tx_buffer),
    },
    .rxbuff = {
        .start = _mqttclient_rx_buffer,
        .length = sizeof(_mqttclient_rx_buffer),
    },
    .state = UMQTT_STATE_INIT,
    .message_callback = _mqttclient_on_publish,
};

/** MQTT connection configuration. */
static struct umqtt_connect_config _connection_config = {
    .keep_alive = CONFIG_MQTT_KEEPALIVE_CONNECT_INTERVAL,
    .client_id = CONFIG_GENERAL_DEVICE_NAME,
    .will_topic = __topic_presence,
    .will_message = (uint8_t *) CONFIG_MQTT_PRESENCE_OFFLINE,
    .will_message_len = __sizeof_str(CONFIG_MQTT_PRESENCE_OFFLINE),
    .flags = _BV(UMQTT_OPT_RETAIN),
};

/** Array of initial sequence PUBLISH messages. Terminated with NULL element. */
static struct mqttclient_init_seq_item _mqttclient_init_seq_items[] = {
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

    /* NULL element. */
    {
        .topic = NULL,
        .value = NULL,
        .value_len = 0,
        .flags = 0,
    },
};

#define __mqttclient_init_seq_items_count ((sizeof(_mqttclient_init_seq_items) / sizeof(_mqttclient_init_seq_items[0])) - 1)

/** Index to current element of _mqttclient_init_seq_items array. */
static uint8_t _mqttclient_init_seq_items_index = 0;

/** Array of subscribe topics. Last element if NULL poiner. */
#if ENABLE_DEVICE_CLASS_REACTOR
  #define _mqttclient_subscribe_topics reactor_subscribe_topics
  #define __mqttclient_subscribe_topics_count reactor_subscribe_topics_count
#else
static char *_mqttclient_subscribe_topics[] = {
    NULL,
};
  #define __mqttclient_subscribe_topics_count ((sizeof(_mqttclient_subscribe_topics) / sizeof(_mqttclient_subscribe_topics[0])) - 1)
#endif


static uint8_t _mqttclient_subscribe_topics_index = 0;

/** Keep track message send in progress. */
static bool _message_sending = false;

/** Keep track if keep alive message send is in progress. */
static bool _keep_alive_sending = false;

/** Keep track if publish message send is in progress. */
static bool _publish_sending = false;

/* Static function prototypes. */

/**
 * Initiate MQTT client timers.
 */
static inline void _mqttclient_init_timers(void);

/**
 * Reset MQTT RX and TX buffers.
 */
static void ICACHE_FLASH_ATTR _mqttclient_reset_buffers(void);

/**
 * Perform reconnect attempt. Called from reconnect timer.
 */
static void ICACHE_FLASH_ATTR _mqttclient_do_reconnect(void);

/**
 * Establish TCP connection with remote MQTT server.
 */
static void ICACHE_FLASH_ATTR _mqttclient_create_connection(void);

/**
 * Stop communication with MQTT broker.
 */
static void ICACHE_FLASH_ATTR _mqttclient_stop_communication(void);

/**
 * Callback function called when TCP connection is established with MQTT broker.
 *
 * @param arg
 */
static void ICACHE_FLASH_ATTR _mqttclient_connect_callback(void *arg);

/**
 * Disconnect callback.
 *
 * @param arg
 */
static void ICACHE_FLASH_ATTR _mqttclient_disconnect_callback(void *arg);

/**
 * Reconnect callback.
 *
 * @param arg
 * @param err
 */
static void ICACHE_FLASH_ATTR _mqttclient_reconnect_callback(void *arg, sint8 err);

/**
 * TCP finish callback.
 *
 * @param arg
 */
static void ICACHE_FLASH_ATTR _mqttclient_write_finish_fn(void *arg);

/**
 * Start all periodic MQTT timers (publish and keep alive messages).
 */
static void ICACHE_FLASH_ATTR _mqttclient_start_mqtt_timers(void);

/**
 * Stop all periodic MQTT timers.
 */
static void ICACHE_FLASH_ATTR _mqttclient_stop_mqtt_timers(void);

/**
 * Send CONNECT message to MQTT broker.
 */
static void ICACHE_FLASH_ATTR _mqttclient_broker_connect(void);

/**
 * Send initial publish messages to MQTT network.
 */
static void ICACHE_FLASH_ATTR _mqttclient_send_init_sequence(void);

/**
 * Subscribe to configured MQTT topics
 */
static void ICACHE_FLASH_ATTR _mqttclient_subscribe(void);

/**
 * Reset all indexes to communication initialization data.
 */
static inline void _mqttclient_reset_comm_indexes(void);

/**
 * Send keep alive message to MQTT broker.
 */
static void ICACHE_FLASH_ATTR _mqttclient_umqtt_keep_alive(void);

/**
 * Send data stored in MQTT RX buffer to broker.
 */
static void ICACHE_FLASH_ATTR _mqttclient_send(void);

/**
 * Data sent callback.
 *
 * @param arg
 */
static void ICACHE_FLASH_ATTR _mqttclient_send_callback(void *arg);

/**
 * Data received callback.
 *
 * @param arg
 * @param pdata
 * @param len
 */
static void ICACHE_FLASH_ATTR _mqttclient_received_callback(void *arg, char *pdata, unsigned short len);

/**
 * Update communication progress state.
 */
static void ICACHE_FLASH_ATTR _mqttclient_update_comm_progress(void);

/**
 * Progress communication.
 */
static void ICACHE_FLASH_ATTR _mqttclient_check_comm_progress(void);

/**
 * Schedule reconnect attempt.
 */
static inline void _mqttclient_schedule_reconnect(void);

#if ENABLE_DEVICE_CLASS_SENSOR
  #if SENSOR_TYPE_SYNCHRONOUS
/**
 * Publish MQTT data to broker.
 */
static void ICACHE_FLASH_ATTR _mqttclient_publish(void);
  #endif
#endif

#if ENABLE_DEVICE_CLASS_SENSOR
  #if SENSOR_TYPE_ASYNCHRONOUS
static void _mqttclient_async_callback(uint8_t topic_index);
  #endif
#endif

/* Implementation. */

void ICACHE_FLASH_ATTR mqttclient_init(void) {
    _mqttclient_init_timers();
    umqtt_init(&_mqttclient_mqtt);
    _mqttclient_reset_buffers();

#if ENABLE_DEVICE_CLASS_SENSOR
  #if SENSOR_TYPE_ASYNCHRONOUS
    sensor_register_notify_callback(_mqttclient_async_callback);
  #endif
#endif
}

void ICACHE_FLASH_ATTR mqttclient_start(void) {
    _mqttclient_is_running = true;
    _mqttclient_create_connection();
}

void ICACHE_FLASH_ATTR mqttclient_stop(void) {
    _mqttclient_is_running = false;
    _mqttclient_stop_communication();
    espconn_disconnect(&_mqttclient_espconn);
    if (espconn_delete(&_mqttclient_espconn)) {
        os_printf("Connection delete error\r\n");
    }
}

static inline void _mqttclient_init_timers(void) {

    /* Initiate timers. */
    os_timer_disarm(&_mqttclient_keep_alive_timer);
#if ENABLE_DEVICE_CLASS_SENSOR
  #if SENSOR_TYPE_SYNCHRONOUS
    os_timer_disarm(&_mqttclient_publish_timer);
  #endif
#endif
    os_timer_disarm(&_mqttclient_reconnect_timer);

    /* Assign functions to timers. */
    os_timer_setfn(&_mqttclient_keep_alive_timer, (os_timer_func_t *) _mqttclient_umqtt_keep_alive, NULL);
#if ENABLE_DEVICE_CLASS_SENSOR
  #if SENSOR_TYPE_SYNCHRONOUS
    os_timer_setfn(&_mqttclient_publish_timer, (os_timer_func_t *) _mqttclient_publish, NULL);
  #endif
#endif
    os_timer_setfn(&_mqttclient_reconnect_timer, (os_timer_func_t *) _mqttclient_do_reconnect, NULL);
}

static void ICACHE_FLASH_ATTR _mqttclient_reset_buffers(void) {
    umqtt_circ_init(&_mqttclient_mqtt.txbuff);
    umqtt_circ_init(&_mqttclient_mqtt.rxbuff);
}

static void ICACHE_FLASH_ATTR _mqttclient_do_reconnect(void) {
    _mqttclient_create_connection();
}

static void ICACHE_FLASH_ATTR _mqttclient_create_connection(void) {
    struct ip_addr ip;
    uint8_t ret;
    _mqttclient_espconn.type = ESPCONN_TCP;
    _mqttclient_espconn.state = ESPCONN_NONE;
    _mqttclient_espconn.reverse = &_mqttclient_espconn;
    _mqttclient_espconn.proto.tcp->local_port = espconn_port();
    _mqttclient_espconn.proto.tcp->remote_port = CONFIG_MQTT_BROKER_PORT;
    ret = ipaddr_aton(CONFIG_MQTT_BROKER_ADDRESS, &ip);
    if (!ret)
        os_printf("broker IP address parse failed\r\n");
    os_memcpy(_mqttclient_espconn.proto.tcp->remote_ip, &ip, sizeof(struct ip_addr));

    espconn_regist_connectcb(&_mqttclient_espconn, _mqttclient_connect_callback);
    espconn_regist_disconcb(&_mqttclient_espconn, _mqttclient_disconnect_callback);
    espconn_regist_reconcb(&_mqttclient_espconn, _mqttclient_reconnect_callback);
    espconn_regist_write_finish(&_mqttclient_espconn, _mqttclient_write_finish_fn);
    espconn_connect(&_mqttclient_espconn);
}

static void ICACHE_FLASH_ATTR _mqttclient_stop_communication(void) {
    _mqttclient_stop_mqtt_timers();
    _mqttclient_reset_comm_indexes();
    commsig_connection_status(false);
}

static void ICACHE_FLASH_ATTR _mqttclient_connect_callback(void *arg) {
    espconn_regist_sentcb(&_mqttclient_espconn, _mqttclient_send_callback);
    espconn_regist_recvcb(&_mqttclient_espconn, _mqttclient_received_callback);
    _mqttclient_broker_connect();
    _mqttclient_start_mqtt_timers();
}

static void ICACHE_FLASH_ATTR _mqttclient_disconnect_callback(void *arg) {
    _mqttclient_reset_buffers();
    _mqttclient_stop_communication();
    _message_sending = false;
    if (_mqttclient_is_running)
        _mqttclient_schedule_reconnect();
}

static void ICACHE_FLASH_ATTR _mqttclient_reconnect_callback(void *arg, sint8 err) {
    _mqttclient_stop_communication();
    switch (err) {
        case ESPCONN_TIMEOUT:
        case ESPCONN_ABRT:
        case ESPCONN_RST:
        case ESPCONN_CLSD:
        case ESPCONN_CONN:
        case ESPCONN_HANDSHAKE:
            _mqttclient_schedule_reconnect();
            break;
    }
}

static void ICACHE_FLASH_ATTR _mqttclient_write_finish_fn(void *arg) {
    commsig_connection_status(false);
}

static void ICACHE_FLASH_ATTR _mqttclient_start_mqtt_timers(void) {
    os_timer_arm(&_mqttclient_keep_alive_timer, CONFIG_MQTT_KEEPALIVE_REQUEST_INTERVAL * 1000, 1);
#if ENABLE_DEVICE_CLASS_SENSOR
  #if SENSOR_TYPE_SYNCHRONOUS
    os_timer_arm(&_mqttclient_publish_timer, CONFIG_MQTT_PUBLISH_INTERVAL * 1000, 1);
  #endif
#endif
}

static void ICACHE_FLASH_ATTR _mqttclient_stop_mqtt_timers(void) {
    os_timer_disarm(&_mqttclient_keep_alive_timer);
#if ENABLE_DEVICE_CLASS_SENSOR
  #if SENSOR_TYPE_SYNCHRONOUS
    os_timer_disarm(&_mqttclient_publish_timer);
  #endif
#endif
}

static void ICACHE_FLASH_ATTR _mqttclient_broker_connect(void) {
    umqtt_connect(&_mqttclient_mqtt, &_connection_config);
    _mqttclient_send();
}

static void ICACHE_FLASH_ATTR _mqttclient_send_init_sequence(void) {
    umqtt_publish(
        &_mqttclient_mqtt,
        _mqttclient_init_seq_items[_mqttclient_init_seq_items_index].topic,
        _mqttclient_init_seq_items[_mqttclient_init_seq_items_index].value,
        _mqttclient_init_seq_items[_mqttclient_init_seq_items_index].value_len,
        _mqttclient_init_seq_items[_mqttclient_init_seq_items_index].flags);
    _mqttclient_init_seq_items_index++;
}

static void ICACHE_FLASH_ATTR _mqttclient_subscribe(void) {
    os_printf("Subscribing %d...\r\n", __mqttclient_subscribe_topics_count);
    umqtt_subscribe(&_mqttclient_mqtt, _mqttclient_subscribe_topics[_mqttclient_subscribe_topics_index]);
    _mqttclient_subscribe_topics_index++;
}

static inline void _mqttclient_reset_comm_indexes(void) {
    _mqttclient_init_seq_items_index = 0;
    _mqttclient_subscribe_topics_index = 0;
}

static void ICACHE_FLASH_ATTR _mqttclient_umqtt_keep_alive(void) {
    if (!_keep_alive_sending) {
        _keep_alive_sending = true;
        umqtt_ping(&_mqttclient_mqtt);
        _mqttclient_send();
    }
}

static void ICACHE_FLASH_ATTR _mqttclient_send(void) {
    if (!_message_sending) {
        uint16_t len = umqtt_circ_pop(&_mqttclient_mqtt.txbuff, _rx_buf, sizeof(_rx_buf));
        if (len) {
            espconn_send(&_mqttclient_espconn, _rx_buf, len);
            _message_sending = true;
            commsig_notify();
        }
    }
}

static void ICACHE_FLASH_ATTR _mqttclient_send_callback(void *arg) {
    _message_sending = false;
    if (_keep_alive_sending)
        _keep_alive_sending = false;
    if (_publish_sending)
        _publish_sending = false;
    _mqttclient_update_comm_progress();
    _mqttclient_check_comm_progress();
    _mqttclient_send();
}

static void ICACHE_FLASH_ATTR _mqttclient_received_callback(void *arg, char *pdata, unsigned short len) {
    enum umqtt_client_state previous_state = _mqttclient_mqtt.state;
    umqtt_circ_push(&_mqttclient_mqtt.rxbuff, (uint8_t *) pdata, (uint16_t) len);
    umqtt_process(&_mqttclient_mqtt);

    /* Check for connection event. */
    if (previous_state != UMQTT_STATE_CONNECTED && _mqttclient_mqtt.state == UMQTT_STATE_CONNECTED) {

        /* Signal established connection. */
        commsig_connection_status(true);
        _mqttclient_comm_state = MQTTCLIENT_COMM_CONNECTED;
    } else if (_mqttclient_mqtt.state == UMQTT_STATE_CONNECTED) {
        /* MQTT connection is already established. Just notify about communication. */
        commsig_notify();
    }

    _mqttclient_update_comm_progress();
    _mqttclient_check_comm_progress();
    _mqttclient_send();
}

static void ICACHE_FLASH_ATTR _mqttclient_update_comm_progress(void) {
    if (_mqttclient_comm_state != MQTTCLIENT_COMM_OPERATIONAL) {
        if (_mqttclient_comm_state == MQTTCLIENT_COMM_CONNECTED) {
            if (_mqttclient_init_seq_items_index == __mqttclient_init_seq_items_count) {
                _mqttclient_comm_state = MQTTCLIENT_COMM_INIT_SEQ_PUBLISHED;
            }
        }

        if (_mqttclient_comm_state == MQTTCLIENT_COMM_INIT_SEQ_PUBLISHED) {
            if (_mqttclient_subscribe_topics_index == __mqttclient_subscribe_topics_count) {
                _mqttclient_comm_state = MQTTCLIENT_COMM_SUBSCRIBED;
            }
        }

        if (_mqttclient_comm_state == MQTTCLIENT_COMM_SUBSCRIBED) {
            _mqttclient_comm_state = MQTTCLIENT_COMM_OPERATIONAL;
        }
    }
}

static void ICACHE_FLASH_ATTR _mqttclient_check_comm_progress(void) {
    switch (_mqttclient_comm_state) {
        case MQTTCLIENT_COMM_CONNECTED:
            _mqttclient_send_init_sequence();
            break;
        case MQTTCLIENT_COMM_INIT_SEQ_PUBLISHED:
            _mqttclient_subscribe();
            break;
        default:
            /* Do nothing. */
            break;
    }
}

static inline void _mqttclient_schedule_reconnect(void) {
    os_timer_arm(&_mqttclient_reconnect_timer, 1000, 0);
}

#if ENABLE_DEVICE_CLASS_SENSOR
  #if SENSOR_TYPE_ASYNCHRONOUS
static void _mqttclient_async_callback(uint8_t topic_index) {
    uint8_t _topic_len;
    uint8_t _data_len;
    char *_topic;
    char *_data;

    if (!_publish_sending && _mqttclient_mqtt.state == UMQTT_STATE_CONNECTED) {
        _publish_sending = true;

        _topic = sensor_get_topic(topic_index, &_topic_len);
        _data = sensor_get_value(topic_index, &_data_len);
        umqtt_publish(&_mqttclient_mqtt, _topic, (uint8_t *) _data, _data_len, 0);

        _mqttclient_send();
    }
}
  #endif
#endif

#if ENABLE_DEVICE_CLASS_SENSOR
  #if SENSOR_TYPE_SYNCHRONOUS
static void ICACHE_FLASH_ATTR _mqttclient_publish(void) {
    uint8_t _i;
    uint8_t _topic_len;
    uint8_t _data_len;
    char *_topic;
    char *_data;

    if (!_publish_sending) {
        _publish_sending = true;

        sensor_read();
        for (_i = 0; _i < sensor_topics_count; _i++) {
            _topic = sensor_get_topic(_i, &_topic_len);
            _data = sensor_get_value(_i, &_data_len);
            umqtt_publish(&_mqttclient_mqtt, _topic, (uint8_t *) _data, _data_len, 0);
        }

        _mqttclient_send();
    }
}
  #endif
#endif

static void ICACHE_FLASH_ATTR _mqttclient_on_publish(struct umqtt_connection *_m, char *topic, uint8_t *data, uint16_t len) {
#if ENABLE_DEVICE_CLASS_REACTOR
    reactor_on_data(topic, data, len);
#endif
}
