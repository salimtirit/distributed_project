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
    int cube = number * number * number;

    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "%d", cube);
    write(socket_fd, response, strlen(response));

    printf("(cube) Request=%d\n", number);
    printf("(cube) Reply sent as %d. Terminating...\n", cube);

    return 0;
}
