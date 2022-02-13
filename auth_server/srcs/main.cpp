#include "main.hpp"
#include "common.hpp"
#include "nets.hpp"
#include "random_generator.hpp"
#include "global.hpp"
#include "http_handler.hpp"

int main(int argc, char *argv[]){

	//Prepare Server;
	server_object so;

	if(so.server_socket_init() < 0){
		//TO-DO: exceptions
		std::cout << "Server Socket Init Failure" << std::endl;
		return 1;
	}

	if(so.server_socket_bind() < 0 ){
		//TO-DO: exceptions
		std::cout << "Server Socket Bind Failure" << std::endl;
		return 2;
	}

	if(so.server_socket_listen() < 0 ){
		//TO-DO: exceptions
		std::cout << "Server Socket Listen Failure" << std::endl;
		return 3;
	}

	so.server_socket_start();

	return 0;

}
