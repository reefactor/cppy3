add_library(cppy3 cppy3.cpp utils.cpp)
#add_library(cppy3 cppy3.cpp cppy3numpy)
set(NUMPY_LIB "${NUMPY_INCLUDE_DIR}/../lib/libnpymath.a")

target_link_libraries(cppy3 ${PYTHON_LIBRARY} ${NUMPY_LIB})

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(cppy3 ${Boost_LIBRARIES})
endif()
