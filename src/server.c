#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_PORT 5000
#define MAX_CLIENTS 10
#define BUF_SIZE 4096

int clients[MAX_CLIENTS];
int client_sz = 0;

void handle_client(int client, int clients[], int *client_sz)
{
    char buff[BUF_SIZE];
    ssize_t bytesReceived;

    while (1)
    {
        bytesReceived = recv(client, buff, sizeof(buff), 0);
        if (bytesReceived <= 0)
        {
            // Handle client disconnection or error here
            close(client);
            // Remove the client from the array (if it exists)
            for (int i = 0; i < *client_sz; i++)
            {
                if (clients[i] == client)
                {
                    // Shift the rest of the clients to fill the gap
                    for (int j = i; j < *client_sz - 1; j++)
                        clients[j] = clients[j + 1];
                    (*client_sz)--;
                    break;
                }
            }
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

void *client_thread(void *arg)
{
    int client = *(int *)arg;
    free(arg);
    handle_client(client, clients, &client_sz);
    pthread_exit(NULL);
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
        int csize = sizeof(caddr);
        int *client = (int *)malloc(sizeof(int));
        *client = accept(server, (struct sockaddr *)&caddr, &csize);
        if (*client == -1)
        {
            perror("accept");
            free(client);
            continue;
        }

        printf("Client connected: %s\n", inet_ntoa(caddr.sin_addr));

        if (client_sz == MAX_CLIENTS)
        {
            puts("Server is full");
            send(*client, "Server is full", 14, 0);
            close(*client);
            free(client);
            continue;
        }

        if (check_client(*client, clients, client_sz))
        {
            clients[client_sz++] = *client;
            pthread_t tid;
            pthread_create(&tid, NULL, client_thread, client);
            pthread_detach(tid);
        }
        else
        {
            // If the client is already in the list, free the allocated memory.
            free(client);
        }
    }

    return 0;
}
