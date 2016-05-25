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
#include "dht.h"
#include "dht22.h"

#define MAXTIMINGS 10000
#define BREAKTIME 20

void dht22_init(void) {
}

enum dht_io_result dht22_read(struct dht_data *data) {
    int counter = 0;
    int laststate = 1;
    int i = 0;
    int j = 0;
    int checksum = 0;

    int _d[100];

    _d[0] = _d[1] = _d[2] = _d[3] = _d[4] = 0;

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
      return DHT_IO_OK;
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
            _d[j/8] <<= 1;
            if (counter > BREAKTIME) {
                _d[j/8] |= 1;
            }
            j++;
        }
    }

    if (j >= 39) {
        checksum = (_d[0] + _d[1] + _d[2] + _d[3]) & 0xFF;
        if (_d[4] == checksum) {
            /* yay! checksum is valid */

            data->humidity = _d[0] * 256 + _d[1];

            data->temperature = (_d[2] & 0x7F)* 256 + _d[3];
            if (_d[2] & 0x80) {
                data->temperature = -data->temperature;
            }
        }
    }
    return DHT_IO_OK;
}
