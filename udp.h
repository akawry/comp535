/*
 * udp.h (collection of functions that implement the UDP (User datagram protocol).
 */

#ifndef __UDP_H_
#define __UDP_H_

#include <sys/types.h>
#include "message.h"

typedef struct _udphdr_t 
{
	ushort source; // source port
	ushort dest;   // dest port
	ushort length;
	ushort checksum;  
} udphdr_t;

// function prototypes

int UDPProcess(gpacket_t *in_pkt);
int UDPChecksum(gpacket_t *in_pkt);

#endif
