cmake_minimum_required(VERSION 3.28)
project(Bookstore_2025 CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(Bookstore_2025
        src/main.cpp
        include/MemoryRiver.h
        include/application.h
        include/command.h
        include/session.h
        include/user.h
        include/book.h
        include/finance.h
        include/log.h
        src/user.cpp
        src/session.cpp
        src/command.cpp
        src/book.cpp
        src/application.cpp
        src/finance.cpp
        src/log.cpp)

set_target_properties(Bookstore_2025 PROPERTIES
        OUTPUT_NAME "code"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
