#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "server.h"
#include "pending.h"
#include "common.h"

void *handle_incoming(void *arg)
{
    int client_sock = *(int *)arg;
    free(arg);

    char peer_name[64] = {0};
    ssize_t n = recv(client_sock, peer_name, sizeof(peer_name) - 1, 0);
    if (n <= 0)
    {
        perror("[Server.c] Socket recv failed!");
        close(client_sock);
        return NULL;
    }

    peer_name[n] = 0;
    printf("[Server.c] Incoming connection request from %s.\n", peer_name);
    pending_add(peer_name, client_sock);
    return NULL;
}

void *server_listen(void *_)
{
    int opt = 1;
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        perror("[Server.c] Socket creation failed!");
        exit(1);
    }

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr = INADDR_ANY};

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("[Server.c] Bind failed!");
        exit(1);
    }
    if (listen(s, 5) != 0)
    {
        perror("[Server.c] Listen failed!");
        close(s);
        exit(1);
    }
    printf("[Server.c] Listening on port %d...\n", SERVER_PORT);

    while (1)
    {
        int cs = accept(s, NULL, NULL);
        int *p = malloc(sizeof(int));
        *p = cs;
        pthread_t tid;
        pthread_create(&tid, NULL, handle_incoming, p);
        pthread_detach(tid);
    }
    return NULL;
}
