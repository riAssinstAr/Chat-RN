#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

extern void *server_listen(void *);
extern void connect_to_request(const char *username);
extern void initiate_connection(const char *username);
extern void pending_list(void);

int main(int argc, char *argv[])
{
    if (daemon(0, 0) < 0)
    {
        perror("daemon");
        exit(EXIT_FAILURE);
    }
    printf("[ChatRN] Chat server started in background.\n");

    if (argc == 3 && strcmp(argv[1], "connect") == 0)
    {
        initiate_connection(argv[2]);
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "accept") == 0)
    {
        connect_to_request(argv[2]);
        return 0;
    }

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_listen, NULL);

    // Enter command loop
    char input[64];
    while (1)
    {
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == 0)
        {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strncmp(input, "accept ", 7) == 0)
        {
            connect_to_request(input + 7);
            return 0;
        }
        else if (strncmp(input, "connect ", 8) == 0)
        {
            initiate_connection(input + 8);
            return 0;
        }
        else if (strcmp(input, "help") == 0)
        {
            printf("\nAvailable commands:\n");
            printf("accept <username> - Accept a connection request from <username>\n");
            printf("connect <username> - Connect to <username>\n");
            printf("list - List pending connections\n\n");
            continue;
        }
        else if (strcmp(input, "list") == 0)
        {
            printf("\nPending connections:\n");
            pending_list();
            continue;
        }
        else if (strlen(input) > 0)
        {
            printf("\n--Unknown command.\n");
            printf("--Type 'help' for available commands.\n\n");
            continue;
        }
    }
    printf("[ChatRN] Server running in background. Type Ctrl+C to exit.\n");
    pthread_exit(NULL);
}
