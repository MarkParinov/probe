#ifndef TYPES_H
#define TYPES_H

#include <iostream>
#include <string>
using namespace std;

enum Generic_Ret_Code {
	GEN_SUCCESS,
	GEN_ITERNAL_ERR,
	GEN_INDEF_ERR,
};

enum Inet_Ret_Code {
	INET_SUCCESS,
	INET_ITERNAL_ERR,
	INET_CONN_FAILED,
	INET_INVALID_ADDR,
	INET_INVALID_PORT,
	INET_SEND_FAILED,
	INET_TIMEOUT,
};

enum Scan_Ret_Code {
    SCAN_SUCCESS,
    SCAN_ITERNAL_ERR,
    SCAN_PORT_CLOSED,
    SCAN_PORT_OPENED,
};

struct Opened_Port_Entry {
	 size_t port;
	 string service;
};

struct Port_Range {
	size_t start;
	size_t end;
};

#endif
