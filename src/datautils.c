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
#include "common.h"
#include "datautils.h"

uint8_t datautils_data_to_int32(int32_t *val, const uint8_t *buf, uint16_t len, uint8_t decimal_digts) {
    int32_t _val = 0;
    uint16_t _i = 0;
    int8_t _digit;
    uint8_t _negative = 0;
    uint8_t _has_decimal_dot = 0;
    char _c;
    while (len--) {
        _c = buf[_i];
        switch (_c) {

            /* Negative sign. */
            case '-':
                if (_i) {
                    /* Dash is not first character. Return error code. */
                    return 1;
                } else {
                    if (len) {
                        /* Negative number. */
                        _negative = 1;
                    } else {
                        /* Data is dash only. */
                        return 1;
                    }
                }
                break;

            /* Decimal dot. */
            case '.':
                if (_has_decimal_dot) {
                    /* Decimall dot is already parsed. Return error code. */
                    return 1;
                }
                _has_decimal_dot = 1;

                /* Read maximum of decimal_digts characters. */
                len = min(decimal_digts, len);
                break;

            /* Digits. */
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                _digit = _c - '0';
                _val += _digit;
                if (len) {
                    _val *= 10;
                }

                if (_has_decimal_dot && decimal_digts) {
                    decimal_digts--;
                }

                break;
            default:
                /* Character isn't digit. Return error code. */
                return 1;
        }
        _i++;
    }

    /* Fixed point arithemtic. */
    while (decimal_digts--) {
        _val *= 10;
    }

    if (_negative) {
        *val = -_val;
    } else {
        *val = _val;
    }
    return 0;
}
