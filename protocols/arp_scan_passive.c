#include "arp_scan_passive.h"
#include "../utils/packet_utils.h"

#include <furi.h>
#include <stdlib.h>
#include <string.h>

#define TAG "ARP_PASSIVE"

#define ARP_HTYPE_ETHERNET 0x0001
#define ARP_PTYPE_IPV4     0x0800

bool arp_scan_passive_init(ArpPassiveState* state, uint16_t max_hosts) {
    memset(state, 0, sizeof(ArpPassiveState));
    state->hosts = malloc(sizeof(ArpPassiveHost) * max_hosts);
    if(!state->hosts) return false;
    state->max_hosts = max_hosts;
    return true;
}

void arp_scan_passive_free(ArpPassiveState* state) {
    if(state->hosts) {
        free(state->hosts);
        state->hosts = NULL;
    }
    state->max_hosts = 0;
    state->count = 0;
}

bool arp_scan_passive_process_frame(ArpPassiveState* state, const uint8_t* frame, uint16_t len) {
    if(len < ETH_HEADER_SIZE) return false;

    uint16_t ethertype = pkt_get_ethertype(frame);
    uint8_t sender_mac[6];
    uint8_t sender_ip[4];

    if(ethertype == ETHERTYPE_ARP) {
        if(len < 42) return false; /* minimum ARP frame: 14 Ethernet + 28 ARP */
        const uint8_t* arp = frame + ETH_HEADER_SIZE;
        if(pkt_read_u16_be(&arp[0]) != ARP_HTYPE_ETHERNET) return false;
        if(pkt_read_u16_be(&arp[2]) != ARP_PTYPE_IPV4) return false;
        if(arp[4] != 6 || arp[5] != 4) return false;
        memcpy(sender_mac, &arp[8], 6);
        memcpy(sender_ip, &arp[14], 4);
    } else if(ethertype == ETHERTYPE_IPV4) {
        /* Any IPv4 traffic reveals its sender too, not just ARP resolution
         * moments. Needed because a host with an already-warm ARP cache
         * entry (e.g. mid `ping -t` from earlier testing) sends no new ARP
         * packets at all — ARP-only listening can miss it completely even
         * with heavy traffic on the wire. */
        if(len < ETH_HEADER_SIZE + 20) return false; /* minimum IPv4 header */
        const uint8_t* ip_hdr = frame + ETH_HEADER_SIZE;
        if((ip_hdr[0] >> 4) != 4) return false; /* version nibble != 4 */
        memcpy(sender_mac, frame + 6, 6); /* Ethernet source MAC */
        memcpy(sender_ip, &ip_hdr[12], 4); /* IP source address */
    } else {
        return false;
    }

    /* Skip 0.0.0.0 sender — that's a duplicate-address probe, not a real host */
    if(sender_ip[0] == 0 && sender_ip[1] == 0 && sender_ip[2] == 0 && sender_ip[3] == 0) {
        return false;
    }

    state->total_seen++;

    /* Refresh MAC if we've already recorded this IP, otherwise add it */
    for(uint16_t i = 0; i < state->count; i++) {
        if(memcmp(state->hosts[i].ip, sender_ip, 4) == 0) {
            memcpy(state->hosts[i].mac, sender_mac, 6);
            return false;
        }
    }

    if(state->count >= state->max_hosts) return false;

    ArpPassiveHost* host = &state->hosts[state->count];
    memcpy(host->ip, sender_ip, 4);
    memcpy(host->mac, sender_mac, 6);
    state->count++;
    return true;
}

static bool arp_passive_ip_known(const ArpPassiveState* state, uint32_t candidate) {
    for(uint16_t i = 0; i < state->count; i++) {
        if(pkt_read_u32_be(state->hosts[i].ip) == candidate) return true;
    }
    return false;
}

bool arp_scan_passive_suggest_ip(
    const ArpPassiveState* state,
    const uint8_t network_ip[4],
    const uint8_t mask[4],
    uint32_t seed,
    uint8_t out_ip[4]) {
    uint32_t ip_addr = pkt_read_u32_be(network_ip);
    uint32_t mask_addr = pkt_read_u32_be(mask);
    uint32_t network = ip_addr & mask_addr;
    uint32_t broadcast = network | ~mask_addr;

    uint32_t first_host = network + 1;
    uint32_t last_host = broadcast - 1;
    if(first_host >= last_host) return false;

    uint32_t num_hosts = last_host - first_host + 1;
    uint32_t start_offset = seed % num_hosts;

    for(uint32_t i = 0; i < num_hosts; i++) {
        uint32_t candidate = first_host + ((start_offset + i) % num_hosts);
        if(!arp_passive_ip_known(state, candidate)) {
            pkt_write_u32_be(out_ip, candidate);
            return true;
        }
    }
    return false;
}
