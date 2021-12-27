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

std::vector<std::string> http_handler::get_store_and_number(char * data){

	std::vector<std::string> store_n_number;


	if(std::string(data).find("favicon") == std::string::npos)
		return store_n_number;

	
	int i = 0;
	while(data[i] != '/'){
		i++;
	}
	i++;

	char buffer[256];
	int buf_i = 0;

	while(data[i]!='?'){

		buffer[buf_i] = data[i];

		i++;
		buf_i++;
	}
	buffer[buf_i] = '\0';

	store_n_number.push_back(std::string(buffer));
	buf_i = 0;

	while(data[i]!='='){
		i++;
	}
	i++;

	while(data[i]!=' '){
		buffer[buf_i] = data[i];
		i++;
		buf_i++;
	}
	buffer[buf_i] = '\0';

	store_n_number.push_back(std::string(buffer));

	return store_n_number;
}

