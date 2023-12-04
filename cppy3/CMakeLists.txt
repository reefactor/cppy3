add_library(cppy3 cppy3.cpp utils.cpp)
target_link_libraries(cppy3 ${PYTHON_LIBRARY})

if(NUMPY_FOUND)
    include_directories(${NUMPY_INCLUDE_DIR})
    target_sources(cppy3 PRIVATE "${CMAKE_CURRENT_LIST_DIR}/cppy3_numpy.cpp")
    set(NUMPY_LIB "${NUMPY_INCLUDE_DIR}/../lib/libnpymath.a")
    target_link_libraries(cppy3 ${NUMPY_LIB})
endif()

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(cppy3 ${Boost_LIBRARIES})
endif()
