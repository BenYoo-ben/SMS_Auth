#include "common.hpp"
#include "nets.hpp"
#include "global.hpp"
#include "session.hpp"
#include "random_generator.hpp"

int server_object::server_socket_init() {
    s_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s_sock < 0) {
        std::cerr << "Socket Generation Fail" << std::endl;
        return -1;
    }

    return 0;
}

int server_object::server_socket_bind() {
    struct sockaddr_in server_address;

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(global_server_port);

    int ret = bind(s_sock, (struct sockaddr *) &server_address, sizeof(server_address));

    if (ret < 0) {
        std::cout << "Socket Bind Fail" << std::endl;
        return -1;
    }

    return 0;
}

int server_object::server_socket_listen() {
    int ret = listen(s_sock, global_server_listen_maximum);

    if (ret < 0) {
        std::cout << "Socket Listen Fail" << std::endl;
        return -1;
    }
    return 0;
}

void server_object::server_socket_start() {
    socklen_t client_addr_size = sizeof(struct sockaddr_in);
    global_object *g_obj = global_object::getInstance();
    g_obj->global_phone_sockets = std::vector<int>();
    g_obj->global_phone_index = 0;
    g_obj->global_auth = std::vector<struct auth_data>();
    g_obj->global_auth_index = std::vector<int>();

    while (true) {
        struct sockaddr_in client_addr;

        int client_socket = accept(s_sock, (struct sockaddr*) &client_addr, &client_addr_size);

        if (client_socket < 0) {
            std::cout << "Client Socket Accept Fail" << std::endl;
            return;
        } else {
            new session_object(client_socket);
        }
    }
}
