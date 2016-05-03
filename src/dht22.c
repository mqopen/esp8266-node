#include <c_types.h>
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include "dht22.h"

#define MAXTIMINGS 10000
#define BREAKTIME 20

struct dht22_data dht22_data;

void dht22_init(void) {
}

enum dht22_io_result dht22_read(void) {
    int counter = 0;
    int laststate = 1;
    int i = 0;
    int j = 0;
    int checksum = 0;

    int data[100];

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    GPIO_OUTPUT_SET(DHT22_GPIO_DQ, 1);
    os_delay_us(250000);
    GPIO_OUTPUT_SET(DHT22_GPIO_DQ, 0);
    os_delay_us(20000);
    GPIO_OUTPUT_SET(DHT22_GPIO_DQ, 1);
    os_delay_us(40);
    GPIO_DIS_OUTPUT(DHT22_GPIO_DQ);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
    while (GPIO_INPUT_GET(DHT22_GPIO_DQ) == 1 && i<100000) {
          os_delay_us(1);
          i++;
    }

    if (i==100000) {
      return DHT22_IO_WRITE_ADDRESS_ERROR;
    }

    for (i = 0; i < MAXTIMINGS; i++) {
        counter = 0;
        while ( GPIO_INPUT_GET(DHT22_GPIO_DQ) == laststate) {
            counter++;
                        os_delay_us(1);
            if (counter == 1000) {
                break;
            }
        }
        laststate = GPIO_INPUT_GET(DHT22_GPIO_DQ);
        if (counter == 1000) {
            break;
        }

        if ((i>3) && (i%2 == 0)) {
            data[j/8] <<= 1;
            if (counter > BREAKTIME) {
                data[j/8] |= 1;
            }
            j++;
        }
    }

    if (j >= 39) {
        checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
        if (data[4] == checksum) {
            /* yay! checksum is valid */

            dht22_data.humidity = data[0] * 256 + data[1];

            dht22_data.temperature = (data[2] & 0x7F)* 256 + data[3];
            if (data[2] & 0x80) {
                dht22_data.temperature = -dht22_data.temperature;
            }
        }
    }
    return DHT22_IO_OK;
}
