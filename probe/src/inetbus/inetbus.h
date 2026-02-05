#ifndef INETBUS_H
#define INETBUS_H

#include <netinet/in.h>
#include <stdint.h>

typedef struct {
	// IPv4 representation of
	// target host in 4 bytes
	unsigned char target_v4[4];
	// Current scanned port
	size_t port;
	// Internal info
	uint8_t inet_mod_state[4];
	// Socket address struct
	struct sockaddr_in addr;
} InetBus;

// Setup functions
void inetbus_init(const char* _host);
void inetbus_update();

// Debug
void inetbus_get_ip_string(char* _buffer);
void inetbus_traceback(const char* _code);

// Core functionality
void inetbus_set_ipv4_target(const char _host[15]);

extern InetBus GlobalInetBus;

#endif
