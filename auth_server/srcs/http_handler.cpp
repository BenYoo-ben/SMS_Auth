#include "http_handler.hpp"

std::string http_handler::get_html(std::string location){

	int fd = open(location.c_str(),O_RDONLY);

	char buffer[2048];

	int size_read = read(fd, buffer, 2048);

	buffer[size_read] = '\0';

	std::string html_page(buffer);

	close(fd);
	return html_page;
}
int http_handler::check_type(char *data){
	std::string data_str(data);

	//AUTH
	if(data_str.find("auth_code")!= std::string::npos){
		return HTTP_DATA_TYPE_AUTH;
	}else if(data_str.find("phone_number") != std::string::npos){
		return HTTP_DATA_TYPE_PHONE;
	}
	else
		return HTTP_DATA_TYPE_TABLE;

}

/* Establishment Process:
 * Phone -> Server : 0x11
 * Server-> Phone  : 0x06
 *
 */
bool http_handler::check_if_phone(char *data){
	bool at = false;

	if(data[0]==0x11)
		at = true;

	return at;
}

/* FORM :
 *	URL = 127.0.0.1:55551/[#table]?phone_number=[phone_number];
 */
std::string http_handler::get_data(char *data, int type){
	printf("!!\n");

	int i = 0;
	char buffer[256];
	int buf_i = 0;

	if(type == HTTP_DATA_TYPE_TABLE){

		while(data[i] != '/'){
			i++;
		}
		i++;

		while(data[i]!='?'){
			buffer[buf_i] = data[i];
			i++;
			buf_i++;
		}
		buffer[buf_i] = '\0';
		return std::string(buffer);
	}
	else if(type == HTTP_DATA_TYPE_PHONE){
		std::string tmp_str = std::string(data);
		int start_pos = tmp_str.find("phone_number=");
		std::string sub_string = tmp_str.substr(start_pos+13,  12);
		std::cout << sub_string << std::endl;

		return sub_string;

	}else if(type == HTTP_DATA_TYPE_AUTH){

		std::string tmp_str = std::string(data);
		int start_pos = tmp_str.find("auth_code=");
		std::string sub_string = tmp_str.substr(start_pos+10, 6);
		std::cout << sub_string << std::endl;

		return sub_string;
	
	}
	return NULL;
}

