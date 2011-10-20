/*
 * udp.c (collection of functions that implement the UDP (User datagram protocol).
 */

#include "simplequeue.h"
#include "protocols.h"
#include "udp.h"
#include "ip.h"
#include <inttypes.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <slack/err.h>
#include <string.h>

udpprt_buff_t* udp_active_ports = NULL;

/**
 * Create a buffer for packets and insert into
 * list of active buffers 
 */

udpprt_buff_t* UDPCreatePortBuffer(uchar addr[], uint16_t port){
	udpprt_buff_t *buff;
	if ((buff = (udpprt_buff_t *)malloc(sizeof(udpprt_buff_t))) == NULL){
		printf("[UDPCreatePortBuffer]:: Not enough room for buffer\n");
		return NULL;
	}
	int i; for (i = 0; i < 4; i++) buff->addr[i] = addr[i];
	buff->port = port;
	buff->next = NULL;
	buff->buff = createSimpleQueue("", MAX_QUEUE_SIZE, 0, 0);
	if (udp_active_ports == NULL){
		udp_active_ports = buff;
	} else {
		udpprt_buff_t *cur = udp_active_ports;
		while (cur->next)
			cur = cur->next;
		cur->next = buff;
	}
	return buff;
}

/**
 * Get the buffer for packets with the given port and address
 */
udpprt_buff_t* UDPGetPortBuffer(uchar addr[], uint16_t port){
	int i, found;
	udpprt_buff_t *cur = udp_active_ports;
	while (cur){
		found = 1;
		for (i = 0; i < 4; i++){
			if (addr[i] != cur->addr[i]){
				found = 0;
				break;
			}
		}
		if (found && cur->port == port)
			return cur;
		cur = cur->next;
	}
	return NULL;
}

/**
 * Start listening on the port for the given address
 */
udpprt_buff_t *UDPOpen(uchar* addr, uint16_t port){
	uchar tmpbuff[64];
	gNtohl(tmpbuff, addr);
	uchar ip_addr[4];
	Dot2IP(addr, ip_addr);
	return UDPCreatePortBuffer(ip_addr, port);
}

/**
 * Dump the contents of the buffer for the given port and address
 */
void UDPReceive(uchar* addr, uint16_t port){
	uchar tmpbuff[64];
	gNtohl(tmpbuff, addr);
	uchar ip_addr[4];
	Dot2IP(addr, ip_addr);
	udpprt_buff_t *buff = UDPGetPortBuffer(ip_addr, port);
	if (buff != NULL){
		gpacket_t *pkt;
		int rvalue;
		int pktsize;	
		while ((rvalue = readQueue(buff->buff, (void **)&pkt, &pktsize)) != EXIT_FAILURE){
			printUDPPacket(pkt);	
		}
	}
}	

int UDPProcess(gpacket_t *in_pkt)
{
	printf("%s", "[UDPProcess]:: packet received for processing\n");

	// extract the packet 	
	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data;
	int iphdrlen = ip_pkt->ip_hdr_len * 4;
	
	// create the udp header 	
	udphdr_t *udphdr = (udphdr_t *)((uchar *)ip_pkt + iphdrlen);

	// packet used checksum (is optional) 
	if (udphdr->checksum != 0){

		// checksum is used 
		if (udphdr->checksum == 0xFFFF)
			udphdr->checksum = 0x0000;

		// calculate the checksum 
		uint16_t cksum = UDPChecksum(in_pkt);

		if (cksum){
			printf("[UDPProcess]:: UDP checksum error! %04X\n", cksum);
			return EXIT_FAILURE;
		}
	}

	// buffer the packet if the buffer exists, otherwise drop the packet
	udpprt_buff_t* buff = UDPGetPortBuffer(ip_pkt->ip_dst, udphdr->dest);
	if (buff != NULL)
		return writeQueue(buff->buff, in_pkt, sizeof(in_pkt));
	else 
		printf("[UDPProcess]:: No one listening on destination ip and source... dropping packet!\n");	

	return EXIT_FAILURE;
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
		word16 =((udppkt_b[i]<<8)&0xFF00)+(udppkt_b[i+1]&0xFF);
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
		
