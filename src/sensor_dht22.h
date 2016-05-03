#ifndef __SENSOR_DHT22_H__
#define __SENSOR_DHT22_H__

#include "dht22.h"

#define sensor_init dht22_init

/**
 * Possible values are:
 *  - 'xxx.xxx'             : len = 7 (temperature, humidity)
 *  - 'E_WRITE_ADDRESS'     : len = 15
 *  - 'E_WRITE_REGISTER'    : len = 16
 *  - 'E_WRITE_VALUE'       : len = 13
 *  - 'E_READ_ADDRESS'      : len = 14
 *  - 'E_INVALID_DATA'      : len = 14
 *
 * maxium possible length: 16 Bytes
 */
#define SENSOR_VALUE_BUFFER_SIZE    16

#endif
