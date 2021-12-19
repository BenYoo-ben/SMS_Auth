#include "session.hpp"
#include "global.hpp"

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

	while(true){

		memset(buffer,0x0,global_expected_MTU);

		bytes_read = read(c_sock,buffer,global_expected_MTU);
		if(bytes_read < 1){
			std::cerr << "Invalid Read" << std::endl;
			close_socket();
			break;
		}
		else{
			
		}
	}
	return NULL;
}

void session_object::close_socket(){
	close(c_sock);

}