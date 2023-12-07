cmake_minimum_required(VERSION 3.12)
project(cppy3)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(USE_BOOST_CONVERT "use Boost.Locale instead of std::codecvt for string conversion" OFF)

set(CMAKE_CXX_FLAGS "-std=c++11 -fPIC -g")

# find Boost if necessary
if(USE_BOOST_CONVERT)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    find_package(Boost REQUIRED)
    message(STATUS "Using Boost string convert: Enabled")
    add_definitions(-DCPPY3_USE_BOOST_CONVERT)
endif()

find_package (Python3 3.5 REQUIRED COMPONENTS Interpreter Development NumPy)

message(STATUS "Found Python: ${Python3_FOUND} ${Python3_INTERPRETER_ID} ${Python3_EXECUTABLE}")
message(STATUS "Found Python3_LIBRARIES: ${Python3_LIBRARIES}")
message(STATUS "Found Python3_Development_FOUND: ${Python3_Development_FOUND}")
message(STATUS "Found Python3_NumPy_FOUND: ${Python3_NumPy_FOUND} ${Python3_NumPy_INCLUDE_DIRS}")

if (Python3_Development_FOUND)
    include_directories(${Python3_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Python dev C headers required (python-dev package)" )
endif()

if(Python3_NumPy_FOUND)
    add_definitions(-DCPPY3_BUILT_WITH_NUMPY)
else()
    message(WARNING "Building cppy3 without numpy.ndarray support because NumPy is not found in system")
endif()

# make lib
add_subdirectory(cppy3)

# make tests
add_subdirectory(tests)

# make examples
add_executable(console examples/console.cpp)
target_link_libraries(console cppy3)
