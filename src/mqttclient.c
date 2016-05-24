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
#include "actsig.h"
#include "sensor.h"
#include "common.h"
#include "mqttclient.h"

/** Timer for ending MQTT Keep Alive messages. */
static os_timer_t _keep_alive_timer;

#if SENSOR_TYPE_SYNCHRONOUS
/** Timer for sending DHT measurements. */
static os_timer_t  _publish_timer;
#endif

/** Timer for limit reconnect attempts. */
static os_timer_t _reconnect_timer;

/** Keep track if MQTT client is running. */
static bool _is_running = false;

/** Signal network activity. */
struct actsig_signal _activity_signal;

/** TCP connection. */
static struct _esp_tcp _mqttclient_tcp;

/** Network connection. */
static struct espconn _mqttclient_espconn = {
    .proto.tcp = &_mqttclient_tcp,
};

static uint8_t _mqttclient_tx_buffer[200];
static uint8_t _mqttclient_rx_buffer[200];

/** MQTT connection structure instance. */
static struct umqtt_connection _mqtt = {
    .txbuff = {
        .start = _mqttclient_tx_buffer,
        .length = sizeof(_mqttclient_tx_buffer),
    },
    .rxbuff = {
        .start = _mqttclient_rx_buffer,
        .length = sizeof(_mqttclient_rx_buffer),
    },
    .state = UMQTT_STATE_INIT,
};

/** MQTT connection configuration. */
static struct umqtt_connect_config _connection_config = {
    .keep_alive = CONFIG_MQTT_KEEPALIVE_CONNECT_INTERVAL,
    .client_id = CONFIG_GENERAL_DEVICE_NAME,
    .will_topic = TOPIC_PRESENCE(CONFIG_GENERAL_DEVICE_NAME),
    .will_message = (uint8_t *) CONFIG_MQTT_PRESENCE_OFFLINE,
    .will_message_len = sizeof(CONFIG_MQTT_PRESENCE_OFFLINE) - 1,
    .flags = _BV(UMQTT_OPT_RETAIN),
};

/** Keep track message send in progress. */
static bool _message_sending = false;

/** Keep track if keep alive message send is in progress. */
static bool _keep_alive_sending = false;

/** Keep track if publish message send is in progress. */
static bool _publish_sending = false;

/* Static function prototypes. */

#if SENSOR_TYPE_SYNCHRONOUS
/**
 * Publish MQTT data to broker.
 */
static void ICACHE_FLASH_ATTR _mqttclient_publish(void);
#endif

/**
 * Send CONNECT message to MQTT broker.
 */
static void ICACHE_FLASH_ATTR _mqttclient_broker_connect(void);

/**
 * Send keep alive message to MQTT broker.
 */
static void ICACHE_FLASH_ATTR _mqttclient_umqtt_keep_alive(void);

/**
 * Establish TCP connection with remote MQTT server.
 */
static void ICACHE_FLASH_ATTR _mqttclient_create_connection(void);

/**
 * Callback function called when TCP connection is established with MQTT broker.
 *
 * @param arg
 */
static void ICACHE_FLASH_ATTR _mqttclient_connect_callback(void *arg);

/**
 * Reconnect callback.
 *
 * @param arg
 * @param err
 */
static void ICACHE_FLASH_ATTR _mqttclient_reconnect_callback(void *arg, sint8 err);

/**
 * Disconnect callback.
 *
 * @param arg
 */
static void ICACHE_FLASH_ATTR _mqttclient_disconnect_callback(void *arg);

/**
 * TCP finish callback.
 *
 * @param arg
 */
static void ICACHE_FLASH_ATTR _mqttclient_write_finish_fn(void *arg);

/**
 * Data received callback.
 *
 * @param arg
 * @param pdata
 * @param len
 */
static void ICACHE_FLASH_ATTR _mqttclient_data_received(void *arg, char *pdata, unsigned short len);

/**
 * Data sent callback.
 *
 * @param arg
 */
static void ICACHE_FLASH_ATTR _mqttclient_data_sent(void *arg);

/**
 * Start all periodic MQTT timers (publish and keep alive messages).
 */
static void ICACHE_FLASH_ATTR _mqttclient_start_mqtt_timers(void);

/**
 * Stop all periodic MQTT timers.
 */
static void ICACHE_FLASH_ATTR _mqttclient_stop_mqtt_timers(void);

/**
 * Perform reconnect attempt. Called from reconnect timer.
 */
static void ICACHE_FLASH_ATTR _mqttclient_do_reconnect(void);

/**
 * Schedule reconnect attempt.
 */
static inline void _mqttclient_schedule_reconnect(void);

/**
 * Send data stored in MQTT RX buffer to broker.
 */
static void ICACHE_FLASH_ATTR _mqttclient_data_send(void);

/**
 * Stop communication with MQTT broker.
 */
static void ICACHE_FLASH_ATTR _mqttclient_stop_communication(void);

static void ICACHE_FLASH_ATTR _mqttclient_reset_buffers(void);

#if SENSOR_TYPE_ASYNCHRONOUS
static void _mqttclient_async_callback(uint8_t topic_index);
#endif

void ICACHE_FLASH_ATTR mqttclient_init(void) {

    /* Signalization LED. */
    actsig_init(&_activity_signal, CONFIG_MQTT_ACTIVITY_LED_BLINK_TRANSMITT_DELAY);
    actsig_set_normal_off(&_activity_signal);

    /* Initiate timers. */
    os_timer_disarm(&_keep_alive_timer);
#if SENSOR_TYPE_SYNCHRONOUS
    os_timer_disarm(&_publish_timer);
#endif
    os_timer_disarm(&_reconnect_timer);

    /* Assign functions for timers. */
    os_timer_setfn(&_keep_alive_timer, (os_timer_func_t *) _mqttclient_umqtt_keep_alive, NULL);
#if SENSOR_TYPE_SYNCHRONOUS
    os_timer_setfn(&_publish_timer, (os_timer_func_t *) _mqttclient_publish, NULL);
#endif
    os_timer_setfn(&_reconnect_timer, (os_timer_func_t *) _mqttclient_do_reconnect, NULL);

    umqtt_init(&_mqtt);
    _mqttclient_reset_buffers();

#if SENSOR_TYPE_ASYNCHRONOUS
    sensor_register_notify_callback(_mqttclient_async_callback);
#endif
}

#if SENSOR_TYPE_ASYNCHRONOUS
static void _mqttclient_async_callback(uint8_t topic_index) {
    uint8_t _topic_len;
    uint8_t _data_len;
    char *_topic;
    char *_data;

    if (!_publish_sending && _mqtt.state == UMQTT_STATE_CONNECTED) {
        _publish_sending = true;

        _topic = sensor_get_topic(topic_index, &_topic_len);
        _data = sensor_get_value(topic_index, &_data_len);
        umqtt_publish(&_mqtt, _topic, (uint8_t *) _data, _data_len, 0);

        _mqttclient_data_send();
    }
}
#endif

void ICACHE_FLASH_ATTR mqttclient_start(void) {
    _is_running = true;
    _mqttclient_create_connection();
}

void ICACHE_FLASH_ATTR mqttclient_stop(void) {
    _is_running = false;
    _mqttclient_stop_communication();
    espconn_disconnect(&_mqttclient_espconn);
    if (espconn_delete(&_mqttclient_espconn)) {
        os_printf("Connection delete error\r\n");
    }
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

static void ICACHE_FLASH_ATTR _mqttclient_connect_callback(void *arg) {
    espconn_regist_sentcb(&_mqttclient_espconn, _mqttclient_data_sent);
    espconn_regist_recvcb(&_mqttclient_espconn, _mqttclient_data_received);
    _mqttclient_broker_connect();
    _mqttclient_start_mqtt_timers();
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

static void ICACHE_FLASH_ATTR _mqttclient_disconnect_callback(void *arg) {
    _mqttclient_reset_buffers();
    _mqttclient_stop_communication();
    _message_sending = false;
    if (_is_running)
        _mqttclient_schedule_reconnect();
}

static void ICACHE_FLASH_ATTR _mqttclient_stop_communication(void) {
    _mqttclient_stop_mqtt_timers();
    actsig_set_normal_off(&_activity_signal);
}

static void ICACHE_FLASH_ATTR _mqttclient_write_finish_fn(void *arg) {
    actsig_set_normal_off(&_activity_signal);
}

static void ICACHE_FLASH_ATTR _mqttclient_data_received(void *arg, char *pdata, unsigned short len) {
    enum umqtt_client_state previous_state = _mqtt.state;
    umqtt_circ_push(&_mqtt.rxbuff, (uint8_t *) pdata, (uint16_t) len);
    umqtt_process(&_mqtt);

    /* Check for connection event. */
    if (previous_state != UMQTT_STATE_CONNECTED && _mqtt.state == UMQTT_STATE_CONNECTED) {

        /* Signal established connection. */
        actsig_set_normal_on(&_activity_signal);

        /* Send presence message. */
        umqtt_publish(&_mqtt,
                        TOPIC_PRESENCE(CONFIG_GENERAL_DEVICE_NAME),
                        (uint8_t *) CONFIG_MQTT_PRESENCE_ONLINE,
                        sizeof(CONFIG_MQTT_PRESENCE_ONLINE) - 1,
                        _BV(UMQTT_OPT_RETAIN));
    }
    _mqttclient_data_send();
}

static void ICACHE_FLASH_ATTR _mqttclient_data_sent(void *arg) {
    _message_sending = false;
    if (_keep_alive_sending)
        _keep_alive_sending = false;
    if (_publish_sending)
        _publish_sending = false;
    _mqttclient_data_send();
}

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
            umqtt_publish(&_mqtt, _topic, (uint8_t *) _data, _data_len, 0);
        }

        _mqttclient_data_send();
    }
}
#endif

static void ICACHE_FLASH_ATTR _mqttclient_umqtt_keep_alive(void) {
    if (!_keep_alive_sending) {
        _keep_alive_sending = true;
        umqtt_ping(&_mqtt);
        _mqttclient_data_send();
    }
}

static void ICACHE_FLASH_ATTR _mqttclient_start_mqtt_timers(void) {
    os_timer_arm(&_keep_alive_timer, CONFIG_MQTT_KEEPALIVE_REQUEST_INTERVAL * 1000, 1);
#if SENSOR_TYPE_SYNCHRONOUS
    os_timer_arm(&_publish_timer, CONFIG_MQTT_PUBLISH_INTERVAL * 1000, 1);
#endif
}

static void ICACHE_FLASH_ATTR _mqttclient_stop_mqtt_timers(void) {
    os_timer_disarm(&_keep_alive_timer);
#if SENSOR_TYPE_SYNCHRONOUS
    os_timer_disarm(&_publish_timer);
#endif
}

static uint8_t _rx_buf[200];

static void ICACHE_FLASH_ATTR _mqttclient_data_send(void) {
    if (!_message_sending) {
        uint16_t len = umqtt_circ_pop(&_mqtt.txbuff, _rx_buf, sizeof(_rx_buf));
        if (len) {
            espconn_send(&_mqttclient_espconn, _rx_buf, len);
            _message_sending = true;
            actsig_notify(&_activity_signal);
        }
    }
}

static void ICACHE_FLASH_ATTR _mqttclient_do_reconnect(void) {
    _mqttclient_create_connection();
}

static inline void _mqttclient_schedule_reconnect(void) {
    os_timer_arm(&_reconnect_timer, 1000, 0);
}

static void ICACHE_FLASH_ATTR _mqttclient_broker_connect(void) {
    umqtt_connect(&_mqtt, &_connection_config);
    _mqttclient_data_send();
}

static void ICACHE_FLASH_ATTR _mqttclient_reset_buffers(void) {
    umqtt_circ_init(&_mqtt.txbuff);
    umqtt_circ_init(&_mqtt.rxbuff);
}
