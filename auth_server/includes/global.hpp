#ifndef AUTH_SERVER_INCLUDES_GLOBAL_HPP_
#define AUTH_SERVER_INCLUDES_GLOBAL_HPP_
#include <stdint.h>
#include <string>

struct auth_data{
    int auth_code;
    clock_t time;
    std::string phone_number;
};

constexpr uint16_t global_server_port = 55551;
constexpr uint32_t global_server_listen_maximum = 10000;
constexpr uint32_t global_expected_MTU = 8192;

#endif  // AUTH_SERVER_INCLUDES_GLOBAL_HPP_
