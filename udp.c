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



uint32_t wordsum(uchar* buff, int words){
	uint16_t sum = 0;
	int i;
	for (i = 0; i < words; i++){
		sum += (buff[0]<<8)&0xFF00;
		sum += buff[1]&0xFF;
		buff += 2;
	}
	return (uint32_t) sum;
}

int UDPProcess(gpacket_t *in_pkt)
{
	printf("%s", "[UDPProcess]:: packet received for processing\n");

	// extract the packet 	
	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data;
	int iphdrlen = ip_pkt->ip_hdr_len * 4;

	// calculate the checksum 
	uint16_t cksum = UDPChecksum(in_pkt);

	// create the udp header 	
	udphdr_t *udphdr = (udphdr_t *)((uchar *)ip_pkt + iphdrlen);
	udphdr->checksum = cksum;

	printf("%hu\n", cksum);

	IPOutgoingPacket(in_pkt, NULL, 0, 0, UDP_PROTOCOL);

	return EXIT_SUCCESS;
}

uint16_t UDPChecksum(gpacket_t *in_pkt)
{
	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data; 
	int iphdrlen = ip_pkt->ip_hdr_len * 4;
	udphdr_t *udphdr = (udphdr_t *)((uchar *)ip_pkt + iphdrlen);
	uchar *udppkt_b = (uchar *)udphdr;

	udphdr->checksum = 0;
	uint32_t sum = 0;
	uint16_t udp_len = ntohs(ip_pkt->ip_pkt_len) - iphdrlen;
	int pad = udp_len % 2 == 0 ? 0 : 1;
	if (pad == 1){
		udppkt_b[udp_len] = 0x0;
	}
	sum += wordsum((uchar *)ip_pkt->ip_dst, 2);
	sum += wordsum((uchar *)ip_pkt->ip_src, 2);
	sum += wordsum((uchar *)ip_pkt + iphdrlen, (udp_len + pad)/2);
	sum += (uint32_t) UDP_PROTOCOL;
	sum += udp_len;	

	while (sum>>16)
		sum = (sum & 0xFFFF)+(sum >> 16);

	sum = ~sum;
	
	return (uint16_t) sum;
}
		
