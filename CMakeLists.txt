cmake_minimum_required(VERSION 3.22)
project(hello)

set(CMAKE_CXX_FLAGS "-O1 -Wall -Wextra -g")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -g")
set(CMAKE_C_FLAGS "-O1 -Wall -Wextra -g")
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -Wextra -g")


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

set(PREFIX "weather-")
add_executable("${PREFIX}server" "${PREFIX}server.c")
add_executable("${PREFIX}client" "${PREFIX}client.c")
target_include_directories("${PREFIX}server" PUBLIC ${ZeroMQ_INCLUDE_DIR})
target_link_libraries     ("${PREFIX}server" PUBLIC ${ZeroMQ_LIBRARY})
target_include_directories("${PREFIX}client" PUBLIC ${ZeroMQ_INCLUDE_DIR})
target_link_libraries     ("${PREFIX}client" PUBLIC ${ZeroMQ_LIBRARY})

add_executable(hash hash.c hash_test.c)
