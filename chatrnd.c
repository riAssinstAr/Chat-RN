#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "chatrn.h"

int main()
{
    if (daemon(0, 0) < 0)
    {
        perror("chatrnd: failed to daemonize");
        exit(EXIT_FAILURE);
    }

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_listen, NULL);

    pthread_exit(NULL);
}
