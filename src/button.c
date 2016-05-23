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
#include <eagle_soc.h>
#include <gpio.h>
#include <ets_sys.h>
#include "sensor.h"
#include "sensor_button.h"
#include "button.h"

static void _button_interrupt_handler(uint32_t intr_mask, void *arg);

void button_init(void) {
    gpio_init();
    gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_ANYEDGE);
    gpio_pin_intr_state_set(GPIO_ID_PIN(4), GPIO_PIN_INTR_ANYEDGE);
    ETS_GPIO_INTR_ATTACH(_button_interrupt_handler, NULL);
    ETS_GPIO_INTR_ENABLE();
}


static void _button_interrupt_handler(uint32_t intr_mask, void *arg) {
    os_printf("interrupt 0x%08x\r\n", intr_mask);
    //gpio_intr_ack(0);


    uint32 gpio_status;
    gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    os_printf("status 0x%08x\r\n", gpio_status);

    sensor_button_notify(BUTTON_ID_1, BUTTON_STATE_HIGH);

    //clear interrupt status
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
}
