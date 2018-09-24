#include "cppy3/cppy3.hpp"
#include <iostream>


int main(int argc, char *argv[]) {
  try {
    // create interpreter
    cppy3::PythonVM instance;

    // interactive python console loop
    std::cout << "Hey, type in command line, e.g. print(2+2*2)" << std::endl;
    size_t i = 0;
    for (std::string line; std::getline(std::cin, line); i++) {
        const cppy3::Var result = cppy3::exec(line);
        std::cout << "[#" << i << " " << result.typeName()<< "]" << result.toUTF8String() << std::endl;
    }
  } catch (const cppy3::PythonException& e) {
    // catch and forward exception from python layer
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
