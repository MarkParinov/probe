/* std */
#include <stdlib.h>
#include <stdio.h>
/* strings */
#include <string.h>

#include "../core/core.h"
#include "../inetbus/inetbus.h"
#include "../types.h"

/* theoretical max length of a service name to be printed */
#define UI_SERVICE_PRINT_PADDING 20

#define UI_PORT_RANGE_START	(size_t)1
#define UI_PORT_RANGE_END	(size_t)1024

/* print probe info before scan */
void banner() {
	printf("PROBE scan initiated:\n");
	printf("~ Build version: v1.2\n");
	printf("~ Inet timeout: %zu\n", INET_TIMEOUT_USEC);

	char ad[15];
	inetbus_get_ip_string(ad);
	printf("~ Target IP: %s\n", ad);
	printf("~ Port range: %zu-%zu\n",
	UI_PORT_RANGE_START, UI_PORT_RANGE_END); printf("\n");
	/*printf("host name = '%s'\n", GlobalInetBus.host_name);*/
}

void print_progress(size_t port) {
    printf("\rprobing %zu port...", port);
    fflush(stdout);
}

int main(int argc, char* argv[]) {

	/* check arguments */
	if (argc < 2) {
		printf("error: target IP argument expected\n");
		return 1;
	}

	/* initiate tables */
	core_fill_pts_table();
	core_fill_bts_table();

	core_check_tables();

	/* open log file */
	inet_log_fd = inet_open_log_file();
	if (inet_log_fd < 0)
		printf("Warning: failed to initialise log file; core modules logging disabed; log_fd=%d\n", inet_log_fd);

	dprintf(inet_log_fd, "[CORE] log entry initiated\n");
	
	dprintf(inet_log_fd, "[CORE] core started\n");
	
	/* start INETBUS */
    enum Generic_Ret_Code code = core_init_inetbus(argv[1]);
    if (code != GEN_SUCCESS)
    	return 1;
    	
    dprintf(inet_log_fd, "[CORE] inetbus initiated\n");

    banner();

    char* banner_buf = (char*)malloc(SERVICE_BANNER_MAX_LEN);
   	memset(banner_buf, 0, SERVICE_BANNER_MAX_LEN);

   	size_t ports_scanned = 0;
   	float average_accuracy = 0;

    int header_printed = 0;

    for (size_t i = UI_PORT_RANGE_START; i < UI_PORT_RANGE_END; i++) {
    	/* core_scan_port(i); */
    	print_progress(i);
        if (core_get_port_state(i, INET_TIMEOUT_USEC) == SCAN_PORT_OPENED) {
			if (!header_printed) {
				printf("\r\033[K");
				printf("\rPORT\tSERVICE");
			    for (int i = strlen("SERVICE"); i < UI_SERVICE_PRINT_PADDING; i++)
			       		printf(" ");
			    printf("ACCURACY\n");
			    header_printed = 1;
			}
			
        	struct Port_Report rep; memset(&rep, 0, sizeof(rep));
        	printf("\r\033[K");
        	
        	/* set report port */
        	rep.port = i;
        	printf("\r%zu\t", i);
        	fflush(stdout);

        	/* try to receive a banner */
        	core_get_server_banner(banner_buf);
        	
        	/* run port matches */
        	core_match_accur_pts(&rep);
        	core_match_accur_bts(banner_buf, &rep);

        	printf("%s", rep.service);
        	
        	/* print info */
        	for (int i = strlen(rep.service); i < UI_SERVICE_PRINT_PADDING; i++)
        		printf(" ");
        	printf("%05.2f%%\n", (float)(rep.accur*100)/255);

        	ports_scanned++;
        	average_accuracy += (float)(rep.accur*100)/255;
        }
    }

    printf("\r\033[K");
    char ad[15];
	inetbus_get_ip_string(ad);
    
    if (!ports_scanned) {
    	printf("Scan for %s completed with no active ports.\n", ad);
    	printf("Try adjusting the connection timeout in microseconds\n");
		printf("with a value greater than %zu.\n", INET_TIMEOUT_USEC);
    } else {
   	    printf("\nScan for %s completed with:\n", ad);
   	    printf("~ %zu opened ports (%zu not shown)\n",
   	    ports_scanned, UI_PORT_RANGE_END-UI_PORT_RANGE_START-ports_scanned);
   	    printf("~ Average accuracy of %05.2f%%\n", average_accuracy/ports_scanned);
    	
    }

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
