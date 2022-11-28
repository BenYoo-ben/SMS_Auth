#ifndef AUTH_SERVER_INCLUDES_RANDOM_GENERATOR_HPP_
#define AUTH_SERVER_INCLUDES_RANDOM_GENERATOR_HPP_ 
#include <random>
#include <chrono>
#include <cmath>
#include "common.hpp"

class random_number_generator{
public:
    int generate_int(int digits);
};

#endif  // AUTH_SERVER_INCLUDES_RANDOM_GENERATOR_HPP_
