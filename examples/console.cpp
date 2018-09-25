#include "cppy3/cppy3.hpp"
#include <iostream>


int main(int argc, char *argv[]) {

    // create interpreter
    cppy3::PythonVM instance;

    std::cout << "Hey, type in command line, e.g. print(2+2*2)" << std::endl << std::endl;
    size_t i = 0;
    for (std::string line; std::getline(std::cin, line); i++) {
        try {

            const cppy3::Var result = cppy3::eval(line.c_str());
            std::cout << std::endl << "[#" << i << " " << result.typeName()<< "] " << result.toUTF8String() << std::endl;

        } catch (const cppy3::PythonException& e) {

            std::cerr << e.what() << std::endl;

        }
    }
    return 0;
}
