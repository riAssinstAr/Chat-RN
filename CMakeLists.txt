cmake_minimum_required(VERSION 3.10)
project(tcp_chat_app C)

set(CMAKE_C_STANDARD 99)

# Build the server
add_executable(server server.c)

# Build the client
add_executable(client client.c)
