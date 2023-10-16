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

struct client_info
{
    int client_id;
    char name[20];
    struct sockaddr_in caddr;
};

struct client_info clients[MAX_CLIENTS];
int client_sz = 0;

void handle_client(struct client_info client, struct client_info clients[], int *client_sz)
{
    char buff[BUF_SIZE];
    char message[BUF_SIZE + sizeof(struct client_info) + 1];
    ssize_t bytesReceived;

    while (1)
    {
        bytesReceived = recv(client.client_id, buff, sizeof(buff), 0);
        if (bytesReceived <= 0)
        {
            // Handle client disconnection or error here
            close(client.client_id);
            // Remove the client from the array (if it exists)
            for (int i = 0; i < *client_sz; i++)
            {
                if (clients[i].client_id == client.client_id)
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

        sprintf(message, "%s: %s", inet_ntoa(client.caddr.sin_addr), buff);

        // Broadcast the received message to all clients
        for (int i = 0; i < *client_sz; i++)
        {
            if (send(clients[i].client_id, message, strlen(message) + 1, 0) == -1)
            {
                perror("send");
            }
        }

        puts(buff);
        fflush(stdout);
    }
}

int check_client(int client_id, struct client_info clients[], int client_sz)
{
    for (int i = 0; i < client_sz; i++)
    {
        if (clients[i].client_id == client_id)
        {
            return 0;
        }
    }
    return 1;
}

void *client_thread(void *arg)
{
    struct client_info client = *(struct client_info *)arg;
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
        int *client_id = (int *)malloc(sizeof(int));
        *client_id = accept(server, (struct sockaddr *)&caddr, &csize);
        if (*client_id == -1)
        {
            perror("accept");
            free(client_id);
            continue;
        }

        printf("Client connected: %s\n", inet_ntoa(caddr.sin_addr));

        if (client_sz == MAX_CLIENTS)
        {
            puts("Server is full");
            send(*client_id, "Server is full", 14, 0);
            close(*client_id);
            free(client_id);
            continue;
        }

        if (check_client(*client_id, clients, client_sz))
        {
            struct client_info client_info = {
                .client_id = *client_id,
                .caddr = caddr};
            clients[client_sz++] = client_info;
            pthread_t tid;
            pthread_create(&tid, NULL, client_thread, &client_info);
            pthread_detach(tid);
        }
        else
        {
            // If the client_id is already in the list, free the allocated memory.
            free(client_id);
        }
    }

    return 0;
}
