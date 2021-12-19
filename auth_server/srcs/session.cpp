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

void *session_object::run(){

	char buffer[global_expected_MTU];
	int bytes_read;

	random_number_generator rng;

	int sms_data = rng.generate_int(6);

	memset(buffer,0x0,global_expected_MTU);
	sprintf(buffer, "%d", sms_data);
	write(c_sock, buffer,global_expected_MTU);

	memset(buffer,0x0,global_expected_MTU);
	bytes_read = read(c_sock,buffer,global_expected_MTU);
	if(bytes_read < 1){
		std::cerr << "Invalid Read" << std::endl;
		close_socket();
	}
	else{
		printf("Auth complete ! : RECV=%d\n",buffer);

	}
	return NULL;
}

void session_object::close_socket(){
	close(c_sock);

}