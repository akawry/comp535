/*
 * udp.h (collection of functions that implement the UDP (User datagram protocol).
 */

#ifndef __UDP_H_
#define __UDP_H_

#include <stdint.h>
#include "message.h"

typedef struct _udphdr_t 
{
	uchar source[2]; // source port
	uchar dest[2];   // dest port
	uchar length[2];
	ushort checksum;  
} udphdr_t;

// function prototypes

int UDPProcess(gpacket_t *in_pkt);
uint16_t UDPChecksum(gpacket_t *in_pkt);

#endif
