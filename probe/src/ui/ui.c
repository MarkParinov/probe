/* std */
#include <stdlib.h>
#include <stdio.h>
/* strings */
#include <string.h>

#include "../core/core.h"
#include "../inetbus/inetbus.h"

/* theoretical max length of a service name to be printed */
#define UI_SERVICE_PRINT_PADDING 20

#define UI_PORT_RANGE_START	(size_t)1
#define UI_PORT_RANGE_END	(size_t)1024

/* print probe info before scan */
void banner() {
	printf("PROBE v1.1 scan initiated\n");
	char ad[15];
	inetbus_get_ip_string(ad);
	printf("~ Target IP: %s\n", ad);
	printf("~ Port range: %zu-%zu\n",UI_PORT_RANGE_START,UI_PORT_RANGE_END);
	printf("\n");
}

int main(int argc, char* argv[]) {

	if (argc < 2) {
		printf("error: target IP argument expected\n");
		return 1;
	}

	banner();
	
	core_fill_pts_table();
	core_fill_bts_table();

	core_check_tables();

	inet_log_fd = inet_open_log_file();
	if (inet_log_fd < 0)
		printf("Warning: failed to initialise log file; core modules logging disabed; log_fd=%d\n", inet_log_fd);

	dprintf(inet_log_fd, "[CORE] log entry initiated\n");
	
	dprintf(inet_log_fd, "[CORE] core started\n");
    core_init_inetbus(argv[1]);
    dprintf(inet_log_fd, "[CORE] inetbus initiated\n");
    
    size_t start = 1;
    size_t end = 10000;

    char* banner_buf = (char*)malloc(SERVICE_BANNER_MAX_LEN);
   	memset(banner_buf, 0, SERVICE_BANNER_MAX_LEN);

    printf("PORT\tSERVICE");
    for (int i = strlen("SERVICE"); i < UI_SERVICE_PRINT_PADDING; i++)
       		printf(" ");
    printf("ACCURACY\n");

    for (size_t i = start; i < end; i++) {
    	/* core_scan_port(i); */
        if (core_get_port_state(i) == SCAN_PORT_OPENED) {
        	struct Port_Report rep; memset(&rep, 0, sizeof(rep));
        	rep.port = i;
        	core_get_server_banner(banner_buf);
        	/* run port matches */
        	core_match_accur_pts(&rep);
        	core_match_accur_bts(banner_buf, &rep);
        	/* print info */
        	printf("%zu\t%s", rep.port, rep.service);
        	for (int i = strlen(rep.service); i < UI_SERVICE_PRINT_PADDING; i++)
        		printf(" ");
        	printf("%05.2f%%\n", (float)(rep.accur*100)/255);
        }
    }
	// for (auto i : scanned_ports) {
	// 	cout << i.port << "\t" << i.service << " opened" << endl;
	// }

	inet_close_log_file();
	
	/*
	int sock = inet_create_raw_socket();

	GlobalInetBus.port = 4000;
	inetbus_update();
	
	inet_connect_socket(sock);

	// inet_send(sock, "Hello World!");

	char* buffer = (char*)malloc(128);
	
	inet_read(sock, buffer, 128, 2);
	printf("data='%s'", buffer);

	free(buffer);
	*/
	
    return 0;
}
