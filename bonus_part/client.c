#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./client <port>\n");
        return 1;
    }

    int port = atoi(argv[1]);

    int clientSocket;
    struct sockaddr_in serverAddress;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Prepare the server address structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0)
    {
        perror("Invalid address/Address not supported");
        exit(1);
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Connection failed");
        exit(1);
    }

    char buffer[1024];

    while (1)
    {
        printf("Enter request: ");
        fgets(buffer, sizeof(buffer), stdin);

        // Remove newline character from the input
        buffer[strcspn(buffer, "\n")] = '\0';

        // Send the request to the server
        write(clientSocket, buffer, strlen(buffer) + 1);

        // Receive and display the server's reply
        memset(buffer, 0, sizeof(buffer));
        read(clientSocket, buffer, sizeof(buffer));
        printf("Result: %s\n", buffer);
    }

    close(clientSocket);

    return 0;
}
