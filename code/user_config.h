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

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

/** Wi-Fi configuration. */
#define CONFIG_WIFI_SSID        "hostapd"
#define CONFIG_WIFI_PASSWORD    "password"

/** Sensor IP. */
#define CONFIG_USE_DHCP             0
#if ! CONFIG_USE_DHCP
#define CONFIG_CLIENT_IP_ADDRESS0   192
#define CONFIG_CLIENT_IP_ADDRESS1   168
#define CONFIG_CLIENT_IP_ADDRESS2   10
#define CONFIG_CLIENT_IP_ADDRESS3   200

#define CONFIG_CLIENT_IP_NETMASK0   255
#define CONFIG_CLIENT_IP_NETMASK1   255
#define CONFIG_CLIENT_IP_NETMASK2   255
#define CONFIG_CLIENT_IP_NETMASK3   0

#define CONFIG_CLIENT_IP_GATEWAY0   192
#define CONFIG_CLIENT_IP_GATEWAY1   168
#define CONFIG_CLIENT_IP_GATEWAY2   10
#define CONFIG_CLIENT_IP_GATEWAY3   1
#endif

/** Broker IP. */
#define CONFIG_MQTT_BROKER_IP_ADDRESS0  192
#define CONFIG_MQTT_BROKER_IP_ADDRESS1  168
#define CONFIG_MQTT_BROKER_IP_ADDRESS2  10
#define CONFIG_MQTT_BROKER_IP_ADDRESS3  1
#define CONFIG_MQTT_BROKER_IP_PORT      1883

#define CONFIG_PROC_TASK_QUEUE_LENGTH   1

/** MQTT mesaage timing. */
#define CONFIG_MQTT_KEEP_ALIVE_INTERVAL_MS  30000
#define CONFIG_MQTT_PUBLISH_INTERVAL_MS     2000

#define CONFIG_MQTT_KEEP_ALIVE          60

/** MQTT client ID. */
#define CONFIG_MQTT_CLIENT_ID           "esp8266-broker"

/** Topics for temperature and pressure. */
#define CONFIG_MQTT_TOPIC_TEMPERATURE   "test/temperature"
#define CONFIG_MQTT_TOPIC_PRESSURE      "test/pressure"

/** LED notify interval. */
#define CONFIG_MQTT_ACTIVE_LED_INTERVAL_MS  100

#endif
