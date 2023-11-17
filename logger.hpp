#pragma once 

#include <iostream>

class Logger {
public:
    static void info(const std::string& message) {
        std::cout << "\033[1;32m[INFO]: " << message << "\033[0m" << std::endl;
    }

    static void warn(const std::string& message) {
        std::cout << "\033[1;33m[WARNING]: " << message << "\033[0m" << std::endl;
    }

    static void error(const std::string& message) {
        std::cerr << "\033[1;31m[ERROR]: " << message << "\033[0m" << std::endl;
    }
};