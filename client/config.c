#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#define CONFIG_FILE "../.config"
#define MAX_LINE 128

int get_user_address(const char *username, char *ip_out, int *port_out)
{
    if (!username || !ip_out || !port_out)
    {
        fprintf(stderr, "[Config.c] Invalid argument to get_user_address()!\n");
        return -1;
    }

    printf("[Config.c] Retrieving address for user %s.\n", username);
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file)
        return -1;

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file))
    {
        char user[64], ip[64];
        int port;
        if (sscanf(line, "%s %s %d", user, ip, &port) == 3)
        {
            if (strcmp(user, username) == 0)
            {
                strcpy(ip_out, ip);
                *port_out = port;
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return -1;
}
