// source header
// #include "./core.hpp"
// std
#include <stdio.h>
#include <stdlib.h>
// strings
#include <string.h>
// signals
#include <signal.h>

// inet module
#include "../inet/inet.h"
// inetbus
#include "../inetbus/inetbus.h"
// custom types
#include "../types.h"
// #include "../port_ranges.hpp"

// using namespace std;

void panic(const char* _code) {
	dprintf(inet_log_fd, "core panic; code='%s'\n", _code);
	printf("core panic; exiting\n");;
	inetbus_traceback(_code);

	exit(1);
}

void handle_signal(int _sig) {
	dprintf(inet_log_fd, "captured signal [%d]\n", _sig);
}

void strip_string(char* _buffer) {
	size_t i = strlen(_buffer)-1;
	while (_buffer[i] == '\n' && _buffer[i] == '\r') {
		_buffer[i] = 0; i--;
	}
}

void global_inetbus_init(const char* _target) {
	inetbus_init(_target);
	GlobalInetBus.port = 0;
}

enum Scan_Ret_Code get_port_state(size_t _port) {
	GlobalInetBus.port = _port;
	inetbus_update();

	int sock = inet_create_stream_sock();
    enum Inet_Ret_Code conn_st = inet_socket_connect(sock);
    close(sock);
    if (conn_st != INET_SUCCESS)
    	return SCAN_PORT_CLOSED;
    else
    	return SCAN_PORT_OPENED;
}

enum Scan_Ret_Code scan_port(size_t _port) {
	// Current algo:
	// -> connect to a port
	// -> receive incoming data
	// -? got any data? (first 10 bytes value >= 0)
	// |-- YES - print the data
	// |--  NO - send a dummy packet
	
	GlobalInetBus.port = _port;
	inetbus_update();

	char* recv_buffer = (char*)malloc(128);

    int sock = inet_create_raw_sock();
    enum Inet_Ret_Code conn_st = inet_socket_connect(sock);
    if (conn_st != INET_SUCCESS)
    	return SCAN_PORT_CLOSED;
    
    if (sock < 0) {
    	return SCAN_ITERNAL_ERR;
	}
	
	inet_read(sock, recv_buffer, 128, 2);
	strip_string(recv_buffer);
	if (recv_buffer[0] >= 1 && recv_buffer[1] >= 1) {
		dprintf(inet_log_fd, 
				"received data from port=%ld; data='%s'\n",
				_port, recv_buffer);

		dprintf(inet_log_fd, "opened port detected\n");
		close(sock);
		return SCAN_PORT_OPENED;
    } else {
    	enum Inet_Ret_Code code = inet_send(sock, "testing");
    	if (code == INET_SEND_FAILED) {
    		close(sock);
    		return SCAN_PORT_CLOSED;
    	}
    	
    	inet_read(sock, recv_buffer, 128, 2);
    	strip_string(recv_buffer);
    	dprintf(inet_log_fd,
    			"received data after sending from port=%ld; data='%s'\n",
    			 _port,  recv_buffer);

		close(sock);
		return SCAN_PORT_OPENED;
    }
}

int main() {

	inet_log_fd = inet_open_log_file();
	dprintf(inet_log_fd, "log entry initiated\n");
	
	dprintf(inet_log_fd, "core started\n");
    inetbus_init("192.168.1.54");
    dprintf(inet_log_fd, "inetbus initiated\n");

    size_t start = 1;
    size_t end = 2048;

    for (size_t i = start; i < end; i++) {
        if (get_port_state(i) == SCAN_PORT_OPENED)
        	printf("%ld opened\n", i);
    }
	// for (auto i : scanned_ports) {
	// 	cout << i.port << "\t" << i.service << " opened" << endl;
	// }

	panic("test");
	inet_close_log_file();
	

    return 0;
}
