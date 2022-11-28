#include "random_generator.hpp"

int random_number_generator::generate_int(int digits){
    std::random_device ran_dev;

    std::mt19937 gen(ran_dev());

    int min = pow(10,digits-1);
    int max = (pow(10, digits) - 1);

    std::uniform_int_distribution<int> dis(min, max);

    return dis(gen);
}
