#include "main.hpp"
#include "common.hpp"
#include "nets.hpp"
#include "random_generator.hpp"

int main(){
    server_object so;

    random_number_generator rng;
    std::cout << rng.generate_int(5) << std::endl;

    if(so.server_socket_init() < 0){
        //TO-DO: exceptions
        std::cout << "1" << std::endl;
    }

    if(so.server_socket_bind() < 0 ){
        //TO-DO: exceptions
        std::cout << "2" << std::endl;
    }

    if(so.server_socket_listen() < 0 ){
        //TO-DO: exceptions
        std::cout << "3" << std::endl;
    }

    so.server_socket_start();

}