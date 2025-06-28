#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "pending.h"
#include "chatrn.h"

#define SERVER_PORT 9000

void *handle_incoming(void *arg)
{
    int client_sock = *(int *)arg;
    free(arg);

    char peer_name[64] = {0};
    ssize_t n = recv(client_sock, peer_name, sizeof(peer_name) - 1, 0);

    if (n <= 0)
    {
        if (n == 0)
            printf("[Server] Client disconnected before sending username.\n");
        else
            perror("[Server] recv failed");
        close(client_sock);
        return NULL;
    }

    peer_name[n] = '\0';

    if (strlen(peer_name) == 0)
    {
        printf("[Server] Received empty username. Rejecting.\n");
        close(client_sock);
        return NULL;
    }

    printf("[Server Incoming] Connection request from %s. Accept with: chatrn accept %s\n", peer_name, peer_name);
    pending_add(peer_name, client_sock);

    return NULL;
}

void *server_listen(void *_)
{
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;

    if (server_sock < 0)
    {
        perror("[Server] Socket creation failed");
        close(server_sock);
        exit(1);
    }

    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("[Server] Socket option (SO_REUSEADDR) failed");
        close(server_sock);
        exit(1);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr = INADDR_ANY};

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[Server] Bind failed");
        close(server_sock);
        exit(1);
    }

    if (listen(server_sock, 5) < 0)
    {
        perror("[Server] Listen failed");
        close(server_sock);
        exit(1);
    }

    printf("[Server] Listening on port %d...\n", SERVER_PORT);

    while (1)
    {
        int client_sock = accept(server_sock, NULL, NULL);
        int *pclient = malloc(sizeof(int));
        if (!pclient)
        {
            perror("[Server] Socket malloc failed");
            close(client_sock);
            continue;
        }
        *pclient = client_sock;

        pthread_t tid;
        pthread_create(&tid, NULL, handle_incoming, pclient);
        pthread_detach(tid);
    }

    return NULL;
}
