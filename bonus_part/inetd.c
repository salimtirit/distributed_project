#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/*
Base code for the bonus part is written by ChatGPT

Prompt 1:
There are two servers named "square" and "cube". The first one takes the square of the number and returns it to the client, and the second one takes the cube of the number and returns it to the client. I want to write a program using C language called "inetd" that listens to ports 5010 and 5020. There is also a code, named "client" that is run as "./client 5010" or "./client 5020". If the client is run with 5010 as the port number the square server will be spawned by my inetd and if it is run with 5020 as the port number the cube server will be spawned. After serving the client the servers will terminate. Here is a sample how the codes should look like on the terminal:

For inetd:
$ ./inetd
(inetd) inetd has started
(inetd) Waiting for ports 5010 & 5020
(inetd) Connection request to square service
(square) Request=6
(square) Reply sent as 36. Terminating...
(inetd) Connection request to cube service
(cube) Request=2
(cube) Reply sent as 8. Terminating...
(inetd) Connection request to cube service
(cube) Request=3
(cube) Reply sent as 27. Terminating...
(inetd) Connection request to square service
(square) Request=8
(square) Reply sent as 64. Terminating...

For client:
$ ./client 5010
Enter request: 6
 Result: 36
$ ./client 5020
Enter request: 2
 Result: 8
$ ./client 5020
Enter request: 3
 Result: 27
$ ./client 5010
Enter request: 8
 Result: 64

--> this gave the first version of the inetd code.
The problem was there were no threads and the inetd was not listening to both ports at the same time.
Also there was only one server and cube and square were not distinguished.

Prompt 2:
This code handles the functionalities of the two servers in it. I want them to be separate servers.
This means there needs to be 3 servers: inetd, square, and cube. These should be in separate codes.
I want the inetd to spawn the other two when needed.
--> This seperated the codes but it was not working

Prompt 3:
what about the client code
--> this gave the first version of the client code

After this whatever I did I could not get the inetd to work using the ChatGPT's code.
So I decided to code it for myself.
*/

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