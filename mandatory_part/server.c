#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>

#define MAX_CLIENTS 10

pid_t child_pids[MAX_CLIENTS];

/*
Most of the code used in the mandatory part is written by ChatGPT

Prompt 1:
I want to create a server client system that runs on a linux machine.
I want you to help me create this using C language.
There is going to be a server and multiple clients (at most 10) that connects to the server.
The server will run in one terminal and the clients will run in different terminals.
When a client connects to the server the server will fork for that specific client.
When we want to run a client we are going to run it like this "./client 1".
The "1" in here corresponds to the client number.
The child process that has been created for this client remembers this client id and when a request comes from a client,
the child process writes this to the terminal like "child #1 Request...".
Each client will read a integer from the input and sends this to the servers correspondin child.
That child will take the square of the integer and sent that back. The square will get printed to the terminal on the client side.
When a negative integer is sent the server will sent it back to the client this means the end of the communication for both of the sides.
The client and the child that corresponds to it will be terminated.
To prevent the zombie processes accumulating it needs to find and terminate them.
--> this gave the first version of the server code

Prompt 2:
what about the client code.
--> this gave the first version of the client code

Then I changed the ports to 5000 and the print statements to the appropriate ones in both codes.

Prompt 3:
The first thing client needs to send is its client id  which has been read from the argsv.   How can I add this
--> this changed the client tode to sent its client id to the server

Prompt 4:
the server also needs to get this number and store as the client id.
The client count will not be needed in this scenario.
What changes int the server code.
--> this changed the server code to store the client id

Prompt 5:
we also send this client id to the client back to confirm what does this change
--> this changed the server code to send the client id back to the client

Prompt 6:
what does this change in the client code.
I want to get the client id back and print something like "This is client #1"
--> gives the final version of the client code

After all the changes I did the last touches.
*/

void handle_client(int client_socket, int client_id)
{
    char buffer[1024];
    int num;

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        ssize_t recv_size = recv(client_socket, buffer, sizeof(buffer), 0);

        if (recv_size < 0)
        {
            perror("Error reading from socket");
            break;
        }
        else if (recv_size == 0)
        {
            printf("(child #%d) Client disconnected\n", client_id);
            break;
        }

        num = atoi(buffer);
        if (num < 0)
        {
            printf("(child #%d) Request=%d Will terminate\n", client_id, num);
            send(client_socket, buffer, strlen(buffer), 0);
            break;
        }

        printf("(child #%d) Request=%d\n", client_id, num);

        int square = num * num;
        sprintf(buffer, "%d", square);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    close(client_socket);
    exit(0);
}

void cleanup_child(int signal)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        int i;
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (child_pids[i] == pid)
            {
                child_pids[i] = 0;
                break;
            }
        }
    }
}

int main()
{
    printf("%s\n", "(parent) Server has started");

    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(5000);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Error binding socket");
        exit(1);
    }

    if (listen(server_socket, 5) < 0)
    {
        perror("Error listening for connections");
        exit(1);
    }

    signal(SIGCHLD, cleanup_child);

    printf("%s\n", "(parent) Waiting for connections");
    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket < 0)
        {
            perror("Error accepting connection");
            exit(1);
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("Error forking child process.");
            exit(1);
        }
        else if (pid == 0)
        {
            close(server_socket);

            char buffer[1024];
            ssize_t recv_size = recv(client_socket, buffer, sizeof(buffer), 0);

            if (recv_size < 0)
            {
                perror("Error reading client ID");
                exit(1);
            }

            int client_id = atoi(buffer);
            printf("(child #%d) Child created for incoming request\n", client_id);

            sprintf(buffer, "%d", client_id);
            send(client_socket, buffer, strlen(buffer), 0);

            handle_client(client_socket, client_id);
        }
        else
        {
            close(client_socket);

            int i;
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                if (child_pids[i] == 0)
                {
                    child_pids[i] = pid;
                    break;
                }
            }
        }
    }
}
