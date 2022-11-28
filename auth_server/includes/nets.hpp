#ifndef AUTH_SERVER_INCLUDES_NETS_HPP_
#define AUTH_SERVER_INCLUDES_NETS_HPP_

#include <sys/socket.h>
#include <arpa/inet.h>
#include "common.hpp"
#include "global.hpp"

class server_object{
private:
    int s_sock;
public:
    int server_socket_init();
    int server_socket_bind();
    int server_socket_listen();

    void server_socket_start();
};
#endif  // AUTH_SERVER_INCLUDES_NETS_HPP_
