#ifndef INET_H
#define INET_H

#include "../types.hpp"
#include <string>
#include <fstream>
#include <cstdio>
#include <csignal>
using namespace std;

class Inet {
	private:
	ofstream _log_stream;
	
    public:
    // Funtions
    Inet() {
    	remove("./inet_log");
   		_log_stream.open("./inet_log", ios::out | ios::app);
        if (!_log_stream) {
            throw runtime_error("Failed to open log file");
        }
   	}

   	~Inet() {
        if (_log_stream.is_open()) {
            _log_stream.close();
        }
    }

   	template<typename T>
    Inet& operator<<(const T& value) {
        _log_stream << value;
        return *this;
    }
    
    // 	Inet
    int create_socket();
    Inet_Ret_Code socket_connect(int _socket);

    Inet_Ret_Code send_data(int _socket, const string &_buffer);
    Inet_Ret_Code read_data(int _socket, string &_buffer, size_t _size, size_t _timeout_sec);

	// Signal handling
};

#endif
