#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_PORT 5000
#define MAX_CLIENTS 10 // Adjust the maximum number of clients as needed
#define BUF_SIZE 4096

void handle_client(int client, int clients[], int *client_sz)
{
    char buff[BUF_SIZE];
    ssize_t bytesReceived;

    bytesReceived = recv(client, buff, sizeof(buff), 0);
    if (bytesReceived == -1)
    {
        perror("recv");
        close(client);
        return;
    }

    // Broadcast the received message to all clients
    for (int i = 0; i < *client_sz; i++)
    {
        int c = clients[i];
        if (c != client)
        {
            if (send(c, buff, bytesReceived, 0) == -1)
            {
                perror("send");
            }
        }
    }

    puts(buff);
    fflush(stdout);
}

int check_client(int client, int clients[], int client_sz)
{
    for (int i = 0; i < client_sz; i++)
    {
        if (clients[i] == client)
        {
            return 0;
        }
    }
    return 1;
}

int main()
{
    char buff[BUF_SIZE];
    struct sockaddr_in caddr;
    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(SERVER_PORT)};

    int server = socket(AF_INET, SOCK_STREAM, 0);
    int clients[MAX_CLIENTS];
    int client_sz = 0;

    if (server == -1)
    {
        perror("socket");
        exit(1);
    }

    if (bind(server, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server, 5) == -1)
    {
        perror("listen");
        exit(1);
    }

    while (1)
    {
        int client, csize = sizeof(caddr);
        client = accept(server, (struct sockaddr *)&caddr, &csize);
        if (client == -1)
        {
            perror("accept");
            continue;
        }

        printf("Client connected: %s\n", inet_ntoa(caddr.sin_addr));

        if (client_sz == MAX_CLIENTS)
        {
            puts("Server is full");
            send(client, "Server is full", 14, 0);
            close(client);
            continue;
        }

        if (check_client(client, clients, client_sz))
        {
            clients[client_sz++] = client;
        }

        handle_client(client, clients, &client_sz);
    }

    return 0;
}
