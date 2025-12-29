// source header
#include "./inet.h"
// std
#include <stdlib.h>
#include <stdio.h>
// signals
#include <signal.h>
// inet
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
// open()
#include <fcntl.h>
// string operations
#include <string.h>

// inetbus
#include "../inetbus/inetbus.h"
// custom types
#include "../types.h"

int inet_log_fd;

// Create or open an inet_log file
// and return its file descriptor
int inet_open_log_file() {
	int fd = open("./.inet_log", O_WRONLY);
	return fd;
}

// Destroy a file descriptor and
// close the file tied to it
int inet_close_log_file() {
	return close(inet_log_fd);
}

// Initialise a new TCP socket of
// type RAW_SOCK
int inet_create_raw_sock() {
	dprintf(inet_log_fd, "creating raw socket\n");
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock <= -1) {
    	dprintf(inet_log_fd, "failed to init raw socket\n");
    	return -1;
    }
    
    return sock;
}

// Initialise a new socket of
// type SOCK_STREAM
int inet_create_stream_sock() {
	dprintf(inet_log_fd, "initing a socket\n");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock <= -1) {
    	dprintf(inet_log_fd, "failed to init socket\n");
    	return -1;
    }
    
    return sock;
}

// Connect to the address that is currently
// set on the inet_bus through the passed
// socket
enum Inet_Ret_Code inet_socket_connect(int _socket) {
	dprintf(inet_log_fd, "connecting socket %d\n", _socket);
	int st = connect(_socket, (struct sockaddr *)&GlobalInetBus.addr,
	            sizeof(GlobalInetBus.addr));
	if (st == 0) {
		dprintf(inet_log_fd, "socket %d connected\n", _socket);
		return INET_SUCCESS;
	} else {
		dprintf(inet_log_fd, "failed to connect socket %d\n", _socket);
		return INET_CONN_FAILED;
	}
}

enum Inet_Ret_Code inet_send(int _socket, const char* _buffer) {
	dprintf(inet_log_fd,
			"sending data over socket=%d\n ^- data='%s'\n",
			_socket, _buffer);
    int code = send(_socket, _buffer, strlen(_buffer), 0);
    if (code == 0) {
    	dprintf(inet_log_fd, "data send successfully\n");
        return INET_SUCCESS;
    } else {
    	dprintf(inet_log_fd, "failed to send data\n");
        return INET_SEND_FAILED;
	}
}

enum Inet_Ret_Code inet_read(int _socket, char* _buffer, size_t _size, size_t _timeout_sec) {
    // Allocate a char buffer, read bytes into it,
    // copy it to the string buffer passed as the
    // argument and free the allocated buffer.
	dprintf(inet_log_fd,
			"reading data over socket=%d\n",_socket); 

	// Timeout logic
	fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(_socket, &readfds);

    struct timeval tv;
    tv.tv_sec = _timeout_sec;
    tv.tv_usec = 0;

    int rv = select(_socket + 1, &readfds, NULL, NULL, &tv);
    if (rv == -1) {
        perror("select error; read_data() of inet.cpp");
        return INET_ITERNAL_ERR;
    } else if (rv == 0) {
        // Timeout
        dprintf(inet_log_fd,
        		"recv timed out after %ld seconds\n",
         		_timeout_sec);
        return INET_TIMEOUT;
    }
	
    recv(_socket, _buffer, _size, 0);

    return INET_SUCCESS;
}
