#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int socket_fd = atoi(argv[1]);

    char buffer[BUFFER_SIZE];
    ssize_t num_bytes;

    num_bytes = read(socket_fd, buffer, BUFFER_SIZE - 1);

    if (num_bytes < 0)
    {
        perror("Socket read failure.");
        exit(EXIT_FAILURE);
    }

    buffer[num_bytes] = '\0';

    int number = atoi(buffer);
    int square = number * number;

    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "%d", square);
    write(socket_fd, response, strlen(response));

    printf("(square) Request=%d\n", number);
    printf("(square) Reply sent as %d. Terminating...\n", square);

    return 0;
}
