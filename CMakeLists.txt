cmake_minimum_required(VERSION 3.23)
project(hello)

set(SOURCE_FILES hello.c)

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

add_executable(hello ${SOURCE_FILES})

target_include_directories(hello PUBLIC ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(hello PUBLIC ${ZeroMQ_LIBRARY})
