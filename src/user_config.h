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

#include "common.h"

/** Sensor IP. */
#define CONFIG_USE_DHCP             0
#if ! CONFIG_USE_DHCP
#define CONFIG_CLIENT_IP_ADDRESS0   10
#define CONFIG_CLIENT_IP_ADDRESS1   0
#define CONFIG_CLIENT_IP_ADDRESS2   0
#define CONFIG_CLIENT_IP_ADDRESS3   54

#define CONFIG_CLIENT_IP_NETMASK0   255
#define CONFIG_CLIENT_IP_NETMASK1   255
#define CONFIG_CLIENT_IP_NETMASK2   255
#define CONFIG_CLIENT_IP_NETMASK3   0

#define CONFIG_CLIENT_IP_GATEWAY0   10
#define CONFIG_CLIENT_IP_GATEWAY1   0
#define CONFIG_CLIENT_IP_GATEWAY2   0
#define CONFIG_CLIENT_IP_GATEWAY3   138
#endif

/** Broker IP. */
#define CONFIG_MQTT_BROKER_IP_ADDRESS0  10
#define CONFIG_MQTT_BROKER_IP_ADDRESS1  0
#define CONFIG_MQTT_BROKER_IP_ADDRESS2  0
#define CONFIG_MQTT_BROKER_IP_ADDRESS3  21

#define CONFIG_PROC_TASK_QUEUE_LENGTH   1

/** MQTT client ID. */
#define _CONFIG_MQTT_CLIENT_ID         chrudim-kitchen-bmp
#define CONFIG_MQTT_CLIENT_ID          "" STR(_CONFIG_MQTT_CLIENT_ID) ""

/** Topics for temperature and pressure. */
#define CONFIG_MQTT_TOPIC_TEMPERATURE   "kitchen/temperature"
#define CONFIG_MQTT_TOPIC_PRESSURE      "kitchen/pressure"

#define CONFIG_MQTT_NODE_PRESENCE_TOPIC        "presence/" STR(_CONFIG_MQTT_CLIENT_ID)

#endif
