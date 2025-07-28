#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "ipc.h"
#include "server.h"
#include "common.h"

void handle_signal(int sig)
{
    ipc_cleanup();
    printf("\n[ChatRND] Caught signal. Exiting.\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    printf("[ChatRND] Starting daemon...\n");

    // if (daemon(0, 0) < 0)
    // {
    //     perror("[Daemon] Failed to daemonize.");
    //     exit(EXIT_FAILURE);
    // }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    printf("[ChatRND] Daemon started. Listening on port %d...\n", SERVER_PORT);

    ipc_init();
    ipc_command_loop();

    pthread_t server_thread;
    if (pthread_create(&server_thread, NULL, server_listen, NULL) != 0)
    {
        perror("[Daemon] Failed to start server thread!");
        exit(EXIT_FAILURE);
    }
    pthread_detach(server_thread);

    while (1)
        pause();

    return 0;
}
