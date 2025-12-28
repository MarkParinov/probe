#ifndef INET_H
#define INET_H

#include <unistd.h>

extern int inet_log_fd;

// File operations
int inet_open_log_file();
int inet_close_file(int _fd);

// Socket creation
int inet_create_raw_sock();
int init_socket();

// Socket inet operations
enum Inet_Ret_Code socket_connect(int _socket);
enum Inet_Ret_Code send_data(int _socket, const char* _buffer);
enum Inet_Ret_Code read_data(int _socket, char* _buffer, size_t _size, size_t _timeout_sec);

#endif
