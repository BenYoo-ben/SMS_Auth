#ifndef AUTH_SERVER_INCLUDES_HTTP_HANDLER_HPP_
#define AUTH_SERVER_INCLUDES_HTTP_HANDLER_HPP_

#include <unistd.h>
#include <fcntl.h>
#include <iconv.h>

#include <string>

#include "common.hpp"

#define HTTP_BUFFER_SIZE 20000

#define HTTP_DATA_TYPE_TABLE    0
#define HTTP_DATA_TYPE_PHONE    1
#define HTTP_DATA_TYPE_AUTH    2
#define HTTP_DATA_TYPE_STYLE_SHEET    3
#define HTTP_DATA_TYPE_AGREEMENT    4
class http_handler{
 public:
    std::string get_html(std::string location);
    std::string get_data(char *data, int data_type);

    int check_type(char *data);
    bool check_if_phone(char *data);

    char *encoding(char *text_input, char *source, char *target);
};

#endif  // AUTH_SERVER_INCLUDES_HTTP_HANDLER_HPP_
