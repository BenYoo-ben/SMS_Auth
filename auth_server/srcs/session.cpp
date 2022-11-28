#include "session.hpp"
#include "global.hpp"
#include "random_generator.hpp"

template<class C, void* (C::* thread_run)()>
void* pthread_member_wrapper(void* data) {
    C* obj = static_cast<C*>(data);
    return (obj->*thread_run)();
}

session_object::session_object(int established_socket) {
    c_sock = established_socket;
    pthread_create(&session_thread, nullptr, pthread_member_wrapper<session_object, &session_object::run>, this);
}

// 0x05 = check if phone is alive(no response = dead)
// meaning Enquiry
int session_object::exchange_auth_with_phone(std::string phone_number, int random_data) {
    if (g_obj->global_phone_sockets.empty()) {
        std::cerr << "No Phone Socket is available!" << std::endl;
        return -1;
    } else {
        if (g_obj->global_phone_index > g_obj->global_phone_sockets.size()) {
            g_obj->global_phone_index = 0;
        }

        int use_socket = g_obj->global_phone_sockets.at(g_obj->global_phone_index);

        char buffer[18];

        // 0x05 = enquiry, checking if phone is alive
        buffer[0] = 0x05;

        int check_size = write(use_socket, buffer, 1);
        if (check_size != 1) {
            g_obj->global_phone_sockets.erase(g_obj->global_phone_sockets.begin() + g_obj->global_phone_index);
            g_obj->global_phone_index = 0;
            std::cerr << "Write Enquiry(0x05) code failed" << std::endl;
            return -2;
        }

        // 0x06 = acknowledgement(ascii)
        check_size = read(use_socket, buffer, 1);

        if (check_size != 1 || buffer[0] != 0x06) {
            g_obj->global_phone_sockets.erase(g_obj->global_phone_sockets.begin() + g_obj->global_phone_index);
            g_obj->global_phone_index = 0;
            std::cerr << "Read ACK(0x06) code for Enquiry failed" << std::endl;
            return -2;
        }

        // 0x01 = start of header(ascii)
        buffer[0] = 0x01;
        check_size = write(use_socket, buffer, 1);

        if (check_size != 1) {
            g_obj->global_phone_sockets.erase(g_obj->global_phone_sockets.begin() + g_obj->global_phone_index);
            g_obj->global_phone_index = 0;
            std::cerr << "Write Header code(0x01) failed" << std::endl;
            return -2;
        }

        check_size = read(use_socket, buffer, 1);

        // 0x06 = acknowledgement(ascii)
        if (check_size != 1 || buffer[0] != 0x06) {
            g_obj->global_phone_sockets.erase(g_obj->global_phone_sockets.begin() + g_obj->global_phone_index);
            g_obj->global_phone_index = 0;
            std::cerr << "Read ACK(0x06) code for Header code failed" << std::endl;
            return -2;
        }

        memset(buffer, 0x00, 18);
        snprintf(buffer, sizeof(buffer), "%s", phone_number.c_str());
        snprintf(buffer + phone_number.length(), sizeof(buffer) - phone_number.length(), "%d", random_data);

        check_size = write(use_socket, buffer, 18);

        if (check_size != 18) {
            std::cerr << "Write [KR]Phone Number + Secrect Code failed" << std::endl;
            return -1;
        }

        check_size = read(use_socket, buffer, 1);

        if (check_size != 1 || buffer[0] != 0x06) {
            std::cerr << "Read ACK(0x06) code for Secret Code failed" << std::endl;
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
        std::cerr << "Read HTTP Request failed" << std::endl;
        close_socket();
    } else {
        int data_type = hh.check_type(buffer);

        if (hh.check_if_phone(buffer)) {
            memset(buffer, 0x00, global_expected_MTU);;
            buffer[0] = 0x06;
            if (write(c_sock, buffer, 1) < 0) {
                std::cerr << "Write ACK(0x06) code for HTTP Request failed" << std::endl;
                return nullptr;
            }
            g_obj->global_phone_sockets.push_back(c_sock);
        } else if (data_type == HTTP_DATA_TYPE_TABLE) {
            memset(buffer, 0x0, global_expected_MTU);
            snprintf(buffer, sizeof(buffer), "%s", hh.get_html(std::string("assets/auth_main.html")).c_str());
            if (write(c_sock, buffer, global_expected_MTU) < 0) {
                std::cerr << "Write html main page failed" << std::endl;
                return nullptr;
            }
        } else if (data_type == HTTP_DATA_TYPE_PHONE) {
            std::string phone_number = hh.get_data(buffer, HTTP_DATA_TYPE_PHONE);
            if (phone_number.empty()) {
                std::cerr << "phone number is invalid(possibly empty)" << std::endl;
                return nullptr;
            }

            random_number_generator rng;
            int sms_auth_code = rng.generate_int(6);

            int ret = 0;
            if ((ret = exchange_auth_with_phone(phone_number, sms_auth_code)) < 0) {
                while (ret == -2) {
                    std::cerr << "Retrying to authenticate..." << std::endl;
                    ret = exchange_auth_with_phone(phone_number, sms_auth_code);
                }

                if (ret == -1) {
                    std::string no_sms_dev_data = hh.get_html(std::string("assets/auth_fail.html"));
                    memset(buffer , 0x0, global_expected_MTU);
                    snprintf(buffer, sizeof(buffer), "%s", no_sms_dev_data.c_str());
                    if (write(c_sock, buffer, global_expected_MTU) < 0) {
                        std::cerr << "Write html fail page failed" << std::endl;
                    }
                    return nullptr;
                }
            }

            // replace SHARPS('#') with phone number
            memset(buffer, 0x0, global_expected_MTU);
            std::string phase_2_data = hh.get_html(std::string("assets/auth_phase_2.html"));
            snprintf(buffer, sizeof(buffer), "%s", phase_2_data.c_str());
            if (write(c_sock, buffer, global_expected_MTU) < 0) {
                std::cerr << "Write html 2nd auth phage failed" << std::endl;
                return nullptr;
            }

            struct auth_data data;
            data.auth_code = sms_auth_code;
            data.time = clock();
            data.phone_number = phone_number;

            g_obj->global_auth.push_back(data);
            g_obj->global_auth_index.push_back(data.auth_code);
        } else if (data_type == HTTP_DATA_TYPE_AUTH) {
            std::string auth_str = hh.get_data(buffer, HTTP_DATA_TYPE_AUTH);
            if (auth_str.empty()) {
                std::cerr << "auth code is invalid(possibly empty)" << std::endl;
                return nullptr;
            }

            int auth = stoi(auth_str);

            std::string table = hh.get_data(buffer, HTTP_DATA_TYPE_TABLE);
            std::string phone = hh.get_data(buffer, HTTP_DATA_TYPE_PHONE);

            if (table.empty()) {
                std::cerr << "table is invalid(possibly empty)" << std::endl;
                return nullptr;
            }
            if (phone.empty()) {
                std::cerr << "phone is invalid(possibly empty)" << std::endl;
                return nullptr;
            }

            auto found_match = std::find(g_obj->global_auth_index.begin(), g_obj->global_auth_index.end(), auth);
            int found_index = std::distance(g_obj->global_auth_index.begin(), found_match);

            if (found_match == g_obj->global_auth_index.end()) {
                // key is not found
                std::string no_auth = hh.get_html(std::string("assets/auth_fail.html"));
                memset(buffer, 0x0, global_expected_MTU);
                snprintf(buffer, sizeof(buffer), "%s", no_auth.c_str());
                if (write(c_sock, buffer, global_expected_MTU) < 0) {
                    std::cerr << "Write html auth fail page failed" << std::endl;
                }
                return nullptr;
            } else {
                // key is found
                struct auth_data found_data = g_obj->global_auth[found_index];

                if (phone != found_data.phone_number) {
                    // auth code is found but the phone number is not correct...
                    std::string no_auth = hh.get_html(std::string("assets/auth_fail.html"));
                    memset(buffer, 0x0, global_expected_MTU);
                    snprintf(buffer, sizeof(buffer), "%s", no_auth.c_str());
                    if (write(c_sock, buffer, global_expected_MTU) < 0) {
                        std::cerr << "Write html auth fail page failed" << std::endl;
                    }

                    return nullptr;
                } else {
                    // key is found and does match user input
                    clock_t now = clock();
                    double time_diff = static_cast<double>((now - found_data.time)) / CLOCKS_PER_SEC;
                    if (time_diff > 1800) {
                        // key is found and does match user input, but code has expired(+30mins)
                        std::string no_auth = hh.get_html(std::string("assets/auth_fail.html"));
                        memset(buffer, 0x0, global_expected_MTU);
                        snprintf(buffer, sizeof(buffer), "%s", no_auth.c_str());
                        if (write(c_sock, buffer, global_expected_MTU) < 0) {
                            std::cerr << "Write html auth fail page failed" << std::endl;
                            return nullptr;
                        }

                        return nullptr;
                    }

                    std::string redirect = hh.get_html(std::string("assets/redirect.html"));

                    redirect.replace(redirect.find("!!!"), 3, table);
                    redirect.replace(redirect.find("@@@"), 3, phone);
                    memset(buffer, 0x0, global_expected_MTU);
                    snprintf(buffer, sizeof(buffer), "%s", redirect.c_str());
                    if (write(c_sock, buffer, global_expected_MTU) < 0) {
                        std::cerr << "Write html redirect page failed" << std::endl;
                        return nullptr;
                    }
                    g_obj->global_auth.erase(g_obj->global_auth.begin() + found_index);
                    g_obj->global_auth_index.erase(g_obj->global_auth_index.begin() + found_index);
                }
            }

        } else if (data_type == HTTP_DATA_TYPE_STYLE_SHEET) {
            std::string style = hh.get_html(std::string("assets/css/style.css"));
            char saa_buffer[STYLE_AND_AGREEMENT_BUFFER];

            memset(saa_buffer, 0x0, STYLE_AND_AGREEMENT_BUFFER);
            snprintf(saa_buffer, sizeof(buffer), "%s", style.c_str());
            if (write(c_sock, saa_buffer, STYLE_AND_AGREEMENT_BUFFER) < 0) {
                std::cerr << "Write css style failed" << std::endl;
            }
        } else if (data_type == HTTP_DATA_TYPE_AGREEMENT) {
            std::string agree = hh.get_html(std::string("assets/agreement.html"));

            char saa_buffer[STYLE_AND_AGREEMENT_BUFFER];
            memset(saa_buffer, 0x0, STYLE_AND_AGREEMENT_BUFFER);

            snprintf(saa_buffer, sizeof(buffer), "%s", agree.c_str());
            if (write(c_sock, saa_buffer, STYLE_AND_AGREEMENT_BUFFER) < 0) {
                std::cerr << "Write html agreement page failed" << std::endl;
            }
        }
    }
    return nullptr;
}

void session_object::close_socket() {
    close(c_sock);
}
