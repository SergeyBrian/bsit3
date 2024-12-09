cmake_minimum_required(VERSION 3.29)
project(bsit_3)

set(CMAKE_CXX_STANDARD 20)

set(COMMON_SRC
        src/common/data.cpp
        src/common/data.hpp
        src/common/alias.hpp
        src/common/alias.cpp
        src/common/utils.cpp
        src/common/utils.hpp
)

add_executable(server src/server/main.cpp
        src/server/os_utils.cpp
        src/server/os_utils.hpp
        ${COMMON_SRC})
add_executable(client src/client/main.cpp
        ${COMMON_SRC})

target_link_libraries(server)
