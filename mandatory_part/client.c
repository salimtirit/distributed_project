#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <client_id>\n", argv[0]);
        exit(1);
    }

    int client_id = atoi(argv[1]);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(5000);

    if (inet_pton(AF_INET, "127.0.0.1", &(server_address.sin_addr)) <= 0)
    {
        perror("Error setting server address");
        exit(1);
    }

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Error connecting to server");
        exit(1);
    }

    char buffer[1024];
    sprintf(buffer, "%d", client_id);
    send(client_socket, buffer, strlen(buffer), 0);

    ssize_t cli_id_back_size = recv(client_socket, buffer, sizeof(buffer), 0);
    if (cli_id_back_size < 0)
    {
        perror("Error receiving client ID from server");
        exit(1);
    }
    buffer[cli_id_back_size] = '\0';
    int received_client_id = atoi(buffer);
    printf("This is client #%d\n", received_client_id);

    while (1)
    {
        int num;
        printf("Enter request (negative to terminate): ");
        scanf("%d", &num);

        sprintf(buffer, "%d", num);
        send(client_socket, buffer, strlen(buffer), 0);

        memset(buffer, 0, sizeof(buffer));
        ssize_t recv_size = recv(client_socket, buffer, sizeof(buffer), 0);

        if (recv_size < 0)
        {
            perror("Error reading from socket");
            break;
        }
        else if (recv_size == 0)
        {
            printf("Server disconnected\n");
            break;
        }

        int square = atoi(buffer);
        if (square < 0)
        {
            printf("%s\n", "    Will terminate");
        }
        else
        {
            printf("    Result: %d\n", square);
        }

        if (num < 0)
        {
            break;
        }
    }

    close(client_socket);
    return 0;
}
