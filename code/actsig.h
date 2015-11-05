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

#ifndef __ACTSIG_H__
#define __ACTSIG_H__

#include <stdbool.h>
#include <os_type.h>

struct actsig_signal {
    uint32_t period;
    bool is_signaling;
    bool normal_state;
    os_timer_t signal_timer;
};

void ICACHE_FLASH_ATTR actsig_init(struct actsig_signal *signal, uint32_t period);
void ICACHE_FLASH_ATTR actsig_notify(struct actsig_signal *signal);
void ICACHE_FLASH_ATTR actsig_set_normal_on(struct actsig_signal *signal);
void ICACHE_FLASH_ATTR actsig_set_normal_off(struct actsig_signal *signal);

#endif
