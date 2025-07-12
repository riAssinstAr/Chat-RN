cmake_minimum_required(VERSION 3.10)
project(chatrn C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# === Include Directories ===
include_directories(
    include
    client
    server
    ipc
)

# === Source Files ===
file(GLOB CLIENT_SRC client/*.c)
file(GLOB SERVER_SRC server/*.c)
file(GLOB COMMON_SRC common/*.c)
file(GLOB IPC_SRC ipc/*.c)

# === Client CLI Binary ===
add_executable(chatrn
    chatrn.c
    ${SERVER_SRC}
    ${INCLUDE_SRC}
    ${CLIENT_SRC}
    ${IPC_SRC}
)

# === Server Daemon Binary ===
add_executable(chatrnd
    chatrnd.c
    ${SERVER_SRC}
    ${INCLUDE_SRC}
    ${CLIENT_SRC}
    ${IPC_SRC}
)

# === Threading Support ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(chatrn PRIVATE Threads::Threads)
target_link_libraries(chatrnd PRIVATE Threads::Threads)