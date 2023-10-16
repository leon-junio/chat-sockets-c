#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define SERVER_PORT 5000
#define BUF_SIZE 4096

int s;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void fatal(char *string)
{
    printf("%s\n", string);
    close(s);
    exit(1);
}

void help()
{
    printf("You can now start typing messages.\n");
    printf("Commands:\n");
    printf("  /setname <name>: Set your name\n");
    printf("  /ping: Ping the server\n");
    printf("  /list: List all connected users\n");
    printf("  /msg <name> <message>: Send a private message to a user\n");
    printf("  /clear: Clear the screen\n");
    printf("  /exit: Exit the program\n");
    printf("  /help: Display this help message\n");
}

void set_name(char *buff)
{
    char *name = strtok(buff, " ");
    name = strtok(NULL, " ");

    if (name == NULL)
    {
        printf("Invalid command. Usage: /setname <name>\n");
    }

    pthread_mutex_lock(&mutex);

    // Send the name to the server
    send(s, buff, strlen(buff) + 1, 0);

    pthread_mutex_unlock(&mutex);
}

void check_command(char *buff)
{
    if (strcmp(buff, "/exit\n") == 0)
    {
        close(s);
        exit(0);
    }
    else if (strcmp(buff, "/clear\n") == 0)
    {
        printf("\033[H\033[J");
    }
    else if (strcmp(buff, "/help\n") == 0)
    {
        help();
    }
    else if (strcmp(buff, "/ping\n") == 0)
    {
        pthread_mutex_lock(&mutex);

        printf("Pinging the server...\n");
        time_t t = clock();
        send(s, buff, strlen(buff) + 1, 0);

        // Wait for the server to respond
        read(s, buff, BUF_SIZE);

        t = clock() - t;

        printf("Server responded in %f seconds.\n", ((double)t) / CLOCKS_PER_SEC);

        pthread_mutex_unlock(&mutex);
    }
    else
    {
        printf("Invalid command. Type '/help' for a list of commands.\n");
    }
}

void *write_messages(void *arg)
{
    char buff[BUF_SIZE], time_string[9];
    int bytes;
    time_t current_time;
    struct tm *time_info;
    while (1)
    {
        bytes = read(s, buff, BUF_SIZE);

        if (bytes <= 0)
            fatal("recv failed: disconnected");

        pthread_mutex_lock(&mutex);

        time(&current_time);
        time_info = localtime(&current_time);

        strftime(time_string, sizeof(time_string), "%H:%M:%S", time_info);

        // Print the messages
        printf("[%s] \033[32m%s\033[0m\n", time_string, buff);

        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char **argv)
{
    int c, res;
    char input[BUF_SIZE];
    struct hostent *h;
    struct sockaddr_in channel;
    pthread_t thread1;

    if (argc != 2)
        fatal("Usage: client <server-name> [name]");

    h = gethostbyname(argv[1]);

    if (!h)
        fatal("gethostbyname failed");

    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s < 0)
        fatal("socket");

    memset(&channel, 0, sizeof(channel));
    channel.sin_family = AF_INET;
    memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
    channel.sin_port = htons(SERVER_PORT);

    c = connect(s, (struct sockaddr *)&channel, sizeof(channel));

    if (c < 0)
        fatal("connect failed");

    if (argc == 3)
    {
        char *name = argv[2];
        char buff[BUF_SIZE];
        sprintf(buff, "/setname %s", name);
        set_name(buff);
    }

    printf("Connected to %s\n", argv[1]);
    help();

    // Create a thread for printing messages from the server
    pthread_create(&thread1, NULL, write_messages, NULL);

    while (1)
    {
        fgets(input, BUF_SIZE, stdin);

        // Check if the user entered a command
        if (input[0] == '/')
        {
            check_command(input);
            continue;
        }

        pthread_mutex_lock(&mutex);

        // Send the message to the server
        res = send(s, input, strlen(input) + 1, 0);

        pthread_mutex_unlock(&mutex);

        if (res < 0)
            fatal("send failed: disconnected");
    }

    pthread_join(thread1, NULL);

    return 0;
}