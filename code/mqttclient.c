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
#include "gpio16.h"
#include "umqtt.h"
#include "bmp180.h"
#include "mqttclient.h"

//#define send_buffer_length      sizeof(sharedbuf.mqtt.send_buffer)
//#define send_buffer_length      sizeof(_mqttclient_send_buffer)
//#define update_state(state)     (mqttclient_state = state)

/**
 * MQTT client current state.
 */
//static enum mqttclient_state mqttclient_state;

/**
 * Timer for ending MQTT Keep Alive messages.
 */
static os_timer_t _keep_alive_timer;

/**
 * Timer for sending DHT measurements.
 */
static os_timer_t  _publish_timer;

/**
 * Blinking with LED.
 */
static os_timer_t  _led_blink_timer;

/**
 * Activity LED indicator state.
 */
static bool _active_led_state;

/**
 * Timer for limit reconnect attempts.
 */
static os_timer_t _reconnect_timer;

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

//static uint8_t *_mqttclient_send_buffer = sharedbuf.mqtt.send_buffer;
static uint8_t _mqttclient_tx_buffer[200];
static uint8_t _mqttclient_rx_buffer[200];

// TODO: make this variable uint16_t
//static int16_t _mqttclient_send_length;

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
//    .message_callback = _umqttclient_handle_message,
//    .state = UMQTT_STATE_INIT,
};

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
 * Turn active LED on.
 */
static inline void _mqttclient_led_on(void);

/**
 * Turn active LED off.
 */
static inline void _mqttclient_led_off(void);

/**
 * Blink with actived LED.
 */
static void ICACHE_FLASH_ATTR _mqttclient_led_active_blink(void);

/**
 * Toggle active LED.
 */
static void ICACHE_FLASH_ATTR _mqttclient_led_toggle(void);

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

void ICACHE_FLASH_ATTR mqttclient_init(void) {

    /* Signalization LED. */
    gpio16_output_conf();
    _mqttclient_led_off();

    /* Initiate timers. */
    os_timer_disarm(&_keep_alive_timer);
    os_timer_disarm(&_publish_timer);
    os_timer_disarm(&_led_blink_timer);
    os_timer_disarm(&_reconnect_timer);

    /* Assign functions for timers. */
    os_timer_setfn(&_keep_alive_timer, (os_timer_func_t *) _mqttclient_umqtt_keep_alive, NULL);
    os_timer_setfn(&_publish_timer, (os_timer_func_t *) _mqttclient_publish, NULL);
    os_timer_setfn(&_led_blink_timer, (os_timer_func_t *) _mqttclient_led_toggle, NULL);
    os_timer_setfn(&_reconnect_timer, (os_timer_func_t *) _mqttclient_do_reconnect, NULL);
}

void ICACHE_FLASH_ATTR mqttclient_start(void) {
    _mqttclient_create_connection();
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
    espconn_regist_sentcb(&_mqttclient_espconn, _mqttclient_data_sent);
    espconn_regist_recvcb(&_mqttclient_espconn, _mqttclient_data_received);
    _mqttclient_led_on();
    _mqttclient_broker_connect();
    _mqttclient_start_mqtt_timers();
}

static void ICACHE_FLASH_ATTR _mqttclient_reconnect_callback(void *arg, sint8 err) {
    _mqttclient_stop_mqtt_timers();
    _mqttclient_led_off();
    _mqttclient_schedule_reconnect();
}

static void ICACHE_FLASH_ATTR _mqttclient_disconnect_callback(void *arg) {
    _mqttclient_stop_mqtt_timers();
    _mqttclient_led_off();
    _mqttclient_schedule_reconnect();
}

static void ICACHE_FLASH_ATTR _mqttclient_write_finish_fn(void *arg) {
    _mqttclient_led_off();
}

static void ICACHE_FLASH_ATTR _mqttclient_data_received(void *arg, char *pdata, unsigned short len) {
    umqtt_circ_push(&_mqtt.rxbuff, (uint8_t *) pdata, (uint16_t) len);
    umqtt_process(&_mqtt);
    _mqttclient_data_send();
}

static void ICACHE_FLASH_ATTR _mqttclient_data_sent(void *arg) {
}

static void ICACHE_FLASH_ATTR _mqttclient_publish(void) {
    enum bmp180_read_status status = bmp180_read(BMP180_OSS_8);
    if (status == BMP180_READ_STATUS_OK) {
        os_printf("T: %d.%d, P: %d\r\n", bmp180_data.temperature / 1000, bmp180_data.temperature % 1000, bmp180_data.pressure);
    } else {
        os_printf("Read failed\r\n");
    }

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

static void ICACHE_FLASH_ATTR _mqttclient_umqtt_keep_alive(void) {
    umqtt_ping(&_mqtt);
    _mqttclient_data_send();
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
    uint16_t len = umqtt_circ_pop(&_mqtt.txbuff, _rx_buf, sizeof(_rx_buf));
    if (len) {
        espconn_send(&_mqttclient_espconn, _rx_buf, len);
        _mqttclient_led_active_blink();
    }
}

static inline void _mqttclient_led_on(void) {
    gpio16_output_set(0);
    _active_led_state = true;
}

static inline void _mqttclient_led_off(void) {
    gpio16_output_set(1);
    _active_led_state = false;
}

static void ICACHE_FLASH_ATTR _mqttclient_led_active_blink(void) {
    _mqttclient_led_toggle();
    os_timer_arm(&_led_blink_timer, CONFIG_MQTT_ACTIVE_LED_INTERVAL_MS, 0);
}

static void ICACHE_FLASH_ATTR _mqttclient_led_toggle(void) {
    if (_active_led_state) {
        _mqttclient_led_off();
    } else {
        _mqttclient_led_on();
    }
}

static void ICACHE_FLASH_ATTR _mqttclient_do_reconnect(void) {
    espconn_connect(&_mqttclient_espconn);
}

static inline void _mqttclient_schedule_reconnect(void) {
    os_timer_arm(&_reconnect_timer, 1000, 0);
}

static void ICACHE_FLASH_ATTR _mqttclient_broker_connect(void) {
    umqtt_init(&_mqtt);
    umqtt_circ_init(&_mqtt.txbuff);
    umqtt_circ_init(&_mqtt.rxbuff);
    umqtt_connect(&_mqtt, CONFIG_MQTT_KEEP_ALIVE, CONFIG_MQTT_CLIENT_ID);
    _mqttclient_data_send();
}

/* **************************************************************************** */


//void mqttclient_notify_broker_unreachable(void) {
//    timer_restart(&disconnected_wait_timer);
//    update_state(MQTTCLIENT_BROKER_DISCONNECTED);
//}

//void mqttclient_process(void) {
//    switch (current_state) {
//        case MQTTCLIENT_BROKER_CONNECTION_ESTABLISHED:
//            _mqttclient_handle_connection_established();
//            break;
//        case MQTTCLIENT_BROKER_DISCONNECTED:
//            _mqttclient_broker_connect();
//            break;
//        case MQTTCLIENT_BROKER_DISCONNECTED_WAIT:
//            _mqttclient_handle_disconnected_wait();
//            break;
//        default:
//            break;
//    }
//}

//void mqttclient_appcall(void) {
//    struct umqtt_connection *conn = uip_conn->appstate.conn;
//
//    if (uip_connected()) {
//        update_state(MQTTCLIENT_BROKER_CONNECTION_ESTABLISHED);
//    }
//
//    if (uip_aborted() || uip_timedout() || uip_closed()) {
//        if (current_state == MQTTCLIENT_BROKER_CONNECTING) {
//            /* Another disconnect in reconnecting phase. Shut down for a while, then try again. */
//            mqttclient_notify_broker_unreachable();
//        } else if (current_state != MQTTCLIENT_BROKER_DISCONNECTED_WAIT) {
//            /* We are not waiting for atother reconnect try. */
//            update_state(MQTTCLIENT_BROKER_DISCONNECTED);
//            mqtt.state = UMQTT_STATE_INIT;
//        }
//    }
//
//    if (uip_newdata()) {
//        umqtt_circ_push(&conn->rxbuff, uip_appdata, uip_datalen());
//        umqtt_process(conn);
//    }
//
//    if (uip_rexmit()) {
//        uip_send(_mqttclient_send_buffer, _mqttclient_send_length);
//    } else if (uip_poll() || uip_acked()) {
//        _mqttclient_send_length = umqtt_circ_pop(&conn->txbuff, _mqttclient_send_buffer, send_buffer_length);
//        if (!_mqttclient_send_length)
//            return;
//        uip_send(_mqttclient_send_buffer, _mqttclient_send_length);
//    }
//}

//static void _mqttclient_handle_connection_established(void) {
//    if (mqtt.state == UMQTT_STATE_CONNECTED) {
//        if (timer_tryrestart(&keep_alive_timer))
//            _mqttclient_umqtt_keep_alive(&mqtt);
//        if (timer_tryrestart(&dht_timer))
//            _mqttclient_send_data();
//    } else if (mqtt.state == UMQTT_STATE_INIT) {
//        _mqttclient_mqtt_init();
//    }
//}

//static void _mqttclient_handle_disconnected_wait(void) {
//    if (timer_tryrestart(&disconnected_wait_timer))
//        _mqttclient_broker_connect();
//}

//static void _mqttclient_send_data(void) {
//    enum dht_read_status status = dht_read();
//    // TODO: remove hardcoded constant
//    char buffer[20];
//    uint8_t len = 0;
//    switch (status) {
//        case DHT_OK:
//            // If status is OK, publish measured data and return from function.
//            len = snprintf(buffer, sizeof(buffer), "%d.%d", dht_data.humidity / 10, dht_data.humidity % 10);
//            umqtt_publish(&mqtt, MQTT_TOPIC_HUMIDITY, (uint8_t *) buffer, len);
//            len = snprintf(buffer, sizeof(buffer), "%d.%d", dht_data.temperature / 10, dht_data.temperature % 10);
//            umqtt_publish(&mqtt, MQTT_TOPIC_TEMPERATURE, (uint8_t *) buffer, len);
//            return;
//        case DHT_ERROR_CHECKSUM:
//            len = snprintf(buffer, sizeof(buffer), "E_CHECKSUM");
//            break;
//        case DHT_ERROR_TIMEOUT:
//            len = snprintf(buffer, sizeof(buffer), "E_TIMEOUT");
//            break;
//        case DHT_ERROR_CONNECT:
//            len = snprintf(buffer, sizeof(buffer), "E_CONNECT");
//            break;
//        case DHT_ERROR_ACK_L:
//            len = snprintf(buffer, sizeof(buffer), "E_ACK_L");
//            break;
//        case DHT_ERROR_ACK_H:
//            len = snprintf(buffer, sizeof(buffer), "E_ACK_H");
//            break;
//    }
//
//    /* Publish error codes. */
//    umqtt_publish(&mqtt, MQTT_TOPIC_HUMIDITY, (uint8_t *)buffer, len);
//    umqtt_publish(&mqtt, MQTT_TOPIC_TEMPERATURE, (uint8_t *)buffer, len);
//}

//static void _mqttclient_mqtt_init(void) {
//    umqtt_init(&mqtt);
//    umqtt_circ_init(&mqtt.txbuff);
//    umqtt_circ_init(&mqtt.rxbuff);
//    umqtt_connect(&mqtt, MQTT_KEEP_ALIVE, MQTT_CLIENT_ID);
//}

//static void _umqttclient_handle_message(struct umqtt_connection __attribute__((unused)) *conn, char *topic, uint8_t *data, int len) {
//}
