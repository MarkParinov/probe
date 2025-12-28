#ifndef CORE_H
#define CORE_H

#include <vector>

#include "../inetbus/inetbus.hpp"
#include "../inet/inet.hpp"

#include "../types.hpp"

class Core {
	public:
	Inet inet;
	vector<Opened_Port_Entry> scanned_ports;

	// OOP
	Core() {
		signal(SIGPIPE, handle_signal);
	}
	// Inet functionality
	void global_inetbus_init(const string &_target);
	Scan_Ret_Code get_port_state(size_t _port);
	Scan_Ret_Code scan_port(size_t _port);

	// Core functions
	void panic(const string &_code);
	void handle_signal(int _sig);
	void strip_string(string &_buffer);
};

#endif
