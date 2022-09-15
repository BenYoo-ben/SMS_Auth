#include "session.hpp"
#include "global.hpp"
#include "random_generator.hpp"

extern std::vector<int> global_phone_sockets;
extern int global_phone_index;

extern std::vector<int> global_auth_index;
extern std::vector<struct auth_data> global_auth;

template<class C, void* (C::* thread_run)()>
void* pthread_member_wrapper(void* data) {
    C* obj = static_cast<C*>(data);
    return (obj->*thread_run)();
}

session_object::session_object(int established_socket) {
    c_sock = established_socket;
    pthread_create(&session_thread, NULL, pthread_member_wrapper<session_object, &session_object::run>, this);
}
/*
 * 0x05 = check if phone is alive(no response = dead)     
 * meaning Enquiry
 */
int session_object::exchange_auth_with_phone(std::string phone_number, int random_data) {
    if (global_phone_sockets.empty()) {
        printf("No Phone Socket is available! \n");
        return -1;
    } else {
        if (global_phone_index > global_phone_sockets.size()) {
            global_phone_index = 0;
        }

        int use_socket = global_phone_sockets.at(global_phone_index);

        char buffer[18];

        // 0x05 = enquiry, checking if phone is alive
        buffer[0] = 0x05;

        int _size = write(use_socket, buffer, 1);

        if (_size != 1) {
            global_phone_sockets.erase(global_phone_sockets.begin() + global_phone_index);
            global_phone_index = 0;
            printf("Write ERR !\n");
            return -2;
        }

        // 0x06 = acknowledgement(ascii)
        _size = read(use_socket, buffer, 1);

        if (_size != 1 || buffer[0] != 0x06) {
            global_phone_sockets.erase(global_phone_sockets.begin() + global_phone_index);
            global_phone_index = 0;
            printf("Read ERR ! \n");
            return -2;
        }

        // 0x01 = start of header(ascii)
        buffer[0] = 0x01;
        _size = write(use_socket, buffer, 1);

        if (_size != 1) {
            global_phone_sockets.erase(global_phone_sockets.begin() + global_phone_index);
            global_phone_index = 0;
            printf("Write ERR !\n");
            return -2;
        }

        _size = read(use_socket, buffer, 1);

        // 0x06 = acknowledgement(ascii)
        if (_size != 1 || buffer[0] != 0x06) {
            global_phone_sockets.erase(global_phone_sockets.begin() + global_phone_index);
            global_phone_index = 0;
            printf("Read ERR !\n");
            return -2;
        }

        memset(buffer, 0x0, 18);
        snprintf(buffer, sizeof(buffer), "%s", phone_number.c_str());
        snprintf(buffer + phone_number.length(), sizeof(buffer) - phone_number.length(), "%d", random_data);

        printf("Send... <%s>\n", buffer);

        _size = write(use_socket, buffer, 18);

        if (_size != 18) {
            printf("Write 2 ERR !\n");
            return -1;
        }

        _size = read(use_socket, buffer, 1);

        if (_size != 1 || buffer[0] != 0x06) {
            printf("Read 2 ERR !\n");
            return -1;
        }

        return 0;
    }
}

void *session_object::run() {
    char buffer[global_expected_MTU];
    int bytes_read;
    http_handler hh;

    // read HTTP request
    bytes_read = read(c_sock, buffer, global_expected_MTU);


    if (bytes_read < 1) {
        std::cerr << "Invalid Read" << std::endl;
        close_socket();
    } else {
        printf("Read complete ! : RECV=%s\n", buffer);

        int data_type = hh.check_type(buffer);

        if (hh.check_if_phone(buffer)) {
            printf("New Phone asking for registration\nsending ack...\n");
            memset(buffer, 0x00, global_expected_MTU);;
            buffer[0] = 0x06;
            if (write(c_sock, buffer, 1) > 0) {
                printf("ack sent... !\n");
            }

            global_phone_sockets.push_back(c_sock);
        } else if (data_type == HTTP_DATA_TYPE_TABLE) {
            printf("TYPE TABLE \n");
            memset(buffer, 0x0, global_expected_MTU);
            snprintf(buffer, sizeof(buffer), "%s", hh.get_html(std::string("assets/auth_main.html")).c_str());
            write(c_sock, buffer, global_expected_MTU);

        } else if (data_type == HTTP_DATA_TYPE_PHONE) {
            printf("TYPE PHONE\n");

            std::string phone_number = hh.get_data(buffer, HTTP_DATA_TYPE_PHONE);
            if (phone_number.empty()) {
                perror("phone number is invalid(possibly empty)");
                return NULL;
            }
            // sms twice = mutex(add to map, delete add again)
            random_number_generator rng;
            int sms_auth_code = rng.generate_int(6);

            int ret;
            if ((ret = exchange_auth_with_phone(phone_number, sms_auth_code)) < 0) {
                while (ret == -2) {
                    printf("Retrying for phone...\n");
                    ret = exchange_auth_with_phone(phone_number, sms_auth_code);
                }

                if (ret == -1) {
                    printf("There are no available SMS Devices...\n");
                    std::string no_sms_dev_data = hh.get_html(std::string("assets/auth_fail.html"));
                    memset(buffer , 0x0, global_expected_MTU);
                    snprintf(buffer, sizeof(buffer), "%s", no_sms_dev_data.c_str());
                    write(c_sock, buffer, global_expected_MTU);
                    return NULL;
                }    
            }

            // replace SHARPS('#') with phone number
            memset(buffer, 0x0, global_expected_MTU);
            std::string phase_2_data = hh.get_html(std::string("assets/auth_phase_2.html"));
            snprintf(buffer, sizeof(buffer), "%s", phase_2_data.c_str());
            write(c_sock, buffer, global_expected_MTU);

            // insert into Vector
            struct auth_data data;
            data.auth_code = sms_auth_code;
            data.time = clock();
            data.phone_number = phone_number;

            printf("NEW DATA: [%d] %d [%s] \n", data.auth_code, data.time, data.phone_number.c_str());

            global_auth.push_back(data);
            global_auth_index.push_back(data.auth_code);
        } else if (data_type == HTTP_DATA_TYPE_AUTH) {
            std::string auth_str = hh.get_data(buffer, HTTP_DATA_TYPE_AUTH);
            if (auth_str.empty()) {
                perror("auth code is invalid(possibly empty)");
                return NULL;
            }
            int auth = stoi(auth_str);
            printf("AUTH = %s\n", auth_str.c_str());

            std::string table = hh.get_data(buffer, HTTP_DATA_TYPE_TABLE);
            std::string phone = hh.get_data(buffer, HTTP_DATA_TYPE_PHONE);

            if (table.empty()) {
                perror("table is invalid(possibly empty)");
                return NULL;
            }
            if (phone.empty()) {
                perror("phone is invalid(possibly empty)");
                return NULL;
            }

            // printf("Auth Code for [%s = %d]\n",phone.c_str(), global_auth_codes.at(phone));

            auto found_match = std::find(global_auth_index.begin(), global_auth_index.end(), auth);
            int found_index = std::distance(global_auth_index.begin(), found_match);

            if (found_match == global_auth_index.end()) {
                // key is not found
                printf("Input Code: [%s] does not exist in auth table\n", auth_str.c_str());
                std::string no_auth = hh.get_html(std::string("assets/auth_fail.html"));
                memset(buffer, 0x0, global_expected_MTU);
                snprintf(buffer, sizeof(buffer), "%s", no_auth.c_str());
                write(c_sock, buffer, global_expected_MTU);
                return NULL;

            } else {
                // key is found
                struct auth_data found_data = global_auth[found_index];

                printf("Found INDEX : %d\n", found_index);

                std::cout << "Found Data:\nCode:" << found_data.auth_code << "\nPhone:"
                    <<found_data.phone_number << std::endl;

                if (phone != found_data.phone_number) {
                    // auth code is found but the phone number is not correct...
                    printf("Input Code: [%s] should go with [%s], input=[%s]\n", auth_str.c_str(),
                            found_data.phone_number.c_str(), phone.c_str());
                    std::string no_auth = hh.get_html(std::string("assets/auth_fail.html"));
                    memset(buffer, 0x0, global_expected_MTU);
                    snprintf(buffer, sizeof(buffer), "%s", no_auth.c_str());
                    write(c_sock, buffer, global_expected_MTU);

                    printf("Erase code associated with phone number :%s\n", phone.c_str());
                    // global_auth.erase(global_auth.begin() + found_index);
                    // global_auth_index.erase(global_auth_index.begin() + found_index);
                    return NULL;
                } else {
                    // key is found and does match user input

                    clock_t now = clock();
                    double time_diff = (double)(now - found_data.time) / CLOCKS_PER_SEC;
                    if (time_diff > 1800) {
                        // key is found and does match user input, but code has expired(+30mins)
                        printf("Input Code: [%s] has expired\n", auth_str.c_str());
                        std::string no_auth = hh.get_html(std::string("assets/auth_fail.html"));
                        memset(buffer, 0x0, global_expected_MTU);
                        snprintf(buffer, sizeof(buffer), "%s", no_auth.c_str());
                        write(c_sock, buffer, global_expected_MTU);

                        printf("Erase code associated with phone number :%s\n", phone.c_str());
                        // global_auth.erase(global_auth.begin() + found_index);
                        // global_auth_index.erase(global_auth_index.begin() + found_index);

                        return NULL;
                    }
                    // TO-DO: make fail attempts into function(redundant, repetitive codes)

                    // holy, you finally got here :) you are all good to go
                    std::string redirect = hh.get_html(std::string("assets/redirect.html"));

                    printf("TABLE = %s\nPHONE = %s\n\n", table.c_str(), phone.c_str());

                    redirect.replace(redirect.find("!!!"), 3, table);
                    redirect.replace(redirect.find("@@@"), 3, phone);
                    memset(buffer, 0x0, global_expected_MTU);
                    snprintf(buffer, sizeof(buffer), "%s", redirect.c_str());
                    write(c_sock, buffer, global_expected_MTU);
                    printf("Erase codes associated with phone number :%s\n", phone.c_str());
                    global_auth.erase(global_auth.begin() + found_index);
                    global_auth_index.erase(global_auth_index.begin() + found_index);
                }
            }

        } else if (data_type == HTTP_DATA_TYPE_STYLE_SHEET) {
            printf("Style Sheet Request\n");
            std::string style = hh.get_html(std::string("assets/css/style.css"));
            char saa_buffer[STYLE_AND_AGREEMENT_BUFFER];

            printf("NEED SIZE : %d\n", style.length());
            memset(saa_buffer, 0x0, STYLE_AND_AGREEMENT_BUFFER);
            snprintf(saa_buffer, sizeof(buffer), "%s", style.c_str());
            write(c_sock, saa_buffer, STYLE_AND_AGREEMENT_BUFFER);

            printf("Style Sent \n");
        } else if (data_type == HTTP_DATA_TYPE_AGREEMENT) {
            printf("Agreement Request\n");
            std::string agree = hh.get_html(std::string("assets/agreement.html"));

            char saa_buffer[STYLE_AND_AGREEMENT_BUFFER];
            memset(saa_buffer, 0x0, STYLE_AND_AGREEMENT_BUFFER);

            snprintf(saa_buffer, sizeof(buffer), "%s", agree.c_str());
            write(c_sock, saa_buffer, STYLE_AND_AGREEMENT_BUFFER);

            printf("Agreement Sent\n");
        }
    }
    printf("END OF SESSION\n");
    return NULL;
}

void session_object::close_socket() {
    close(c_sock);
}
