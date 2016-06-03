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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <c_types.h>

#define _BV(b)      (1<<(b))
#define abs(x)      ((x)<0?(-x):(x))
#define max(x, y)   ((x)<(y)?(y):(x))

#define _STR(x) #x
#define STR(x) _STR(x)

#define TOPIC(t)            CONFIG_GENERAL_DEVICE_LOCATION "/" t
#define __topic_service     "i/" CONFIG_GENERAL_DEVICE_NAME
#define __topic_presence    __topic_service "/presence"
#define __topic_arch        __topic_service "/arch"
#define __topic_variant     __topic_service "/variant"
#define __topic_link        __topic_service "/link"
#define __topic_ip          __topic_service "/ip"
#define __topic_class       __topic_service "/class"
#define __topic_sensor      __topic_service "/sensor"
#define __topic_fwversion   __topic_service "/fwversion"
#define __topic_hwversion   __topic_service "/hwversion"


//#define TOPIC_PRESENCE(t)   "presence/" t

/** Get length of string. */
#define __sizeof_str(s)     (sizeof(s) - 1)

#endif
