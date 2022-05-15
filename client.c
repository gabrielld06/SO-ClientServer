#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define b17 bzero
int main()
{
    int op, id_busca;
    char buffer[1024];
    char field[512];
    char *campo;
    char id[4];

    int server_socket;
    struct sockaddr_in server_addr;
    socklen_t addr_size;

    char *ip = "127.0.0.1";
    int port = 5566;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Nao foi possivel criar o socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Socket de servidor criado com sucesso\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    int n = connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (n < 0)
    {
        perror("Nao foi possivel conectar ao servidor\n");
        exit(EXIT_FAILURE);
    }
    printf("Servidor conectado\n");
    // send message
    // bzero(buffer, 1024); // zera o buffer
    // strcpy(buffer, "test");
    // printf("Client Message: %s\n", buffer);
    // send(server_socket, buffer, strlen(buffer), 0);

    // // receive response
    // bzero(buffer, 1024);
    // recv(server_socket, buffer, sizeof(buffer), 0);
    // printf("Server Response: %s\n", buffer);

    // close(server_socket);
    // printf("Server disconnected\n");

    char *buscar = "buscar";
    char *cadastrar = "cadastrar";
    char server_response[1024];

    do
    {
        printf("==========================\n");
        printf("1 - Buscar cadastro\n");
        printf("2 - Inserir cadastro\n");
        printf("3 - sair\n");
        printf("==========================\n");
        printf("Insira a opcao: ");
        scanf("%d", &op);
        fgetc(stdin);
        fflush(stdin);
        switch (op)
        {
        case 1: // Buscar
            if (send(server_socket, buscar, strlen(buscar), 0) < 0)
            {
                perror("Falha ao enviar requisicao\n");
                exit(EXIT_FAILURE);
            }

            if (recv(server_socket, server_response, 1024, 0) < 0)
            {
                printf("falhou\n");
                break;
            }

            printf("%s\n", server_response);
            printf("%d\n", strcmp(server_response, "Entre com o id a ser buscado: "));
            if (!strcmp(server_response, "Entre com o id a ser buscado: "))
            {
                bzero(server_response, 1024);
                fflush(stdin);
                fgets(id, 4, stdin);
                if (send(server_socket, id, strlen(id), 0) < 0)
                {
                    perror("Falha ao enviar requisicao\n");
                    exit(EXIT_FAILURE);
                }

                if (recv(server_socket, server_response, 1024, 0) < 0)
                {
                    puts("recv falhou\n");
                    break;
                }

                bzero(id, 4);

                // puts(server_response);
                printf("\n");
                campo = strtok(server_response, "|");
                printf("Id: %s\n", campo);
                campo = strtok(NULL, "|");
                printf("Nome: %s\n", campo);
                campo = strtok(NULL, "|");
                printf("Sobrenome: %s\n", campo);
                campo = NULL;
                bzero(server_response, 1024);
            }
            break;

        case 2:
            printf("fon1\n");
            if (send(server_socket, cadastrar, strlen(cadastrar), 0) < 0)
            {
                perror("Falha ao enviar requisicao\n");
                exit(EXIT_FAILURE);
            }
            bzero(server_response, 1024);
            if (recv(server_socket, server_response, 1024, 0) < 0)
            {
                printf("falhou\n");
                break;
            }

            printf("|%s|\n", server_response);
            printf("|%d|\n", strcmp(server_response, "Entre com o id a ser cadastrado: "));
            if (!strcmp(server_response, "Entre com o id a ser cadastrado: "))
            {

                bzero(server_response, 1024);
                bzero(buffer, 1024);

                fflush(stdin);

                fgets(field, 4, stdin);
                printf("%s\n", field);
                field[strlen(field) - 1] = '\0';
                strcat(buffer, field);
                bzero(field, 512);

                printf("Entre com o nome: \n");
                fflush(stdin);
                fgets(field, 512, stdin);
                field[strlen(field) - 1] = '\0';
                strcat(buffer, "|");
                strcat(buffer, field);
                bzero(field, 512);

                printf("Entre com o sobrenome: \n");
                fflush(stdin);
                fgets(field, 512, stdin);
                field[strlen(field) - 1] = '\0';
                strcat(buffer, "|");
                strcat(buffer, field);
                bzero(field, 512);

                if (send(server_socket, buffer, strlen(buffer), 0) < 0)
                {
                    perror("Falha ao enviar requisicao\n");
                    exit(EXIT_FAILURE);
                }

                if (recv(server_socket, server_response, 1024, 0) < 0)
                {
                    printf("recv falhou\n");
                    break;
                }
                b17(buffer, 1024);
                printf("%s\n", server_response);
                b17(server_response, 1024);
            }

            break;

        case 3:
            printf("Saindo\n");
            break;

        default:
            printf("Opcao Invalida\n");
        }
    } while (op != 3);
}