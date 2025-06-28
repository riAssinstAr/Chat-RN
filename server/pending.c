#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_PENDING 20

typedef struct
{
    char username[64];
    int sock;
} PendingConn;

static PendingConn pending[MAX_PENDING];
static int pending_count = 0;

void pending_add(const char *username, int sock)
{
    for (int i = 0; i < pending_count; ++i)
    {
        if (strcmp(pending[i].username, username) == 0)
        {
            printf("[Pending] Already pending username '%s' ignored.\n", username);
            return;
        }
    }

    if (pending_count < MAX_PENDING)
    {
        strncpy(pending[pending_count].username, username, sizeof(pending[pending_count].username));
        pending[pending_count].sock = sock;
        pending_count++;

        printf("[Pending] Added pending user: %s (Socket=%d)\n", username, sock);
    }
    else
    {
        printf("[Pending] Too many pending connections\n");
    }
}

int pending_get(const char *username)
{
    printf("\n[Pending] Looking for pending user: %s\n", username);

    for (int i = 0; i < pending_count; ++i)
    {
        if (strcmp(pending[i].username, username) == 0)
        {
            int sock = pending[i].sock;

            for (int j = i; j < pending_count - 1; ++j)
            {
                pending[j] = pending[j + 1];
            }
            pending_count--;
            return sock;
        }
    }
    return -1;
}

void pending_list()
{
    if (pending_count == 0)
    {
        printf("No users are waiting.\n");
        return;
    }

    printf("%d user(s) waiting to chat:\n", pending_count);
    for (int i = 0; i < pending_count; ++i)
    {
        printf("  • %s\n", pending[i].username);
    }
    printf("\n");
    return;
}
