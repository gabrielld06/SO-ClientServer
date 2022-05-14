#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int op, id_busca;
    char buffer[1024];

    int server_socket;
    struct sockaddr_in server_addr;
    socklen_t addr_size;

    char *ip = "127.0.0.1";
    int port = 5566;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0) {
        perror("Nao foi possivel criar o socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Socket de servidor criado com sucesso\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    int n = connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(n < 0) {
        perror("Nao foi possivel conectar ao servidor\n");
        exit(EXIT_FAILURE);
    }
    printf("Servidor conectado\n");
    // send message
    bzero(buffer, 1024); // zera o buffer
    strcpy(buffer, "test");
    printf("Client Message: %s\n", buffer);
    send(server_socket, buffer, strlen(buffer), 0);

    //receive response
    bzero(buffer, 1024);
    recv(server_socket, buffer, sizeof(buffer), 0);
    printf("Server Response: %s\n", buffer);

    close(server_socket);
    printf("Server disconnected\n");

    do {
        printf("==========================\n");
        printf("1 - Buscar cadastro\n");
        printf("2 - Inserir cadastro\n");
        printf("3 - Remover cadastro\n");
        printf("4 - sair\n");
        printf("==========================");
        scanf("%d", &op);
        switch(op) {
            case 1: // Buscar
            printf("Digite o ID para realizar a busca:\n");
            scanf("%d", &id_busca);

            break;

            case 2: // Inserir
            printf("Digite as informacoes de cadastro:\n");
            scanf("%s", buffer);

            break;

            case 3: // Remover
            break;

            case 4:
            printf("Saindo\n");
            break;

            default:
            printf("Opcao Invalida\n");
        }
    } while(op != 4);
    


}