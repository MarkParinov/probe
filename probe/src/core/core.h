#ifndef CORE_H
#define CORE_H

#include "../inetbus/inetbus.h"
#include "../inet/inet.h"

#include "../types.h"

/* INET functionality */
void global_inetbus_init(const char* _target);
enum Scan_Ret_Code get_port_state(size_t _port);
enum Scan_Ret_Code scan_port(size_t _port);

/* core procedures */
void core_panic(const char* _code);
void core_handle_signal(int _sig);
void core_strip_string(char* _buffer);
void core_init_inetbus(const char* _target);

void core_scan_port(size_t _port, struct Port_Report* _report);
void core_get_server_banner(char* _buffer);

/* core functions */
enum Scan_Ret_Code core_get_port_state(size_t _port);


/* accuracy procedures */

/* pts */
void core_add_entry_to_pts_table
(size_t _port, char _service[SERVICE_NAME_MAX_LEN], size_t _ind);

void core_fill_pts_table();

void core_match_accur_pts
(struct Port_Report* _report);

/* bts */
void core_add_entry_to_bts_table
(char _service[SERVICE_NAME_MAX_LEN], char _banner[SERVICE_BANNER_MAX_LEN], size_t _ind);

void core_fill_bts_table();

int core_compare_banners
(char _banner[SERVICE_BANNER_MAX_LEN], size_t _ind);

void core_match_accur_bts
(char _banner[SERVICE_BANNER_MAX_LEN], struct Port_Report* _report);

/* procedures */
void core_check_tables();

#endif
