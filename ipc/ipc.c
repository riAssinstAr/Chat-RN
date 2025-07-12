#include <errno.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include "ipc.h"
#include "pending.h"

#define SOCKET_PATH "/tmp/chatrn.sock"

static int server_sock;

void send_fd(int socket, int fd_to_send)
{
    struct msghdr msg = {0};
    char buf[CMSG_SPACE(sizeof(int))];
    memset(buf, 0, sizeof(buf));

    struct iovec io = {.iov_base = "FD", .iov_len = 2};
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));

    memcpy(CMSG_DATA(cmsg), &fd_to_send, sizeof(int));

    if (sendmsg(socket, &msg, 0) < 0)
        perror("[IPC] Socket sendmsg for FD failed!");
}

void ipc_init(void)
{
    unlink(SOCKET_PATH);

    server_sock = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (server_sock < 0)
    {
        perror("[IPC] Socket creation failed!");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("[IPC] Socket bind failed!");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    if (listen(server_sock, 5) < 0)
    {
        perror("[IPC] Socket listen failed!");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("[IPC] UNIX socket created.\n");
}

void *ipc_command_loop_thread(void *_)
{
    while (1)
    {
        int client = accept(server_sock, NULL, NULL);
        if (client < 0)
        {
            perror("[IPC] Socket accept failed!");
            continue;
        }

        char buf[256];
        buf[sizeof(buf) - 1] = '\0';
        ssize_t len = recv(client, buf, sizeof(buf) - 1, 0);
        if (len <= 0)
        {
            close(client);
            continue;
        }
        buf[len] = '\0';
        printf("[IPC] Received: %s request!\n", buf);

        if (strncmp(buf, "-a ", 3) == 0 || strncmp(buf, "-d ", 3) == 0)
        {
            const char *user = buf + 3;
            int sock = pending_get(user);
            if (sock < 0)
            {
                const char *msg = "[IPC] No such pending request!\n";
                send(client, msg, strlen(msg), 0);
                close(client);
                continue;
            }
            send_fd(client, sock);
        }
        else
        {
            const char *msg = "[IPC] Unsupported command!\n";
            send(client, msg, strlen(msg), 0);
        }
        close(client);
    }
    return NULL;
}

void ipc_command_loop(void)
{
    pthread_t t;
    pthread_create(&t, NULL, ipc_command_loop_thread, NULL);
    pthread_detach(t);
}

void ipc_cleanup(void)
{
    close(server_sock);
    unlink(SOCKET_PATH);
}
