#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_PORT 5000
#define BUF_SIZE 4096

int s, bytes;

void *write_messages(void *arg)
{
    char buff[BUF_SIZE];
    int count = 0;
    while (1)
    {
        // Clear the terminal screen
        printf("\033[H\033[J");
        printf("Client ID: %d\n", count++);

        // Print the messages
        bytes = read(s, buff, BUF_SIZE);
        write(1, buff, bytes);

        // Sleep for a while
        sleep(1);
    }
}

void *get_input(void *arg)
{
    char input[BUF_SIZE];
    while (1)
    {
        printf("Type a message: ");
        fgets(input, BUF_SIZE, stdin);

        // Send the message to the server
        write(s, input, strlen(input) + 1);
    }
}

void fatal(char *string)
{
    printf("%s\n", string);
    exit(1);
}

int main(int argc, char **argv)
{
    int c;
    struct hostent *h;
    struct sockaddr_in channel;
    pthread_t thread1, thread2;

    if (argc != 2)
        fatal("Usage: client server-name");

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

<<<<<<< HEAD
    write(s, argv[2], strlen(argv[2]) + 1);
    sleep(5);

    printf("%s","KEEPING CONNECTION OPEN \n");
=======
    // Create two threads (one for reading and one for writing)
    pthread_create(&thread1, NULL, write_messages, NULL);
    pthread_create(&thread2, NULL, get_input, NULL);
>>>>>>> 723b826e724c342b90a329ef65a371415c64885e

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}