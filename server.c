#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>

#define MAX_CLIENTS 10

int client_count = 0;
int client_sockets[MAX_CLIENTS];
pid_t child_pids[MAX_CLIENTS];

void handle_client(int client_socket, int client_id) {
    char buffer[1024];
    int num;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t recv_size = recv(client_socket, buffer, sizeof(buffer), 0);

        if (recv_size < 0) {
            perror("Error reading from socket");
            break;
        } else if (recv_size == 0) {
            printf("Client #%d disconnected\n", client_id);
            break;
        }

        num = atoi(buffer);
        if (num < 0) {
            printf("Server: Terminating client #%d\n", client_id);
            send(client_socket, buffer, strlen(buffer), 0);
            break;
        }

        printf("Child #%d Request: %d\n", client_id, num);

        int square = num * num;
        sprintf(buffer, "%d", square);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    close(client_socket);
    exit(0);
}

void cleanup_child(int signal) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        int i;
        for (i = 0; i < client_count; i++) {
            if (child_pids[i] == pid) {
                printf("Child #%d terminated\n", i + 1);
                child_pids[i] = 0;
                break;
            }
        }
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error opening socket");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Error listening for connections");
        exit(1);
    }

    signal(SIGCHLD, cleanup_child);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            exit(1);
        }

        if (client_count == MAX_CLIENTS) {
            printf("Maximum number of clients reached. Connection rejected.\n");
            close(client_socket);
            continue;
        }

        client_count++;
        int client_id = client_count;

        pid_t pid = fork();
        if (pid == 0) {
            close(server_socket);
            handle_client(client_socket, client_id);
        } else {
            close(client_socket);
            child_pids[client_id - 1] = pid;
        }
    }
}
