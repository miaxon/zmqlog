
cmake_minimum_required(VERSION 2.8.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
message(STATUS "Generating build using CMake v${CMAKE_VERSION}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

include_directories( 
    "${CMAKE_SOURCE_DIR}/lib" ,
    "${CMAKE_SOURCE_DIR}/ini"
)
# Get source files list
file(GLOB SRC_LIST *.cpp)
file(GLOB H_LIST *.h)


add_executable(zmqlog ${SRC_LIST} ${H_LIST})
target_link_libraries(zmqlog zmq zmqpp zlog fmt)


