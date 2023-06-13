#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

void handleSquare(int clientSocket)
{
    char buffer[1024];
    int num;

    memset(buffer, 0, sizeof(buffer));

    // Read the request from the client
    if (read(clientSocket, buffer, sizeof(buffer)) > 0)
    {
        num = atoi(buffer);
        int result = num * num;

        // Send the square result back to the client
        snprintf(buffer, sizeof(buffer), "%d", result);
        write(clientSocket, buffer, strlen(buffer) + 1);

        printf("(square) Request=%d\n", num);
        printf("(square) Reply sent as %d. Terminating...\n", result);
    }

    close(clientSocket);
}

int main()
{
    int port = 5010;

    int serverSocket;
    struct sockaddr_in serverAddress;

    // Create socket for the server
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Prepare the address structure for the server
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    // Bind the server socket to the specified port
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Binding failed");
        exit(1);
    }

    // Listen for connections on the port
    listen(serverSocket, 1);

    printf("(square) square server has started\n");
    printf("(square) Waiting for connections on port %d\n", port);

    while (1)
    {
        int clientSocket;
        struct sockaddr_in clientAddress;
        socklen_t clientLength = sizeof(clientAddress);

        // Accept incoming connections
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLength);
        if (clientSocket < 0)
        {
            perror("Accept failed");
            exit(1);
        }

        printf("(square) Connection request received\n");

        // Handle the request
        handleSquare(clientSocket);
    }

    close(serverSocket);

    return 0;
}
