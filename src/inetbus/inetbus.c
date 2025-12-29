// source header
#include "./inetbus.h"
// std
#include <stdio.h>
#include <stdlib.h>
// strings
#include <string.h>

InetBus GlobalInetBus;

// Convert a standard typed IPv4 string to a 
// 4-byte array
void inetbus_set_ipv4_target(const char _host[15]) {

	unsigned char out[4] = {0}; // Output array
	char buffer[3] = {0}; // 3-char buffer for current sector
	int sector = 0; // Current sector
	int buf_i = 0; // Current buffer index
	int tr_i = 0; // Current host index

	while (tr_i < 15) {
		// Dump the buffer into the current
		// sector of 'out' if a dot symbol
		// is encountered...
		if (_host[tr_i] == '.' || _host[tr_i] == 0) {
			int b = atoi(buffer);
			if (b < 0 || b > 255) { // Byte value check
				fprintf(stderr,
				"failed to parse IPv4; sector=%d; byte=%d\n",
				sector, b);
				return;
			} else {
				out[sector] = b;
				memset(buffer, 0, 3);
				buf_i = 0;
				sector++;
			}
			// Host string isn't always 15 bytes long,
			// so we stop parsing after getting
			// a null terminator char, meaning that
			// the host string is over
			if (tr_i == 0)
				break;
		} else if (_host[tr_i] != 0) {
			 // ...add a character to the buffer
			 // otherwise
			buffer[buf_i] = _host[tr_i];
			buf_i++;
		}

		tr_i++;
	}

	// Copy the parsed byte array tp the bus' value
	memcpy(GlobalInetBus.target_v4, out, 4);
}


// Match the BUS' attributes to each other, very specific
// operations, for example matching _port to _addr's 
// sin_port field
void inetbus_update() {
    GlobalInetBus.addr.sin_port = htons(GlobalInetBus.port);
}

// Initialise InetBus, set target and port, init sockaddr
void inetbus_init(const char* _host) {

	inetbus_set_ipv4_target(_host);
	GlobalInetBus.port = 0;
	
	GlobalInetBus.addr.sin_family = AF_INET;
	GlobalInetBus.addr.sin_addr.s_addr = INADDR_ANY;
	
	inetbus_update();
}

// Print BUS fields
void inetbus_traceback(const char* _code) {

	printf(" [INETBUS TRACEBACK; CODE='%s']\n", _code);
	printf("   target = ");
	// Print target byte by byte
	for (int i = 0; i < 3; i++)
		printf("%d.", GlobalInetBus.target_v4[i]);
	printf("%d\n", GlobalInetBus.target_v4[3]);
	
	printf("   port = %ld\n", GlobalInetBus.port);
	printf("   inet module state = '%s'\n",
			GlobalInetBus.inet_mod_state);	
}
