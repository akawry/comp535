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

//basically, send_udp would gather: src IP, des IP, src Port, des Port, Data
//and calculate: length and checksum, then pass it to IPOutgoingPacket
//Also need to break messages if the length is higher than maximum length:65507=65535-8(udpHdr)-20(IPHdr)
void UDPSEND(int udp_dst_port, uchar *des_ip, int udp_src_port, char *data, int len_byte){
	
	char tmpbuf[MAX_TMPBUF_LEN];
	gpacket_t *out_pkt = (gpacket_t *) malloc(sizeof(gpacket_t));
	uchar *dataptr;
	uint16_t chksum;
	uchar iface_ip[MAX_MTU][4];
	int left_bytes;
	uint16_t tmpCount;
	
	//extract the ip packet header and header length
	ip_packet_t *ip_pkt = (ip_packet_t *)(out_pkt->data.data);
	int iphdrlen = ip_pkt->ip_hdr_len *4;
	
	// Find location of udpheader 
	udphdr_t *udphdr = (udphdr_t *)((uchar *)ip_pkt + iphdrlen);
	// Put header info to the gpacket
	udphdr->dest = htonl(udp_dst_port);	//TODO: check port range convert from int to uint16_t
	udphdr->source = htonl(udp_src_port);
	udphdr->checksum = 0;
	
	// Put IP to the gpacket
	COPY_IP(ip_pkt->ip_dst, gHtonl(tmpbuf, dst_ip));
	
	if ((count = findAllInterfaceIPs(MTU_tbl, iface_ip)) == 0)
	{
		iface_ip[0] = Dot2IP("192.168.0.1", ip_addr);
	}
	COPY_IP(ip_pkt->ip_src, gHtonl(tmpbuf, iface_ip[0]));
	
	// Looping to send packets
	for(left_bytes = pkt_size ; left_bytes>0 ; left_bytes-65507)
	{
		tmpCount = len_byte > 65507 ? 65507 : left_bytes ;
		udphdr->length = htonl(tmpCount + 8);
		// Put data into gpacket, and break message using if it reach the max length
		dataptr = (uchar *)udphdr + 8;
		strncpy(dataptr, data, tmpCount);
		
		data = data[tmpCount];

		// Calculate checksum
		chksum = UDPChecksum(out_pkt);
		udphdr->checksum = htonl(tmpbuf, chksum);

		verbose(2, "[sendPingPacket]:: Sending... ICMP ping to  %s", IP2Dot(tmpbuf, dst_ip));

		// send the message to the IP routine for further processing
		// IPOutgoingPacket(/context, packet, IPaddr, size, newflag, UDP_PROTOCOL)
		IPOutgoingPacket(out_pkt, dst_ip, ip_data_size, 1, 17);
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
		
