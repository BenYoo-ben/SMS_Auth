#include "session.hpp"
#include "global.hpp"
#include "random_generator.hpp"

template<class C, void* (C::* thread_run)()>
void* pthread_member_wrapper(void* data) {
	C* obj = static_cast<C*>(data);
	return (obj->*thread_run)();
}

session_object::session_object(int established_socket, int phone_sock){
	p_sock = phone_sock;
	c_sock = established_socket;
	pthread_create(&session_thread, NULL, pthread_member_wrapper<session_object, &session_object::run>, this);
}

void *session_object::run(){

	char buffer[global_expected_MTU];
	int bytes_read;
	http_handler hh;
	

	bytes_read = read(c_sock, buffer, global_expected_MTU);

/*
	random_number_generator rng;
	int sms_data = rng.generate_int(6);
	write(c_sock, buffer,global_expected_MTU);

	memset(buffer,0x0,global_expected_MTU);
	bytes_read = read(c_sock,buffer,global_expected_MTU);
	*/
	if(bytes_read < 1){
		std::cerr << "Invalid Read" << std::endl;
		close_socket();
	}
	else{
		printf("Read complete ! : RECV=%s\n",buffer);
		
		memset(buffer,0x0,global_expected_MTU);	
		sprintf(buffer, "%s", hh.get_html(std::string("assets/auth_main.html")).c_str());
		write(c_sock, buffer,global_expected_MTU);

		bytes_read = read(c_sock, buffer, global_expected_MTU);
		if(bytes_read >= 1){
			printf("Data Read ! \n");
			memset(buffer,0x0,global_expected_MTU);
			std::vector<std::string> store_n_number = hh.get_store_and_number(buffer);
			std::string store = store_n_number.at(0);
			std::string number = store_n_number.at(1);

			random_number_generator rng;
			int sms_data = rng.generate_int(6);

			memset(buffer,0x0,global_expected_MTU);
			sprintf(buffer,"%s",number.c_str());
			sprintf(buffer+number.length(),"%d",sms_data);
			write(p_sock, buffer,18);

			close_socket();
		}

	}
	return NULL;
}

void session_object::close_socket(){
	close(c_sock);

}