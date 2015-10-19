#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <c_types.h>

/**
 * Init network.
 */
void ICACHE_FLASH_ATTR network_init(void);

/**
 * Connect to AP.
 */
void ICACHE_FLASH_ATTR network_connect(void);

/**
 * Check IP connectivity.
 */
void ICACHE_FLASH_ATTR network_check_ip(void);

#endif
