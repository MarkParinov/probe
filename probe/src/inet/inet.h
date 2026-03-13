/*
	INET module header file
*/

#ifndef INET_H
#define INET_H

#include <unistd.h>
#include <stdint.h>

extern int inet_log_fd;

/* File operations */
int inet_open_log_file();
int inet_close_log_file();

/* Socket creation */
int inet_create_raw_socket();
int inet_create_stream_socket();

/* Socket inet operations */
enum Inet_Ret_Code inet_connect_socket(int _socket);
enum Inet_Ret_Code inet_send(int _socket, const char* _buffer);
enum Inet_Ret_Code inet_read(int _socket, char* _buffer, size_t _size, size_t _timeout_sec);

/* IP header flags in HOST byte order */
#define INET_IP_RESERVED_FLAG 1 << 15
#define INET_IP_DF_FLAG 1 << 14
#define INET_IP_MF_FLAG 1 << 13


#define INET_TCP 1

/*	IP header fields structure */
struct inet_ip_header {
	/* 0-3 bytes */
	/* on little endian header length comes first, 
	then the IP version */
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ihl:4;	/* header length */
    unsigned int ver:4;	/* version */
#elif __BYTE_ORDER == __BIG_ENDIAN
    unsigned int ver:4;	/* version */
    unsigned int ihl:4;	/* header length */
#endif
	uint8_t			tos;	/* type of service */
	uint16_t		t_len;	/* total length */

	/* 4-7 bytes */
	uint16_t		id;		/* identification */
	uint16_t		f_off;	/* fragment offset */

	/* 8-11 bytes */
	uint8_t			ttl;	/* time to live */
	uint8_t			proto;		/* protocol */
	uint16_t		chk;	/* header checksum */

	/* 12-15 bytes */
	uint32_t		src;	/* source address */

	/* 16-19 bytes */
	uint32_t		dst;	/* destination address */

	uint8_t			opt[0];	/* variable length options */
	
} __attribute__((packed));

/*	TCP header fiel .ds structure */
struct inet_tcp_header {
    /* 0-3 bytes */
    uint16_t src;	/* source port */
    uint16_t dst;	/* destination port */
    
    /* 4-7 bytes */
    uint32_t seq;
    
    /* 8-11 bytes */
    uint32_t ack_seq;

	/* 12-13 bytes */
	union {
		uint16_t flags_off;

		struct {
		#if __BYTE_ORDER == __LITTLE_ENDIAN
		    /* On wire (network order): [doff|res1][flags] */
		    /* In memory (little-endian): flags come first! */
		    uint16_t res1:4;   /* Reserved bits */
		    uint16_t doff:4;   /* Data offset (32-bit words) */
		    uint16_t fin:1;    /* FIN flag */
		    uint16_t syn:1;    /* SYN flag */
		    uint16_t rst:1;    /* RST flag */
		    uint16_t psh:1;    /* PSH flag */
		    uint16_t ack:1;    /* ACK flag */
		    uint16_t urg:1;    /* URG flag */
		    uint16_t ece:1;    /* ECE flag */
		    uint16_t cwr:1;    /* CWR flag */
		#elif __BYTE_ORDER == __BIG_ENDIAN
		    /* On wire and in memory are the same */
		    uint16_t doff:4;   /* Data offset comes first */
		    uint16_t res1:4;   /* Reserved bits */
		    uint16_t cwr:1;    /* CWR flag */
		    uint16_t ece:1;    /* ECE flag */
		    uint16_t urg:1;    /* URG flag */
		    uint16_t ack:1;    /* ACK flag */
		    uint16_t psh:1;    /* PSH flag */
		    uint16_t rst:1;    /* RST flag */
		    uint16_t syn:1;    /* SYN flag */
		    uint16_t fin:1;    /* FIN flag */
		#endif
		} flags;
	} flag_bits;
    
    /* 14-15 bytes */
    uint16_t wnd;	/* receive window */
    
    /* 16-17 bytes */
    uint16_t chk;	/* checksum */
    
    /* 18-19 bytes */
    uint16_t uptr;	/* urgent pointer */

    /* 19-... bytes */
    uint8_t opt[0];	/* options */
} __attribute__((packed));

/*	pseudo header for checksum calculation */
struct inet_pseudo_tcp_header {
    uint32_t src_addr;     // source IP address
    uint32_t dst_addr;    // destination IP address
    uint8_t  reserved;     // reserved (zero)
    uint8_t  protocol;     // protocol (IPPROTO_TCP = 6)
    uint16_t tcp_length;   // TCP segment length (header + data) [bytes]
};

/*	defines an abstract inet packet structure, which
	is used to the final data holding a packet with
	TCP and IP headers applied to it manualy*/
struct inet_packet {
	struct inet_tcp_header	tcp_hdr;
	struct inet_ip_header	ip_hdr;
	uint8_t*				payload;
};

#endif
