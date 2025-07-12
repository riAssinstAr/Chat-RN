#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "pending.h"
#include "../include/common.h"

typedef struct
{
    char username[64];
    int sock;
} PendingConn;

static PendingConn pending[MAX_PENDING];
static int pending_count = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void pending_add(const char *username, int sock)
{
    if (!username || username[0] == '\0')
    {
        printf("[Pending.c] Rejecting empty username!\n");
        return;
    }

    pthread_mutex_lock(&lock);
    for (int i = 0; i < pending_count; i++)
    {
        if (strcmp(pending[i].username, username) == 0)
        {
            pending[i].sock = sock;
            pthread_mutex_unlock(&lock);
            printf("[Pending.c] Updated existing pending request for %s.\n", username);
            return;
        }
    }

    if (pending_count < MAX_PENDING)
    {
        snprintf(pending[pending_count].username, sizeof(pending[pending_count].username), "%s", username);
        pending[pending_count].sock = sock;
        pending_count++;
        printf("[Pending.c] Added new pending request for %s.\n", username);
    }
    else
    {
        printf("[Pending.c] Pending queue full. Could not add %s!\n", username);
    }
    pthread_mutex_unlock(&lock);
}

int pending_get(const char *username)
{
    pthread_mutex_lock(&lock);
    for (int i = 0; i < pending_count; i++)
    {
        if (strcmp(pending[i].username, username) == 0)
        {
            int sock = pending[i].sock;
            pending[i] = pending[--pending_count];
            pthread_mutex_unlock(&lock);
            printf("[Pending.c] Found and removed request for %s.\n", username);
            return sock;
        }
    }
    pthread_mutex_unlock(&lock);
    return -1;
}

void pending_list(void)
{
    pthread_mutex_lock(&lock);
    if (pending_count == 0)
    {
        printf("[Pending.c] No pending requests!\n");
    }
    else
    {
        printf("[Pending.c] %d pending request%s:\n", pending_count, pending_count > 1 ? "s" : "");
        for (int i = 0; i < pending_count; i++)
        {
            printf("  • %s\n", pending[i].username);
        }
    }
    pthread_mutex_unlock(&lock);
}
