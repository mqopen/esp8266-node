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
#include "mqttclient_data.h"
#include "mqttclient.h"

#if MQTTCLIENT_PUBLISH_INITIAL_STATE
  #if ENABLE_DEVICE_CLASS_SENSOR
    #define __mqttclient_initial_publish_count sensor_topics_count
    #define __mqttclient_get_topic sensor_get_topic
    #define __mqttclient_get_flags sensor_get_flags
  #elif ENABLE_DEVICE_CLASS_REACTOR
    #define __mqttclient_initial_publish_count reactor_respond_topics_count
    #define __mqttclient_get_topic reactor_respond_get_topic
    #define __mqttclient_get_flags reactor_respond_get_flags
  #else
    #error Unsupported device class with initial publish!
  #endif
#endif

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

/** Current communication state. */
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

/** Index to current element of _mqttclient_init_seq_items array. */
static uint8_t _mqttclient_init_seq_items_index = 0;

#if MQTTCLIENT_PUBLISH_INITIAL_STATE
/** Initial publish index. */
static uint8_t _mqttclient_initial_publish_index = 0;

/**
 * Keep track if at least one initial publish happend. This variable is set to
 * false if at least one initial publish message has been sent.
 */
static bool _mqttclient_initial_publish_blank = true;
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
static void ICACHE_FLASH_ATTR _mqttclient_do_comm_progress(void);

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

#if MQTTCLIENT_PUBLISH_INITIAL_STATE
/**
 * Publish initial data.
 */
static void _mqttclient_initial_publish(void);

static inline uint8_t _mqttclient_get_initial_value(uint8_t index, char **buf, uint8_t *buf_len);
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
#if ENABLE_DEVICE_CLASS_SENSOR && SENSOR_TYPE_ASYNCHRONOUS
            sensor_notify_lock();
#endif
    umqtt_init(&_mqttclient_mqtt);
    _mqttclient_stop_mqtt_timers();
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
        mqttclient_data_init_seq_items[_mqttclient_init_seq_items_index].topic,
        mqttclient_data_init_seq_items[_mqttclient_init_seq_items_index].value,
        mqttclient_data_init_seq_items[_mqttclient_init_seq_items_index].value_len,
        mqttclient_data_init_seq_items[_mqttclient_init_seq_items_index].flags);
    _mqttclient_init_seq_items_index++;
}

static void ICACHE_FLASH_ATTR _mqttclient_subscribe(void) {
    umqtt_subscribe(
        &_mqttclient_mqtt,
        mqttclient_data_subscribe_topics[_mqttclient_subscribe_topics_index]);
    _mqttclient_subscribe_topics_index++;
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
    _keep_alive_sending = false;
    _publish_sending = false;
    _mqttclient_update_comm_progress();
    _mqttclient_do_comm_progress();
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
    _mqttclient_do_comm_progress();
    _mqttclient_send();
}

static void ICACHE_FLASH_ATTR _mqttclient_update_comm_progress(void) {

    /*
     * Check if client isn't already in operational state. If it is, all communication
     * initialization steps are already done.
     */
    if (_mqttclient_comm_state != MQTTCLIENT_COMM_OPERATIONAL) {

        /* Check if MQTT connection was just established. */
        if (_mqttclient_comm_state == MQTTCLIENT_COMM_CONNECTED) {

            /* Check if all service topics has been published. */
            if (_mqttclient_init_seq_items_index == mqttclient_data_init_seq_items_count) {
                _mqttclient_comm_state = MQTTCLIENT_COMM_INIT_SEQ_PUBLISHED;
                _mqttclient_init_seq_items_index = 0;
//#if MQTTCLIENT_PUBLISH_INITIAL_STATE
//                sensor_notify_lock();
//#endif
            }
        }

        if (_mqttclient_comm_state == MQTTCLIENT_COMM_INIT_SEQ_PUBLISHED) {
#if MQTTCLIENT_PUBLISH_INITIAL_STATE
            if (_mqttclient_initial_publish_index == __mqttclient_initial_publish_count) {
                _mqttclient_comm_state = MQTTCLIENT_COMM_INIT_STATE_PUBLISHED;
                _mqttclient_initial_publish_index = 0;
            }
#else
            /* Check if all topics has been subscribed. */
            if (_mqttclient_subscribe_topics_index == mqttclient_data_subscribe_topics_count) {
                _mqttclient_comm_state = MQTTCLIENT_COMM_SUBSCRIBED;
                _mqttclient_subscribe_topics_index = 0;
            }
#endif
        }

#if MQTTCLIENT_PUBLISH_INITIAL_STATE
        if (_mqttclient_comm_state == MQTTCLIENT_COMM_INIT_STATE_PUBLISHED) {
            if (_mqttclient_subscribe_topics_index == mqttclient_data_subscribe_topics_count) {
                _mqttclient_comm_state = MQTTCLIENT_COMM_SUBSCRIBED;
                _mqttclient_subscribe_topics_index = 0;
                _mqttclient_initial_publish_blank = true;
            }
        }
#endif

        /* Check if all topics has been subscribed. */
        if (_mqttclient_comm_state == MQTTCLIENT_COMM_SUBSCRIBED) {

            /* Enter operation state. */
            _mqttclient_comm_state = MQTTCLIENT_COMM_OPERATIONAL;
#if ENABLE_DEVICE_CLASS_SENSOR && SENSOR_TYPE_ASYNCHRONOUS
            sensor_notify_release();
#endif
        }
    }
}

static void ICACHE_FLASH_ATTR _mqttclient_do_comm_progress(void) {
    switch (_mqttclient_comm_state) {
        case MQTTCLIENT_COMM_CONNECTED:
            _mqttclient_send_init_sequence();
            break;
        case MQTTCLIENT_COMM_INIT_SEQ_PUBLISHED:
#if MQTTCLIENT_PUBLISH_INITIAL_STATE
            _mqttclient_initial_publish();

            /*
             * If nothing was published, functions _mqttclient_update_comm_progress
             * and _mqttclient_do_comm_progress will not be called from callback
             * handlers. So call them now.
             */
            if (_mqttclient_initial_publish_blank) {
                _mqttclient_update_comm_progress();
                _mqttclient_do_comm_progress();
            }
#else
            _mqttclient_subscribe();
#endif
            break;
#if MQTTCLIENT_PUBLISH_INITIAL_STATE
        case MQTTCLIENT_COMM_INIT_STATE_PUBLISHED:
            _mqttclient_subscribe();
            break;
#endif
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
    uint8_t _flags;
    char *_topic;
    char *_data;

    if (!_publish_sending && _mqttclient_mqtt.state == UMQTT_STATE_CONNECTED) {
        _publish_sending = true;

        _topic = sensor_get_topic(topic_index, &_topic_len);
        _data = sensor_get_value(topic_index, &_data_len);
        _flags = sensor_get_flags(topic_index);
        umqtt_publish(&_mqttclient_mqtt, _topic, (uint8_t *) _data, _data_len, _flags);

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
    uint8_t _flags;
    char *_topic;
    char *_data;

    if (!_publish_sending) {
        _publish_sending = true;

        sensor_read();
        for (_i = 0; _i < __mqttclient_initial_publish_count; _i++) {
            _topic = sensor_get_topic(_i, &_topic_len);
            _data = sensor_get_value(_i, &_data_len);
            _flags = sensor_get_flags(_i);
            umqtt_publish(&_mqttclient_mqtt, _topic, (uint8_t *) _data, _data_len, _flags);
        }

        _mqttclient_send();
    }
}
  #endif
#endif

static void ICACHE_FLASH_ATTR _mqttclient_on_publish(struct umqtt_connection *_m, char *topic, uint8_t *data, uint16_t len) {
#if ENABLE_DEVICE_CLASS_REACTOR
    uint8_t i;
    char *_respond_topic;
    char *_respond_data;
    uint8_t _respond_topic_len;
    uint8_t _respond_data_len;
    uint8_t _respond_flags;

    /* Process received data. */
    reactor_on_data(topic, data, len);

    /* Respond to network. */
    for (i = 0; i < reactor_respond_topics_count; i++) {
        if (reactor_respond_is_updated(i)) {
            _respond_topic = reactor_respond_get_topic(i, &_respond_topic_len);
            _respond_data = reactor_respond_get_value(i, &_respond_data_len);
            _respond_flags = reactor_respond_get_flags(i);
            umqtt_publish(
                &_mqttclient_mqtt,
                _respond_topic,
                (uint8_t *) _respond_data,
                _respond_data_len,
                _respond_flags);
        }
    }

    reactor_respond_commit();
    _mqttclient_send();
#endif
}

#if MQTTCLIENT_PUBLISH_INITIAL_STATE
static void _mqttclient_initial_publish(void) {
    char *_topic;
    char *_data;
    uint8_t _data_len;
    uint8_t _topic_len;
    uint8_t _flags;

    /* Get sensor data until some data are relevant. */
    while (!_mqttclient_get_initial_value(_mqttclient_initial_publish_index, &_data, &_data_len)) {
        _mqttclient_initial_publish_index++;

        /* Check if there are more topics. */
        if (_mqttclient_initial_publish_index == __mqttclient_initial_publish_count) {
            return;
        }
    }

    /* We have relevant data. */
    _mqttclient_initial_publish_blank = false;

    _topic = __mqttclient_get_topic(_mqttclient_initial_publish_index, &_topic_len);
    _flags = __mqttclient_get_flags(_mqttclient_initial_publish_index);
    umqtt_publish(&_mqttclient_mqtt, _topic, (uint8_t *) _data, _data_len, _flags);
    _mqttclient_initial_publish_index++;
}

static inline uint8_t _mqttclient_get_initial_value(uint8_t index, char **buf, uint8_t *buf_len) {
  #if ENABLE_DEVICE_CLASS_SENSOR
    return sensor_get_initial_value(index, buf, buf_len);
  #elif ENABLE_DEVICE_CLASS_REACTOR
    *buf = reactor_respond_get_value(index, buf_len);
    return 1;
  #else
    #error Unsupported implementation!
  #endif
}
#endif
