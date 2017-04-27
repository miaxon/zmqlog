

cmake_minimum_required(VERSION 2.8.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -fPIC")
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# Get source files list
file(GLOB SRC_LIST *.cpp *.c)

file(GLOB H_LIST *.h)


add_library(ini SHARED ${SRC_LIST} ${H_LIST})
target_link_libraries(ini)


