#ifndef __NODE_H__
#define __NODE_H__

enum node_state {
    NODE_STATE_AP_ASSOCIATING,      /**< Node is associating with the access point. */
    NODE_STATE_IP_CONFIGURING,      /**< Node is obtainig IP address. */
    NODE_STATE_BROKER_CONNECTING,   /**< Node is connecting to broker. */
};

#endif
