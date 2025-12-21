// source header file
#include "inet.hpp"
// std
#include <iostream>
#include <fstream>
#include <filesystem>
// signals
#include <csignal>
// inet
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
// inetbus
#include "../inetbus/inetbus.hpp"
// custom types
#include "../types.hpp"

using namespace std;

int Inet::create_socket() {
	_log_stream << "initing a socket" << endl;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock <= -1) {
    	_log_stream << "failed to init socket\n";
    	return -1;
    }
    
    return sock;
}

// Connect to the address that is currently
// set on the INET bus through the passed
// socket
Inet_Ret_Code Inet::socket_connect(int _socket) {
	_log_stream << "connecting socket " << _socket << endl;
	int st = connect(_socket, (struct sockaddr *)&GlobalInetBus.addr,
	            sizeof(GlobalInetBus.addr));
	if (st == 0) {
		_log_stream << "socket " << _socket << " connected\n";
		return INET_SUCCESS;
	} else {
		_log_stream << "failed to connect socket " << _socket << endl;
		return INET_CONN_FAILED;
	}
}

Inet_Ret_Code Inet::send_data(int _socket, const string &_buffer) {
	_log_stream << "sending data over socket " << _socket << "; data=" << _buffer << endl;
    int code = send(_socket, _buffer.c_str(), _buffer.size(), 0);
    if (code == 0) {
    	_log_stream << "data send successfully\n";
        return INET_SUCCESS;
    } else {
    	_log_stream << "failed to send data\n";
        return INET_SEND_FAILED;
	}
}

Inet_Ret_Code Inet::read_data(int _socket, string &_buffer, size_t _size, size_t _timeout_sec) {
    // Allocate a char buffer, read bytes into it,
    // copy it to the string buffer passed as the
    // argument and free the allocated buffer.
	_log_stream << "reading data over socket " << _socket << endl; 

	fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(_socket, &readfds);

    timeval tv;
    tv.tv_sec = _timeout_sec;
    tv.tv_usec = 0;

    int rv = select(_socket + 1, &readfds, NULL, NULL, &tv);
    if (rv == -1) {
        perror("select error");
        return INET_ITERNAL_ERR;
    } else if (rv == 0) {
        // Timeout
        _log_stream << "recv timed out after " << _timeout_sec << " seconds\n";
        return INET_TIMEOUT;
    }

	char *tmp = (char *)malloc(_size);
	
    recv(_socket, tmp, _size, 0);

    _buffer = tmp;
    free(tmp);

    return INET_SUCCESS;
}
