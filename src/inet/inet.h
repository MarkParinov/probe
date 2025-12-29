#ifndef INET_H
#define INET_H

#include <unistd.h>

extern int inet_log_fd;

// File operations
int inet_open_log_file();
int inet_close_log_file();

// Socket creation
int inet_create_raw_sock();
int inet_create_stream_sock();

// Socket inet operations
enum Inet_Ret_Code inet_socket_connect(int _socket);
enum Inet_Ret_Code inet_send(int _socket, const char* _buffer);
enum Inet_Ret_Code inet_read(int _socket, char* _buffer, size_t _size, size_t _timeout_sec);

#endif
