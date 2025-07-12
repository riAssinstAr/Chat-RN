#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <termios.h>
#include <sys/socket.h>
#include "client.h"

#define MAX_INPUT 1024

typedef struct
{
    const char *username;
    int sock;
    char *input_buffer;
    pthread_mutex_t *input_lock;
    volatile sig_atomic_t *active;
} ChatArgs;

static struct termios orig_termios;

void enable_raw_mode()
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void redraw_prompt(const char *input)
{
    printf("\r\033[K\033[1;32m[You]\033[0m: %s", input);
    fflush(stdout);
}

void *receive_loop(void *arg)
{
    ChatArgs *c = arg;
    char buf[1024];

    while (*c->active)
    {
        int n = recv(c->sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0)
        {
            if (n < 0)
                perror("[Session.c] recv failed!");
            else
                printf("\r\033[K\033[1;31m[Connection closed by peer!]\033[0m\n");

            *c->active = 0;
            shutdown(c->sock, SHUT_RDWR);
            break;
        }
        buf[n] = '\0';

        pthread_mutex_lock(c->input_lock);
        printf("\r\033[K\033[1;34m[%s]\033[0m: %s\n", c->username, buf);

        if (*c->active)
            redraw_prompt(c->input_buffer);

        pthread_mutex_unlock(c->input_lock);
    }
    return NULL;
}

void *send_loop(void *arg)
{
    ChatArgs *c = arg;
    int sock = c->sock;
    char *input = c->input_buffer;
    pthread_mutex_t *lock = c->input_lock;
    volatile sig_atomic_t *active = c->active;

    int pos = 0;
    enable_raw_mode();
    redraw_prompt(input);

    while (*active)
    {
        char ch;
        if (read(STDIN_FILENO, &ch, 1) <= 0)
            break;

        pthread_mutex_lock(lock);

        if (!*active)
        {
            pthread_mutex_unlock(lock);
            break;
        }

        if (ch == 127 || ch == '\b')
        {
            if (pos > 0)
            {
                pos--;
                input[pos] = '\0';
            }
        }
        else if (ch == '\n')
        {
            input[pos] = '\0';
            printf("\n");

            if (strcmp(input, "/quit") == 0)
            {
                *active = 0;
                shutdown(sock, SHUT_RDWR);
                pthread_mutex_unlock(lock);
                break;
            }

            if (input[0] != '\0')
            {
                send(sock, input, strlen(input), 0);
            }
            pos = 0;
            input[0] = '\0';
        }
        else if (pos < MAX_INPUT - 1)
        {
            input[pos++] = ch;
            input[pos] = '\0';
        }

        if (*active)
            redraw_prompt(input);
        pthread_mutex_unlock(lock);
    }
    disable_raw_mode();
    return NULL;
}

void start_chat_session(int sock, const char *username)
{
    printf("[Session.c] Chat session with %s.\n\n", username);

    char *input_buffer = calloc(MAX_INPUT, 1);
    pthread_mutex_t *lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(lock, NULL);

    volatile sig_atomic_t *active = malloc(sizeof(sig_atomic_t));
    *active = 1;

    ChatArgs *c = malloc(sizeof(ChatArgs));
    c->username = username;
    c->sock = sock;
    c->input_buffer = input_buffer;
    c->input_lock = lock;
    c->active = active;

    pthread_t r, s;
    pthread_create(&r, NULL, receive_loop, c);
    pthread_create(&s, NULL, send_loop, c);

    pthread_join(r, NULL);
    pthread_join(s, NULL);

    close(sock);
    pthread_mutex_destroy(lock);

    free(lock);
    free(input_buffer);
    free((void *)active);
    free(c);
}
