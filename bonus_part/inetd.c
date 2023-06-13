#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

typedef struct s_idAndSocket
{
    int id;
    int socket;
} idAndSocket;

void *listenPorts(idAndSocket *vargp)
{
    idAndSocket myIdAndSocket = *(idAndSocket *)vargp;
    int myid = myIdAndSocket.id;
    int socket = myIdAndSocket.socket;

    int clientSocket;
    struct sockaddr_in clientAddress;
    socklen_t clientLength = sizeof(clientAddress);
    while (1)
    {
        // Accept incoming connections for the square server
        clientSocket = accept(socket, (struct sockaddr *)&clientAddress, &clientLength);
        if (clientSocket < 0)
        {
            perror("Accept failed");
            exit(1);
        }

        if (myid == 0)
        {
            printf("(inetd) Connection request to square service\n");
        }
        else if (myid == 1)
        {
            printf("(inetd) Connection request to cube service\n");
        }

        // Fork a child process to handle the square request
        pid_t pid = fork();

        if (pid == 0)
        {
            // Child process
            close(socket);

            char socket_fd_str[10];
            sprintf(socket_fd_str, "%d", clientSocket);

            if (myid == 0)
            {
                execl("./square", "square", socket_fd_str, NULL);
            }
            else if (myid == 1)
            {
                execl("./cube", "cube", socket_fd_str, NULL);
            }
            // handleSquare(clientSocket);
            exit(0);
        }
        else if (pid < 0)
        {
            perror("Fork failed");
            exit(1);
        }
    }

    return NULL;
}

int main()
{
    int squarePort = 5010;
    int cubePort = 5020;

    int squareSocket, cubeSocket;
    struct sockaddr_in squareAddress, cubeAddress;

    // Create socket for the square server
    squareSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (squareSocket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Prepare the address structure for the square server
    squareAddress.sin_family = AF_INET;
    squareAddress.sin_addr.s_addr = INADDR_ANY;
    squareAddress.sin_port = htons(squarePort);

    // Bind the square socket to the specified port
    if (bind(squareSocket, (struct sockaddr *)&squareAddress, sizeof(squareAddress)) < 0)
    {
        perror("Binding failed");
        exit(1);
    }

    // Create socket for the cube server
    cubeSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (cubeSocket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Prepare the address structure for the cube server
    cubeAddress.sin_family = AF_INET;
    cubeAddress.sin_addr.s_addr = INADDR_ANY;
    cubeAddress.sin_port = htons(cubePort);

    // Bind the cube socket to the specified port
    if (bind(cubeSocket, (struct sockaddr *)&cubeAddress, sizeof(cubeAddress)) < 0)
    {
        perror("Binding failed");
        exit(1);
    }

    // Listen for connections on both ports
    listen(squareSocket, 1);
    listen(cubeSocket, 1);

    printf("(inetd) inetd has started\n");
    printf("(inetd) Waiting for ports %d & %d\n", squarePort, cubePort);

    int i;
    pthread_t tid[2];
    idAndSocket idsAndSockets[2];

    for (i = 0; i < 2; i++)
    {
        idsAndSockets[i].id = i;
        if (i == 0)
        {
            idsAndSockets[i].socket = squareSocket;
        }
        else
        {
            idsAndSockets[i].socket = cubeSocket;
        }
        pthread_create(&tid[i], NULL, listenPorts, &idsAndSockets[i]);
    }
    for (i = 0; i < 2; i++)
    {
        pthread_join(tid[i], NULL);
    }

    close(squareSocket);
    close(cubeSocket);

    return 0;
}