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

    if (strcmp(cmd, "-c") == 0)
    {
        initiate_connection(user);
    }
    else if (strcmp(cmd, "-a") == 0)
    {
        accept_session(user);
    }
    else if (strcmp(cmd, "-d") == 0)
    {
        decline_session(user);
    }
    else if (strcmp(cmd, "-l") == 0)
    {
        pending_list();
    }
    else if (strcmp(cmd, "help") == 0)
    {
        printf("Usage: chatrn <connect|accept|pending|help> <user>\n");
        printf("Commands:\n");
        printf("  -c <user>   - Connect to a user\n");
        printf("  -a <user>    - Accept a pending connection request\n");
        printf("  -l          - List pending requests\n");
        printf("  help             - Show this help message\n");
    }
    else
    {
        perror("Unknown Command!\n");
        return 1;
    }

    return 0;
}
