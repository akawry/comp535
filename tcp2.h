/*
 * tcp.h (collection of functions that implement the TCP (transfer control protocol).
 */

#ifndef __TCP_H_
#define __TCP_H_

#include <sys/time.h>
#include <stdint.h>
#include "message.h"
#include "simplequeue.h"


#define TCP_MSL 10
#define TCP_HEADER_LENGTH 20
#define TCP_MAX_WIN_SIZE 100
#define TCP_DEFAULT_WIN_SIZE 64
#define TCP_RTT 2
#define TCP_TIMEOUT 15

/**
 * taken from src/sys/netinet/tcp.h
 */
#define TCPOPT_TIMESTAMP 8
#define TCPOLEN_TIMESTAMP 10
#define	TCPOPT_EOL 0
#define	TCPOPT_NOP 1
#define	TCPOPT_MAXSEG		2
#define    TCPOLEN_MAXSEG		4
#define TCPOPT_WINDOW		3
#define    TCPOLEN_WINDOW		3
#define TCPOPT_SACK_PERMITTED	4		/* Experimental */
#define    TCPOLEN_SACK_PERMITTED	2
#define TCPOPT_SACK		5		/* Experimental */
#define TCPOPT_TIMESTAMP 8
#define    TCPOLEN_TIMESTAMP		10
#define    TCPOLEN_TSTAMP_APPA		(TCPOLEN_TIMESTAMP+2) /* appendix A */
#define    TCPOPT_TSTAMP_HDR		\
    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_TIMESTAMP<<8|TCPOLEN_TIMESTAMP)

typedef struct _tcphdr_t 
{
	uint16_t sport;
	uint16_t dport;
	uint32_t seq;
	uint32_t ack_seq;

	#  if __BYTE_ORDER == __LITTLE_ENDIAN
        uint8_t res1:4;
        uint8_t doff:4;
        uint8_t FIN:1;
        uint8_t SYN:1;
        uint8_t RST:1;
        uint8_t PSH:1;
        uint8_t ACK:1;
        uint8_t URG:1;
        uint8_t	res2:2;

        #  elif __BYTE_ORDER == __BIG_ENDIAN
        uint8_t doff:4;
        uint8_t res1:4;
        uint8_t res2:2;
        uint8_t URG:1;
        uint8_t ACK:1;
        uint8_t PSH:1;
        uint8_t RST:1;
        uint8_t SYN:1;
        uint8_t FIN:1;
        #  endif

	uint16_t win_size;
	uint16_t checksum;
	uint16_t urg_ptr;
  
} tcphdr_t;

typedef struct _tcpsocket_t
{
	uint16_t tcp_port;
	uchar tcp_ip[4];
} tcpsocket_t;

#define TCP_LISTEN 1
#define TCP_SYN_SENT 2 
#define TCP_SYN_RECEIVED 3
#define TCP_ESTABLISHED 4
#define TCP_FIN_WAIT_1 5
#define TCP_FIN_WAIT_2 6 
#define TCP_CLOSE_WAIT 7
#define TCP_CLOSING 8
#define TCP_LAST_ACK 9
#define TCP_TIME_WAIT 10

typedef struct _tcpresend_t
{
	int len;
	uint32_t seq;
	time_t time_enqueued;
	gpacket_t *pkt;
	struct tcpresend_t *next;
} tcpresend_t;

typedef struct _tcptcb_t
{
	tcpsocket_t *tcp_source;
	tcpsocket_t *tcp_dest;

	int tcp_state;  //  state of the connection (one of LISTEN, SYN-SENT, SYN-RECEIVED, ESTABLISHED, FIN-WAIT-1, FIN-WAIT-2, CLOSE-WAIT, CLOSING, LAST-ACK, TIME-WAIT)
	time_t syn_sent;
	int tcp_was_passive;

	/*
	 * Send Sequence Variables
	 */
	uint32_t tcp_SND_UNA; //  send unacknowledged (oldest unacknowledged sequence number)
      	uint32_t tcp_SND_NXT; //  send next (next sequence number to be sent)
      	uint16_t tcp_SND_WND; //  send window
      	uint16_t tcp_SND_UP;  //  send urgent pointer
      	uint32_t tcp_SND_WL1; //  segment sequence number used for last window update
      	uint32_t tcp_SND_WL2; //  segment acknowledgment number used for last window update
      	uint32_t tcp_ISS;     //  initial send sequence number


	/*
	 * Receive Sequence Variables
	 */
	uint32_t tcp_RCV_NXT; //  receive next
      	uint16_t tcp_RCV_WND; //  receive window
      	uint16_t tcp_RCV_UP;  //  receive urgent pointer
      	uint32_t tcp_IRS;     //  initial receive sequence number

	/*
	 * Segment Sequence Variables
	 */
	uint32_t tcp_SEG_SEQ; //  segment sequence number (first sequence number of a segment)
      	uint32_t tcp_SEG_ACK; //  segment acknowledgment number acknowledgment (next sequence number expected by the receiving TCP)
      	uint16_t tcp_SEG_LEN; //  segment length (the number of octets occupied by the data in the segment)
      	uint16_t tcp_SEG_WND; //  segment window
      	uint16_t tcp_SEG_UP;  //  segment urgent pointer

	char rcv_buff[TCP_MAX_WIN_SIZE];	//receive buffer 
	tcpresend_t *tcp_send_queue;	//send buffer 	
	tcpresend_t *tcp_receieve_queue; //receive queue

	struct tcptcb_t *next; // linked list
} tcptcb_t;


// function prototypes
void ClearBuffer(uchar* buff);
gpacket_t *TCPNewPacket(tcptcb_t* con);
void TCPSendLastAck(int sig);
void TCPCloseWaiting(int sig);
tcptcb_t *TCPRemoveConnection(tcptcb_t *conn);
int TCPClose(uchar src_ip[], uint16_t src_port, uchar dest_ip[], uint16_t dest_port);
int TCPOpen(uchar src_ip[], uint16_t src_port, uchar dest_ip[], uint16_t dest_port);
int TCPProcess(gpacket_t *in_pkt);
uint16_t TCPChecksum(gpacket_t *in_pkt);

#endif
