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

tcptcb_t *active_connections = NULL;

int TCPCompareSocket(tcpsocket_t *s, uchar ip[], uint16_t port){
	if (s->tcp_port != port)
		return 0;
	int i;
	for (i = 0; i < 4; i++)
		if (s->tcp_ip[i] != ip[i]) return 0;
	return 1;
}

int TCPIsPassive(tcpsocket_t *s){
	if (s->tcp_port != 0)
		return 0;
	int i;
	for (i = 0; i < 4; i++)
		if (s->tcp_ip[i] != '0') return 0;
	return 1;
}

tcptcb_t *TCPGetConnection(uchar src_ip[], uint16_t src_port, uchar dest_ip[], uint16_t dest_port){
	tcptcb_t* cur = active_connections;
	while (cur != NULL){
		if (TCPCompareSocket(cur->tcp_source, src_ip, src_port)){
			if (TCPIsPassive(cur->tcp_dest) || TCPCompareSocket(cur->tcp_dest, dest_ip, dest_port))
				return cur;
		}
		cur = cur->next;
	}
	return NULL;
}

tcpsocket_t *TCPNewSocket(uchar ip[], uint16_t port){
	tcpsocket_t* socket;
	if ((socket = (tcpsocket_t *)malloc(sizeof(tcpsocket_t))) == NULL){
		printf("[TCPNewSocket]:: Not enough room for socket\n");
		return NULL;
	}
	int i;
	for (i = 0; i < 4; i++) socket->tcp_ip[i] = ip[i];
	socket->tcp_port = port;
	return socket;
}

tcptcb_t *TCPNewConnection(uchar src_ip[], uint16_t src_port, uchar dest_ip[], uint16_t dest_port){
	tcptcb_t *con;
	if ((con = (tcptcb_t *)malloc(sizeof(tcptcb_t))) == NULL){
		printf("[TCPNewConnection]:: Not enough room for connection\n");
		return NULL;
	}
	con->tcp_source = TCPNewSocket(src_ip, src_port);
	con->tcp_dest = TCPNewSocket(dest_ip, dest_port);
	con->next = NULL;
	return con;
}

tcpsocket_t *TCPOpen(uchar src_ip[], uint16_t src_port, uchar dest_ip[], uint16_t dest_port){
	tcptcb_t *tcp_conn = TCPGetConnection(src_ip, src_port, dest_ip, dest_port);
	if (tcp_conn == NULL){
		tcp_conn = TCPNewConnection(src_ip, src_port, dest_ip, dest_port);
		if (active_connections == NULL){
			active_connections = tcp_conn;
		} else {
			tcptcb_t* cur = active_connections;
			while (cur->next != NULL) cur = cur->next;
			cur->next = tcp_conn;
		}
	}
	return tcp_conn;
}
		

int TCPProcess(gpacket_t *in_pkt){
	printf("%s", "[TCPProcess]:: packet received for processing\n");

	// extract the packet 	
	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data;
	int iphdrlen = ip_pkt->ip_hdr_len * 4;

	// create the tcp header 	
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + iphdrlen);
	tcphdr->checksum = 0;
	int checksum = TCPChecksum(in_pkt);
	if (checksum){
		printf("[TCPProcess]:: Checksum error! Dropping packet ...\n");
	}
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
