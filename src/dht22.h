#ifndef __DHT22_H__
#define __DHT22_H__

#include <c_types.h>

/** GPIO DG pin. */
#define DHT22_GPIO_DQ   2

enum dht22_io_result {
    DHT22_IO_OK,                        /**< Communication is OK. */
    DHT22_IO_WRITE_ADDRESS_ERROR,       /**< Write address not acknowledged. */
    DHT22_IO_WRITE_REGISTER_ERROR,      /**< Write of destination register not acknowledged. */
    DHT22_IO_WRITE_VALUE_ERROR,         /**< Write of register value not acknowledged (Write operation only). */
    DHT22_IO_READ_ADDRESS_ERROR,        /**< Read address not acknowledged. */
    DHT22_IO_INVALID_DATA,              /**< Invalid data. */
};

struct dht22_data {
    uint16_t humidity;
    int16_t temperature;
};

extern struct dht22_data dht22_data;

void dht22_init(void);

enum dht22_io_result dht22_read(void);

#endif
