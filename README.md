# Bad-RJ — Flipper Zero LAN Tester (W5500)

Turn your **Flipper Zero + W5500 Lite** module into a professional-grade portable LAN tester. Analyze Ethernet links, discover network neighbors, scan subnets, fingerprint DHCP servers --- all from a pocket-sized device.

![Flipper Zero](https://img.shields.io/badge/Flipper%20Zero-OFW-orange)
![License](https://img.shields.io/badge/license-MIT-blue)
![Language](https://img.shields.io/badge/language-C99-green)
![Build](https://img.shields.io/badge/build-ufbt-yellow)
![Version](https://img.shields.io/badge/version-2.9.0-brightgreen)

![Main menu](docs/screenshots/main_menu.png)

---

## Features

| Feature | Description |
|---|---|
| **Auto Test** | One-touch automated diagnostics: Link → DHCP → Ping GW → DNS → Internet Ping → LLDP/CDP → ARP count. Auto-cycles on cable replug. |
| **Link Info** | PHY link status, speed (10/100 Mbps), duplex (Half/Full), MAC address, W5500 version check |
| **DHCP Analyzer** | Discover-only analysis (no IP lease taken), option fingerprinting, full offer parsing |
| **ARP Scanner** | Active subnet scan with batch requests, OUI vendor lookup (~120 vendors), duplicate detection |
| **Ping** | Echo request/reply to any IP with configurable count and timeout |
| **Continuous Ping** | Real-time RTT graph with min/max/avg and packet loss, configurable interval |
| **DNS Lookup** | Resolve hostnames via UDP DNS, supports custom DNS server |
| **Traceroute** | ICMP-based hop-by-hop path discovery, accepts IPs and hostnames with DNS resolve |
| **Ping Sweep** | ICMP sweep of an entire subnet with interactive host list — click to ping, scan, or WOL |
| **Port Scanner** | TCP connect scan: Top-20, Top-100 presets, or custom port range (1-65535) |
| **LLDP/CDP** | Passive IEEE 802.1AB & Cisco CDP neighbor discovery with full TLV parsing |
| **mDNS/SSDP** | Discover services and devices via multicast DNS and UPnP/SSDP |
| **STP/VLAN** | Passive BPDU listener + 802.1Q VLAN tag detection |
| **Statistics** | Frame counters by type (unicast/broadcast/multicast) and EtherType |
| **Wake-on-LAN** | Send magic packets to any MAC address |
| **Packet Capture** | Standalone PCAP traffic dump — capture raw Ethernet frames to .pcap file on SD card |
| **ETH Bridge** | USB-to-Ethernet bridge: phone/PC gets LAN access via Flipper (CDC-ECM), optional PCAP traffic dump to SD card |
| **PXE Download** | Download iPXE and EFI boot files from the internet directly to SD card, for use with your own PXE/DHCP infrastructure |
| **File Manager** | Web-based file manager: browse, download, upload, delete files on microSD via HTTP from any browser on the LAN. Supports custom CSS/JS themes from SD card |
| **SNMP GET** | Query device info via SNMPv1/v2c: sysName, sysDescr, sysUpTime, ifOperStatus |
| **NTP Diagnostics** | NTP server analysis: stratum, root delay/dispersion, reference ID, RTT, UTC time |
| **Apply NTP Sync** | Apply NTP time from last NTP Diagnostics result to Flipper clock |
| **NetBIOS Query** | Discover Windows machine names, workgroups, and MAC addresses |
| **DNS Poison Check** | Compare local vs public DNS (8.8.8.8) to detect poisoning or split-horizon |
| **ARP Watch** | Passive ARP monitoring: detect spoofing, duplicate IPs, gratuitous ARP storms |
| **Rogue DHCP** | Send Discover, collect Offers from multiple servers, detect unauthorized DHCP |
| **Rogue RA** | Listen for IPv6 Router Advertisements, detect unauthorized routers |
| **DHCP Fingerprint** | Identify client OS by DHCP option 55 (Windows, Linux, macOS, Android, etc.) |
| **802.1X Probe** | Send EAPOL-Start to detect 802.1X port authentication, identify EAP type |
| **VLAN Hopping** | Send 802.1Q tagged frames to test VLAN isolation (Top 10 / Custom VLANs) |
| **TFTP Client** | Download config files from network equipment via TFTP, save to SD card |
| **IPMI v1.5** | Query BMC: chassis power status, device ID, firmware version |
| **History** | All scan results auto-saved with timestamps, browsable and deletable |
| **Settings** | Auto-save, sound/vibro, custom DNS, static IP / Network Mode config, ping config, target persistence, MAC Changer |

### UX Highlights

- **Hierarchical menu**: features grouped into Port Info, Scan, Diagnostics, Traffic, Security, Utilities
- **Link status in header**: see UP/DOWN, speed, duplex without entering Link Info
- **DHCP caching**: single negotiation shared across all operations — no repeated 15s waits
- **Visual progress**: countdown timers for listeners, ASCII progress bars for scans
- **LED/vibro feedback**: green blink on success, red on error (optional, toggle in Settings)
- **Smart defaults**: IP inputs pre-populated with DHCP gateway, last-used targets remembered across sessions

## Hardware

### Required

- **Flipper Zero** — Official firmware (OFW), confirmed working on release 1.4.3
- **W5500 Lite** Ethernet module (or any W5500-based board with SPI)

### Firmware Compatibility

Built and tested against **Official Flipper firmware 1.4.3** (API 87.1). Custom firmware forks (e.g. Unleashed) use a different API level — if you build for one, `ufbt` must target that firmware's own SDK index or the `.fap` won't be recognized by the Apps menu.

### Where to buy

- [W5500 Ethernet Module for Flipper Zero](https://flipperaddons.com/product/w5500-ethernet/) — ready-to-use module with RJ45

### Wiring

```
W5500 Module    Flipper Zero GPIO
─────────────   ─────────────────
MOSI (MO)   →   A7  (pin 2)
SCLK (SCK)  →   B3  (pin 5)
CS   (nSS)  →   A4  (pin 4)
MISO (MI)   →   A6  (pin 3)
RESET (RST) →   C3  (pin 7)
3V3  (VCC)  →   3V3 (pin 9)
GND  (G)    →   GND (pin 8 or 11)
```

> The W5500 is powered via Flipper's OTG 3.3V output, which is enabled automatically when the app starts.

## Building

### Prerequisites

- [ufbt](https://github.com/flipperdevices/flipperzero-ufbt) (micro Flipper Build Tool)

### Build & Install

```bash
cd lan_tester
ufbt build              # build only
ufbt launch             # build and run on Flipper via USB
ufbt install            # install .fap to Flipper's SD card
```

The compiled `.fap` file will appear in `dist/`. You can also copy it manually to the Flipper's SD card at `/ext/apps/GPIO/`.

## Architecture

```
├── application.fam              # FAP manifest
├── lan_tester_app.c             # Entry point, ViewDispatcher, feature logic
├── lan_tester_app.h             # Shared types and app state
│
├── hal/
│   ├── w5500_hal.c              # SPI, GPIO, MACRAW socket management
│   └── w5500_hal.h
│
├── usb_eth/
│   ├── usb_eth.c / .h           # USB CDC-ECM network device (init/deinit/send/recv)
│   └── usb_descriptors.c / .h   # USB device & config descriptors, endpoint callbacks
│
├── bridge/
│   ├── eth_bridge.c             # Bidirectional L2 frame forwarding engine
│   ├── eth_bridge.h
│   ├── pcap_dump.c              # PCAP traffic dump to SD card (Wireshark-compatible)
│   └── pcap_dump.h
│
├── protocols/
│   ├── lldp.c / lldp.h         # IEEE 802.1AB LLDP parser
│   ├── cdp.c / cdp.h           # Cisco CDP parser (LLC/SNAP)
│   ├── arp_scan.c / arp_scan.h  # ARP request builder & reply parser
│   ├── arp_watch.c / .h        # ARP spoofing & storm detection
│   ├── dhcp_discover.c / .h     # DHCP Discover builder & Offer parser
│   ├── dhcp_fingerprint.c / .h  # OS fingerprinting via DHCP option 55
│   ├── dns_lookup.c / .h       # DNS A-record resolver via UDP
│   ├── dns_poison.c / .h       # DNS poisoning check (local vs public)
│   ├── discovery.c / .h        # mDNS + SSDP service discovery
│   ├── eapol_probe.c / .h      # 802.1X EAPOL-Start probe
│   ├── icmp.c / icmp.h         # ICMP Echo (ping) via IPRAW
│   ├── ipmi_client.c / .h      # IPMI v1.5 over LAN (chassis, device ID)
│   ├── netbios_query.c / .h    # NetBIOS Name Query (NBSTAT)
│   ├── ntp_diag.c / .h         # NTP diagnostics (stratum, offset, RTT)
│   ├── port_scan.c / .h        # TCP connect port scanner
│   ├── rogue_dhcp.c / .h       # Rogue DHCP server detection
│   ├── rogue_ra.c / .h         # Rogue IPv6 Router Advertisement detection
│   ├── snmp_client.c / .h      # SNMP v1/v2c GET client (BER/ASN.1)
│   ├── stp_vlan.c / .h         # STP BPDU parser + 802.1Q VLAN detection
│   ├── tftp_client.c / .h      # TFTP file download client (RFC 1350)
│   ├── traceroute.c / .h       # ICMP traceroute with TTL
│   ├── vlan_hop.c / .h         # VLAN hopping test (802.1Q tagged frames)
│   ├── wol.c / .h              # Wake-on-LAN magic packet
│   ├── ping_graph.c / .h       # Ring buffer RTT graph for continuous ping
│   ├── mac_changer.c / .h      # Random/custom MAC with SD persistence
│   ├── lldp.c / .h             # IEEE 802.1AB LLDP parser
│   ├── cdp.c / .h              # Cisco Discovery Protocol parser
│   ├── http_download.c / .h    # HTTP file downloader (for PXE boot files)
│   ├── file_manager.c / .h    # Web-based SD card file manager (HTTP server)
│   └── history.c / .h          # Timestamped result storage on SD card
│
├── utils/
│   ├── oui_lookup.c / .h       # MAC → Vendor (top ~120 OUI prefixes)
│   └── packet_utils.c / .h     # Endian helpers, checksums, formatters
│
├── assets/
│   └── icon.png                 # 10x10 FAP icon
│
└── lib/
    └── ioLibrary_Driver/        # WIZnet W5500 driver
```

## Usage

1. Connect the W5500 module to Flipper Zero using the wiring diagram above
2. Plug an Ethernet cable into the W5500's RJ45 port
3. Open **GPIO → bad-rj** on the Flipper
4. The menu header shows link status (e.g. `LAN [UP 100M FD]`)
5. Select a category and then a tool:

### Port Info
- **Link Info** — link status, speed, duplex, MAC. Use first to verify hardware.
- **DHCP Analyze** — sends Discover, parses Offer. Does **not** take an IP lease.
- **LLDP/CDP** — listens up to 60s for switch neighbor advertisements.
- **STP/VLAN** — listens 30s for BPDU frames and 802.1Q VLAN tags.
- **SNMP GET** — query sysName, sysDescr, sysUpTime, ifStatus via SNMPv1/v2c.

### Scan
- **ARP Scan** — scans local subnet via DHCP-detected range, shows IP/MAC/vendor.
- **Ping Sweep** — ICMP sweep of a CIDR range, auto-detected or manually entered.
- **mDNS/SSDP** — discovers services via multicast DNS and UPnP.
- **NetBIOS Query** — discover Windows machine names and workgroups.
- **Port Scan (Top 20/100/Custom)** — TCP connect scan of common ports.

### Diagnostics
- **Ping** — 4 pings to any IP (default: gateway from DHCP).
- **Continuous Ping** — live RTT graph with loss tracking, runs until Back.
- **DNS Lookup** — resolves a hostname via the DHCP-provided DNS server.
- **Traceroute** — hop-by-hop ICMP path discovery up to 30 hops.
- **NTP Diagnostics** — stratum, root delay, reference ID, RTT, UTC time and clock diff.
- **Apply NTP Sync** — apply cached NTP time to Flipper clock (run NTP Diagnostics first).
- **DNS Poison Check** — compare local vs public DNS responses.

### Traffic
- **Packet Capture** — capture raw Ethernet frames to .pcap file on SD card.
- **ETH Bridge** — USB-to-Ethernet bridge via CDC-ECM with optional PCAP dump.
- **Statistics** — frame counters by type and EtherType (10s capture).

### Security
- **ARP Watch** — detect ARP spoofing, duplicate IPs, gratuitous ARP storms (15s scan).
- **Rogue DHCP** — send Discover, detect unauthorized DHCP servers.
- **Rogue RA (IPv6)** — listen for unauthorized Router Advertisements (15s scan).
- **DHCP Fingerprint** — identify client OS by option 55 parameter list (30s listen).
- **802.1X Probe** — send EAPOL-Start, detect port authentication and EAP type.
- **VLAN Hop Top10** — test VLAN isolation on common VLANs (1,2,10,20,50,100,150,200,300,999).
- **VLAN Hop Custom** — test user-specified VLAN IDs (comma-separated).
### Utilities
- **Wake-on-LAN** — send magic packet to wake a device by MAC address.
- **PXE Download** — download common PXE boot files (iPXE/EFI) to SD card for use with your own PXE/DHCP infrastructure.
- **File Manager** — web-based SD card file manager via HTTP on port 80.
- **TFTP Client** — download config files from network equipment to SD card.
- **IPMI Query** — query BMC chassis status, device ID, firmware version.

### Settings
- **Auto-save results** — ON/OFF, controls automatic history saving.
- **Sound & vibro** — ON/OFF, controls LED/vibro notifications.
- **Clear History** — delete all saved result files.
- **MAC Changer** — generate random MAC or enter custom, saved to SD.
- **Target persistence** — last-used IP/hostname per tool saved to settings.conf, restored on next launch.

### File Manager Custom Themes

File Manager supports custom CSS and JavaScript loaded from SD card. Place files in:

- **apps_data/lan_tester/web/custom.css** --- replaces default styles
- **apps_data/lan_tester/web/custom.js** --- replaces default sorting script

Files are detected once when File Manager starts. To apply changes, restart the tool.

Example themes and an advanced custom.js (with search, breadcrumbs, multi-select, drag-and-drop upload, file preview) are included in **docs/filemanager_themes/**. Copy the desired file to the path above on your SD card.

## Technical Details

- **W5500 MACRAW mode**: Socket 0 with `MFEN=0` (promiscuous --- receives all frames including multicast)
- **Worker thread**: 8 KB stack, non-blocking UI via ViewDispatcher + worker pattern
- **DHCP caching**: single negotiation, result reused across all subsequent operations
- **Memory-safe**: large buffers heap-allocated, frame buffer on heap (4 KB app stack), bounds checking on all parsers
- **Endianness**: manual big-endian parsing --- no float printf, no `htons`/`ntohs`

## OUI Vendor Database

The built-in lookup table covers ~120 common OUI prefixes including:

> Cisco, HP/HPE, Dell, Intel, Broadcom, Realtek, Apple, Samsung, Huawei, TP-Link, Ubiquiti, Juniper, Arista, MikroTik, Netgear, ASUS, D-Link, Synology, QNAP, VMware, Microsoft, Google, Amazon, Lenovo, Supermicro, Aruba, Fortinet, Palo Alto, WIZnet, Raspberry Pi, Espressif, and more.

## Credits

- **Bad-RJ** is a fork of [dok2d/fz-W5500-lan-analyse](https://github.com/dok2d/fz-W5500-lan-analyse), maintained by nullsp3ct0r and [WhiteCatto](https://github.com/WhiteCatto)
- Custom "Hacker Detective" loading screen (`hacker_gui.c`) by Edenh
- Based on [arag0re/fz-eth-troubleshooter](https://github.com/arag0re/fz-eth-troubleshooter) (fork of [karasevia/finik_eth](https://github.com/karasevia/finik_eth))
- Uses [WIZnet ioLibrary_Driver](https://github.com/Wiznet/ioLibrary_Driver) for W5500 hardware abstraction
- Built for [Flipper Zero OFW](https://github.com/flipperdevices/flipperzero-firmware)

## License

MIT License. See [LICENSE](LICENSE) for details.
