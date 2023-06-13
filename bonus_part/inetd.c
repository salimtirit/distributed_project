#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

void spawnServer(const char *serverName, int clientSocket);

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
    inetdAddress.sin_port = 0;

    // Bind the inetd socket to any available port
    if (bind(inetdSocket, (struct sockaddr *)&inetdAddress, sizeof(inetdAddress)) < 0)
    {
        perror("Binding failed");
        exit(1);
    }

    // Get the dynamically assigned port number
    struct sockaddr_in socketAddress;
    socklen_t addressLength = sizeof(socketAddress);
    getsockname(inetdSocket, (struct sockaddr *)&socketAddress, &addressLength);
    int inetdPort = ntohs(socketAddress.sin_port);

    // Listen for connections on the inetd socket
    listen(inetdSocket, 1);

    printf("(inetd) inetd has started\n");
    printf("(inetd) Waiting for ports %d & %d\n", squarePort, cubePort);

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
            spawnServer("square", clientSocket);
        }
        else if (clientAddress.sin_port == htons(cubePort))
        {
            spawnServer("cube", clientSocket);
        }
    }

    close(inetdSocket);

    return 0;
}

void spawnServer(const char *serverName, int clientSocket)
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
