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

#include <c_types.h>
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include "common.h"
#include "dht.h"
#include "dht11.h"

#define MAXTIMINGS 10000
#define BREAKTIME 20

struct dht_data_raw {
    uint8_t humidity_msb;
    uint8_t humidity_lsb;
    uint8_t temperature_msb;
    uint8_t temperature_lsb;
    uint8_t checksum;
} __attribute__((__packed__));


/* Static function protypes. */
static inline void DHT_SDA_OUTPUT(void);
static inline void DHT_SDA_INPUT(void);
static inline void DHT_SDA_HIGH(void);
static inline void DHT_SDA_LOW(void);
static inline uint8_t DHT_SDA_READ(void);

void dht11_init(void) {
    DHT_SDA_OUTPUT();
    DHT_SDA_HIGH();
}

#define DHT_DATA_BYTE_LEN       5

enum dht_io_result dht11_read(struct dht_data *data) {

    uint8_t buf[5];
    uint16_t timeout;
    uint8_t result;
    uint8_t i;
    uint8_t j;

    /* Reset port. */
    //DHT_SDA_OUTPUT();
    //DHT_SDA_HIGH();
    //_delay_ms(100);

    /* Send request. */
    DHT_SDA_LOW();
    os_delay_us(1100);
    DHT_SDA_HIGH();
    DHT_SDA_INPUT();
    os_delay_us(40);

    if (DHT_SDA_READ())
        return DHT_IO_WRITE_ADDRESS_ERROR;
    os_delay_us(80);

    if (!DHT_SDA_READ())
        return DHT_IO_WRITE_ADDRESS_ERROR;
    os_delay_us(80);

    for (j = 0; j < DHT_DATA_BYTE_LEN; j++) {
        result = 0;
        for (i = 0; i < 8; i++) {
            timeout = 0;
            while (!DHT_SDA_READ()) {
                if (timeout++ > 200)
                    return DHT_IO_WRITE_ADDRESS_ERROR;
            }

            os_delay_us(30);
            if (DHT_SDA_READ())
                result |= _BV(7 - i);

            timeout = 0;
            while (DHT_SDA_READ()) {
                if(timeout++ > 200)
                    return DHT_IO_WRITE_ADDRESS_ERROR;
            }
        }
        *(((uint8_t *) buf) + j) = result;
    }

    /* Reset port. */
    DHT_SDA_OUTPUT();
    DHT_SDA_HIGH();

    os_printf("%d %d, %d %d, %d\n\r", buf[0], buf[1], buf[2], buf[3], buf[4]);
    return DHT_IO_OK;
}

static inline void DHT_SDA_OUTPUT(void) {
}

static inline void DHT_SDA_INPUT(void) {
}

static inline void DHT_SDA_HIGH(void) {
    GPIO_OUTPUT_SET(DHT_GPIO_DQ, 1);
}

static inline void DHT_SDA_LOW(void) {
    GPIO_OUTPUT_SET(DHT_GPIO_DQ, 0);
}

static inline uint8_t DHT_SDA_READ(void) {
    return !GPIO_INPUT_GET(DHT_GPIO_DQ);
}
