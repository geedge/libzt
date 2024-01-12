#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/tcpip.h"
extern "C" {
#include "netif/tapif.h"
}

static netif default_network_interface;

void init_default_netif(const ip4_addr_t* ipaddr, const ip4_addr_t* netmask, const ip4_addr_t* gw)
{
    netif_add(&default_network_interface, ipaddr, netmask, gw, nullptr, tapif_init, tcpip_input);

    netif_set_default(&default_network_interface);
}

netif& get_default_netif()
{
    return default_network_interface;
}
