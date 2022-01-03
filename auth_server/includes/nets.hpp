#ifndef NETS_DEFINED
#define NETS_DEFINED

#include <sys/socket.h>
#include <arpa/inet.h>

class server_object{
private:
	//server socket;
	int s_sock;

public:

	int server_socket_init();
	int server_socket_bind();
	int server_socket_listen();

	void server_socket_start();


};


#endif