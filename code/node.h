#ifndef __NODE_H__
#define __NODE_H__

#define node_update_state(state)    node_current_state = (state)

enum node_state {
    NODE_STATE_AP_ASSOCIATING,      /**< Node is associating with the access point. */
    NODE_STATE_IP_CONFIGURING,      /**< Node is obtainig IP address. */
    NODE_STATE_BROKER_CONNECTING,   /**< Node is connecting to broker. */
    NODE_STATE_OPERATIONAL,         /**< Node is connected to MQTT broker and ready to transmit data. */
};

extern enum node_state node_current_state;

#endif
