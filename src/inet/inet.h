#ifndef INET_H
#define INET_H

extern int inet_log_fd;

// File operations
int inet_open_log_file();
int inet_close_file(int _fd);

#endif
