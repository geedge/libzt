#ifndef LWIP_LINUX_UNIX_TAP_NETWORK_INTERFACE_H
#define LWIP_LINUX_UNIX_TAP_NETWORK_INTERFACE_H

#include "lwip/netif.h"

void init_default_netif(const ip4_addr_t* ipaddr, const ip4_addr_t* netmask, const ip4_addr_t* gw);

netif& get_default_netif();


#endif //LWIP_LINUX_UNIX_TAP_NETWORK_INTERFACE_H