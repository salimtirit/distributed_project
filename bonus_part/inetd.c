#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>


int main() {
    int squarePort = 5010;
    int cubePort = 5020;

    int squareSocket, cubeSocket;
    struct sockaddr_in squareAddress, cubeAddress;

    // Create socket for the square server
    squareSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (squareSocket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Prepare the address structure for the square server
    squareAddress.sin_family = AF_INET;
    squareAddress.sin_addr.s_addr = INADDR_ANY;
    squareAddress.sin_port = htons(squarePort);

    // Bind the square socket to the specified port
    if (bind(squareSocket, (struct sockaddr *)&squareAddress, sizeof(squareAddress)) < 0) {
        perror("Binding failed");
        exit(1);
    }

    // Create socket for the cube server
    cubeSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (cubeSocket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Prepare the address structure for the cube server
    cubeAddress.sin_family = AF_INET;
    cubeAddress.sin_addr.s_addr = INADDR_ANY;
    cubeAddress.sin_port = htons(cubePort);

    // Bind the cube socket to the specified port
    if (bind(cubeSocket, (struct sockaddr *)&cubeAddress, sizeof(cubeAddress)) < 0) {
        perror("Binding failed");
        exit(1);
    }

    // Listen for connections on both ports
    listen(squareSocket, 1);
    listen(cubeSocket, 1);

    printf("(inetd) inetd has started\n");
    printf("(inetd) Waiting for ports %d & %d\n", squarePort, cubePort);

    while (1) {
        int clientSocket;
        struct sockaddr_in clientAddress;
        socklen_t clientLength = sizeof(clientAddress);

        // Accept incoming connections
        // Accept incoming connections for the square server
        clientSocket = accept(squareSocket, (struct sockaddr *)&clientAddress, &clientLength);
        if (clientSocket < 0) {
            perror("Accept failed");
            exit(1);
        }

        printf("(inetd) Connection request to square service\n");

        // Fork a child process to handle the square request
        pid_t pid = fork();

        if (pid == 0) {
            // Child process
            close(squareSocket);
            handleSquare(clientSocket);
            exit(0);
        } else if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }

        // Accept incoming connections for the cube server
        clientSocket = accept(cubeSocket, (struct sockaddr *)&clientAddress, &clientLength);
        if (clientSocket < 0) {
            perror("Accept failed");
            exit(1);
        }

        printf("(inetd) Connection request to cube service\n");

        // Fork a child process to handle the cube request
        pid = fork();

        if (pid == 0) {
            // Child process
            close(cubeSocket);
            handleCube(clientSocket);
            exit(0);
        } else if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }
    }

    close(squareSocket);
    close(cubeSocket);

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
