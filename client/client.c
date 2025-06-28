#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "config.h"
#include "pending.h"
#include "chatrn.h"

void connect_to_request(const char *username)
{
    int sock = pending_get(username);

    if (sock != -1)
    {
        printf("\n[Client] Accepting chat with %s.\n", username);
        start_chat_session(sock, username);
        close(sock);
        return;
    }
}

void initiate_connection(const char *username)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    char ip[64];
    int port;

    if (get_user_address(username, ip, &port) != 0)
    {
        printf("\n[Client] User '%s' not found in config.\n", username);
        return;
    }

    if (sock < 0)
    {
        perror("[Client] Socket creation failed.");
        return;
    }

    struct sockaddr_in target = {
        .sin_family = AF_INET,
        .sin_port = htons(port)};
    if (inet_pton(AF_INET, ip, &target.sin_addr) <= 0)
    {
        perror("[Client] Invalid IP address.");
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr *)&target, sizeof(target)) < 0)
    {
        perror("[Client] Socket connection failed.");
        close(sock);
        return;
    }

    if (send(sock, getlogin(), strlen(getlogin()) + 1, 0) < 0)
    {
        perror("[Client] Send name failed.");
        close(sock);
        return;
    }

    start_chat_session(sock, username);
    close(sock);
    return;
}
