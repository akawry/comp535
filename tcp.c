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

void TCPPrintTCB(tcptcb_t *tcb){
	char tmpbuf[MAX_TMPBUF_LEN];
	printf("source ip: %s, source port: %d\tdest ip: %s, dest port:%d\n", 
		IP2Dot(tmpbuf, gNtohl(tmpbuf+20, (tcb->tcp_source)->tcp_ip)), (tcb->tcp_source)->tcp_port,
		IP2Dot(tmpbuf+60, gNtohl((tmpbuf+80), (tcb->tcp_dest)->tcp_ip)), (tcb->tcp_dest)->tcp_port);
}

gpacket_t *TCPNewPacket(tcptcb_t* con){
	// allocate the IP packet 
	char tmpbuf[MAX_TMPBUF_LEN];
	gpacket_t *out_pkt = (gpacket_t *) malloc(sizeof(gpacket_t));
	
	//extract the ip packet header and header length
	ip_packet_t *ip_pkt = (ip_packet_t *)(out_pkt->data.data);
	int iphdrlen = 20;
	
	//create the tcp header 
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + iphdrlen);
	tcphdr->dport = (con->tcp_dest)->tcp_port;
	tcphdr->sport = (con->tcp_source)->tcp_port;

	// Put IP to the gpacket (for tcp checksumming)
	COPY_IP(ip_pkt->ip_dst, gHtonl(tmpbuf, (con->tcp_dest)->tcp_ip));
	COPY_IP(ip_pkt->ip_src, gHtonl(tmpbuf, (con->tcp_source)->tcp_ip));

	return out_pkt;
}

int TCPRequestClose(tcptcb_t *con){
	printf("[TCPRequestClose]:: Requesting a close.\n");

	gpacket_t *out_pkt = TCPNewPacket(con);
	ip_packet_t *ip_pkt = (ip_packet_t *)(out_pkt->data.data);
	int iphdrlen = 20;
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + iphdrlen);
	
	return EXIT_FAILURE;
}

int TCPRequestConnection(tcptcb_t *con){
	
	printf("[TCPRequestConnection]:: Requesting a connection.\n");

	gpacket_t *out_pkt = TCPNewPacket(con);
	ip_packet_t *ip_pkt = (ip_packet_t *)(out_pkt->data.data);
	int iphdrlen = 20;
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + iphdrlen);

	// set a random ISN 
	unsigned int iseed = (unsigned int)time(NULL);
  	srand (iseed);
	con->tcp_ISS = rand();
	
	tcphdr->seq = con->tcp_ISS;
	tcphdr->SYN = (uint8_t)1;
	ip_pkt->ip_pkt_len = iphdrlen + TCP_HEADER_LENGTH;
	tcphdr->checksum = TCPChecksum(out_pkt);

	// send the pakcet to the ip module for further processing 	
	int success = IPOutgoingPacket(out_pkt, (con->tcp_dest)->tcp_ip, iphdrlen + TCP_HEADER_LENGTH, 1, TCP_PROTOCOL);
	if (success == EXIT_SUCCESS){
		printf("[TCPRequestConnection]:: Sent out following request:\n");
		printTCPPacket(out_pkt);
		con->tcp_state = TCP_SYN_SENT;
	} else {
		printf("[TCPRequestConnection]:: Failed to send request!\n");
	}
	
	return success;
}

int TCPAcknowledgeConnectionRequest(gpacket_t *in_pkt, tcptcb_t* con){
	printf("[TCPAcknowledgeConnectionRequest]:: Acknowledging connection request.\n");

	int iphdrlen = 20;
	char tmpbuf[MAX_TMPBUF_LEN];
	
	ip_packet_t *ip_pkt_in = (ip_packet_t *)(in_pkt->data.data);
	tcphdr_t *tcphdr_in = (tcphdr_t *)((uchar *)ip_pkt_in + iphdrlen);

	// was a passive waiting connection, to update the socket in the tcb
	if (TCPIsPassive(con->tcp_dest)){
		COPY_IP((con->tcp_dest)->tcp_ip, gNtohl(tmpbuf, ip_pkt_in->ip_src));
		(con->tcp_dest)->tcp_port = tcphdr_in->sport;
	}

	// create a response 
	gpacket_t *out_pkt = TCPNewPacket(con);
	ip_packet_t *ip_pkt_out = (ip_packet_t *)(out_pkt->data.data);
	tcphdr_t *tcphdr_out = (tcphdr_t *)((uchar *)ip_pkt_out + iphdrlen);

	// set a random ISN for my sequence number
	if (tcphdr_in->ACK == 0){
		printf("[TCPAcknowledgeConnectionRequest]:: Second part of handshaking phase ...\n");
	  	srand (tcphdr_in->seq);
		con->tcp_ISS = rand();
		con->tcp_state = TCP_SYN_RECEIVED;	
		tcphdr_out->seq = con->tcp_ISS;
		tcphdr_out->ack_seq = tcphdr_in->seq+1;
		tcphdr_out->SYN = (uint8_t)1;

	// third phase of handshake 
	} else if (tcphdr_in->ACK == 1){
		printf("[TCPAcknowledgeConnectionRequest]:: Third part of handshaking phase ...\n");
		con->tcp_state = TCP_ESTABLISHED;
		tcphdr_out->seq = con->tcp_ISS + 1;
		tcphdr_out->ack_seq = tcphdr_in->seq + 1;
		tcphdr_out->SYN = (uint8_t)0;
	}
	tcphdr_out->ACK = (uint8_t)1;
	con->tcp_IRS = tcphdr_in->seq;
	ip_pkt_out->ip_pkt_len = iphdrlen + TCP_HEADER_LENGTH;

	int success = IPOutgoingPacket(out_pkt, (con->tcp_dest)->tcp_ip, iphdrlen + TCP_HEADER_LENGTH, 1, TCP_PROTOCOL);
	if (success == EXIT_SUCCESS){
		printf("[TCPAcknowledgeConnectionRequest]:: Sent out following ACK:\n");
		printTCPPacket(out_pkt);
	} else {
		printf("[TCPAcknowledgeConnectionRequest]:: Failed to send out ACK!\n");
	}

	return success;
}

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
		if (s->tcp_ip[i] != 0) return 0;

	return 1;
}

tcptcb_t *TCPGetConnection(uchar src_ip[], uint16_t src_port, uchar dest_ip[], uint16_t dest_port){
	tcptcb_t* cur = active_connections;
	if (cur == NULL){
		printf("[TCPGetConnection]:: Dont have any connections yet!\n");
	}
	while (cur != NULL){
		// TCPPrintTCB(cur);
		if (TCPCompareSocket(cur->tcp_source, src_ip, src_port)){

			// have a socket passively waiting and still in the listen state 
			if (TCPIsPassive(cur->tcp_dest)){
				if (cur->tcp_state == TCP_LISTEN)
					return cur;

			// have a socket waiting with exact destination socket
			} else {
				if (TCPCompareSocket(cur->tcp_dest, dest_ip, dest_port))
					return cur;
			}
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
	printf("[TCPNewConnection]:: Allocating new connection struct\n");
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

tcptcb_t *TCPRemoveConnection(tcptcb_t *conn){
	if (active_connections != NULL){
		if (active_connections == conn){
			active_connections = active_connections->next;
			return active_connections;
		} else {
			tcptcb_t *prev;
			tcptcb_t *cur = active_connections;
			while (cur != NULL){
				if (cur == conn){
					prev->next = cur->next;
					free(cur);
					return prev->next;				
				} else {
					prev = cur;
					cur = cur->next;
				}
			}
		}
	}
	return NULL;
}

void TCPOpen(uchar src_ip[], uint16_t src_port, uchar dest_ip[], uint16_t dest_port){

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

		// passive OPEN, so listen for requests
		if (TCPIsPassive(tcp_conn->tcp_dest)){

			printf("[TCPOpen]:: Was a passive OPEN ... start listening ... \n");
			tcp_conn->tcp_state = TCP_LISTEN;

		// active OPEN, send out the request to the destination
		} else {
			TCPRequestConnection(tcp_conn);
		}
	}
}

void TCPClose(uchar src_ip[], uint16_t src_port, uchar dest_ip[], uint16_t dest_port){

	tcpsocket_t* dest = TCPNewSocket(dest_ip, dest_port);

	tcptcb_t* cur = active_connections;
	if (cur == NULL){
		printf("[TCPClose]:: Dont have any connections to close!\n");
	}
	while (cur != NULL){

		if (TCPCompareSocket(cur->tcp_source, src_ip, src_port)){

			// have a socket passively waiting and still in the listen state 
			if (TCPIsPassive(cur->tcp_dest)){
				printf("[TCPClose]:: Closing connection: ");
				TCPPrintTCB(cur);
				cur = TCPRemoveConnection(cur);
				
			// have a socket waiting with exact destination socket
			} else if (TCPIsPassive(dest) || TCPCompareSocket(cur->tcp_dest, dest_ip, dest_port)) {
				TCPRequestClose(cur);
				cur = cur->next;
			} else {
				cur = cur->next;
			}
		}
	}
}
		

int TCPProcess(gpacket_t *in_pkt){
	printf("%s", "[TCPProcess]:: packet received for processing\n");
	printTCPPacket(in_pkt);

	uchar tmpbuff[MAX_TMPBUF_LEN];

	// extract the packet 	
	ip_packet_t *ip_pkt = (ip_packet_t *)in_pkt->data.data;
	int iphdrlen = ip_pkt->ip_hdr_len * 4;

	// create the tcp header 	
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + iphdrlen);
	
	/* 
	 * BEGIN STATE MACHINE HERE
	 */
	tcptcb_t* conn = TCPGetConnection(gNtohl(tmpbuff, ip_pkt->ip_dst), tcphdr->dport, gNtohl(tmpbuff+20, ip_pkt->ip_src), tcphdr->sport);
	
	// no TCB's waiting for where the packet came from 
	if (conn == NULL){
		printf("[TCPProcess]:: No one listening on destination ip and port.... dropping the packet\n");
		return EXIT_FAILURE;
	}

	// request for open 
	if (tcphdr->SYN == 1){
		TCPAcknowledgeConnectionRequest(in_pkt, conn);
	
	} else {
		
		if (tcphdr->ACK == 1){

			// final ACK for handshake 
			if (conn->tcp_state == TCP_SYN_RECEIVED){
				conn->tcp_state = TCP_ESTABLISHED;
				printf("[TCPProcess]:: Connection fully established ... \n");
			}
		}

	}		
	
	/*tcphdr->checksum = 0;
	int checksum = TCPChecksum(in_pkt);
	if (checksum){
		printf("[TCPProcess]:: Checksum error! Dropping packet ...%02X\n", checksum);
	}*/
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
