/*
 * udp.c (collection of functions that implement the UDP (User datagram protocol).
 */

#include "protocols.h"
#include "udp.h"
#include "ip.h"
#include <inttypes.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <slack/err.h>
#include <string.h>

int UDPProcess(gpacket_t *in_pkt)
{
	printf("%s", "[UDPProcess]:: packet received for processing\n");

	// extract the packet 	
	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data;
	int iphdrlen = ip_pkt->ip_hdr_len * 4;
	
	// create the udp header 	
	udphdr_t *udphdr = (udphdr_t *)((uchar *)ip_pkt + iphdrlen);

	// calculate the checksum 
	uint16_t cksum = UDPChecksum(in_pkt);
	
	// original checksum wasn't done, so 1's compliment it 
	if (cksum == 0){
		cksum = ~cksum;
	}
	udphdr->checksum = cksum;

	IPOutgoingPacket(in_pkt, NULL, 0, 0, UDP_PROTOCOL);

	return EXIT_SUCCESS;
}

uint16_t UDPChecksum(gpacket_t *in_pkt)
{
	uint16_t prot_udp = 17;
	uint16_t pad = 0;
	uint16_t word16;
	uint32_t sum = 0;
	int i;	

	// extract the packet data and ip header length 
	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data; 
	int iphdrlen = ip_pkt->ip_hdr_len * 4;
	uchar* buff = (uchar *)ip_pkt + iphdrlen;

	// create a udpheader 
	udphdr_t *udphdr = (udphdr_t *)((uchar *)ip_pkt + iphdrlen);
	uchar *udppkt_b = (uchar *)udphdr;

	// get the udp packet length and pad with a trailing 0 if the number of octets is odd  	
	uint16_t len_udp = ntohs(ip_pkt->ip_pkt_len) - iphdrlen;
	if (len_udp % 2 != 0){
		pad = 1;

		// preserve the original ip packet 
		udppkt_b[len_udp] = 0x0;
	}
	
	// sum the udp header and data 
	for (i = 0; i < len_udp + pad; i += 2){
		word16 =((buff[i]<<8)&0xFF00)+(buff[i+1]&0xFF);
		sum = sum + (uint32_t)word16;
	}	

	// add the ip source and destination 
	for (i = 0; i < 4; i += 2){
		word16 =((ip_pkt->ip_src[i]<<8)&0xFF00)+(ip_pkt->ip_src[i+1]&0xFF);
		sum = sum + word16;

		word16 =((ip_pkt->ip_dst[i]<<8)&0xFF00)+(ip_pkt->ip_dst[i+1]&0xFF);
		sum = sum + word16; 	
	}

	sum = sum + prot_udp + len_udp;

	while (sum>>16)
		sum = (sum & 0xFFFF)+(sum >> 16);

	sum = ~sum;
	
	return ((uint16_t)sum);
}
		
