/*
 * tcp.c (collection of functions that implement the TCP (transmission control protocol).
 */

#include "protocols.h"
#include "message.h"
#include "tcp.h"
#include "ip.h"
#include <inttypes.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <slack/err.h>
#include <string.h>

int TCPProcess(gpacket_t *in_pkt){
	printf("%s", "[TCPProcess]:: packet received for processing\n");

	// extract the packet 	
	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data;
	int iphdrlen = ip_pkt->ip_hdr_len * 4;

	// create the tcp header 	
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + iphdrlen);
	tcphdr->checksum = 0;
	printf("checksum: %04X\n", TCPChecksum(in_pkt));
	return EXIT_FAILURE;
}

uint16_t TCPChecksum(gpacket_t *in_pkt) {
	uint16_t prot_tcp = 6;
	uint16_t pad = 0;
	uint16_t word16;
	uint32_t sum = 0;
	int i;	

	// extract the packet data and ip header length 
	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data; 
	int iphdrlen = ip_pkt->ip_hdr_len * 4;

	// create a udpheader 
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + iphdrlen);
	uchar *tcppkt_b = (uchar *)tcphdr;

	// get the udp packet length and pad with a trailing 0 if the number of octets is odd  	
	uint16_t len_tcp = ntohs(ip_pkt->ip_pkt_len) - iphdrlen;
	if (len_tcp % 2 != 0){
		pad = 1;

		// preserve the original ip packet 
		tcppkt_b[len_tcp] = 0x0;
	}
	
	// sum the udp header and data 
	for (i = 0; i < len_tcp + pad; i += 2){
		word16 =((tcppkt_b[i]<<8)&0xFF00)+(tcppkt_b[i+1]&0xFF);
		sum = sum + (uint32_t)word16;
	}	

	// add the ip source and destination 
	for (i = 0; i < 4; i += 2){
		word16 =((ip_pkt->ip_src[i]<<8)&0xFF00)+(ip_pkt->ip_src[i+1]&0xFF);
		sum = sum + word16;

		word16 =((ip_pkt->ip_dst[i]<<8)&0xFF00)+(ip_pkt->ip_dst[i+1]&0xFF);
		sum = sum + word16; 	
	}

	sum = sum + prot_tcp + len_tcp;

	while (sum>>16)
		sum = (sum & 0xFFFF)+(sum >> 16);

	sum = ~sum;
	
	return ((uint16_t)sum);
}
