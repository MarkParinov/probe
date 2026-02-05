#ifndef TYPES_H
#define TYPES_H

#include <unistd.h>
#include <stdint.h>

/* service max values */
#define SERVICE_NAME_MAX_LEN 	20
#define SERVICE_BANNER_MAX_LEN 	50

/* table length parameters */
#define SERVICE_TABLE_MAX_LEN 64

/* service info accuracy parameters */
#define PORT_ACCUR_PTS_DIGIT 	1 << 6;
#define PORT_ACCUR_BANNER_DIGIT	1 << 7;



/*
	Return codes
*/

enum Generic_Ret_Code {
	GEN_SUCCESS,
	GEN_ITERNAL_ERR,
	GEN_INDEF_ERR,
};

enum Inet_Ret_Code {
	INET_SUCCESS,
	INET_ITERNAL_ERR,
	INET_OVERFLOW,
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
	 char service[30];
};

struct Port_Range {
	size_t start;
	size_t end;
};

enum Port_State {
	PORT_OPENED,
	PORT_CLOSED,
	PORT_FILTERED,
	PORT_UNKNOWN
};

struct Port_Report {
	size_t 		port;
	enum 		Port_State state;
	char 		service[SERVICE_NAME_MAX_LEN];
	uint8_t		accur;
};

/* PortToService enumeration table entry */
struct PTS_Table_Entry {
	size_t 	port;
	char 	service[SERVICE_NAME_MAX_LEN];
};

/* BannerToService enumeration table entry */
struct BTS_Table_Entry {
	char	service[SERVICE_NAME_MAX_LEN];
	char 	banner[SERVICE_BANNER_MAX_LEN];
};

#endif
