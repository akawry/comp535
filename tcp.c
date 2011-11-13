/*
 * tcp.c (collection of functions that implement the TCP (transmission control protocol).
 */

#include "protocols.h"
#include "message.h"
#include "tcp.h"
#include "ip.h"
#include <inttypes.h>
#include <sys/time.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <slack/err.h>
#include <string.h>

tcptcb_t *active_connections = NULL;

int TCPSendRST(gpacket_t *in_pkt){
	ip_packet_t *ip_pkt_in = (ip_packet_t *)(in_pkt->data.data);
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt_in + ip_pkt_in->ip_hdr_len*4);
	
	// switch source and destination
	uint16_t dport = tcphdr->dport;
	tcphdr->dport = tcphdr->sport;
	tcphdr->sport = dport;

	// send ack_seq
	uint32_t seq = ntohl(tcphdr->seq) + 1;
	tcphdr->seq = 0;
	tcphdr->ack_seq = htonl(seq);

	// reset flags 
	tcphdr->ACK = 1;
	tcphdr->SYN = 0;
	tcphdr->FIN = 0;
	tcphdr->RST = 1;
	tcphdr->PSH = 0;
	tcphdr->URG = 0;

	// remove options, set win size to 0 
	ip_pkt_in->ip_pkt_len = htons(ip_pkt_in->ip_hdr_len*4 + TCP_HEADER_LENGTH);
	tcphdr->doff = 5;
	tcphdr->win_size = 0;

	tcphdr->checksum = 0;
	tcphdr->checksum = TCPChecksum(in_pkt);

	int success = IPOutgoingPacket(in_pkt, NULL, NULL, 0, TCP_PROTOCOL);
	if (success == EXIT_SUCCESS){
		printf("[TCPSendRST]:: Sent out following RST:\n");
		printTCPPacket(in_pkt);
	} else {
		printf("[TCPSendRST]:: Failed to send out RST!!\n");
	}
	return success;
}

void TCPResendConnectionRequest(int sig){
	tcptcb_t *cur = active_connections;
	time_t now = time(NULL);
	while (cur != NULL){
		if (cur->tcp_state == TCP_SYN_SENT){
			if (difftime(now, cur->syn_sent) < TCP_TIMEOUT){
				printf("[TCPResendConnectionRequest]:: Resending connection request for ");
				TCPPrintTCB(cur);
				TCPRequestConnection(cur);
			} else {
				printf("[TCPResendConnectionRequest]:: Connection attempt timed out!\n");
				cur->tcp_state = TCP_LISTEN;
				cur->syn_sent = NULL;
			}
		} 
		cur = cur->next;
	}
}

void TCPResetConnection(tcptcb_t *con){
	con->tcp_SND_UNA = 0;
	con->tcp_SND_NXT = 0;
	con->tcp_SND_WND = 0;
	con->tcp_SND_UP = 0;
	con->tcp_SND_WL1 = 0;
	con->tcp_SND_WL2 = 0;
	con->tcp_ISS = 0;

	con->tcp_RCV_WND = TCP_DEFAULT_WIN_SIZE;
	con->tcp_RCV_NXT = 0;
	con->tcp_RCV_UP = 0;
	con->tcp_IRS = 0;

	con->tcp_SEG_SEQ = 0;
	con->tcp_SEG_ACK = 0;
	con->tcp_SEG_LEN = 0;
	con->tcp_SEG_WND = 0;
	con->tcp_SEG_UP = 0;

	con->next = NULL;
	con->tcp_send_queue = NULL;
	con->syn_sent = NULL;
}


int TCPAcknowledgeReceived(gpacket_t *in_pkt, tcptcb_t *con){
	ip_packet_t *ip_pkt_in = (ip_packet_t *)(in_pkt->data.data);
	tcphdr_t *tcphdr_in = (tcphdr_t *)((uchar *)ip_pkt_in + ip_pkt_in->ip_hdr_len*4);
	int iphdrlen = ip_pkt_in->ip_hdr_len * 4;

	gpacket_t *out_pkt = TCPNewPacket(con);
	ip_packet_t *ip_pkt_out = (ip_packet_t *)(out_pkt->data.data);
	tcphdr_t *tcphdr_out = (tcphdr_t *)((uchar *)ip_pkt_out + ip_pkt_in->ip_hdr_len*4);
	uint16_t seg_len = ntohs(ip_pkt_in->ip_pkt_len) - (iphdrlen + tcphdr_in->doff*4);

	con->tcp_SND_UNA = tcphdr_in->ack_seq;

	//tcphdr_out->seq = con->tcp_SND_NXT;
	tcphdr_out->seq = tcphdr_in->ack_seq;
	tcphdr_out->ack_seq = htonl(ntohl(tcphdr_in->seq) + seg_len);
	tcphdr_out->ACK = (uint8_t)1;

	ip_pkt_out->ip_pkt_len = htons(ip_pkt_out->ip_hdr_len*4 + TCP_HEADER_LENGTH);
	tcphdr_out->checksum = 0;
	tcphdr_out->checksum = htons(TCPChecksum(out_pkt));
	
	int success = IPOutgoingPacket(out_pkt, (con->tcp_dest)->tcp_ip, TCP_HEADER_LENGTH, 1, TCP_PROTOCOL);
	if (success == EXIT_SUCCESS){
		printf("[TCPAcknowledgeReceived]:: Sent following ACK: \n");
		printTCPPacket(out_pkt);
	} else {
		printf("[TCPAcknowledgeReceived]:: Failed to send out ACK!\n");
	}
	return success;
}

int TCPWithinRcvWindow(gpacket_t *in_pkt, tcptcb_t *con){
	ip_packet_t *ip_pkt = (ip_packet_t *)(in_pkt->data.data);
	int iphdrlen = ip_pkt->ip_hdr_len * 4;
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + iphdrlen);
	uint16_t seg_len = ntohs(ip_pkt->ip_pkt_len) - (iphdrlen + tcphdr->doff*4);
	uint32_t seq = ntohl(tcphdr->seq);
	printf("[TCPWithinRcvWindow]:: Segment length was %d\n", seg_len);

	// segment length of 0
	if (seg_len == 0){
		if (con->tcp_RCV_WND == 0){
			return seq == con->tcp_RCV_NXT;
		} else {
			return con->tcp_RCV_NXT <= seq && seq < con->tcp_RCV_NXT + con->tcp_RCV_WND;
		}
	// segment length > 0	
	} else {
		if (con->tcp_RCV_WND == 0){
			return 0;
		} else {
			return ((con->tcp_RCV_NXT <= seq) && (seq < con->tcp_RCV_NXT + con->tcp_RCV_WND)) ||
				((con->tcp_RCV_NXT <= seq + seg_len - 1) && (seq + seg_len - 1 < con->tcp_RCV_NXT + con->tcp_RCV_WND));
		}
	}

	// if we're here, we have bigger problems to worry about ... 
	return 0;
}

void TCPPrintTCB(tcptcb_t *tcb){
	char tmpbuf[MAX_TMPBUF_LEN];
	printf("source ip: %s, source port: %d\tdest ip: %s, dest port:%d\n", 
		IP2Dot(tmpbuf, gNtohl(tmpbuf+20, (tcb->tcp_source)->tcp_ip)), (tcb->tcp_source)->tcp_port,
		IP2Dot(tmpbuf+60, gNtohl((tmpbuf+80), (tcb->tcp_dest)->tcp_ip)), (tcb->tcp_dest)->tcp_port);
	printf("State is: %i\n",tcb->tcp_state);
	printf("SND_UNA = %lu, SND_NXT = %lu, SND_WND = %u, SND_UP = %u, SND_WL1 = %lu, SND_WL2 = %lu, ISS = %lu\n",tcb->tcp_SND_UNA,tcb->tcp_SND_NXT,tcb->tcp_SND_WND,tcb->tcp_SND_UP,tcb->tcp_SND_WL1,tcb->tcp_SND_WL2,tcb->tcp_ISS);
	printf("RCV_NXT = %lu, RCV_WND = %u, RCV_UP = %u, IRS = %lu\n",tcb->tcp_RCV_NXT,tcb->tcp_RCV_WND,tcb->tcp_RCV_UP,tcb->tcp_IRS);
	printf("SEG_SEQ = %lu, SEG_ACK = %lu, SEG_LEN = %u, SEG_WND = %u, SEG_UP = %u\n",tcb->tcp_SEG_SEQ,tcb->tcp_SEG_ACK,tcb->tcp_SEG_LEN,tcb->tcp_SEG_WND,tcb->tcp_SEG_UP);
}

gpacket_t *TCPNewPacket(tcptcb_t* con){
	// allocate the IP packet 
	char tmpbuf[MAX_TMPBUF_LEN];
	gpacket_t *out_pkt = (gpacket_t *) malloc(sizeof(gpacket_t));
	
	//extract the ip packet header and header length
	ip_packet_t *ip_pkt = (ip_packet_t *)(out_pkt->data.data);
	ip_pkt->ip_hdr_len = 5;
	
	//create the tcp header 
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + ip_pkt->ip_hdr_len*4);
	tcphdr->dport = htons((con->tcp_dest)->tcp_port);
	tcphdr->sport = htons((con->tcp_source)->tcp_port);
	tcphdr->doff = 5;
	tcphdr->ACK = 0;
	tcphdr->SYN = 0;
	tcphdr->FIN = 0;
	tcphdr->RST = 0;
	tcphdr->PSH = 0;
	tcphdr->URG = 0;
	tcphdr->win_size = htons(con->tcp_RCV_WND);

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
	
	tcphdr->ACK = (uint8_t)1;
	tcphdr->FIN = (uint8_t)1;
	ip_pkt->ip_pkt_len = htons(ip_pkt->ip_hdr_len*4 + TCP_HEADER_LENGTH);
	tcphdr->checksum = 0;
	tcphdr->checksum = htons(TCPChecksum(out_pkt));

	int success = IPOutgoingPacket(out_pkt, (con->tcp_dest)->tcp_ip, TCP_HEADER_LENGTH, 1, TCP_PROTOCOL);
	if (success == EXIT_SUCCESS){
		printf("[TCPRequestClose]:: Sent out the following request: \n");
		printTCPPacket(out_pkt);
		con->tcp_state = TCP_FIN_WAIT_1;
	} else {
		printf("[TCPRequestClose]:: Failed to send out close request!\n");
	}
		

	return success;
}

void TCPSendLastAck(int sig){
	printf("[TCPSendLastAck]:: Sending all final acknowledgements ... \n");
	tcptcb_t *cur = active_connections;
	while (cur != NULL){
		if (cur->tcp_state == TCP_CLOSE_WAIT){
			printf("[TCPSendLastAck]:: Sending last ACK for ");
			TCPPrintTCB(cur);

			int iphdrlen = 20;
			gpacket_t *out_pkt = TCPNewPacket(cur);
			ip_packet_t *ip_pkt_out = (ip_packet_t *)(out_pkt->data.data);
			tcphdr_t *tcphdr_out = (tcphdr_t *)((uchar *)ip_pkt_out + iphdrlen);
			tcphdr_out->FIN = (uint8_t)1;
			tcphdr_out->ACK = (uint8_t)1;
			ip_pkt_out->ip_pkt_len = htons(ip_pkt_out->ip_hdr_len*4 + TCP_HEADER_LENGTH);
			tcphdr_out->checksum = 0;
			tcphdr_out->checksum = htons(TCPChecksum(out_pkt));
			IPOutgoingPacket(out_pkt, (cur->tcp_dest)->tcp_ip, TCP_HEADER_LENGTH, 1, TCP_PROTOCOL);
			
			cur->tcp_state = TCP_LAST_ACK;
		}
		cur = cur->next;
	}
}

void TCPCloseWaiting(int sig){
	printf("[TCPCloseWaiting]:: Closing all waiting connections ... \n");
	tcptcb_t *cur = active_connections;
	while (cur != NULL){
		if (cur->tcp_state == TCP_TIME_WAIT){
			printf("[TCPCloseWaiting]:: Closing ");
			TCPPrintTCB(cur);
			cur = TCPRemoveConnection(cur);
		} else {
			cur = cur->next;
		}
	}
}

int TCPAcknowledgeCloseRequest(gpacket_t* in_pkt, tcptcb_t *conn){

	printf("[TCPAcknowledgeConnectionRequest]:: Acknowledging close request. Current state: %d\n", conn->tcp_state);

	int iphdrlen = 20;
	int next_state = conn->tcp_state;	

	ip_packet_t *ip_pkt_in = (ip_packet_t *)(in_pkt->data.data);
	tcphdr_t *tcphdr_in = (tcphdr_t *)((uchar *)ip_pkt_in + iphdrlen);

	if (conn->tcp_state == TCP_ESTABLISHED){
		next_state = TCP_CLOSE_WAIT;

		signal (SIGALRM, TCPSendLastAck);
       		alarm (2);

	} else if (conn->tcp_state == TCP_FIN_WAIT_2){
		next_state = TCP_TIME_WAIT;

		signal(SIGALRM, TCPCloseWaiting);
		alarm (TCP_MSL);
	}

	gpacket_t *out_pkt = TCPNewPacket(conn);
	ip_packet_t *ip_pkt_out = (ip_packet_t *)(out_pkt->data.data);
	tcphdr_t *tcphdr_out = (tcphdr_t *)((uchar *)ip_pkt_out + iphdrlen);
	
	uint32_t seq = ntohl(tcphdr_in->seq) + 1;
	tcphdr_out->seq = tcphdr_in->ack_seq;
	tcphdr_out->ack_seq = htonl(seq);
	tcphdr_out->ACK = (uint8_t)1;

	ip_pkt_out->ip_pkt_len = htons(ip_pkt_out->ip_hdr_len*4 + TCP_HEADER_LENGTH);
	tcphdr_out->checksum = 0;
	tcphdr_out->checksum = htons(TCPChecksum(out_pkt));
	
	int success = IPOutgoingPacket(out_pkt, (conn->tcp_dest)->tcp_ip, TCP_HEADER_LENGTH, 1, TCP_PROTOCOL);
	if (success == EXIT_SUCCESS){
		printf("[TCPAcknowledgeCloseRequest]:: Updated state: %d. Sent out the following request: \n", next_state);
		printTCPPacket(out_pkt);
		conn->tcp_state = next_state;
	} else {
		printf("[TCPAcknowledgeCloseRequest]:: Failed to send out close request!\n");
	}
	
	return success;
}

int TCPRequestConnection(tcptcb_t *con){
	
	printf("[TCPRequestConnection]:: Requesting a connection.\n");

	gpacket_t *out_pkt = TCPNewPacket(con);
	ip_packet_t *ip_pkt = (ip_packet_t *)(out_pkt->data.data);
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + ip_pkt->ip_hdr_len * 4);

	// set a random ISN 
	con->tcp_ISS = rand();
	con->tcp_SND_UNA = con->tcp_ISS;
	tcphdr->seq = htonl(con->tcp_ISS);
	tcphdr->SYN = (uint8_t)1;
	
	ip_pkt->ip_pkt_len = htons(ip_pkt->ip_hdr_len*4 + TCP_HEADER_LENGTH);
	tcphdr->checksum = 0;
	tcphdr->checksum = htons(TCPChecksum(out_pkt));

	// set a retransmit timer for the request
	signal (SIGALRM, TCPResendConnectionRequest);
       	alarm (TCP_RTT);

	// set a timeout for the request
	if (con->syn_sent == NULL){
		con->syn_sent = time(NULL);
	}

	// send the pakcet to the ip module for further processing 	
	int success = IPOutgoingPacket(out_pkt, (con->tcp_dest)->tcp_ip, TCP_HEADER_LENGTH, 1, TCP_PROTOCOL);
	if (success == EXIT_SUCCESS){
		printf("[TCPRequestConnection]:: Sent out following request:\n");
		printTCPPacket(out_pkt);
		con->tcp_state = TCP_SYN_SENT;
	} else {
		printf("[TCPRequestConnection]:: Failed to send request!\n");
	}
	
	return success;
}

void TCPProcessOptions(tcphdr_t *in, tcphdr_t *out){
	uchar* buff = (uchar *)in + TCP_HEADER_LENGTH;
	uchar* out_buff = (uchar *)out + TCP_HEADER_LENGTH;
	int i = 0;
	uchar tmpbuff[4];
	while (i < (in->doff - 5) * 4){
		uint8_t kind = (uint8_t) buff[i];
		uint8_t len = (uint8_t) buff[i+1];
		if (kind == TCPOPT_NOP){
				len = 1;
		} else if (kind == TCPOPT_TIMESTAMP){
			uint32_t tval = htonl(buff[i+2]<<24|buff[i+3]<<16|buff[i+4]<<8|buff[i+5]);
			uint32_t techo = buff[i+6]<<24|buff[i+7]<<16|buff[i+8]<<8|buff[i+9];
			unsigned long hdr = htonl(TCPOPT_TSTAMP_HDR);
			unsigned long mytime = htonl(time(NULL));
			memcpy(out_buff, &hdr, 4);
			memcpy(out_buff + 4, &mytime, 4);
			memcpy(out_buff + 8, &tval, 4);

			out->doff = TCP_HEADER_LENGTH/4 + 3;
		}
		i += len;
	}
	printf("\n");
}

int TCPAcknowledgeConnectionRequest(gpacket_t *in_pkt, tcptcb_t* con){
	printf("[TCPAcknowledgeConnectionRequest]:: Acknowledging connection request. Connection state is %u\n", con->tcp_state);

	int iphdrlen = 20;
	char tmpbuf[MAX_TMPBUF_LEN];
	int next_state = con->tcp_state;	

	ip_packet_t *ip_pkt_in = (ip_packet_t *)(in_pkt->data.data);
	tcphdr_t *tcphdr_in = (tcphdr_t *)((uchar *)ip_pkt_in + iphdrlen);

	// was a passive waiting connection, to update the socket in the tcb
	if (TCPIsPassive(con->tcp_dest)){
		COPY_IP((con->tcp_dest)->tcp_ip, gNtohl(tmpbuf, ip_pkt_in->ip_src));
		(con->tcp_dest)->tcp_port = ntohs(tcphdr_in->sport);
	}

	// create a response 
	gpacket_t *out_pkt = TCPNewPacket(con);
	ip_packet_t *ip_pkt_out = (ip_packet_t *)(out_pkt->data.data);
	tcphdr_t *tcphdr_out = (tcphdr_t *)((uchar *)ip_pkt_out + iphdrlen);
	uint32_t seq = ntohl(tcphdr_in->seq) + 1;
	tcphdr_out->ack_seq = htonl(seq);
	
	// received first syn
	if (con->tcp_state == TCP_LISTEN || con->tcp_state == TCP_SYN_RECEIVED){
		printf("[TCPAcknowledgeConnectionRequest]:: Second part of handshaking phase ...\n");
		// set a random ISN for my sequence number
		con->tcp_ISS = rand();
		con->tcp_SND_NXT = con->tcp_ISS + 1;
		con->tcp_SND_UNA = con->tcp_ISS;
		next_state = TCP_SYN_RECEIVED;	
		tcphdr_out->seq = htonl(con->tcp_ISS);
		tcphdr_out->SYN = (uint8_t)1;

		TCPProcessOptions(tcphdr_in, tcphdr_out);
		
	// third phase of handshake 
	} else if (con->tcp_state == TCP_SYN_SENT){
		printf("[TCPAcknowledgeConnectionRequest]:: Third part of handshaking phase ...\n");
		next_state = TCP_ESTABLISHED;
		tcphdr_out->seq = tcphdr_in->ack_seq;
		con->tcp_SND_UNA = tcphdr_in->ack_seq;
		con->tcp_RCV_NXT = seq;
		con->tcp_SND_NXT = ntohl(tcphdr_out->seq);

	// must be an error 
	} else {
		// TODO: figure out proper handling
		return EXIT_FAILURE;
	}

	tcphdr_out->ACK = (uint8_t)1;
	con->tcp_IRS = ntohl(tcphdr_in->seq);
	ip_pkt_out->ip_pkt_len = htons(ip_pkt_out->ip_hdr_len*4 + TCP_HEADER_LENGTH + (tcphdr_out->doff*4 - TCP_HEADER_LENGTH));
	tcphdr_out->checksum = 0;
	tcphdr_out->checksum = htons(TCPChecksum(out_pkt));

	int success = IPOutgoingPacket(out_pkt, (con->tcp_dest)->tcp_ip, TCP_HEADER_LENGTH + (tcphdr_out->doff*4 - TCP_HEADER_LENGTH), 1, TCP_PROTOCOL);
	if (success == EXIT_SUCCESS){
		printf("[TCPAcknowledgeConnectionRequest]:: Sent out following ACK:\n");
		con->tcp_state = next_state;
		TCPPrintTCB(con);
		printTCPPacket(out_pkt);

		if (con->tcp_state == TCP_ESTABLISHED)
			printf("[TCPAcknowledgeConnectionRequest]:: Connection established ...\n");
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
	TCPResetConnection(con);
	ClearBuffer(con->rcv_buff);

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

void TCPEnqueueSend(gpacket_t *out, tcptcb_t *con){

	ip_packet_t *ip_pkt = (ip_packet_t *)out->data.data; 
	int len_tcp = ntohs(ip_pkt->ip_pkt_len) - ip_pkt->ip_hdr_len * 4;
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *) ip_pkt + ip_pkt->ip_hdr_len * 4);	

	tcpresend_t *cur = con->tcp_send_queue;
	tcpresend_t *pkt = (tcpresend_t *)malloc(sizeof(tcpresend_t));
	
	pkt->seq = tcphdr->seq;
	pkt->len = len_tcp;
	pkt->pkt = out;
	pkt->time_enqueued = time(NULL);
	pkt->next = NULL;
	if (cur == NULL){
		con->tcp_send_queue = pkt;
	} else {
		while (cur->next != NULL) cur = cur->next;
		cur->next = pkt;
	}
}

void TCPResendAll(int sig){
	tcptcb_t *cur = active_connections;
	time_t now = time(NULL);
	while (cur != NULL){
		tcpresend_t *q = cur->tcp_send_queue;
		while (q != NULL){
			if (difftime(now, q->time_enqueued) > TCP_RTT){				
				IPOutgoingPacket(q->pkt, (cur->tcp_dest)->tcp_ip, q->len + TCP_HEADER_LENGTH, 1, TCP_PROTOCOL);
			}
			q = q->next;
		}
		cur = cur->next;
	}
}

tcpresend_t *TCPGetByACK(uint32_t ack, tcptcb_t *con){
	tcpresend_t *cur = con->tcp_send_queue;
	while (cur != NULL){
		if (ntohl(cur->seq) + cur->len == ntohl(ack)){
			return cur;
		}
		cur = cur->next;
	}
	return NULL;
}

// example usage: TCPRemoveSent(TCPGetByAck(ack, con), con);	
void TCPRemoveSent(tcpresend_t *pkt, tcptcb_t *con){
	if (pkt == NULL)
		return;

	tcpresend_t *cur = con->tcp_send_queue;
	// TODO: error checking ... 
	if (cur == pkt){
		con->tcp_send_queue = cur->next;
		free(cur);
	} else {
		tcpresend_t *prev = cur;
		while (cur->next != NULL){
			if (cur == pkt){
				prev->next = cur->next;
				free(cur);
				break;
			}
			prev = cur;
			cur = cur->next;
		}
	}
}

// example usage: TCPRemoveReceieve(pkt, con);	
void TCPRemoveReceieve(tcpresend_t *pkt, tcptcb_t *con){
	if (pkt == NULL)
		return;

	tcpresend_t *cur = con->tcp_receieve_queue;
	// TODO: error checking ... 
	if (cur == pkt){
		printf("[TCPRemoveReceive]:: Removing head ... \n");
		con->tcp_receieve_queue = cur->next;
		free(cur);
	} else {
		tcpresend_t *prev = cur;
		while (cur->next != NULL){
			if (cur == pkt){
				prev->next = cur->next;
				free(cur);
				break;
			}
			prev = cur;
			cur = cur->next;
		}
	}
}

// This will enqueue the packet in the receiver's queue
// if the data lies in the receiver window 
void TCPEnqueueReceived(gpacket_t *in, tcptcb_t *con){
	tcpresend_t *cur = con->tcp_receieve_queue;
	ip_packet_t *ip_pkt = (ip_packet_t *)in->data.data; 
	int len_tcp = ntohs(ip_pkt->ip_pkt_len) - ip_pkt->ip_hdr_len * 4;
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *) ip_pkt + ip_pkt->ip_hdr_len * 4);	
	tcpresend_t *nxt;

	tcpresend_t *pkt = (tcpresend_t *)malloc(sizeof(tcpresend_t));
	pkt->len = len_tcp - tcphdr->doff *4;
	pkt->seq = ntohl(tcphdr->seq);
	pkt->time_enqueued = time(NULL);
	pkt->pkt = in;
	pkt->next = NULL;

	if (cur == NULL){// the receiver queue is empty
		con->tcp_receieve_queue = pkt;
		printf("[TCPEnqueueReceived]:: receiver queue is empty, inserting seq: %lu\n",pkt->seq);
		return;
	}else if (pkt->seq < cur->seq){// in pkt should put in front of receiver queue
		//printf("[TCPEnqueueReceived]:: pkt seq %lu < cur seq %lu\n",pkt->seq, cur->seq);
		printf("[TCPEnqueueReceived]:: pkt in front of queue\n");
		pkt->next = cur;
		con->tcp_receieve_queue = pkt;
		return;
	}

	while (cur != NULL){// insert pkt
		nxt = cur->next;
		if (nxt==NULL){
			//printf("[TCPEnqueueReceived]:: next is null, insert cur %lu\n", cur->seq);
			printf("[TCPEnqueueReceived]:: next is null, insert pkt\n");
			cur->next = pkt;
			return;
		}else if ((pkt->seq > cur->seq) && (pkt->seq < nxt->seq)){
			// in pkt is earlier than cur, insert the pkt
			//printf("[TCPEnqueueReceived]:: pkt seq %lu > cur seq %lu\n",pkt->seq, cur->seq);
			printf("[TCPEnqueueReceived]:: inserting pkt\n");
			pkt->next = nxt;
			cur->next = pkt;
			return;
		}else if (pkt->seq == cur->seq){
			// received the same pak again, ignore, but resend ack
			return;
		}

		cur = cur->next;
	}
	return;
	
}

// this will shift the receiver window from RCV.NXT to 
void TCPShiftQueue(tcptcb_t *con){
	TCPPrintTCB(con);
	// Check if receiver queue start at RCV.NXT
	tcpresend_t *cur = con->tcp_receieve_queue;
	printf("[TCPShiftQueue]:: cur seq is %lu\n",cur->seq);
		
	while (cur->seq == con->tcp_RCV_NXT){
		// Correct next packet, push it.
		TCPWriteToReceiveBuffer(con,cur->pkt);
		con->tcp_RCV_NXT = cur->seq + cur->len;

		TCPRemoveReceieve(cur, con);
		printf("[TCPShiftQueue]:: cur seq and rcv.nxt are %u %u\n",cur->seq,con->tcp_RCV_NXT);
	}
}

void TCPWriteToReceiveBuffer(tcptcb_t *con, gpacket_t *in){
	uchar *buffer = con->rcv_buff;
	int i = 0; char c;
	while (i < TCP_MAX_WIN_SIZE && (c = buffer[i]) != NULL) {
		i++;
	}

	// receive buffer is full!
	// TODO: handle this 
	if (i == TCP_MAX_WIN_SIZE){
		con->tcp_RCV_WND = 0;
		return;
	}

	ip_packet_t *ip_pkt = (ip_packet_t *)in->data.data; 
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + ip_pkt->ip_hdr_len * 4);
	int len_tcp = ntohs(ip_pkt->ip_pkt_len) - (ip_pkt->ip_hdr_len * 4 + tcphdr->doff * 4);
	printf("[TCPWriteToReceiveBuffer]:: About to write %d bytes to buffer ... \n", len_tcp);
	printf("%s\n", (uchar *)tcphdr + tcphdr->doff * 4);	
	memcpy(buffer + i, (uchar *)tcphdr + tcphdr->doff * 4, len_tcp); 
}


void ClearBuffer(uchar *buff){
	int i = 0; 
	for (i = 0; i < TCP_MAX_WIN_SIZE; i++) buff[i] = NULL;
}


void TCPReceive(uchar src_ip[], uint16_t src_port, uchar dest_ip[], uint16_t dest_port, uchar* recv_buff){
	tcptcb_t *con = TCPGetConnection(src_ip, src_port, dest_ip, dest_port);
	
	if (con != NULL){
		printf("[TCPReceive]:: Copying into buffer ... \n");
		strcpy(recv_buff, con->rcv_buff);
		printf("Receive buffer was: %s\n", con->rcv_buff);
		ClearBuffer(con->rcv_buff);
		
		return EXIT_SUCCESS; 
	} else {
		printf("[TCPReceive]:: No one listening on port!\n");
		return EXIT_FAILURE;
	}
}

// TODO: need to test
void TCPSend(uchar src_ip[],uint16_t src_port, uchar dest_ip[], uint16_t dest_port, uchar* buff, int len){
   	char tmpbuf[MAX_TMPBUF_LEN];    
	printf("[TCPSend]:: Going to send out packet.\n");
	
	tcptcb_t * con = TCPGetConnection(src_ip, src_port, dest_ip, dest_port);
	gpacket_t *out_pkt = TCPNewPacket(con);
	ip_packet_t *ip_pkt = (ip_packet_t *)(out_pkt->data.data);
	tcphdr_t *tcphdr = (tcphdr_t *)((uchar *)ip_pkt + ip_pkt->ip_hdr_len * 4);

	tcphdr->seq = htonl(con->tcp_SND_NXT);
	tcphdr->ack_seq = htonl(con->tcp_RCV_NXT);
	con->tcp_SND_NXT += len;

	//temporarely set the urgent pointer to 0
	tcphdr->urg_ptr =0;
	tcphdr->ACK = (uint8_t)1;
	tcphdr->PSH = (uint8_t)1;

	strncpy((uchar *)tcphdr + tcphdr->doff *4 , buff , len);

	ip_pkt->ip_pkt_len = htons(ip_pkt->ip_hdr_len*4 + TCP_HEADER_LENGTH + len);
	tcphdr->checksum = 0;
	tcphdr->checksum = htons(TCPChecksum(out_pkt));

	int success = IPOutgoingPacket(out_pkt, (con->tcp_dest)->tcp_ip, tcphdr->doff * 4 + len, 1, TCP_PROTOCOL);
	if (success == EXIT_SUCCESS){
		printf("[TCPSend]:: Sent out the following request: \n");
		printTCPPacket(out_pkt);
		
		//after transmission, put on the unacked list
		TCPEnqueueSend(out_pkt, con);

		//set the timer for retransmission
		signal (SIGALRM, TCPResendAll);
		alarm (TCP_RTT);
		
	} else {
		printf("[TCPSend]:: Failed to sent out the following request: \n");
	}

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
	uint16_t tcp_len = ntohs(ip_pkt->ip_pkt_len) - iphdrlen;

	/* 
	 * BEGIN STATE MACHINE HERE
	 */
	tcptcb_t* conn = TCPGetConnection(gNtohl(tmpbuff, ip_pkt->ip_dst), ntohs(tcphdr->dport), gNtohl(tmpbuff+20, ip_pkt->ip_src), ntohs(tcphdr->sport));
	
	int checksum = TCPChecksum(in_pkt);
	if (checksum){
		printf("[TCPProcess]:: Checksum error! Dropping packet ...%02X\n", ntohs(checksum));
		return EXIT_FAILURE;
	}

	// no TCB's waiting for where the packet came from 
	if (conn == NULL){
		printf("[TCPProcess]:: No one listening on destination ip and port.... \n");
		if (tcphdr->SYN == 1){
			printf("[TCPProcess]:: Sending out an RST\n");
			TCPSendRST(in_pkt);
		}
		return EXIT_FAILURE;
	}

	conn->tcp_SND_WND = ntohs(tcphdr->win_size);

	// request for open 
	if (tcphdr->SYN == 1){
	
		// connection got screwed up.. start over
		if (conn->tcp_state == TCP_ESTABLISHED)
			conn->tcp_state == TCP_LISTEN;

		TCPAcknowledgeConnectionRequest(in_pkt, conn);
	
	// request for close
	} else if (tcphdr->FIN == 1){
		TCPAcknowledgeCloseRequest(in_pkt, conn);

	// regular data segment or ack
	} else if (tcphdr->ACK == 1){

		// final ACK for handshake 
		if (conn->tcp_state == TCP_SYN_RECEIVED){
			conn->tcp_state = TCP_ESTABLISHED;
			printf("[TCPProcess]:: Connection fully established ... \n");

		// first ack for initiated close 
		} else if (conn->tcp_state == TCP_FIN_WAIT_1){
			conn->tcp_state = TCP_FIN_WAIT_2;

		// waiting for final ack to close 
		} else if (conn->tcp_state == TCP_LAST_ACK){
			printf("[TCPProcess]:: Terminating connection ... \n");
			TCPRemoveConnection(conn);
		// regular segment 		
		} else if (conn->tcp_state == TCP_ESTABLISHED){

			// have some data 
			if (tcp_len - tcphdr->doff * 4 > 0){
				if (TCPWithinRcvWindow(in_pkt, conn)){
					printf("[TCPProcess]:: Received segment within receive window ... enqueuing.\n");
					TCPEnqueueReceived(in_pkt, conn);
					TCPAcknowledgeReceived(in_pkt, conn);
					TCPShiftQueue(conn);
				} else {
					printf("[TCPProcess]:: Packet was outside of receive window .. dropping!\n");
				}
			// probably an ACK
			} else {
				printf("[TCPProcess]:: Sequence number %u was ACK'd ... \n", ntohl(tcphdr->seq));
				
				// remove from sender queue
				TCPRemoveSent(TCPGetByACK(tcphdr->seq, conn), conn);
			}
				
		} else if (tcphdr->RST == 1){

			if (conn->tcp_state == TCP_SYN_SENT || conn->tcp_state == TCP_SYN_RECEIVED){
				printf("[TCPProcess]:: Received RST\n");
				if (TCPWithinRcvWindow(in_pkt, conn)){
					printf("[TCPProcess]:: Valid RST, returning to LISTEN state ... \n"); 
					conn->tcp_state = TCP_LISTEN;
				} else {
					printf("[TCPProcess]:: Invalid RST, ignoring ... \n");
				}
			} else {
				printf("[TCPProcess]:: Received RST, aborting!! \n");
				//TODO: figure out state to put connection into
			}
				
		}

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

