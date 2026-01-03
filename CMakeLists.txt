cmake_minimum_required(VERSION 3.12)
project(cppy3)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(CPPY3_USE_BOOST_CONVERT "use Boost.Locale instead of std::codecvt for string conversion" OFF)
option(CPPY3_BUILD_EXECUTABLES "Build autotests and examples" ON)
option(CPPY3_LINK_NUMPY "Require numpy dependency lib" OFF)
option(CPPY3_AUTODETECT_NUMPY "Autodetect NumPy in system or venv" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()


# find Boost if necessary
if(USE_BOOST_CONVERT)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    find_package(Boost REQUIRED)
    message(STATUS "Using Boost string convert: Enabled")
    add_definitions(-DCPPY3_USE_BOOST_CONVERT)
endif()


set(PYTHON3_COMPONENTS Interpreter Development)
if(CPPY3_AUTODETECT_NUMPY)
    message(STATUS "Autodetecting NumPy in system or venv")
    find_package(Python3 3.5 REQUIRED COMPONENTS ${PYTHON3_COMPONENTS})
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "try:\n    import numpy\n    print(numpy.get_include())\nexcept:\n    pass"
        OUTPUT_VARIABLE __NumPy_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT __NumPy_INCLUDE_DIR STREQUAL "")
        message("NumPy detected: ${__NumPy_INCLUDE_DIR}")
        set(CPPY3_LINK_NUMPY ON)
    endif()
endif()

if(CPPY3_LINK_NUMPY)
    list(APPEND PYTHON3_COMPONENTS NumPy)
    add_definitions(-DPython3_FIND_VIRTUALENV=FIRST)
endif()
find_package(Python3 3.5 REQUIRED COMPONENTS ${PYTHON3_COMPONENTS})

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
    message(WARNING "Building cppy3 without numpy.ndarray support because NumPy not found in system")
endif()

# make lib
add_subdirectory(cppy3)

if(CPPY3_BUILD_EXECUTABLES)
    # make tests
    add_subdirectory(tests)
    # make examples
    add_executable(console examples/console.cpp)
    target_link_libraries(console cppy3)
endif()
