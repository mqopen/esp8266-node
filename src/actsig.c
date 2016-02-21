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

#include <osapi.h>
#include <c_types.h>
#include "gpio16.h"
#include "actsig.h"

/* Static function prototypes. */

/**
 * Handle scheduled signal timer.
 *
 * @param arg Signal object.
 */
static void ICACHE_FLASH_ATTR _actsig_handle_timer(void *arg);

/**
 * Toggle signal. Modify is_signaling attribute of signal object AFTER calling this
 * routine.
 *
 * @param signal Signal object.
 */
static void ICACHE_FLASH_ATTR _actsig_toggle(struct actsig_signal *signal);

/**
 * Turn signal on.
 *
 * @param signal Signal object.
 */
static void ICACHE_FLASH_ATTR _actsig_turn_on(struct actsig_signal *signal);

/**
 * Turn signal off.
 *
 * @param signal Signal object.
 */
static void ICACHE_FLASH_ATTR _actsig_turn_off(struct actsig_signal *signal);

/* Implementation. */

void ICACHE_FLASH_ATTR actsig_init(struct actsig_signal *signal, uint32_t period) {
    signal->period = period;
    os_timer_disarm(&signal->signal_timer);
    os_timer_setfn(&signal->signal_timer, _actsig_handle_timer, signal);

    signal->is_signaling = false;
    signal->normal_state = false;

    /* DEBUG */
    gpio16_output_conf();
}

void ICACHE_FLASH_ATTR actsig_notify(struct actsig_signal *signal) {
    if (!signal->is_signaling) {
        _actsig_toggle(signal);
        signal->is_signaling = true;
        os_timer_arm(&signal->signal_timer, signal->period, 0);
    }
}

void ICACHE_FLASH_ATTR actsig_set_normal_on(struct actsig_signal *signal) {
    signal->normal_state = true;
    _actsig_turn_on(signal);
}

void ICACHE_FLASH_ATTR actsig_set_normal_off(struct actsig_signal *signal) {
    signal->normal_state = false;
    _actsig_turn_off(signal);
}

static void ICACHE_FLASH_ATTR _actsig_handle_timer(void *arg) {
    _actsig_toggle((struct actsig_signal *) arg);
    ((struct actsig_signal *) arg)->is_signaling = false;
}

static void ICACHE_FLASH_ATTR _actsig_toggle(struct actsig_signal *signal) {
    if (signal->normal_state) {
        /* Normally on. */
        if (signal->is_signaling) {
            /* Currently off. */
            _actsig_turn_on(signal);
        } else {
            /* Currently on. */
            _actsig_turn_off(signal);
        }
    } else {
        /* Normally off. */
        if (signal->is_signaling) {
            /* Currently on. */
            _actsig_turn_off(signal);
        } else {
            /* Currently off. */
            _actsig_turn_on(signal);
        }
    }
}

static void ICACHE_FLASH_ATTR _actsig_turn_on(struct actsig_signal *signal) {
    gpio16_output_set(0);
}

static void ICACHE_FLASH_ATTR _actsig_turn_off(struct actsig_signal *signal) {
    gpio16_output_set(1);
}
