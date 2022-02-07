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
/*
 * 0x05 = check if phone is alive(no response = dead)	 
 * meaning Enquiry
 */
int session_object::exchange_auth_with_phone(std::string phone_number, int random_data){

	if(global_phone_sockets.empty()){
		printf("No Phone Socket is available! \n");
		return -1;

	}
	else{
		if(global_phone_index > global_phone_sockets.size()){
			global_phone_index = 0;
		}

		int use_socket = global_phone_sockets.at(global_phone_index);


		char buffer[18];

		// 0x05 = enquiry, checking if phone is alive
		buffer[0] = 0x05;

		int _size = write(use_socket, buffer, 1);

		if( _size != 1){
			global_phone_sockets.erase(global_phone_sockets.begin() + global_phone_index);
			global_phone_index = 0;
			printf("Write ERR !\n");
			return -2;
		}

		// 0x06 = acknowledgement(ascii)
		_size = read(use_socket, buffer, 1);

		if(_size != 1 || buffer[0]!=0x06){
			global_phone_sockets.erase(global_phone_sockets.begin() + global_phone_index);
			global_phone_index = 0;
			printf("Read ERR ! \n");
			return -2;
		}

		// 0x01 = start of header(ascii)
		buffer[0] = 0x01;
		_size = write(use_socket, buffer, 1);

		if(_size != 1){
			global_phone_sockets.erase(global_phone_sockets.begin() + global_phone_index);
			global_phone_index = 0;
			printf("Write ERR !\n");
			return -2;
		}

		_size = read(use_socket, buffer, 1);

		// 0x06 = acknowledgement(ascii)
		if(_size != 1 || buffer[0]!= 0x06){
			global_phone_sockets.erase(global_phone_sockets.begin() + global_phone_index);
			global_phone_index = 0;
			printf("Read ERR !\n");
			return -2;
		}

		memset(buffer,0x0,18);
		sprintf(buffer,"%s",phone_number.c_str());
		sprintf(buffer+phone_number.length()-1,"%d",random_data);

		printf("Send... <%s>\n",buffer);

		_size = write(use_socket, buffer, 18);

		if(_size != 18){
			printf("Write 2 ERR !\n");
			return -1;
		}

		_size = read(use_socket, buffer, 1);

		if(_size != 1 || buffer[0] != 0x06){
			printf("Read 2 ERR !\n");
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
			if(phone_number.empty()){
				perror("phone number is invalid(possibly empty)");	
				return NULL;
			}

			if(global_auth_codes.find(phone_number)==global_auth_codes.end())
			{
//sms twice = mutex(add to map, delete add again)
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
						std::string no_sms_dev_data = hh.get_html(std::string("assets/auth_fail.html"));
						memset(buffer ,0x0, global_expected_MTU);
						sprintf(buffer, "%s", no_sms_dev_data.c_str());
						write(c_sock, buffer, global_expected_MTU);
						return NULL;
					}	

				}


				//replace SHARPS('#') with phone number
				memset(buffer, 0x0, global_expected_MTU);
				std::string phase_2_data = hh.get_html(std::string("assets/auth_phase_2.html"));
				sprintf(buffer, "%s", phase_2_data.c_str());
				write(c_sock, buffer, global_expected_MTU);
				//insert into MAP
				global_auth_codes.insert({phone_number, sms_auth_code});
				global_auth_timeouts.insert({phone_number, clock()});
			}
			else{
				//duplicate, just ignore this request
				return NULL;
			}
		}else if(data_type == HTTP_DATA_TYPE_AUTH){

			std::string auth_str = hh.get_data(buffer, HTTP_DATA_TYPE_AUTH);
			if(auth_str.empty()){
				perror("auth code is invalid(possibly empty)");
				return NULL;
			}
			int auth = stoi(auth_str);
			printf("AUTH = %s\n", auth_str.c_str());

			std::string table = hh.get_data(buffer, HTTP_DATA_TYPE_TABLE);
			std::string phone = hh.get_data(buffer, HTTP_DATA_TYPE_PHONE);

			if(table.empty()){
				perror("table is invalid(possibly empty)");
				return NULL;
			}
			if(phone.empty()){
				perror("phone is invalid(possibly empty)");
				return NULL;
			}

			//printf("Auth Code for [%s = %d]\n",phone.c_str(), global_auth_codes.at(phone));
			std::map<std::string, int>::iterator it;

			it = global_auth_codes.find(phone);
			if(it == global_auth_codes.end())
			{
				//key is not found
				printf("Input Code: [%s] does not exist in man\n",auth_str.c_str());
				std::string no_auth = hh.get_html(std::string("assets/auth_fail.html"));
				memset(buffer, 0x0, global_expected_MTU);
				sprintf(buffer, "%s", no_auth.c_str());
				write(c_sock, buffer, global_expected_MTU);
				return NULL;

			}
			else
			{
				//key is found
				int phone_number = it->second;
				printf("Found Auth Code = %d\n",phone_number);
				if(phone_number != auth)
				{
					//key is found but doesn't match user input

					printf("Input Code: [%s] does not exist in man\n",auth_str.c_str());
					std::string no_auth = hh.get_html(std::string("assets/auth_fail.html"));
					memset(buffer, 0x0, global_expected_MTU);
					sprintf(buffer, "%s", no_auth.c_str());
					write(c_sock, buffer, global_expected_MTU);

					printf("Erase code associated with phone number :%s\n", phone.c_str());	
					global_auth_codes.erase(it);

					return NULL;

				}
				else
				{
					//key is found and does match user input
					std::map<std::string, clock_t>::iterator it2 = global_auth_timeouts.find(phone);

					clock_t now = clock();
					if( (float( now - it2->second) / CLOCKS_PER_SEC) > 1800)
					{

						//key is found and does match user input, but code has expired(+30mins) 
						printf("Input Code: [%s] has expired\n",auth_str.c_str());
						std::string no_auth = hh.get_html(std::string("assets/auth_fail.html"));
						memset(buffer, 0x0, global_expected_MTU);
						sprintf(buffer, "%s", no_auth.c_str());
						write(c_sock, buffer, global_expected_MTU);

						printf("Erase code associated with phone number :%s\n", phone.c_str());	
						global_auth_codes.erase(it);

						return NULL;
					}

					//TO-DO: make fail attempts into function(redundant, repetitive codes)

					// holy, you finally got here :) you are all good to go
					std::string redirect = hh.get_html(std::string("assets/redirect.html"));

					printf("TABLE = %s\nPHONE = %s\n\n",table.c_str(), phone.c_str());

					redirect.replace(redirect.find("!!!"),3,table);
					redirect.replace(redirect.find("@@@"),3,phone);
					memset(buffer, 0x0, global_expected_MTU);
					sprintf(buffer, "%s", redirect.c_str());
					write(c_sock, buffer, global_expected_MTU);
					printf("Erase codes associated with phone number :%s\n", phone.c_str());
					global_auth_codes.erase(it);

				}
			}


		}
		else if(data_type == HTTP_DATA_TYPE_STYLE_SHEET){
			printf("Style Sheet Request\n");
			std::string style = hh.get_html(std::string("assets/css/style.css"));
			memset(buffer,0x0,global_expected_MTU);
			sprintf(buffer, "%s", style.c_str());
			write(c_sock, buffer, global_expected_MTU);

			printf("Style Sent \n");
		}
		else if(data_type == HTTP_DATA_TYPE_AGREEMENT){
			printf("Agreement Request\n");
			std::string agree = hh.get_html(std::string("assets/agreement.html"));

			memset(buffer,0x0,global_expected_MTU);

			sprintf(buffer,"%s",agree.c_str());
			write(c_sock, buffer, global_expected_MTU);

			printf("Agreement Sent\n");
		}

	}
	printf("END OF SESSION\n");
	return NULL;
}

void session_object::close_socket(){
	close(c_sock);

}
