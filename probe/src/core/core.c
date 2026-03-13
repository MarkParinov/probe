/* source header */
#include "./core.h"
/* std */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/* strings */
#include <string.h>
/* signals */
#include <signal.h>

/* inet module */
#include "../inet/inet.h"
/* inetbus */
#include "../inetbus/inetbus.h"
/* custom types */
#include "../types.h"

void core_panic(const char* _code) {
	printf("\nPROBE core module panic; exit code: '%s'\n", _code);
	inetbus_traceback("panic");

	exit(1);
}

void core_handle_signal(int _sig) {
	dprintf(inet_log_fd, "[CORE] captured signal [%d]\n", _sig);
}

void core_strip_string(char* _buffer) {
	size_t i = strlen(_buffer)-1;
	while (_buffer[i] == '\n' || _buffer[i] == '\r') {
		_buffer[i] = 0; i--;
	}
}

void core_init_inetbus(const char* _target) {
	inetbus_init(_target);
	GlobalInetBus.port = 0;
}

enum Scan_Ret_Code core_get_port_state(size_t _port) {
	GlobalInetBus.port = _port;
	inetbus_update();

	int sock = inet_create_stream_socket();
    enum Inet_Ret_Code conn_st = inet_connect_socket(sock);
    close(sock);
    if (conn_st != INET_SUCCESS)
    	return SCAN_PORT_CLOSED;
    else
    	return SCAN_PORT_OPENED;
}

/*	port scanning procedure; file a report */
void core_scan_port(size_t _port, struct Port_Report* _report) {

	_report->port = _port;

	dprintf(inet_log_fd,
	"[CORE] starting new core_scan_port instance; port=%ld\n",
			_port);
	
	GlobalInetBus.port = _port;
	inetbus_update();

	char* recv_buffer = (char*)malloc(128);

    int sock = inet_create_stream_socket();
    if (sock < 0) {
    	dprintf(inet_log_fd, "[CORE] failed to create stream socket\n");
    	close(sock);
    	_report->state = PORT_UNKNOWN;
    	return;
    }
    
    enum Inet_Ret_Code conn_st = inet_connect_socket(sock);
    if (conn_st != INET_SUCCESS) {
    	dprintf(inet_log_fd, "[CORE] failed to connect socket\n");
    	close(sock);
    	_report->state = PORT_CLOSED;
    	return;
    }
	
	inet_read(sock, recv_buffer, 128, 2);
	core_strip_string(recv_buffer);
	if (recv_buffer[0] >= 1 && recv_buffer[1] >= 1) {
		core_strip_string(recv_buffer);
		dprintf(inet_log_fd,
				"[CORE] received data from port=%ld; data='%s'\n",
				_port, recv_buffer);

		dprintf(inet_log_fd, "[CORE] opened port detected\n");
		close(sock);
		_report->state = PORT_OPENED;
		return;
    } else {
    	enum Inet_Ret_Code code = inet_send(sock, "testing");
    	if (code == INET_SEND_FAILED) {
    		close(sock);
    		_report->state = PORT_FILTERED;
    		return;
    	}
    	
    	inet_read(sock, recv_buffer, 128, 2);
    	core_strip_string(recv_buffer);
    	dprintf(inet_log_fd,
    			"[CORE] received data after sending from port=%ld; data='%s'\n",
    			 _port,  recv_buffer);

		close(sock);
		_report->state = PORT_OPENED;
		return;
    }
}

void core_get_server_banner(char* _buffer) {
	/* intiate a new connection */
	int sock = inet_create_stream_socket();
	if (sock < 0)
		return;
	
	enum Inet_Ret_Code ret = inet_connect_socket(sock);
	if (ret != 0) {
		printf("conenction failed\n");
		return;
	}

	memset(_buffer, 0, SERVICE_BANNER_MAX_LEN);

	inet_read(sock, _buffer, SERVICE_BANNER_MAX_LEN, 2);
	core_strip_string(_buffer);
	/* printf("received banner: '%s'\n", _buffer); */
};

/*
	PORT ACCURACY CALCULATION
*/

/* PTS operations */

struct PTS_Table_Entry ptst[SERVICE_TABLE_MAX_LEN];

void core_add_entry_to_pts_table
(size_t _port, char _service[SERVICE_NAME_MAX_LEN], size_t _ind) {
	ptst[_ind].port = _port;
	memcpy(ptst[_ind].service, _service, SERVICE_NAME_MAX_LEN);
}

void core_fill_pts_table() {
	size_t i = 0;

	/* WELL-KNOWN */
	core_add_entry_to_pts_table(7,		"echo",			i); i++;
	core_add_entry_to_pts_table(20,		"ftp-data",		i); i++;
	core_add_entry_to_pts_table(21,		"ftp-control",	i); i++;
	core_add_entry_to_pts_table(22,		"ssh",			i); i++;
	core_add_entry_to_pts_table(23,		"telnet",		i); i++;
	core_add_entry_to_pts_table(24,		"mail",			i); i++;
	core_add_entry_to_pts_table(53,		"dns",			i); i++;
	core_add_entry_to_pts_table(80,		"http",			i); i++;
	core_add_entry_to_pts_table(106,	"MACOS-pass",	i); i++;
	core_add_entry_to_pts_table(115,	"sftp",			i); i++;
	core_add_entry_to_pts_table(138,	"netbios-ds",	i); i++;
	core_add_entry_to_pts_table(139,	"netbios-ss",	i); i++;
	core_add_entry_to_pts_table(156,	"sql-service",	i); i++;
	core_add_entry_to_pts_table(199,	"UNIX-snmp",	i); i++;
	core_add_entry_to_pts_table(312,	"MACOS-xsan",	i); i++;
	core_add_entry_to_pts_table(443,	"https",		i); i++;
	core_add_entry_to_pts_table(445,	"MICROSOFT-ds",	i); i++;
	core_add_entry_to_pts_table(540,	"UNIX-UNIX-cp",	i); i++;
	core_add_entry_to_pts_table(655,	"tinc-vpn",		i); i++;
	core_add_entry_to_pts_table(989,	"ftps-data",	i); i++;
	core_add_entry_to_pts_table(990,	"ftps-control",	i); i++;
}

void core_match_accur_pts
(struct Port_Report* _report) {
	for (size_t i = 0; i < SERVICE_TABLE_MAX_LEN; i++)
		if (ptst[i].port == _report->port) {
			if (_report->service[0] == 0) {
				memcpy(_report->service, ptst[i].service, SERVICE_NAME_MAX_LEN);
				_report->accur |= PORT_ACCUR_PTS_DIGIT;
				return;
			}
		}
	memcpy(_report->service, "unknown", SERVICE_NAME_MAX_LEN);
}

/* BTS operations */

struct BTS_Table_Entry btst[SERVICE_TABLE_MAX_LEN];

void core_add_entry_to_bts_table
(char _service[SERVICE_NAME_MAX_LEN], char _banner[SERVICE_BANNER_MAX_LEN], size_t _ind) {
	memcpy(btst[_ind].service, _service, SERVICE_NAME_MAX_LEN);
	memcpy(btst[_ind].banner, _banner, SERVICE_BANNER_MAX_LEN);
}

void core_fill_bts_table() {
	size_t i = 0;
	core_add_entry_to_bts_table("ftp-control", "220 (vsFTPd", i); i++;
	core_add_entry_to_bts_table("ssh", "SSH-", i); i++;
}
/*	compare a banner pattern in a report entry to the _ind
	element of the BTS table */
int core_compare_banners(char _banner[SERVICE_BANNER_MAX_LEN], size_t _ind) {
	/* create a banner buffer and copy the BTS
	table's entry banner into it */
	char buf[SERVICE_BANNER_MAX_LEN];
	memcpy(buf, btst[_ind].banner, SERVICE_BANNER_MAX_LEN);
	int flag = 1;

	/* printf("comparing '%s' with '%s'\n", _banner, buf); */
	
	size_t i = 0;
	while (buf[i] == _banner[i]) {
		if (!buf[i] && !_banner[i]) {
			flag = 1;
			break;
		}
	
		if (buf[i] != _banner[i])
			flag = 0;
		i++;
	}

	/* printf("compare result: %d\n", flag); */
	return flag;
}

void core_match_accur_bts
(char _banner[SERVICE_BANNER_MAX_LEN], struct Port_Report* _report) {
	if (!strcmp(_report->service, "unknown") || _report->service[0] == 0)
		return;
	
		
	for (size_t i = 0; i < SERVICE_TABLE_MAX_LEN; i++) {
		/* printf("%s; %s\n", _report->service, btst[i].service); */
		if (!strcmp(_report->service, btst[i].service)) {
			if (core_compare_banners(_banner, i))
				_report->accur |= PORT_ACCUR_BANNER_DIGIT;
			return;
		}
	}
}

/* ACCURACY PROCEDURES */

void core_check_tables() {
	int flag = 0;
	for (size_t i = 0; i < SERVICE_TABLE_MAX_LEN; i++) {
		if (!btst[i].service[0]) {
			flag = 1;
			break;
		}
		flag = 0;
		printf("bts table check: service='%s'; index=%zu ... ",
				btst[i].service, i);
		for (size_t j = 0; j < SERVICE_TABLE_MAX_LEN; j++) {
			if (!ptst[j].service[0]) {
				printf("NOT FOUND\n");
				break;
			}
			if (!strcmp(btst[i].service, ptst[j].service)) {
				printf("OK\n");
				flag = 1;
				break;
			}
		}
		if (!flag) {
			core_panic("table error");
			return;
		}
	}
}
