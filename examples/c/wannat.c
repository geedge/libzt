/**
 * libzt C API example
 *
 * Pingable node
 */

#include "ZeroTierSockets.h"

#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>

void process_packet(char* buffer, int len)
{
    struct iphdr* iph = (struct iphdr*)buffer;

    unsigned char bytes[4];
    bytes[0] = iph->daddr & 0xFF;
    bytes[1] = (iph->daddr >> 8) & 0xFF;
    bytes[2] = (iph->daddr >> 16) & 0xFF;
    bytes[3] = (iph->daddr >> 24) & 0xFF;

    printf("Destination IP: %d.%d.%d.%d\n", bytes[0], bytes[1], bytes[2], bytes[3]);
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("\nUsage:\n");
        printf("pingable-node <net_id>\n");
        exit(0);
    }
    long long int net_id = strtoull(argv[1], NULL, 16);   // At least 64 bits

    int err = ZTS_ERR_OK;
    char* storage_path = "./";

    if ((err = zts_init_from_storage(storage_path)) != ZTS_ERR_OK) {
        printf("Unable to start service, error = %d. Exiting.\n", err);
        exit(1);
    }

    printf("Starting node...\n");
    zts_node_start();

    printf("Waiting for node to come online\n");
    while (! zts_node_is_online()) {
        zts_util_delay(50);
    }

    printf("My public identity (node ID) is %llx\n", (long long int)zts_node_get_id());
    char keypair[ZTS_ID_STR_BUF_LEN] = { 0 };
    unsigned int len = ZTS_ID_STR_BUF_LEN;
    if (zts_node_get_id_pair(keypair, &len) != ZTS_ERR_OK) {
        printf("Error getting identity keypair. Exiting.\n");
    }
    printf("Identity [public/secret pair] = %s\n", keypair);

    printf("Joining network %llx\n", net_id);
    if (zts_net_join(net_id) != ZTS_ERR_OK) {
        printf("Unable to join network. Exiting.\n");
        exit(1);
    }

    printf("Waiting for join to complete\n");
    while (! zts_net_transport_is_ready(net_id)) {
        zts_util_delay(50);
    }

    printf("Waiting for address assignment from network\n");
    while (! (err = zts_addr_is_assigned(net_id, ZTS_AF_INET))) {
        zts_util_delay(500);
    }

    char ipstr[ZTS_IP_MAX_STR_LEN] = { 0 };
    zts_addr_get_str(net_id, ZTS_AF_INET, ipstr, ZTS_IP_MAX_STR_LEN);
    printf("Join %llx from another machine and ping me at %s\n", net_id, ipstr);

    // Do network stuff!
    // zts_bsd_socket, zts_bsd_connect, etc

    int raw_fd;
    char buffer[1500];   // Buffer to hold the incoming packets
    // struct zts_sockaddr_in sa_zts;
    // int len_temp = sizeof(sa_zts);

    raw_fd = zts_bsd_socket(ZTS_AF_INET, ZTS_SOCK_RAW, ZTS_IPPROTO_IP);

    if (raw_fd < 0) {
        printf("Error creating raw socket.\n");
        return -1;
    }
    char *interface = "tp0";
    if (zts_bsd_setsockopt(raw_fd, ZTS_SOL_SOCKET, ZTS_SO_BINDTODEVICE, interface, strlen(interface)) < 0) {
        printf("Error binding to interface");
        return 1;
    }

    // struct zts_sockaddr_in sa_zts;
    // zts_socklen_t sa_len = sizeof(sa_zts);
    // sa_zts.sin_family = ZTS_AF_INET;
    // sa_zts.sin_port = 0;
    // sa_zts.sin_addr.s_addr = htonl(ZTS_INADDR_ANY);

    // if (zts_bsd_bind(raw_fd, (struct sockaddr*)&sa_zts, sizeof(sa_zts)) < 0) {
    //     printf("Error bind");
    //     return 1;
    // }

    while (1) {
        int recv_len = zts_bsd_recvfrom(raw_fd, buffer, sizeof(buffer), 0, NULL,
                                        NULL);   // Receive a packet
        if (recv_len > 0) {
            process_packet(buffer, recv_len);   // Process the packet
        }
    }

    printf("Stopping node\n");
    return zts_node_stop();
}
