#ifndef AUTH_SERVER_INCLUDES_SESSION_HPP_
#define AUTH_SERVER_INCLUDES_SESSION_HPP_

#include <pthread.h>
#include <unistd.h>

#include <string>

#include "common.hpp"
#include "http_handler.hpp"
#include "global.hpp"

#define STYLE_AND_AGREEMENT_BUFFER 50000
class session_object{
private:
    std::string ID;
    int c_sock;

    pthread_t session_thread;

    global_object *g_obj = global_object::getInstance();
public:
    explicit session_object(int established_socket);

    void close_socket();
    int exchange_auth_with_phone(std::string phone_number, int random_data);
    void *run();
};
#endif  // AUTH_SERVER_INCLUDES_SESSION_HPP_
