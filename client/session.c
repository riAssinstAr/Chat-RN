#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "chatrn.h"

int chat_sock;
typedef struct
{
    const char *username;
    int sock;
} ChatArgs;

void *receive_loop(void *arg)
{
    ChatArgs *chat = (ChatArgs *)arg;
    char buffer[1024];

    while (1)
    {
        int n = recv(chat->sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0)
        {
            if (n == 0)
            {
                printf("\r\n[ChatRN] Connection closed by peer.\n");
                shutdown(chat_sock, SHUT_RDWR);
                return NULL;
            }
            else
            {
                perror("\r\n[ChatRN] Error in receiving the message!");
            }
            break;
        }

        buffer[n] = '\0';

        printf("\r\033[1;34m[%s]\033[0m: %s: ", chat->username, buffer);
        printf("\r\033[1;32m[You]\033[0m: ");

        fflush(stdout);
    }
    return NULL;
}

void *send_loop(void *arg)
{
    char buffer[1024];
    while (1)
    {
        printf("\r\033[1;32m[You]\033[0m: ");
        fflush(stdout);
        char *input = fgets(buffer, sizeof(buffer), stdin);

        if (strcmp(input, "\r\033[1;32m[You]\033[0m: /quit") == 0 || strcmp(input, "\r\033[1;32m[You]\033[0m: /q") == 0)
        {
            printf("\n[ChatRN] Exiting chat session.\n");
            shutdown(chat_sock, SHUT_RDWR);
            return NULL;
        }

        if (send(chat_sock, buffer, strlen(buffer), 0) < 0)
        {
            perror("\r\n[ChatRN] Error in sending the message!");
            break;
        }
    }
    return NULL;
}

void start_chat_session(int sock, const char *username)
{
    printf("\r\n[ChatRN] Starting chat session with %s...\n", username);
    chat_sock = sock;

    ChatArgs *chat_args = malloc(sizeof(ChatArgs));
    chat_args->username = username;
    chat_args->sock = sock;

    pthread_t send_thread, recv_thread;
    pthread_create(&recv_thread, NULL, receive_loop, chat_args);
    pthread_create(&send_thread, NULL, send_loop, NULL);

    pthread_join(send_thread, NULL);
    shutdown(chat_sock, SHUT_RDWR);
    close(chat_sock);
    free(chat_args);
}
