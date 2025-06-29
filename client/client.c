#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include "client.h"
#include "config.h"
#include "pending.h"
#include "session.h"
#include "../include/common.h"

static int recv_fd(int socket)
{
    struct msghdr msg = {0};

    char m_buffer[256];
    struct iovec io = {
        .iov_base = m_buffer,
        .iov_len = sizeof(m_buffer)};

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    char c_buffer[CMSG_SPACE(sizeof(int))];
    msg.msg_control = c_buffer;
    msg.msg_controllen = sizeof(c_buffer);

    if (recvmsg(socket, &msg, 0) < 0)
    {
        perror("[Client.c] recvmsg failed!");
        return -1;
    }

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg == NULL || cmsg->cmsg_type != SCM_RIGHTS)
    {
        fprintf(stderr, "[Client.c] Did not receive file descriptor!\n");
        return -1;
    }

    int fd;
    memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));
    return fd;
}

void initiate_connection(const char *user)
{
    char ip[64];
    int port;

    if (get_user_address(user, ip, &port) < 0)
    {
        fprintf(stderr, "[Client.c] Unknown user %s!\n", user);
        fprintf(stderr, "[Client.c] Add users in the config file.\n");
        return;
    }

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        perror("[Client.c] Socket creation failed!");
        return;
    }

    struct sockaddr_in a = {
        .sin_family = AF_INET,
        .sin_port = htons(port)};

    if (inet_pton(AF_INET, ip, &a.sin_addr) <= 0)
    {
        perror("[Client.c] Invalid IP address!");
        close(s);
        return;
    }
    if (connect(s, (struct sockaddr *)&a, sizeof(a)) < 0)
    {
        perror("[Client.c] Socket connection failed!");
        close(s);
        return;
    }

    const char *username = getlogin();
    if (!username)
        username = "userx";
    if (send(s, username, strlen(username) + 1, 0) < 0)
    {
        perror("[Client.c] Failed to send your username!");
        close(s);
        return;
    }

    printf("[Client.c] Connected to %s. Starting chat session...\n", user);
    start_chat_session(s, user);
}

void accept_session(const char *user)
{
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    strncpy(addr.sun_path, "/tmp/chatrn.sock", sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("[Client.c] Socket connection failed!");
        exit(1);
    }

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "accept %s", user);
    send(sock, cmd, strlen(cmd), 0);

    int chat_sock = recv_fd(sock);
    if (chat_sock < 0)
    {
        fprintf(stderr, "[Client.c] Failed to receive chat socket!\n");
        close(sock);
        return;
    }

    close(sock);
    start_chat_session(chat_sock, user);
}
