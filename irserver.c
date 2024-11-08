/*
 * irserver.c - picow infrared relay
 * Copyright (C) 2023 Sanjay Rao
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <string.h>
#include <time.h>

#include "hardware/sync.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"

// user settings are concentrated in config.h
#include "config.h"

#define LED_PIN	(CYW43_WL_GPIO_LED_PIN)

typedef void ignore;

#define KEEPALIVEMAX	80
static char keepalivemsg_global[KEEPALIVEMAX+8+1];
static int keepalivemsglen_global;
static unsigned int keepalivecount_global;
static unsigned char password8_global[8]=UDP_PASSWORD8;

struct arg_udp_recv {
	struct udp_pcb *pcb_send;
	unsigned short sequence;
};

static void pulsepin(int pin, unsigned int micros) {
while (1) {
	if (micros<=26) {
		if (!micros) break;
		if (micros>=14) {
			gpio_put(pin,1);
			sleep_us(12);
			gpio_put(pin,0); // this is a 14th micro
			micros-=14;
			if (micros) sleep_us(micros);
			break;
		}
		sleep_us(micros);
		break;
	}
	gpio_put(pin,1);
	sleep_us(12);
	gpio_put(pin,0);
	sleep_us(12);
	micros-=26;
}
}
static inline void sleeppin(unsigned int micros) {
sleep_us(micros);
}

static void sendircode(int pin, unsigned char *codes, unsigned int codeslen) {
// uint32_t flags;

// flags=save_and_disable_interrupts();

while (1) {
	unsigned int u1,u2;

	u1=*codes;
	codes++;
	u1=u1<<8;
	u1+=*codes;
	codes++;
	if (!u1) break;

	(void)pulsepin(pin,u1);

	if (codeslen<=4) break;
	codeslen-=4;

	u2=*codes;
	codes++;
	u2=u2<<8;
	u2+=*codes;
	codes++;
	if (!u2) break;

	(void)sleeppin(u2);

	if (codeslen<=2) break;
}

// (void)restore_interrupts(flags);
}

static void first_blink() {
cyw43_arch_gpio_put(LED_PIN, 1);
cyw43_arch_poll();
sleep_ms(100);
cyw43_arch_gpio_put(LED_PIN, 0);
cyw43_arch_poll();
}
static void pre_blink() {
first_blink();
sleep_ms(100);
}
static void blink() {
sleep_ms(100);
first_blink();
}

static void export_pbuf(unsigned char *dest, unsigned int destlen, struct pbuf *pbuf) {
while (1) {
	unsigned int len;
	len=pbuf->len;
	if (len>=destlen) {
		memcpy(dest,pbuf->payload,destlen);
		break;
	}
	memcpy(dest,pbuf->payload,len);
	pbuf=pbuf->next;
	if (!pbuf) break;
	dest+=len;
	destlen-=len;
}
}

static void callback_udp_recv(void *arg_in, struct udp_pcb *pcb_recv, struct pbuf *pbuf_in, const ip_addr_t *addr, u16_t port) {
struct arg_udp_recv *arg;
unsigned char packet[512];
unsigned int packetlen;

arg=(struct arg_udp_recv *)arg_in;

if (!pbuf_in) goto error;
packetlen=pbuf_in->tot_len;
if (packetlen>512) goto error;

(void)export_pbuf(packet,512,pbuf_in);

if (packetlen>=26) {
	unsigned short replyport;
	unsigned short sequence;
	unsigned short expected_packetlen;
	unsigned char reply_ipv4_0, reply_ipv4_1, reply_ipv4_2, reply_ipv4_3;

	if (memcmp(packet,"UDP2IR01",8)) goto error;
	if (memcmp(packet+8,password8_global,8)) {
		pre_blink();
		goto error;
	}
	expected_packetlen=packet[16]*256+packet[17];
	sequence=packet[18]*256+packet[19];
	reply_ipv4_0=packet[20];
	reply_ipv4_1=packet[21];
	reply_ipv4_2=packet[22];
	reply_ipv4_3=packet[23];
	replyport=packet[24]*256+packet[25];
	if (expected_packetlen!=packetlen) goto error;

	if (!sequence || (sequence == arg->sequence)) {
		unsigned short nseq;
		nseq=arg->sequence;
		if (packetlen!=26) {
			(void)sendircode(IR_PIN,packet+26,packetlen-26);
			nseq+=1;
			if (!nseq) nseq=1;
		}
		if (replyport) {
			struct pbuf *pbuf;
			char msg[80];
			int msglen;

//			snprintf(msg,80,"Request was %u bytes, from %s:%u, sequence %u\n",packetlen,ip4addr_ntoa(addr),replyport,arg->sequence);
			snprintf(msg,80,"UDP2IR01 ACK %u next %u",arg->sequence,nseq);
			msglen=strlen(msg);
			pbuf=pbuf_alloc(PBUF_TRANSPORT,msglen,PBUF_RAM);
			pbuf_take(pbuf,msg,msglen);
			{
				ip_addr_t replyip;
				IP4_ADDR(&replyip, reply_ipv4_0, reply_ipv4_1, reply_ipv4_2, reply_ipv4_3);
				udp_sendto(arg->pcb_send,pbuf,&replyip,replyport);
			}

			pbuf_free(pbuf);
		}
		arg->sequence=nseq;
	}
}
first_blink();

pbuf_free(pbuf_in);
return;
error:
	first_blink(); blink(); blink();
	if (pbuf_in) pbuf_free(pbuf_in);

}

static inline void sethex_byte(unsigned char *dest, unsigned int b) {
static unsigned char hexchars[16]="0123456789abcdef";
dest[0]=hexchars[(b&0xf0)>>4];
dest[1]=hexchars[(b&0xf)];
}

static inline void nl_sethex_uint(unsigned char *dest, unsigned int ui) {
(void)sethex_byte(dest,ui>>24);
dest+=2;
(void)sethex_byte(dest,ui>>16);
dest+=2;
(void)sethex_byte(dest,ui>>8);
dest+=2;
(void)sethex_byte(dest,ui);
dest+=2;
*dest='\n';
}

static int sendkeepalive(struct udp_pcb *pcb_send) {
struct pbuf *pbuf;
ip_addr_t addr;
char countstr[10];
int ncountstr;
int r;

if (!(KEEPALIVE_UDP_PORT)) return 0;

(void)cyw43_arch_lwip_begin();
	r=cyw43_wifi_link_status(&cyw43_state,CYW43_ITF_STA);
(void)cyw43_arch_lwip_end();
if (r!=CYW43_LINK_JOIN) {
	(void)first_blink();
}

IP4_ADDR(&addr, KEEPALIVE_IP0, KEEPALIVE_IP1, KEEPALIVE_IP2, KEEPALIVE_IP3);
keepalivecount_global+=1;
(void)nl_sethex_uint((unsigned char *)keepalivemsg_global+keepalivemsglen_global,keepalivecount_global);

pbuf=pbuf_alloc(PBUF_TRANSPORT,keepalivemsglen_global+9,PBUF_RAM);
if (pbuf) {
	pbuf_take(pbuf,keepalivemsg_global,keepalivemsglen_global+9);
	(void)cyw43_arch_lwip_begin();
		udp_sendto(pcb_send,pbuf,&addr,KEEPALIVE_UDP_PORT);
	(void)cyw43_arch_lwip_end();

	pbuf_free(pbuf);
}
return 0;
}


int main() {
struct arg_udp_recv arg_udp={};
struct udp_pcb *pcb_send,*pcb_recv;
ip_addr_t destaddr;

stdio_init_all();

gpio_init(LED_PIN);
gpio_set_dir(LED_PIN,GPIO_OUT);
gpio_init(IR_PIN);
gpio_set_dir(IR_PIN,GPIO_OUT);

#if 1
if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
#else
if (cyw43_arch_init()) {
#endif
		printf("failed to initialise\n");
		return 1;
}

(void)first_blink(); (void)blink();

cyw43_arch_enable_sta_mode();

(void)blink();

printf("Connecting to WiFi...\n");
while (1) {
	if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
			printf("Failed to connect.\n");
			sleep_ms(30*1000);
	} else {
			printf("Connected.\n");
			break;
	}
}

(ignore)cyw43_wifi_pm(&cyw43_state,CYW43_NO_POWERSAVE_MODE);

(void)blink();

(void)cyw43_arch_lwip_begin();
	IP4_ADDR(&destaddr,0,0,0,0);
	pcb_send=udp_new();
	udp_connect(pcb_send,&destaddr,0);
(void)cyw43_arch_lwip_end();

arg_udp.pcb_send=pcb_send;
arg_udp.sequence=1;

(void)cyw43_arch_lwip_begin();
	pcb_recv=udp_new();
	udp_bind(pcb_recv,IP_ADDR_ANY,UDP_PORT);
	(void)udp_recv(pcb_recv,callback_udp_recv,&arg_udp);
(void)cyw43_arch_lwip_end();

snprintf(keepalivemsg_global,KEEPALIVEMAX,"IR Server on %s:"UDP_PORT_STRING" ",ip4addr_ntoa(netif_ip4_addr(netif_list)));
keepalivemsglen_global=strlen(keepalivemsg_global);

(void)blink();
(void)blink();
(void)blink();

{
	unsigned int keepalivefuse=0;
	while (1) {
#if PICO_CYW43_ARCH_POLL
		cyw43_arch_poll();
		sleep_ms(1);
		if (!keepalivefuse) {
			(ignore)sendkeepalive(pcb_send);
			keepalivefuse=2000;
		}
#else
		sleep_ms(1000);
		if (!keepalivefuse) {
			(ignore)sendkeepalive(pcb_send);
			keepalivefuse=3;
		}
#endif
		keepalivefuse--;
	}
}
cyw43_arch_deinit();
return 0;
}
