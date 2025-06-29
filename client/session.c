#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/socket.h>
#include "client.h"

typedef struct
{
    const char *username;
    int sock;
} ChatArgs;

void *receive_loop(void *arg)
{
    ChatArgs *c = arg;
    char buf[1024];
    while (1)
    {
        int n = recv(c->sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0)
        {
            if (n < 0)
                perror("[Session.c] recv failed!");
            else
            {
                printf("\033[F\033[2K\033[1;31m[Connection closed by peer.]\033[0m\n");
                return NULL;
            }
            break;
        }

        buf[n] = '\0';
        // Clear current input line
        printf("\r\033[K");

        // Print the incoming message
        printf("\033[1;34m[%s]\033[0m: %s\n", c->username, buf);

        // Redraw the input prompt
        printf("\033[1;32m[You]\033[0m: ");
        fflush(stdout);
    }
    return NULL;
}

void *send_loop(void *arg)
{
    int sock = *(int *)arg;
    free(arg);

    char buf[1024];
    while (1)
    {
        printf("\033[1;32m[You]\033[0m: ");
        fflush(stdout);

        if (!fgets(buf, sizeof(buf), stdin))
            break;

        buf[strcspn(buf, "\n")] = '\0';

        if (strcmp(buf, "quit") == 0)
        {
            shutdown(sock, SHUT_RDWR);
            break;
        }

        if (buf[0] != '\0')
            send(sock, buf, strlen(buf), 0);
    }
    return NULL;
}

void start_chat_session(int sock, const char *username)
{
    printf("[Session.c] Chat session with %s.\n", username);

    ChatArgs *c = malloc(sizeof(ChatArgs));
    c->username = username;
    c->sock = sock;

    int *psock = malloc(sizeof(int));
    *psock = sock;

    pthread_t r, s;
    pthread_create(&r, NULL, receive_loop, c);
    pthread_create(&s, NULL, send_loop, psock);

    pthread_join(s, NULL);
    close(c->sock);
    free(c);
    return;
}
