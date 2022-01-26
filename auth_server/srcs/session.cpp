#include "session.hpp"
#include "global.hpp"
#include "random_generator.hpp"

template<class C, void* (C::* thread_run)()>
void* pthread_member_wrapper(void* data) {
	C* obj = static_cast<C*>(data);
	return (obj->*thread_run)();
}

session_object::session_object(int established_socket){
	c_sock = established_socket;
	pthread_create(&session_thread, NULL, pthread_member_wrapper<session_object, &session_object::run>, this);
}

int session_object::exchange_auth_with_phone(std::string phone_number, int random_data){

	if(global_phone_sockets.empty()){
		return -1;
	}
	else{
		if(global_phone_index > global_phone_sockets.size()){
			global_phone_index = 0;
		}

		int use_socket = global_phone_sockets.at(global_phone_index);

		char buffer[18];

		// 0x01 = start of header(ascii)
		buffer[0] = 0x01;

		int _size = write(use_socket, buffer, 1);

		if(_size != 1){
			global_phone_sockets.erase(global_phone_sockets.begin() + global_phone_index);
			global_phone_index = 0;
			return -2;
		}

		_size = read(use_socket, buffer, 1);

		// 0x06 = acknowledgement(ascii)
		if(_size != 1 || buffer[0]!= 0x06){
			global_phone_sockets.erase(global_phone_sockets.begin() + global_phone_index);
			global_phone_index = 0;
			return -2;
		}

		memset(buffer,0x0,18);
		sprintf(buffer,"%s",phone_number.c_str());
		sprintf(buffer+phone_number.length(),"%d",random_data);

		printf("Send... <%s>\n",buffer);

		_size = write(use_socket, buffer, 18);

		if(_size != 7){
			return -1;
		}

		_size = read(use_socket, buffer, 1);

		if(_size != 1 || buffer[0] != 0x06){
			return -1;
		}

		return 0;
	}
}

void *session_object::run(){

	char buffer[global_expected_MTU];
	int bytes_read;
	http_handler hh;

	//read HTTP request
	bytes_read = read(c_sock, buffer, global_expected_MTU);


	if(bytes_read < 1){
		std::cerr << "Invalid Read" << std::endl;
		close_socket();
	}
	else{
		printf("Read complete ! : RECV=%s\n",buffer);

		int data_type = hh.check_type(buffer);

		if(hh.check_if_phone(buffer)){
			printf("New Phone asking for registration\nsending ack...\n");
			memset(buffer,0x0,global_expected_MTU);;
			buffer[0] = 0x06;
			if(write(c_sock, buffer, 1) > 0){
				printf("ack sent... !\n");
			}
			
			global_phone_sockets.push_back(c_sock);

		}else if(data_type == HTTP_DATA_TYPE_TABLE){
			printf("TYPE TABLE \n");
			memset(buffer,0x0,global_expected_MTU);	
			sprintf(buffer, "%s", hh.get_html(std::string("assets/auth_main.html")).c_str());
			write(c_sock, buffer,global_expected_MTU);

		}else if(data_type == HTTP_DATA_TYPE_PHONE){

			printf("TYPE PHONE\n");

			std::string phone_number = hh.get_data(buffer, HTTP_DATA_TYPE_PHONE);

			random_number_generator rng;
			int sms_auth_code = rng.generate_int(6);

			int ret;
			if( (ret = exchange_auth_with_phone(phone_number, sms_auth_code)) < 0)
			{
				while(ret == -2)
				{
					ret = exchange_auth_with_phone(phone_number, sms_auth_code);
				}
				
				if(ret == -1)
				{
					printf("There are no available SMS Devices...\n");
					std::string no_sms_dev_data = hh.get_html(std::string("assets/no_sms_dev.html"));
					memset(buffer ,0x0, global_expected_MTU);
					sprintf(buffer, "%s", no_sms_dev_data.c_str());
					write(c_sock, buffer, global_expected_MTU);
					return NULL;
				}	
			
			}

			global_auth_codes.insert({phone_number, sms_auth_code});

			//replace SHARPS('#') with phone number
			memset(buffer, 0x0, global_expected_MTU);
			std::string phase_2_data = hh.get_html(std::string("assets/auth_phase_2.html"));
			phase_2_data.replace(phase_2_data.find("####"), 4, phone_number);
			sprintf(buffer, "%s", phase_2_data.c_str());
			write(c_sock, buffer, global_expected_MTU);


		}else if(data_type == HTTP_DATA_TYPE_AUTH){
			std::string auth_from_user_string = hh.get_data(buffer, HTTP_DATA_TYPE_AUTH);
			/* resume here, get phone number, and if correct ? redirect to Wix */

		}

	}
	printf("END OF SESSION\n");
	return NULL;
}

void session_object::close_socket(){
	close(c_sock);

}
