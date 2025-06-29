#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "client.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        perror("Usage: chatrn <connect|accept> <user>\n");
        return 1;
    }

    const char *cmd = argv[1];
    const char *user = argv[2];

    if (strcmp(cmd, "connect") == 0)
    {
        initiate_connection(user);
    }
    else if (strcmp(cmd, "accept") == 0)
    {
        accept_session(user);
    }
    else if (strcmp(cmd, "pending") == 0)
    {
        pending_list();
    }
    else if (strcmp(cmd, "help") == 0)
    {
        printf("Usage: chatrn <connect|accept|pending|help> <user>\n");
        printf("Commands:\n");
        printf("  connect <user>   - Connect to a user\n");
        printf("  accept <user>    - Accept a pending connection request\n");
        printf("  pending          - List pending requests\n");
        printf("  help             - Show this help message\n");
    }
    else
    {
        perror("Unknown command!\n");
        return 1;
    }

    return 0;
}
