#ifndef __SENSOR_BMP180_H__
#define __SENSOR_BMP180_H__

#define sensor_init bmp180_init

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
