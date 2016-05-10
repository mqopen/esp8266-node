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

#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

#include <c_types.h>


/**
 * OneWire DQ pin.
 */
#define ONEWIRE_DQ_GPIO CONFIG_BUS_ONEWIRE_GPIO_DQ
#if CONFIG_BUS_ONEWIRE_GPIO_DQ == 0
  #define ONEWIRE_DQ_MUX PERIPHS_IO_MUX_GPIO0_U
  #define ONEWIRE_DQ_FUNC FUNC_GPIO0
#elif CONFIG_BUS_ONEWIRE_GPIO_DQ == 2
  #define ONEWIRE_DQ_MUX PERIPHS_IO_MUX_GPIO2_U
  #define ONEWIRE_DQ_FUNC FUNC_GPIO2
#elif CONFIG_BUS_ONEWIRE_GPIO_DQ == 14
  #define ONEWIRE_DQ_MUX PERIPHS_IO_MUX_MTMS_U
  #define ONEWIRE_DQ_FUNC FUNC_GPIO14
#else
  #error Unsupported OneWire DQ pin number!
#endif

/**
 * Initialize onewire bus.
 *
 * @param pin GPIO pin address.
 * @param ddr Address of ddr register.
 * @param port Address of PORT register.
 */
void onewire_init();

/**
 * Perform onewire reset sequence.
 *
 * @return Non-zero if presence pulse was detected, zero otherwise.
 */
uint8_t onewire_reset(void);

/**
 * Read byte.
 *
 * @return Readed byte.
 */
uint8_t onewire_read(void);

/**
 * Write byte.
 *
 * @param v Byte to write.
 */
void onewire_write(uint8_t v);

/**
 * Write sigle bit.
 */
void onewire_write_bit(uint8_t v);

/**
 * Read single bit.
 */
uint8_t onewire_read_bit(void);

#endif
