#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUF_SIZE 1024

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    listen(server_fd, 1);
    printf("Server listening on port %d...\n", PORT);

    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0)
    {
        perror("accept");
        exit(1);
    }
    printf("Client connected.\n");

    while (1)
    {
        printf("You: ");
        fgets(buffer, BUF_SIZE, stdin);
        send(client_fd, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUF_SIZE);
        int bytes = recv(client_fd, buffer, BUF_SIZE, 0);
        if (bytes <= 0)
            break;

        printf("Client: %s", buffer);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
