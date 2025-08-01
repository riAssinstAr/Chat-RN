#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "client.h"
#include "pending.h"

int main(int argc, char *argv[])
{
    const char *cmd = argv[1];
    const char *user = argv[2];

    if (strcmp(cmd, "-c") == 0 || strcmp(cmd, "connect") == 0)
    {
        initiate_connection(user);
    }
    else if (strcmp(cmd, "-a") == 0 || strcmp(cmd, "accept") == 0)
    {
        accept_session(user);
    }
    else if (strcmp(cmd, "-d") == 0 || strcmp(cmd, "decline") == 0)
    {
        decline_session(user);
    }
    else if (strcmp(cmd, "-l") == 0 || strcmp(cmd, "list") == 0)
    {
        pending_list();
    }
    else if (strcmp(cmd, "-h") == 0 || strcmp(cmd, "help") == 0)
    {
        printf("Usage: chatrn <connect|accept|pending|help> <user>\n");
        printf("Commands:\n");
        printf("  connect <user>    - Connect to a user\n");
        printf("  accept <user>     - Accept a pending connection request\n");
        printf("  list              - List pending requests\n");
        printf("  help              - Show this help message\n");
    }
    else
    {
        perror("Unknown Command!\n");
        return 0;
    }

    return 0;
}
