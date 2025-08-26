cmake_minimum_required(VERSION 3.10)

project(SentinelDNS)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/dns_parser.cpp
    src/logger.cpp
)

add_executable(SentinelDNS ${SOURCES})