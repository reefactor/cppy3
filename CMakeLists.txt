cmake_minimum_required(VERSION 3.1)
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

# find Python 3.x
set(REQUIRED_PYTHON_VERSION "3.5")
message(STATUS "Recomended python version is ${REQUIRED_PYTHON_VERSION}")
find_package(PythonInterp ${REQUIRED_PYTHON_VERSION} REQUIRED)
find_package(PythonLibs ${PYTHON_VERSION_STRING} REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

# find numpy 
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(NumPy)
if(NUMPY_FOUND)
    add_definitions(-DCPPY3_BUILT_WITH_NUMPY)
else()
    message(WARNING "Build cppy3 without numpy.ndarray support because NumPy is not found in system")
endif()

# make lib
add_subdirectory(cppy3)

# make tests
add_executable(tests tests/tests.cpp)
target_link_libraries(tests cppy3)

# make examples
add_executable(console examples/console.cpp)
target_link_libraries(console cppy3)
