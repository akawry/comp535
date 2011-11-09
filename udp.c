/*
 * udp.c (collection of functions that implement the UDP (User datagram protocol).
 */

#include "mtu.h"
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
extern mtu_entry_t MTU_tbl[MAX_MTU];

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
int UDPOpen(uchar ip_addr[], uint16_t port){
	int success;
	uchar tmpbuf[4];

	udpprt_buff_t *buff;
	if ((buff = UDPGetPortBuffer(ip_addr, port)) == NULL){
		UDPCreatePortBuffer(ip_addr, port);
		success = EXIT_SUCCESS;
	} else {
		success = EXIT_FAILURE;
	}

	if (success == EXIT_SUCCESS)
		printf("[UDPOpen]:: Listening on port %hu...\n", port);
	else
		printf("[UDPOpen]:: Error: already listening on port %hu!\n", port);

	return success;
}

/**
 * Dump the contents of the buffer for the given port and address into the
 * provided string buffer 
 */
int UDPReceive(uchar ip_addr[], uint16_t port, uchar* recv_buff){
	udpprt_buff_t *buff = UDPGetPortBuffer(ip_addr, port);

	if (buff != NULL){
		int rvalue;
		int pktsize;
		int iphdrlen;
		gpacket_t *in_pkt;
		ip_packet_t *ip_pkt;
		udphdr_t *udphdr;	
	
		while ((rvalue = readQueue(buff->buff, (void **)&in_pkt, &pktsize)) != EXIT_FAILURE){
			ip_pkt = (ip_packet_t *)in_pkt->data.data;
			iphdrlen = ip_pkt->ip_hdr_len * 4;
			udphdr = (udphdr_t *)((uchar *)ip_pkt + iphdrlen);		
			memcpy(recv_buff, (uchar *)ip_pkt + iphdrlen + 8, udphdr->length - 8);
			recv_buff += udphdr->length - 8;
		}
		return EXIT_SUCCESS; 
	} else {
		printf("[UDPReceive]:: No one listening on port!\n");
		return EXIT_FAILURE;
	} 
}

void printUDPPacketNew(gpacket_t *msg)
{

	ip_packet_t *ip_pkt = (ip_packet_t *)msg->data.data;
	int iphdrlen = 20;
	udphdr_t *udphdr = (udphdr_t *)((uchar *)ip_pkt + iphdrlen);
	printf("\nUDP: ----- UDP Header -----\n");
	printf("UDP: Source	: 0x%04X\n", htonl(udphdr->source))>>16;
	printf("UDP: Dest	: 0x%04X\n", htonl(udphdr->dest))>>16;
	printf("UDP: Length	: 0x%04x\n", htonl(udphdr->length))>>16;
	printf("UDP: Checksum	: 0x%04X\n", htonl(udphdr->checksum))>>16;
	printf("\n");
}

int UDPSend(uchar dst_ip[], uint16_t udp_dest_port, uint16_t udp_src_port, uchar* buff, int len){
	char tmpbuf[MAX_TMPBUF_LEN];
	gpacket_t *out_pkt = (gpacket_t *) malloc(sizeof(gpacket_t));
	uchar iface_ip[MAX_MTU][4];
	int left_bytes, copied = 0, count;
	uint16_t tmpCount;
	
	//extract the ip packet header and header length
	ip_packet_t *ip_pkt = (ip_packet_t *)(out_pkt->data.data);
	int iphdrlen = 20;
	
	// Find location of udpheader 
	udphdr_t *udphdr = (udphdr_t *)((uchar *)ip_pkt + iphdrlen);

	// Put header info to the gpacket
	//udphdr->dest = udp_dest_port;
	//udphdr->source = udp_src_port;
	udphdr->dest = htonl(udp_dest_port)>>16;
	udphdr->source = htonl(udp_src_port)>>16;

	udphdr->checksum = 0;
	
	// Put IP to the gpacket (for udp checksumming)
	COPY_IP(ip_pkt->ip_dst, gHtonl(tmpbuf, dst_ip));
	
	if ((count = findAllInterfaceIPs(MTU_tbl, iface_ip)) == 0) {
		Dot2IP("192.168.0.1", iface_ip[0]);
	}

	COPY_IP(ip_pkt->ip_src, gHtonl(tmpbuf, iface_ip[0]));

	// Looping to send packets
	for(left_bytes = len ; left_bytes >= 0 ; left_bytes -= MAX_UDP_PAYLOAD) {
		tmpCount = left_bytes >= MAX_UDP_PAYLOAD ? MAX_UDP_PAYLOAD : left_bytes;
		udphdr->length = htonl(tmpCount + 8)>>16;

		// Put data into gpacket, and break message using if it reach the max length
		strncpy((uchar *)udphdr + 8, buff + copied, tmpCount);
		copied += tmpCount;

		// Calculate checksum
		//udphdr->checksum = htonl(UDPChecksum(ip_pkt))>>16;
		printf("Checksum is : %0x",udphdr->checksum);
		// send the message to the IP routine to ship it out
		printUDPPacketNew(out_pkt);
		IPOutgoingPacket(out_pkt, dst_ip, tmpCount + 8 + iphdrlen, 1, 17);
	}

	return EXIT_SUCCESS;
}	

int UDPProcess(gpacket_t *in_pkt)
{
	printf("%s", "[UDPProcess]:: packet received for processing\n");
	printUDPPacket(in_pkt);

	// extract the packet 	
	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data;
	int iphdrlen = ip_pkt->ip_hdr_len * 4;
	uchar tmpbuff[MAX_TMPBUF_LEN];

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
	udpprt_buff_t* buff = UDPGetPortBuffer(gNtohl(tmpbuff, ip_pkt->ip_dst), ~checksum((uchar *)ip_pkt + iphdrlen + 2, 1));
	if (buff != NULL){
		writeQueue(buff->buff, in_pkt, sizeof(in_pkt));
		printf("[UDPProcess]:: Buffered packet... \n");
	} else {
		printf("[UDPProcess]:: No one listening on destination ip and source... dropping packet!\n");
	}	

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
		
