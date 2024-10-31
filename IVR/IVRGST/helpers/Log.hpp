#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>

#define LOG_W std::cout << "\033[1;33m[" << __FUNCTION__ << ":" << __LINE__ << "] "
#define LOG_I std::cout << "\033[1;30m[" << __FUNCTION__ << ":" << __LINE__ << "] "
#define LOG_D std::cout << "\033[1;32m[" << __FUNCTION__ << ":" << __LINE__ << "] "
#define LOG_E std::cerr << "[" << __FUNCTION__ << ":" << __LINE__ << "] "
#define ENDL "\033[0m\n"

#endif