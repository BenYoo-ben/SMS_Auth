#include "http_handler.hpp"

std::string http_handler::get_html(std::string location){

	int fd = open(location.c_str(),O_RDONLY);

	char buffer[18000];


	int size_read = read(fd, buffer, 18000);

	buffer[size_read] = '\0';

	std::string html_page(buffer);
	
	printf("LENGTH : %d\n",html_page.length());
	close(fd);
	return html_page;
}
int http_handler::check_type(char *data){
	std::string data_str(data);

	//AUTH
	if(data_str.find(".css")!=std::string::npos){
		return HTTP_DATA_TYPE_STYLE_SHEET;
	}
	else if(data_str.find("agreement")!=std::string::npos){
		return HTTP_DATA_TYPE_AGREEMENT;
	}
	else if(data_str.find("auth_code")!= std::string::npos){
		return HTTP_DATA_TYPE_AUTH;
	}else if(data_str.find("phone_number") != std::string::npos){
		return HTTP_DATA_TYPE_PHONE;
	}
	else{
		return HTTP_DATA_TYPE_TABLE;
	}

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

	printf("Get data of type [%d]\n",type);
	int i = 0;
	char buffer[256];
	int buf_i = 0;

	if(type == HTTP_DATA_TYPE_TABLE){
		//LOGIC MUST BE FIXED HERE(segfault)
		while(data[i] != '/' && i < 256){
			i++;
			printf("i:%d\n",i);
		}
		i++;

		while(data[i]!='?' && i < 256){
			buffer[buf_i] = data[i];
			i++;
			buf_i++;

			printf("i:%d, buf_i:%d\n",i,buf_i);
		}
		buffer[buf_i] = '\0';
		return std::string(buffer);
	}
	else if(type == HTTP_DATA_TYPE_PHONE){
		std::string tmp_str = std::string(data);
		unsigned int start_pos = tmp_str.find("phone_number=");
		if(start_pos!=std::string::npos)
		{
			std::string sub_string = tmp_str.substr(start_pos+13,  12);
			std::cout << sub_string << std::endl;

			return sub_string;	
		}
		else
			return "";

	}else if(type == HTTP_DATA_TYPE_AUTH){

		std::string tmp_str = std::string(data);
		unsigned int start_pos = tmp_str.find("auth_code=");
		if(start_pos!=std::string::npos){
			std::string sub_string = tmp_str.substr(start_pos+10, 6);
			std::cout << sub_string << std::endl;

			return sub_string;
		}
		else
			return "";
	}
	return "";
}

char * http_handler::encoding(char *text_input, char *source, char *target)
{
	iconv_t it;

	int input_len = strlen(text_input) + 1;
	int output_len = input_len * 2;


	size_t in_size = input_len;
	size_t out_size = output_len;

	char *output = (char *)malloc(output_len);

	char *output_buf = output;
	char *input_buf = text_input;

	it = iconv_open(target, source); 
	int ret = iconv(it, &input_buf, &in_size, &output_buf, &out_size);


	iconv_close(it);

	return output;
}



void session_invalid(char *str)
{

}
