cmake_minimum_required(VERSION 3.27)
project(bsit_3)

set(CMAKE_CXX_STANDARD 20)

set(COMMON_SRC
        src/common/data.cpp
        src/common/data.hpp
        src/common/alias.hpp
        src/common/alias.cpp
        src/common/logging.hpp
        src/common/errors.hpp
        src/common/proto/request.cpp
        src/common/proto/request.hpp
        src/common/proto/message.cpp
        src/common/proto/message.hpp
        src/common/proto/response.cpp
        src/common/proto/response.hpp
        src/common/proto/packable.cpp
        src/common/proto/packable.hpp
        src/common/proto/proto.cpp
        src/common/proto/proto.hpp
        src/common/tcp_utils.cpp
        src/common/tcp_utils.hpp
        src/common/proto/encryption/encryption.cpp
        src/common/proto/encryption/encryption.hpp
        src/common/utils.cpp
        src/common/utils.hpp
)

add_executable(server src/server/main.cpp
        src/server/os_utils.cpp
        src/server/os_utils.hpp
        ${COMMON_SRC}
        src/server/server/handlers.cpp
        src/server/server/handlers.hpp
        src/server/server/tcp.cpp
        src/server/server/tcp.hpp)
add_executable(client src/client/main.cpp
        src/client/cli/cli.cpp
        src/client/cli/cli.hpp
        src/client/connector/connector.cpp
        src/client/connector/connector.hpp
        ${COMMON_SRC}
        src/common/utils.cpp
        src/common/utils.hpp
        src/common/str_utils.hpp
        src/common/str_utils.cpp
        src/client/connector/context.cpp
        src/client/connector/context.hpp)

target_link_libraries(server)
