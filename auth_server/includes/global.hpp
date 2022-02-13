#ifndef DEFINED_GLOBAL
#define DEFINED_GLOBAL

struct auth_data{
	int auth_code;
	clock_t time;
	std::string phone_number;
}

static unsigned short global_server_port = 55551;
static unsigned int global_server_listen_maximum = 10000;
static unsigned int global_expected_MTU = 3000;

static std::vector<int> global_phone_sockets;
static int global_phone_index;
static std::vector<struct auth_data> global_auth;
#endif
