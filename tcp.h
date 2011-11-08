/*
 * tcp.h (collection of functions that implement the TCP (transfer control protocol).
 */

#ifndef __TCP_H_
#define __TCP_H_

#include <stdint.h>
#include "message.h"


typedef struct _tcphdr_t 
{
	uint16_t sport;
	uint16_t dport;
	uint32_t seq;
	uint32_t ack_seq;

	#if BYTE_ORDER == LITTLE_ENDIAN
	uint8_t	res:6;
	uint8_t off:4;
	#endif

	#if BYTE_ORDER == BIG_ENDIAN
	uint8_t	off:4;
	uint8_t res:6;
	#endif
	
	/* Flags */
	uint8_t urg:1;
	uint8_t ack:1;
	uint8_t psh:1;
	uint8_t rst:1;
	uint8_t syn:1;
	uint8_t fin:1;

	uint16_t win_size;
	uint16_t checksum;
	uint16_t urg_ptr;
  
} tcphdr_t;

typedef struct _tcpsocket_t
{
	uint16_t tcp_port;
	uchar tcp_ip[4];
} tcpsocket_t;

typedef struct _tcptcb_t
{
	tcpsocket_t *tcp_source;
	tcpsocket_t *tcp_dest;
	struct tcptcb_t *next; // linked list
} tcptcb_t;

// function prototypes
int TCPProcess(gpacket_t *in_pkt);
uint16_t TCPChecksum(gpacket_t *in_pkt);

#endif
