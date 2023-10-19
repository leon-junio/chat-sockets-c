/*
*
* MESSAGE SOCKET SERVER 
* Server mode
* @author: Edmar Melandes de Oliveira e Leon Junio Martins
*
*/

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

// Save client info
struct client_info
{
    int client_id;
    char name[64];
    struct sockaddr_in caddr;
};

// All Clients array (length: MAX_CLIENTS)
// Each client have a struct and a thread running when the connection is up
struct client_info *clients[MAX_CLIENTS];
// Array clients counter
int client_sz = 0;

// Remove some client from connections and reallocate the position in the array
// Removing a client drop all info about that client connection (break broadcast and unicast)
void remove_client(int client_id)
{
    for (int i = 0; i < client_sz; i++)
    {
        if (clients[i]->client_id == client_id)
        {
            // Shift the rest of the clients to fill the gap
            for (int j = i; j < client_sz - 1; j++)
                clients[j] = clients[j + 1];
            (client_sz)--;
            break;
        }
    }
}

// Check if a string buff is a command and perform the action associated with it
// All commands sent back some message to the client
int check_command(char *buff, struct client_info *client)
{
    if (strcmp(buff, "/quit\n") == 0)
    {
        close(client->client_id);
        remove_client(client->client_id);
    }
    else if (strstr(buff, "/setname") != NULL)
    {
        char *name = strtok(buff, " ");
        name = strtok(NULL, " ");
        // check if name is already taken
        for (int i = 0; i < client_sz; i++)
        {
            if (strcmp(clients[i]->name, name) == 0)
            {
                char message[BUF_SIZE];
                sprintf(message, "server: Name already taken");
                send(client->client_id, message, strlen(message) + 1, 0);
                return 1;
            }
        }
        // remove \n
        name[strlen(name) - 1] = '\0';
        strcpy(client->name, name);
        char message[BUF_SIZE];
        sprintf(message, "server: Name set to %s", name);
        send(client->client_id, message, strlen(message) + 1, 0);
    }
    else if (strcmp(buff, "/list\n") == 0)
    {
        char message[BUF_SIZE];
		char *aux = (char*) malloc(BUF_SIZE);
        for (int i = 0; i < client_sz; i++)
        {
            sprintf(message, "%d: %s <%s>\n", i, clients[i]->name, inet_ntoa(clients[i]->caddr.sin_addr));
            strcat(aux,message);
        }
		send(client->client_id, aux, strlen(aux) + 1, 0);
    }
    else if (strcmp(buff, "/ping\n") == 0)
    {
        char message[BUF_SIZE];
        sprintf(message, "server: Pong");
        send(client->client_id, message, strlen(message) + 1, 0);
    }
    else if (strstr(buff, "/msg") != NULL)
    {
        printf("%s", buff);
        char *name = strtok(buff, " ");
        name = strtok(NULL, " ");
        char *msg = strtok(NULL, " ");
        printf("%s\n", msg);
        char message[BUF_SIZE];
        if (strcmp(name, "Anonymous") != 0)
        {
            int found = 0;
            for (int i = 0; i < client_sz; i++)
            {
                if (strcmp(clients[i]->name, name) == 0)
                {
                    found = 1;
                    sprintf(message, "\033[35m%s <%s> -> %s: %s\033[0m", client->name, inet_ntoa(client->caddr.sin_addr), clients[i]->name, msg);
                    int status = send(clients[i]->client_id, message, strlen(message) + 1, 0);
                    if (status >= 0)
                        send(client->client_id, message, strlen(message) + 1, 0);
                    else
                    {
                        sprintf(message, "server: Cannot send message to %s", name);
                        send(client->client_id, message, strlen(message) + 1, 0);
                    }
                    break;
                }
            }
            if (!found)
            {
                sprintf(message, "server: Cannot find user %s", name);
                send(client->client_id, message, strlen(message) + 1, 0);
            }
        }
        else
        {
            sprintf(message, "server: Cannot send message to Anonymous");
            send(client->client_id, message, strlen(message) + 1, 0);
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

// This function is called each time that a new client is created and run until the client disconnect from the server
// This function can read new messages from the client, perform broadcasts and call commands from check_command function
void handle_client(struct client_info *client)
{
    char buff[BUF_SIZE];
    char message[BUF_SIZE + sizeof(struct client_info) + 1];
    ssize_t bytesReceived;

    while (1)
    {
        bytesReceived = recv(client->client_id, buff, sizeof(buff), 0);
        if (bytesReceived <= 0)
        {
            printf("Client disconnected: %s\n", inet_ntoa(client->caddr.sin_addr));
            // Handle client disconnection or error here
            close(client->client_id);
            // Remove the client from the array (if it exists)
            remove_client(client->client_id);
            return;
        }

        printf("data received from: %s", client->name);

        printf("%s\n", buff);

        if (check_command(buff, client) == 0)
        {
            sprintf(message, "\033[32m%s <%s>: %s\033[0m", client->name, inet_ntoa(client->caddr.sin_addr), buff);
            // Broadcast the received message to all clients
            for (int i = 0; i < client_sz; i++)
            {
                int st = send(clients[i]->client_id, message, strlen(message) + 1, 0);
                if (st == -1)
                {
                    perror("send");
                }
            }
        }
        puts(buff);
        fflush(stdout);
    }
}

//Check if the client exists in the clients array
int check_client(int client_id)
{
    for (int i = 0; i < client_sz; i++)
    {
        if (clients[i]->client_id == client_id)
        {
            return 0;
        }
    }
    return 1;
}

// Client thread start the read loop for data that come from netif
// Each thread will run in parallel
void *client_thread(void *arg)
{
    struct client_info *client = (struct client_info *)arg;
    handle_client(client);
    pthread_exit(NULL);
}

//The main thread that accept new clients for the server and start all the other services
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
	
	// wait for new clients
    while (1)
    {
        int csize = sizeof(caddr);
        int *client_id = (int *)malloc(sizeof(int));
        struct client_info *client_info = malloc(sizeof(struct client_info));
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
            send(*client_id, "server: Server is full", 14, 0);
            close(*client_id);
            free(client_id);
            continue;
        }
		
		// start client receive thread
        if (check_client(*client_id) == 1)
        {
            *client_info = (struct client_info){
                .client_id = *client_id,
                .caddr = caddr,
                .name = "Anonymous"};
            clients[client_sz++] = client_info;
            pthread_t tid;
            pthread_create(&tid, NULL, client_thread, client_info);
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
