#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

#if 0
#include "lwipopts_examples_common.h"
#else
// this section was copied from lwipopts_examples_common.h
// I'm assuming this is public domain
#ifndef NO_SYS
#define NO_SYS                      1
#endif
#ifndef LWIP_SOCKET
#define LWIP_SOCKET                 0
#endif
#if PICO_CYW43_ARCH_POLL
#define MEM_LIBC_MALLOC             1
#else
#define MEM_LIBC_MALLOC             0
#endif
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              24
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_MSS                     1460
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_NETCONN                0
#define MEM_STATS                   0
#define SYS_STATS                   0
#define MEMP_STATS                  0
#define LINK_STATS                  0
#define LWIP_CHKSUM_ALGORITHM       3
#define LWIP_DHCP                   1
#define LWIP_IPV4                   1
#define LWIP_UDP                    1
#define LWIP_NETIF_TX_SINGLE_PBUF   1
#define DHCP_DOES_ARP_CHECK         0
#define LWIP_DHCP_DOES_ACD_CHECK    0
// end of lwipopts_examples_common.h
#endif

#include "config.h"

#if ISSTATICIP
#ifdef LWIP_DHCP
#undef LWIP_DHCP
#endif
#define LWIP_DHCP	0
#undef CYW43_DEFAULT_IP_STA_ADDRESS
#undef CYW43_DEFAULT_IP_STA_MASK
#undef CYW43_DEFAULT_IP_STA_GATEWAY
#define CYW43_DEFAULT_IP_STA_ADDRESS	LWIP_MAKEU32 STATICIP_ADDR
#define CYW43_DEFAULT_MASK	LWIP_MAKEU32 STATICIP_MASK
#define CYW43_DEFAULT_IP_STA_GATEWAY	LWIP_MAKEU32 STATICIP_GATEWAY
#else
#define LWIP_DHCP	1
#endif

#endif
