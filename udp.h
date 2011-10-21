/*
 * udp.h (collection of functions that implement the UDP (User datagram protocol).
 */

#ifndef __UDP_H_
#define __UDP_H_

#include <stdint.h>
#include "simplequeue.h"
#include "message.h"

#define MAX_UDP_PAYLOAD 65507

typedef struct _udphdr_t 
{
	uint16_t source;
	uint16_t dest;
	uint16_t length;
	uint16_t checksum;  
} udphdr_t;

typedef struct _udpprt_buff_t
{
	uchar addr[4];
	uint16_t port;
	simplequeue_t *buff;
	struct udpprt_buff_t *next;
} udpprt_buff_t;

// function prototypes

int UDPProcess(gpacket_t *in_pkt);
uint16_t UDPChecksum(gpacket_t *in_pkt);
int UDPOpen(uchar ip_addr[], uint16_t port);
int UDPReceive(uchar ip_addr[], uint16_t port, uchar* recv_buff);
int UDPSend(uchar dst_ip[], uint16_t udp_dest_port, uint16_t udp_src_port, uchar* buff, int len);

#endif
