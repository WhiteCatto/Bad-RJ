#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * Passive ARP Scan — discovers hosts by listening for ARP traffic that's
 * already on the wire (requests, replies, gratuitous ARPs, and the ARP
 * probes Windows/etc. send during their own APIPA/link-local negotiation),
 * instead of actively probing every address in a subnet.
 *
 * Unlike active ARP Scan, this needs no IP configured on the Flipper at
 * all — MACRAW capture works regardless of the chip's own address — and
 * it scales to huge subnets (e.g. APIPA's /16) since it never has to
 * enumerate a range.
 */

/* Max unique hosts recorded during a passive listen (heap-allocated) */
#define ARP_PASSIVE_MAX_HOSTS 32

typedef struct {
    uint8_t ip[4];
    uint8_t mac[6];
} ArpPassiveHost;

typedef struct {
    ArpPassiveHost* hosts; /* heap-allocated, capacity = max_hosts */
    uint16_t max_hosts;
    uint16_t count;
    uint16_t total_seen; /* total qualifying ARP packets observed, including repeats */
} ArpPassiveState;

/** Allocate the host array (capacity = max_hosts). Returns false on OOM. */
bool arp_scan_passive_init(ArpPassiveState* state, uint16_t max_hosts);

/** Free the host array. Safe to call on a zeroed/already-freed state. */
void arp_scan_passive_free(ArpPassiveState* state);

/**
 * Process one raw Ethernet frame. If it's a valid ARP packet (any opcode)
 * with a non-zero sender IP, records or refreshes that sender's IP+MAC.
 * Returns true if a *new* host was recorded (list was not already full).
 */
bool arp_scan_passive_process_frame(ArpPassiveState* state, const uint8_t* frame, uint16_t len);

/**
 * Suggest a host IP inside [network_ip & mask, broadcast] that was not
 * observed as in-use by anything recorded in `state`. Starts from a
 * pseudo-random offset (seeded from `seed`, e.g. furi_get_tick()) so
 * repeated calls/devices don't all converge on the same address, and
 * skips the network/broadcast addresses.
 *
 * This is a suggestion only: nothing on the wire has actually verified
 * the address is free (that needs a real duplicate-address ARP probe,
 * same as APIPA itself does) — it just avoids anything we *know* is taken.
 *
 * Returns true and fills out_ip if a free-looking candidate was found.
 */
bool arp_scan_passive_suggest_ip(
    const ArpPassiveState* state,
    const uint8_t network_ip[4],
    const uint8_t mask[4],
    uint32_t seed,
    uint8_t out_ip[4]);
