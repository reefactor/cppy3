add_executable(tests tests.cpp)
target_link_libraries(tests cppy3)
include_directories(${PROJECT_SOURCE_DIR})

if(Python3_NumPy_FOUND)
    include_directories(${Python3_NumPy_INCLUDE_DIRS})
    set(NUMPY_LIB "${Python3_NumPy_INCLUDE_DIRS}/../lib/libnpymath.a")
    target_link_libraries(tests ${NUMPY_LIB})
endif()
