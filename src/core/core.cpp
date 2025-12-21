#include <iostream>
#include <fstream>

#include <csignal>

#include <vector>

#include "../inetbus/inetbus.hpp"
#include "../inet/inet.hpp"

#include "../types.hpp"

using namespace std;

Inet inet;
vector<Opened_Port_Entry> scanned_ports;

void handle_signal(int _sig) {
	inet << "captured signal [" << _sig << "]\n";
}

void strip_string(string &_buffer) {
	size_t i = _buffer.size()-1;
	while (_buffer[i] == '\n' && _buffer[i] == '\r') {
		_buffer[i] = 0; i--;
	}
}

void global_inetbus_init(const string &_target) {
	GlobalInetBus.init(_target);
	GlobalInetBus.set_port(0);
}

Scan_Ret_Code get_port_state(size_t _port) {
	GlobalInetBus.set_port(_port);
	GlobalInetBus.update();

	int sock = inet.create_socket();
    Inet_Ret_Code conn_st = inet.socket_connect(sock);
    close(sock);
    if (conn_st != INET_SUCCESS)
    	return SCAN_PORT_CLOSED;
    else
    	return SCAN_PORT_OPENED;
}

Scan_Ret_Code scan_port(size_t _port) {
	// Current algo:
	// -> connect to a port
	// -> receive incoming data
	// -? got any data? (first 10 bytes value >= 0)
	// |-- YES - print the data
	// |--  NO - send a dummy packet
	
	GlobalInetBus.set_port(_port);
	GlobalInetBus.update();

	string recv_buffer;

    int sock = inet.create_socket();
    Inet_Ret_Code conn_st = inet.socket_connect(sock);
    if (conn_st != INET_SUCCESS)
    	return SCAN_PORT_CLOSED;
    
    if (sock < 0) {
    	return SCAN_ITERNAL_ERR;
	}
	
	inet.read_data(sock, recv_buffer, 128, 2);
	strip_string(recv_buffer);
	if (recv_buffer[0] >= 1 && recv_buffer[1] >= 1) {
		inet << "received data from port " << _port << ": '" << recv_buffer << "'\n'"; 
		inet << (int)recv_buffer[0] << " " << (int)recv_buffer[1] << '\n';

		cout << "got opened\n";
		close(sock);
		return SCAN_PORT_OPENED;
    } else {
    	auto code = inet.send_data(sock, "testing");
    	if (code == INET_SEND_FAILED) {
    		close(sock);
    		return SCAN_PORT_CLOSED;
    	}
    	
    	inet.read_data(sock, recv_buffer, 128, 2);
    	strip_string(recv_buffer);
    	inet << "received data after data sent from port " << _port << ": '" << recv_buffer << "'\n'";

		close(sock);
		return SCAN_PORT_OPENED;
    }
}

int main() {
	signal(SIGPIPE, handle_signal);
	inet << "core started\n";
    global_inetbus_init("192.168.1.54");
    inet << "inetbus inited\n";

	size_t start, end;
	start = 1; end = 500;

    for (size_t i = start; i < end; i++) {
    	cout << i << endl;
        if (get_port_state(i) == SCAN_PORT_OPENED)
        	scanned_ports.push_back(Opened_Port_Entry{(size_t)i, "test"});
    }
	for (auto i : scanned_ports) {
		cout << i.port << "\t" << i.service << " opened" << endl;
	}

    return 0;
}
