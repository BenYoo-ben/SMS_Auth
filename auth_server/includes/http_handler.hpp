#ifndef HTTP_HANDLER_DEFINED
#define HTTP_HANDLER_DEFINED

#include "common.hpp"
#include <unistd.h>
#include <fcntl.h>

#define HTTP_DATA_TYPE_TABLE	0
#define HTTP_DATA_TYPE_PHONE	1
#define HTTP_DATA_TYPE_AUTH		2

class http_handler{

private:

	std::string html_page=
	"HTTP/1.1 200 OK\n"
	"Content-Type:text/html\n"
	"Content-Length: 16\n\n"
	"<h3>Phone</h3>\n"
	"<input type=\"text\" id=\"phone_number_input\"> 010######## </input>\r\n"
	"<input type=\"submit\" id=\"phone_number_button\"> submit </button>"
	;

public:

	std::string get_html(std::string location);

	std::string get_data(char * data, int data_type);

	int check_type(char *data);
	bool check_if_phone(char *data);
};

#endif
