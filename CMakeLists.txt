cmake_minimum_required(VERSION 3.23)
project(hello)


find_package(PkgConfig)
pkg_check_modules(PC_ZeroMQ QUIET zmq)

find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.h
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

add_executable(server server.c)
add_executable(client client.c)

target_include_directories(server PUBLIC ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(server PUBLIC ${ZeroMQ_LIBRARY})
target_include_directories(client PUBLIC ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(client PUBLIC ${ZeroMQ_LIBRARY})
