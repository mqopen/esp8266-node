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
 * Initialize MQTT client.
 */
void ICACHE_FLASH_ATTR mqttclient_init(void);

/**
 * Start MQTT client main logic.
 */
void ICACHE_FLASH_ATTR mqttclient_start(void);
//void mqttclient_notify_broker_unreachable(void);
//void mqttclient_process(void);
//void mqttclient_appcall(void);

#endif
