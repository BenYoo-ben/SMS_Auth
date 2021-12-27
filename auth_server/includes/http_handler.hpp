#ifndef HTTP_HANDLER_DEFINED
#define HTTP_HANDLER_DEFINED

#include "common.hpp"
#include <unistd.h>
#include <fcntl.h>

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

	std::vector<std::string> get_store_and_number(char * data);

	bool check_if_valid(char *data);
	bool check_if_phone(char *data);
};

#endif
