#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    char buffer[1024];
    char *ip = "127.0.0.1";
    int port = 5566;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0) {
        perror("Nao foi possivel fazer o socket do servidor\n");
        exit(EXIT_FAILURE);
    }
    printf("Socket de servidor criado com sucesso\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    int n = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(n < 0) {
        perror("Erro de conexao\n");
        exit(EXIT_FAILURE);
    }
    printf("Conexao com a porta: %d estabelecia com sucesso\n", port);

    listen(server_socket, 3); // acho que isso são os 3 servidores
    printf("Listening...\n");

    // usar aquele pipe aqui

    while(1) {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        printf("Client conectado\n");

        // receive client message
        bzero(buffer, 1024); //zera o buffer
        recv(client_socket, buffer, sizeof(buffer), 0);
        printf("Client message: %s\n", buffer);

        // send response
        bzero(buffer, 1024);
        strcpy(buffer, "fon");
        printf("Server Response: %s\n", buffer);
        send(client_socket, buffer, strlen(buffer), 0);

        close(client_socket);
        printf("client disconnected\n");
    }

    return 0;
}