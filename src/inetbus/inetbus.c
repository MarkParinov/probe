// source header
#include "./inetbus.h"
// std
#include <stdio.h>
#include <stdlib.h>
// strings
#include <string.h>
/* sockets */
#include <arpa/inet.h>

InetBus GlobalInetBus;

/*	Match the BUS' attributes to each other, very specific
	operations, for example matching _port to _addr's 
	sin_port field */
void inetbus_update() {
    GlobalInetBus.addr.sin_port = htons(GlobalInetBus.port);
}

/* initialise INETBUS, set target and port, init sockaddr */
void inetbus_init(const char* _host) {

	// inetbus_set_ipv4_target(_host);
	// memcpy(GlobalInetBus.target_v4, out, 4);
	GlobalInetBus.port = 0;
	
	GlobalInetBus.addr.sin_family = AF_INET;
	GlobalInetBus.addr.sin_port = 0;
	inet_pton(AF_INET,_host, &GlobalInetBus.addr.sin_addr);
	
	inetbus_update();
}

void inetbus_get_ip_string(char* _buffer) {
	inet_ntop(AF_INET, &GlobalInetBus.addr.sin_addr, _buffer, INET_ADDRSTRLEN);
}

/* print BUS fields */
void inetbus_traceback(const char* _code) {

	printf(" [INETBUS TRACEBACK; CODE='%s']\n", _code);
	printf("   IPv4 = ");
	char ad[15];
	inetbus_get_ip_string(ad);
	printf("%s\n", ad);
	
	printf("   port = %ld\n", GlobalInetBus.port);
	printf("   inet module state = '%s'\n", GlobalInetBus.inet_mod_state);
}
