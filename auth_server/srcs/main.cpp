#include "common.hpp"
#include "nets.hpp"
#include "random_generator.hpp"
#include "http_handler.hpp"
#include "global.hpp"

// init global obj
global_object* global_object::instance = nullptr;
int main(int argc, char *argv[]) {
    server_object so;

    if (so.server_socket_init() < 0) {
        std::cout << "Server Socket Init Failure" << std::endl;
        return 1;
    }

    if (so.server_socket_bind() < 0) {
        std::cout << "Server Socket Bind Failure" << std::endl;
        return 2;
    }

    if (so.server_socket_listen() < 0) {
        std::cout << "Server Socket Listen Failure" << std::endl;
        return 3;
    }

    so.server_socket_start();

    return 0;
}
