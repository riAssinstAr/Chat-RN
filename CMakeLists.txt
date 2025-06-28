cmake_minimum_required(VERSION 3.10)
project(chatrn C)

set(CMAKE_C_STANDARD 99)

add_executable(chatrn
    main.c
    config.c
    server.c
    client.c
    pending.c
    chatsession.c
)

# Add pthread if you're using threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(chatrn PRIVATE Threads::Threads)
