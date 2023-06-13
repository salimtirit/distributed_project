#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

void handleClient(int clientSocket, const char *serverName);

int main()
{
    int squarePort = 5010;
    int cubePort = 5020;

    int inetdSocket;
    struct sockaddr_in inetdAddress;

    // Create socket for the inetd server
    inetdSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (inetdSocket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Prepare the address structure for the inetd server
    inetdAddress.sin_family = AF_INET;
    inetdAddress.sin_addr.s_addr = INADDR_ANY;
    inetdAddress.sin_port = htons(squarePort); // Use squarePort as the default port for inetd

    // Bind the inetd socket to the squarePort
    if (bind(inetdSocket, (struct sockaddr *)&inetdAddress, sizeof(inetdAddress)) < 0)
    {
        perror("Binding failed");
        exit(1);
    }

    // Listen for connections on the inetd socket
    listen(inetdSocket, 1);

    printf("(inetd) inetd has started\n");
    printf("(inetd) Waiting for connections on port %d\n", squarePort);

    while (1)
    {
        int clientSocket;
        struct sockaddr_in clientAddress;
        socklen_t clientLength = sizeof(clientAddress);

        // Accept incoming connections
        clientSocket = accept(inetdSocket, (struct sockaddr *)&clientAddress, &clientLength);
        if (clientSocket < 0)
        {
            perror("Accept failed");
            exit(1);
        }

        printf("(inetd) Connection request received\n");

        // Determine which server to spawn based on the client's requested port
        if (clientAddress.sin_port == htons(squarePort))
        {
            handleClient(clientSocket, "square");
        }
        else if (clientAddress.sin_port == htons(cubePort))
        {
            handleClient(clientSocket, "cube");
        }
    }

    close(inetdSocket);

    return 0;
}

void handleClient(int clientSocket, const char *serverName)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        // Child process
        close(0);
        dup(clientSocket);
        close(clientSocket);

        // Execute the appropriate server program
        execl(serverName, serverName, NULL);

        exit(0);
    }
    else if (pid < 0)
    {
        perror("Fork failed");
        exit(1);
    }
}
