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
#include "bmp180.h"
#include "mqttclient.h"

/**
 * Timer for ending MQTT Keep Alive messages.
 */
static os_timer_t _keep_alive_timer;

/**
 * Timer for sending DHT measurements.
 */
static os_timer_t  _publish_timer;

/**
 * Timer for limit reconnect attempts.
 */
static os_timer_t _reconnect_timer;

/**
 * Keep track if MQTT client is running.
 */
static bool _is_running = false;

/**
 * Signal network activity.
 */
struct actsig_signal _activity_signal;

/**
 * TCP connection.
 */
static struct _esp_tcp _mqttclient_tcp;

/**
 * Network connection.
 */
static struct espconn _mqttclient_espconn = {
    .proto.tcp = &_mqttclient_tcp,
};

static uint8_t _mqttclient_tx_buffer[200];
static uint8_t _mqttclient_rx_buffer[200];


/* MQTT connection structure instance. */
static struct umqtt_connection _mqtt = {
    .txbuff = {
        .start = _mqttclient_tx_buffer,
        .length = sizeof(_mqttclient_tx_buffer),
    },
    .rxbuff = {
        .start = _mqttclient_rx_buffer,
        .length = sizeof(_mqttclient_rx_buffer),
    },
};

/**
 * Keep track message send in progress.
 */
static bool _message_sending = false;

/* Static function prototypes. */

/**
 * Publish MQTT data to broker.
 *
 * @todo Not implemented yet.
 */
static void ICACHE_FLASH_ATTR _mqttclient_publish(void);

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

void ICACHE_FLASH_ATTR mqttclient_init(void) {

    /* Signalization LED. */
    actsig_init(&_activity_signal, CONFIG_MQTT_ACTIVE_LED_INTERVAL_MS);
    actsig_set_normal_off(&_activity_signal);

    /* Initiate timers. */
    os_timer_disarm(&_keep_alive_timer);
    os_timer_disarm(&_publish_timer);
    os_timer_disarm(&_reconnect_timer);

    /* Assign functions for timers. */
    os_timer_setfn(&_keep_alive_timer, (os_timer_func_t *) _mqttclient_umqtt_keep_alive, NULL);
    os_timer_setfn(&_publish_timer, (os_timer_func_t *) _mqttclient_publish, NULL);
    os_timer_setfn(&_reconnect_timer, (os_timer_func_t *) _mqttclient_do_reconnect, NULL);

    umqtt_init(&_mqtt);
    _mqttclient_reset_buffers();
}

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
    _mqttclient_espconn.type = ESPCONN_TCP;
    _mqttclient_espconn.state = ESPCONN_NONE;
    _mqttclient_espconn.reverse = &_mqttclient_espconn;
    _mqttclient_espconn.proto.tcp->local_port = espconn_port();
    _mqttclient_espconn.proto.tcp->remote_port = CONFIG_MQTT_BROKER_IP_PORT;
    IP4_ADDR(&ip, CONFIG_MQTT_BROKER_IP_ADDRESS0,
                    CONFIG_MQTT_BROKER_IP_ADDRESS1,
                    CONFIG_MQTT_BROKER_IP_ADDRESS2,
                    CONFIG_MQTT_BROKER_IP_ADDRESS3);
    // TODO: hardcoded IP address length literal
    os_memcpy(_mqttclient_espconn.proto.tcp->remote_ip, &ip, 4);

    espconn_regist_connectcb(&_mqttclient_espconn, _mqttclient_connect_callback);
    espconn_regist_disconcb(&_mqttclient_espconn, _mqttclient_disconnect_callback);
    espconn_regist_reconcb(&_mqttclient_espconn, _mqttclient_reconnect_callback);
    espconn_regist_write_finish(&_mqttclient_espconn, _mqttclient_write_finish_fn);
    espconn_connect(&_mqttclient_espconn);
}

static void ICACHE_FLASH_ATTR _mqttclient_connect_callback(void *arg) {
    os_printf("Connected\r\n");
    espconn_regist_sentcb(&_mqttclient_espconn, _mqttclient_data_sent);
    espconn_regist_recvcb(&_mqttclient_espconn, _mqttclient_data_received);
    actsig_set_normal_on(&_activity_signal);
    _mqttclient_broker_connect();
    _mqttclient_start_mqtt_timers();
}

static void ICACHE_FLASH_ATTR _mqttclient_reconnect_callback(void *arg, sint8 err) {
    _mqttclient_stop_communication();
    os_printf("Reconnect\r\n");
    switch (err) {
        case ESPCONN_TIMEOUT:
            os_printf("ESPCONN_TIMEOUT\r\n");
            break;
        case ESPCONN_ABRT:
            os_printf("ESPCONN_ABRT\r\n");
            break;
        case ESPCONN_RST:
            os_printf("ESPCONN_RST\r\n");
            break;
        case ESPCONN_CLSD:
            os_printf("ESPCONN_CLSD\r\n");
            break;
        case ESPCONN_CONN:
            os_printf("ESPCONN_CONN\r\n");
            break;
        case ESPCONN_HANDSHAKE:
            os_printf("ESPCONN_HANDSHAKE\r\n");
            break;
    }

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
    os_printf("Disconnect\r\n");
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
    os_printf("Write finish\r\n");
    actsig_set_normal_off(&_activity_signal);
}

static void ICACHE_FLASH_ATTR _mqttclient_data_received(void *arg, char *pdata, unsigned short len) {
    umqtt_circ_push(&_mqtt.rxbuff, (uint8_t *) pdata, (uint16_t) len);
    umqtt_process(&_mqtt);
    _mqttclient_data_send();
}

static void ICACHE_FLASH_ATTR _mqttclient_data_sent(void *arg) {
    _message_sending = false;
    _mqttclient_data_send();
}

static void ICACHE_FLASH_ATTR _mqttclient_publish(void) {
    if (!_message_sending) {
        enum bmp180_read_status status = bmp180_read(BMP180_OSS_8);
        // TODO: hardcoded constant
        char buf[20];
        uint16_t len;
        if (status == BMP180_READ_STATUS_OK) {
            len = os_sprintf(buf, "%d.%d", bmp180_data.temperature / 1000, bmp180_data.temperature % 1000);
            umqtt_publish(&_mqtt, CONFIG_MQTT_TOPIC_TEMPERATURE, (uint8_t *) buf, len);
            len = os_sprintf(buf, "%d", bmp180_data.pressure);
            umqtt_publish(&_mqtt, CONFIG_MQTT_TOPIC_PRESSURE, (uint8_t *) buf, len);
        }
        _mqttclient_data_send();
    }
}

static void ICACHE_FLASH_ATTR _mqttclient_umqtt_keep_alive(void) {
    if (!_message_sending) {
        umqtt_ping(&_mqtt);
        _mqttclient_data_send();
    }
}

static void ICACHE_FLASH_ATTR _mqttclient_start_mqtt_timers(void) {
    os_timer_arm(&_keep_alive_timer, CONFIG_MQTT_KEEP_ALIVE_INTERVAL_MS, 1);
    os_timer_arm(&_publish_timer, CONFIG_MQTT_PUBLISH_INTERVAL_MS, 1);
}

static void ICACHE_FLASH_ATTR _mqttclient_stop_mqtt_timers(void) {
    os_timer_disarm(&_keep_alive_timer);
    os_timer_disarm(&_publish_timer);
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
    umqtt_connect(&_mqtt, CONFIG_MQTT_KEEP_ALIVE, CONFIG_MQTT_CLIENT_ID);
    _mqttclient_data_send();
}

static void ICACHE_FLASH_ATTR _mqttclient_reset_buffers(void) {
    umqtt_circ_init(&_mqtt.txbuff);
    umqtt_circ_init(&_mqtt.rxbuff);
}
