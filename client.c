#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUF_SIZE 1024

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost for now

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    printf("Connected to server.\n");

    while (1)
    {
        memset(buffer, 0, BUF_SIZE);
        int bytes = recv(sockfd, buffer, BUF_SIZE, 0);
        if (bytes <= 0)
            break;

        printf("Server: %s", buffer);

        printf("You: ");
        fgets(buffer, BUF_SIZE, stdin);
        send(sockfd, buffer, strlen(buffer), 0);
    }

    close(sockfd);
    return 0;
}
