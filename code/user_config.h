#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define CONFIG_WIFI_SSID        "hostapd"
#define CONFIG_WIFI_PASSWORD    "password"

#define CONFIG_USE_DHCP     0
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

#define CONFIG_MQTT_BROKER_IP_ADDRESS0  192
#define CONFIG_MQTT_BROKER_IP_ADDRESS1  168
#define CONFIG_MQTT_BROKER_IP_ADDRESS2  10
#define CONFIG_MQTT_BROKER_IP_ADDRESS3  1
#define CONFIG_MQTT_BROKER_IP_PORT      1883

#define CONFIG_PROCESS_TASK_PRIORITY    0
#define CONFIG_PROC_TASK_QUEUE_LENGTH   1

#define CONFIG_MQTT_KEEP_ALIVE_INTERVAL_MS  2000
#define CONFIG_MQTT_PUBLISH_INTERVAL_MS     2000

#define CONFIG_MQTT_KEEP_ALIVE          10
#define CONFIG_MQTT_CLIENT_ID           "esp8266-broker"

#endif
