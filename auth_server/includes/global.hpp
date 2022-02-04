#ifndef DEFINED_GLOBAL
#define DEFINED_GLOBAL

static unsigned short global_server_port = 55551;
static unsigned int global_server_listen_maximum = 10000;
static unsigned int global_expected_MTU = 1500;

static std::vector<int> global_phone_sockets;
static int global_phone_index;
static std::map<std::string, int> global_auth_codes;
static std::map<std::string, clock_t> global_auth_timeouts;
#endif
