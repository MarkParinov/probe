#ifndef INETBUS_H
#define INETBUS_H

#include <string>
#include <mutex>
#include <netinet/in.h>
using namespace std;

class InetBus {
	private:
		// Mutex
		mutable mutex _mtx;
		// Network info
		string _target;
		size_t _port;
		// Internal info
		string _inet_mod_state;
	public:
		// Open fields
		sockaddr_in addr;
		
		// Basic functions
		void update();
		void init(const string &_target);
		void traceback(const string &_code);
		// Attribute set and get
		void set_target(const string &_value);
		string get_target();
		
		void set_port(const size_t &_value);
		size_t get_port();
		
		void set_mod_state(const string &_value);
		string get_mod_state();

		sockaddr_in get_addr();
};

extern InetBus GlobalInetBus;

#endif
