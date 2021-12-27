#ifndef DEFINED_SESSION
#define DEFINED_SESSION

#include <pthread.h>
#include <unistd.h>
#include "common.hpp"
#include "http_handler.hpp"

class session_object{
private:
	std::string ID;
	//client socket
	int c_sock;


	pthread_t session_thread;
public:
	session_object(int established_socket);

	void close_socket();
	void *run();

};

#endif