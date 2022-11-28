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

class global_object {
private:
    global_object() {};
    global_object(const global_object& other);
    ~global_object() {};

    static global_object* instance;
public:
    static global_object* getInstance() {
        if (instance == nullptr) {
            instance = new global_object();
        }
        return instance;
    }
    std::vector<int> global_phone_sockets;
    uint32_t global_phone_index;
    std::vector<int> global_auth_index;
    std::vector<struct auth_data> global_auth;
};

#endif  // AUTH_SERVER_INCLUDES_GLOBAL_HPP_
