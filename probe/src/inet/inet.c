/* source header */
#include "./inet.h"

/* std */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
/* math and bitwise */
#include <math.h>
/* format */
#include <inttypes.h>
/* signals */
#include <signal.h>
/* inet */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
/* open() */
#include <fcntl.h>
/* string operations */
#include <string.h>

/* inetbus */
#include "../inetbus/inetbus.h"
/* custom types */
#include "../types.h"

/* inet log file descriptor*/
int inet_log_fd = 1;

/*	Create or open an inet_log file
	and return its file descriptor */
int inet_open_log_file() {
	remove("./.inet_log");
	fclose(fopen("./.inet_log", "w"));
	int fd = open("./.inet_log", O_WRONLY);
	return fd;
}

/*	destroy the log file descriptor and close the file
	tied to it */
int inet_close_log_file() {
	return close(inet_log_fd);
}

/*	make an entry to the .inet_log file
	using the standard format method */
void inet_log(const char* _format, ...) {
	va_list args;
	va_start(args, _format);
	
	#ifdef INET_LOG
		dprintf(inet_log_fd, "[INET] ");
		vdprintf(inet_log_fd, _format, args);
	#endif

	va_end(args);
}

/* 	initialise a new TCP socket of type RAW_SOCK */
int inet_create_raw_socket() {
	/* create raw socket */
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
   		inet_log("failed to init raw socket\n");
    	return -1;
    }

	/* disable automatic ip header addition to assemble
	it manualy */
	int one = 1;
	/* 'one' variable is used as the value for the option
	in setsockopt; the function itself accepts values of
	type void* (any) in order to set the value to an option,
	which means we're supposed to pass the reference to the
	value we want to set and it's size for the function to work
	properly and change the option */
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
	    perror("setsockopt; inet.c");
	    close(sock);
	    return -1;
	}

    inet_log("created raw socket=%d\n", sock);
    return sock;
}

/* 	Initialise a new socket of type SOCK_STREAM */
int inet_create_stream_socket() {

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sock < 0) {
    	inet_log("failed to create stream socket\n");
    	return -1;
    }

    inet_log("created stream socket=%d\n", sock);
    return sock;
}

/* 	connect to the address that is currently set on the
	inet_bus through the passed socket */
enum Inet_Ret_Code inet_connect_socket(int _socket) {
	inet_log("connecting socket=%d; port=%ld\n", _socket, GlobalInetBus.port);
	int st = connect(_socket, (struct sockaddr *)&GlobalInetBus.addr,
	            sizeof(GlobalInetBus.addr));
	if (st == 0) {
		inet_log("socket=%d connected\n", _socket);
		return INET_SUCCESS;
	} else {
		inet_log("failed to connect socket=%d\n", _socket);
		return INET_CONN_FAILED;
	}
}

/*	send data over passed socket */
enum Inet_Ret_Code inet_send(int _socket, const char* _buffer) {
	inet_log("sending data over socket=%d\n ^- data='%s'\n",
			_socket, _buffer);
    int code = send(_socket, _buffer, strlen(_buffer), 0);
    if (code == 0) {
    	inet_log("data send successfully\n");
        return INET_SUCCESS;
    } else {
    	inet_log("failed to send data\n");
        return INET_SEND_FAILED;
	}
}

/*	read data over passed socket and write it to
	the buffer */
enum Inet_Ret_Code inet_read(int _socket, char* _buffer, size_t _size, size_t _timeout_sec) {
	inet_log("reading data over socket=%d\n", _socket); 

	/* timeout logic */
	fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(_socket, &readfds);

    struct timeval tv;
    tv.tv_sec = _timeout_sec;
    tv.tv_usec = 0;

    int rv = select(_socket + 1, &readfds, NULL, NULL, &tv);
    if (rv == -1) {
        perror("select error; read_data() of inet.c");
        return INET_ITERNAL_ERR;
    } else if (rv == 0) {
        /* timeout */
        inet_log("recv timed out after %ld seconds\n", _timeout_sec);
        return INET_TIMEOUT;
    }
	
    recv(_socket, _buffer, _size, 0);

    return INET_SUCCESS;
}

/*
*	PACKET STRUCTURING LOGIC
*/

void inet_print_buffer_bin(uint8_t *_buf, size_t _size) {
	for (size_t i = 0; i < _size; i++) {
		printf("%.8b ", _buf[i]);
		if ((i+1)%4 == 0)
			printf("\n");
	}
}

void inet_buffer_byte(uint8_t *_buf, size_t _size) {
	for (size_t i = 0; i < _size; i++) {
		printf("%.4x ", _buf[i]);
		if ((i+1)%8 == 0)
			printf("\n");
	}
}

size_t inet_get_buffer_strlen(uint8_t* _buf, size_t _size) {
	size_t i = _size-1;
	size_t out = _size;
	while (_buf[i] == 0) {
		i--;
		out--;
	}

	return out;
}

uint16_t inet_calculate_ip_checksum(struct inet_ip_header* _hdr) {
	for (size_t i = 0; i < sizeof(*_hdr); i++) {
		printf("%.2x ", *((uint8_t*)_hdr + i));
		if ((i+1)%4 == 0)
			printf("\n");
	}
	printf("\n\n");

	uint32_t sum = 0;

	for (size_t i = 0; i < sizeof(*_hdr); i++)
		sum += *((uint8_t*)_hdr + i);
	

	/* printf("sum=%u [0x%x]\n", sum, sum); */
	

	uint32_t mask = (1 << 16) - 1;
	
	uint16_t right = sum & mask;
	uint16_t left = sum >> 16;

	uint16_t checksum = right + left;
	checksum = ~checksum;

	printf("mask=%x; right=%x; left=%x\n", mask, right, left);

	printf("checksum=%x\n", checksum);

	return checksum;
}

uint16_t inet_verify_ip_checksum(struct inet_ip_header* _hdr) {
	uint16_t chk = inet_calculate_ip_checksum(_hdr);
	printf("chk=%x\n", chk);

	return chk;
}

uint16_t inet_calculate_tcp_checksum(struct inet_tcp_header* _hdr, size_t _payload_size) {
	for (size_t i = 0; i < sizeof(*_hdr); i++) {
		printf("%.2x ", *((uint8_t*)_hdr + i));
		if ((i+1)%4 == 0)
			printf("\n");
	}

	struct inet_pseudo_tcp_header ph;
	ph.src_addr = _hdr->src;
	ph.dst_addr = _hdr->dst;
	ph.reserved = 0;
	ph.protocol = 6;
	ph.tcp_length = _payload_size + sizeof(*_hdr);

	uint32_t sum = 0;

	for (size_t i = 0; i < sizeof(ph)/2; i++)
		sum += *((uint16_t*)_hdr + i);

	/* printf("tcp: sum=%x\n", sum); */

	uint32_t mask = (1 << 16) - 1;
	
	uint16_t right = sum & mask;
	uint16_t left = sum >> 16;

	uint16_t checksum = right + left;
	checksum = ~checksum;

	/* printf("mask=%x; right=%x; left=%x\n", mask, right, left);

	printf("checksum=%x\n", checksum); */

	return checksum;
}

/*	fill _buffer of size _size with a SYN packet; returns
	a source port to which the response to the packet
	will be sent */
uint16_t inet_create_syn_packet(uint8_t* _buffer, size_t _size) {
	inet_log("SYN packet: creation started\n");
	/* ip header */
	struct inet_ip_header ip_h;
	memset(&ip_h, 0, sizeof(ip_h));
	/* tcp header */
	struct inet_tcp_header tcp_h;
	memset(&tcp_h, 0, sizeof(tcp_h));

	/* IP header structuring */

	/* version and header length */
	ip_h.ver = 4;
	ip_h.ihl = 5;

	/* type of service */
	ip_h.tos = 0;

	/* identification */
	ip_h.id = htons(0xee27);

	/* IP Flags */
	ip_h.f_off = htons(0xffff & INET_IP_DF_FLAG);

	/* time to live */
	ip_h.ttl = 54;

	/* protocol */
	ip_h.proto = 6;

	/* srouce address */
	ip_h.src = htonl(0x924b755b);

	/* destination address */
	ip_h.dst = htonl(0xac158075);
	
	/* TCP header structuring */

	/* here we choose a random port from the
	dynamic range; thorugh that port we'll get the
	respone packet (SYN-ACK hopefully) */
	uint16_t src_port = 49152 + (rand() % 16384);
	inet_log("SYN packet: source=%u; dest=%u\n", src_port, GlobalInetBus.port);
	
	/* source and destination port */
	tcp_h.src = htons(50000); /* src_port */
	tcp_h.dst = htons(443); /* GlobalInetBus.port */

	/* sequence and ACK sequence (both zero in this case) */
	tcp_h.seq = htonl(0x40004011);
	tcp_h.ack_seq = htons(0x4011);

	/* set the SYN flag on */
	tcp_h.flag_bits.flags.syn = 1;

	/* set the receive window to maximum amount */
	tcp_h.wnd = 0x0;
	
	/* set urgent ptr to zero */
	tcp_h.uptr = 0;

	/* calculate checksum */
	uint16_t c = inet_calculate_tcp_checksum(&tcp_h, 0);
	tcp_h.chk = htons(c);
	printf("\n\n\n");
	inet_calculate_tcp_checksum(&tcp_h, 0);

	printf("SYN PACKET TRACEBACK\n");
	printf("\nIP header:\n\n");
	for (size_t i = 0; i < (sizeof(ip_h)); i++) {
		printf("%.2x ", *((uint8_t*)&ip_h + i));
		if ((i+1)%4 == 0)
			printf("\n");
	}

	printf("\nTCP header:\n\n");
	for (size_t i = 0; i < (sizeof(tcp_h)); i++) {
		printf("%.2x ", *((uint8_t*)&tcp_h + i));
		if ((i+1)%4 == 0)
			printf("\n");
	}

	printf("\nDUMP:\n\n");
	for (size_t i = 0; i < (sizeof(ip_h)); i++) {
		printf("%.2x ", *((uint8_t*)&ip_h + i));
		if ((i+1)%4 == 0)
			printf("\n");
	}

	for (size_t i = 0; i < (sizeof(tcp_h)); i++) {
		printf("%.2x ", *((uint8_t*)&tcp_h + i));
		if ((i+1)%4 == 0)
			printf("\n");
	}
	printf("\n\n");
	
	// inet_verify_tcp_checksum(&tcp_h);
	
	return src_port;
}

/*void inet_copy_field(uint8_t* _dest, size_t _pos, const void* _src, size_t _size) {
	uint8_t* dst = _dest;
	const uint8_t* src = _src;
	for (size_t i = 0; i < _size; i++) {
		dst[i+_pos] = src[i];
	}
}*/

/* struct inet_tcp_header* inet_create_tcp_header
(uint16_t _src, uint16_t _dst, uint32_t _seq, uint32_t _ack,) {

} */

/*	append a passed IP header struct to the begining
	of _buffer; by default the function assumes that
	the buffer is capable of storing the header in it,
	therefore is of size >= buf_size+header_size */
enum Inet_Ret_Code inet_apply_ip_header
(uint8_t* _buffer, size_t _size, size_t _p_size, struct inet_ip_header* _header) {
	/* header size in bytes */
	size_t ihl = _header->ihl*4;

	if (_p_size + ihl > _size)
		return INET_OVERFLOW;
	
	/* temporary buffer; storage of inital _buffer
	state */
	uint8_t tmp[_p_size];
	memcpy(tmp, _buffer, _p_size);

	/* copy header struct into the packet buffer;
	inet_tcp_header's 'packed' attribute makes
	it possible */
	memcpy(_buffer, _header, ihl);

	/* append passed buffer in it's initial state
	to itself; with that we've appended the header
	to the buffer */
	for (size_t i = 0; i < _p_size; i++)
		_buffer[i+ihl] = tmp[i];

	return INET_SUCCESS;
}

/*	append a passed TCP header struct to the begining
	of _buffer; by default the function assumes that
	the buffer is capable of storing the header in it,
	therefore is of size >= buf_size+header_size */
enum Inet_Ret_Code inet_apply_tcp_header
(uint8_t* _buffer, size_t _size, size_t _p_size, struct inet_tcp_header* _header) {
	/* header size in bytes */
		size_t ihl = _header->flag_bits.flags.doff*4;
	
		if (_p_size + ihl > _size)
			return INET_OVERFLOW;
		
		/* temporary buffer; storage of inital _buffer
		state */
		uint8_t tmp[_p_size];
		memcpy(tmp, _buffer, _p_size);
	
		/* copy header struct into the packet buffer;
		inet_tcp_header's 'packed' attribute makes
		it possible */
		memcpy(_buffer, _header, ihl);
	
		/* append passed buffer in it's initial state
		to itself; with that we've appended the header
		to the buffer */
		for (size_t i = 0; i < _p_size; i++)
			_buffer[i+ihl] = tmp[i];
		
		
		/* for (size_t i = 0; i < ihl; i++) {
			printf("%.8b ", hdr[i]);
			if ((i+1)%4 == 0)
				printf("\n");
		} */
	
		return INET_SUCCESS;
}
/*
int main() {
	inet_log_fd = inet_open_log_file();
	uint8_t buffer[100];
	inet_create_syn_packet(buffer, sizeof(buffer));


	inet_close_log_file();
	return 0;
}
*/
