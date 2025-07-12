#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <string.h>
#include <arpa/inet.h>
#include "client.h"
#include "config.h"
#include "pending.h"
#include "session.h"

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

    int chat_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in a = {
        .sin_family = AF_INET,
        .sin_port = htons(port)};
    if (inet_pton(AF_INET, ip, &a.sin_addr) <= 0)
    {
        perror("[Client.c] Socket inet_pton failed!");
        close(chat_sock);
        return;
    }
    if (connect(chat_sock, (struct sockaddr *)&a, sizeof(a)) < 0)
    {
        perror("[Client.c] Socket connect failed!");
        close(chat_sock);
        return;
    }

    const char *username = getenv("USER");
    if (!username)
    {
        username = "Peer";
        printf("[Client.c] getlogin() failed, sendning username 'Peer'!\n");
    }
    send(chat_sock, username, strlen(username) + 1, 0);
    printf("[Client.c] Waiting for confirmation from peer...\n");

    char buf[8];
    ssize_t n = recv(chat_sock, buf, sizeof(buf) - 1, 0);
    if (n <= 0)
    {
        perror("[Client.c] Socket recv failed!");
        close(chat_sock);
        return;
    }
    buf[n] = '\0';

    // Check conformation from peer
    if (strcmp(buf, "ACCEPT\n") == 0)
    {
        printf("[Client.c] Peer accepted the connection.\n");
        printf("[Client.c] Starting chat session with %s...\n", user);

        start_chat_session(chat_sock, user);
    }
    else if (strcmp(buf, "DECLINE\n") == 0)
    {
        printf("[Client.c] Peer declined the connection!\n");
        return;
    }
    close(chat_sock);
}

void accept_session(const char *user)
{
    int unix_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    strncpy(addr.sun_path, "/tmp/chatrn.sock", sizeof(addr.sun_path) - 1);
    connect(unix_sock, (struct sockaddr *)&addr, sizeof(addr));

    char cmnd[128];
    snprintf(cmnd, sizeof(cmnd), "-a %s", user);
    send(unix_sock, cmnd, strlen(cmnd), 0);
    int chat_sock = recv_fd(unix_sock);
    close(unix_sock);

    // Send confirmation to the peer
    const char *confirmation = "ACCEPT\n";
    if (send(chat_sock, confirmation, strlen(confirmation), 0) < 0)
    {
        perror("[Client.c] Socket send failed!");
        close(chat_sock);
        return;
    }

    printf("[Client.c] Accepted session request from %s.\n", user);
    printf("[Client.c] Starting chat session with %s...\n", user);

    start_chat_session(chat_sock, user);
    close(chat_sock);
}

void decline_session(const char *user)
{
    int unix_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    strncpy(addr.sun_path, "/tmp/chatrn.sock", sizeof(addr.sun_path) - 1);
    connect(unix_sock, (struct sockaddr *)&addr, sizeof(addr));

    char cmnd[128];
    snprintf(cmnd, sizeof(cmnd), "-d %s", user);
    send(unix_sock, cmnd, strlen(cmnd), 0);
    int chat_sock = recv_fd(unix_sock);
    close(unix_sock);

    // Send confirmation to the peer
    const char *confirmation = "DECLINE\n";
    if (send(chat_sock, confirmation, strlen(confirmation), 0) < 0)
    {
        perror("[Client.c] Socket send failed!");
        close(chat_sock);
        return;
    }

    close(chat_sock);
    printf("[Client.c] Declined session request from %s.\n", user);
}
