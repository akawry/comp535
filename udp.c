/*
 * udp.c (collection of functions that implement the UDP (User datagram protocol).
 */

#include "udp.h"
#include "ip.h"
#include <stdlib.h>
#include <slack/err.h>
#include <string.h>

int UDPProcess(gpacket_t *in_pkt)
{
	// extract the packet 	
	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data;
	int iphdrlen = ip_pkt->ip_hdr_len * 4;

	// calculate the checksum 
	int check_sum = UDPChecksum(in_pkt);

	// create the udp header 	
	udphdr_t *udphdr = (udphdr_t *)((uchar *)ip_pkt + iphdrlen);	
	
	verbose(2, "[UDPProcess]:: packet received for processing.. NOT YET IMPLEMENTED!! ");
	return EXIT_SUCCESS;

}

int UDPChecksum(gpacket_t *in_pkt)
{

	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data; 

	// initialize sum to udp protocol (17)
	int sum = ip_pkt->ip_prot;

	// add source 
	sum += checksum((uchar *)ip_pkt->ip_src, 2);
		
	// add dest
	sum += checksum((uchar *)ip_pkt->ip_dst, 2);

	// get upd length
	uint16_t udp_len = checksum((uchar *)ip_pkt + 80, 1);

	// add all the data
	int iphdrlen = ip_pkt->ip_hdr_len * 4;
	sum += checksum((uchar *)ip_pkt + iphdrlen, udp_len);
	
	// add udp length
	sum += udp_len;

	return sum;
}
