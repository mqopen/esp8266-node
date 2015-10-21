#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <c_types.h>

/**
 * Network states.
 */
enum network_state {
    NETWORK_STATE_INIT,             /**< Network init phase. */
    NETWORK_STATE_AP_ASSOCIATING,   /**< Associating with access point. */
    NETWORK_STATE_IP_CONFIGURING,   /**< Configuring device IP address. */
    NETWORK_STATE_UP,               /**< Network connectivity is up. */
};

/**
 * Current network state.
 */
extern enum network_state network_state;

/**
 * Module responsible for IP setup for future connection with MQTT broker.
 */

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

/**
 * Send data over network.
 */
void ICACHE_FLASH_ATTR network_send(void *buf, uint16_t len);
#endif
