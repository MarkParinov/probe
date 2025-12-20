#include "inetbus.hpp"

#include <iostream>

InetBus GlobalInetBus;

// Match the BUS' attributes to each other, very specific
// operations, for example matching _port to _addr's 
// sin_port field
void InetBus::update() {
    addr.sin_port = htons(_port);
}

// Initialise InetBus, set target and port, init sockaddr
void InetBus::init(const string &_trgt) {
	_target = _trgt;
	_port = 0;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	
	InetBus::update();
}

// Print BUS fields
void InetBus::traceback(const string &_code) {
	cout << " [BUS TRACEBACK; CODE='" << _code << "']:" << endl;
	cout << "   target = '" << _target << "'\n";
	cout << "   port = " << _port << endl;
	cout << "   inet module state = '" << _inet_mod_state << "'\n";	
}

// Attribute set and get
void InetBus::set_target(const string &_value) {
	lock_guard<mutex> lock(_mtx);
	_target = _value;
}
string InetBus::get_target() {
	return _target;
}

void InetBus::set_port(const size_t &_value) {
	lock_guard<mutex> lock(_mtx);
	_port = _value;
}
size_t InetBus::get_port() {
	return _port;
}

void InetBus::set_mod_state(const string &_value) {
	lock_guard<mutex> lock(_mtx);
	_inet_mod_state = _value;
}
string InetBus::get_mod_state() {
	return _inet_mod_state;
}

sockaddr_in InetBus::get_addr() {
	return addr;
}
