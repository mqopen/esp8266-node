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

#ifndef __MQTTCLIENT_H__
#define __MQTTCLIENT_H__

/**
 * MQTT client module is responsible for establishing connection with a broker
 * server, sending messages end receiving data.
 */

/*
 * Helper macro that defines whether MQTT client is configured to send some data
 * to broker when initializing a MQTT connection.
 *
 * This is commonly needed for asynchronous sensors. When device connects to the
 * network, it must send a sensor current state, if relevant.
 */
#if ENABLE_DEVICE_CLASS_SENSOR && SENSOR_TYPE_ASYNCHRONOUS
  #define MQTTCLIENT_PUBLISH_INITIAL_STATE     1
#elif ENABLE_DEVICE_CLASS_REACTOR && REACTOR_RESPOND
  #define MQTTCLIENT_PUBLISH_INITIAL_STATE     1
#else
  #define MQTTCLIENT_PUBLISH_INITIAL_STATE     0
#endif

/**
 * MQTT client state.
 */
enum mqttclient_state {
    MQTTCLIENT_BROKER_DISCONNECTED,             /**< Client is disconnected from broker server. */
    MQTTCLIENT_BROKER_DISCONNECTED_WAIT,        /**< Client is disconnected from server and is waiting some to to next connection try. */
    MQTTCLIENT_BROKER_CONNECTING,               /**< Client is connecting. */
    MQTTCLIENT_BROKER_CONNECTION_ESTABLISHED,   /**< Connection sucessfully established. */
    MQTTCLIENT_BROKER_DISCONNECTING,            /**< Client is disconnecting. */
};

/**
 * Phases of communication with MQTT broker.
 */
enum mqttclient_comm_state {
    MQTTCLIENT_COMM_INIT,                   /**< Initial phase. */
    MQTTCLIENT_COMM_CONNECTED,              /**< Messages CONNECT and CONNACK
                                                    exchanged successfully. */
    MQTTCLIENT_COMM_INIT_SEQ_PUBLISHED,     /**< Initial PUBLISH messages sent. */
#if MQTTCLIENT_PUBLISH_INITIAL_STATE
    MQTTCLIENT_COMM_INIT_STATE_PUBLISHED,   /**< Initial sensor state published. */
#endif
    MQTTCLIENT_COMM_SUBSCRIBED,             /**< Subscribed to MQTT topics. */
    MQTTCLIENT_COMM_OPERATIONAL,            /**< MQTT client is operational. */
};

/**
 * Initialize MQTT client.
 */
void ICACHE_FLASH_ATTR mqttclient_init(void);

/**
 * Start MQTT client main logic.
 */
void ICACHE_FLASH_ATTR mqttclient_start(void);

/**
 * Stop MQTT client
 */
void ICACHE_FLASH_ATTR mqttclient_stop(void);

#endif
