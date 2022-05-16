#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define b17 bzero
int main()
{
    int op, id_busca;
    char buffer[1024], cadastro[512];
    char *field;
    char id[512];

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
        fflush(stdin);
        scanf("%d", &op);
        fgetc(stdin);
        if (op == 1)
        {
            if (send(server_socket, buscar, strlen(buscar), 0) < 0)
            {
                perror("Falha ao enviar requisicao\n");
                exit(EXIT_FAILURE);
            }

            bzero(server_response, 1024);
            if (recv(server_socket, server_response, 1024, 0) < 0)
            {
                printf("Falha ao receber resposta\n");
                break;
            }

            printf("%s\n", server_response);

            if (!strcmp(server_response, "Digite o id do cadastro a ser buscado: "))
            {
                bzero(id, 512);
                fgets(id, 512, stdin);

                if (send(server_socket, id, strlen(id), 0) < 0)
                {
                    perror("Falha ao enviar requisicao\n");
                    exit(EXIT_FAILURE);
                }

                bzero(server_response, 1024);
                if (recv(server_socket, server_response, 1024, 0) < 0)
                {
                    perror("Falha ao receber resposta\n");
                    break;
                }

                if (!strcmp(server_response, "Cadastro nao encontrado"))
                {
                    printf("%s\n", server_response);
                }
                else
                {
                    field = strtok(server_response, "|");
                    printf("Id: %s\n", field);
                    field = strtok(NULL, "|");
                    printf("Nome: %s\n", field);
                    field = strtok(NULL, "|");
                    printf("Sobrenome: %s\n", field);
                    field = NULL;
                    bzero(server_response, 1024);
                }
            }
        }
        else if (op == 2)
        {
            if (send(server_socket, cadastrar, strlen(cadastrar), 0) < 0)
            {
                perror("Falha ao enviar requisicao\n");
                exit(EXIT_FAILURE);
            }

            bzero(server_response, 1024);
            if (recv(server_socket, server_response, 1024, 0) < 0)
            {
                printf("Falha ao receber resposta\n");
                break;
            }

            printf("%s\n", server_response);

            if (!strcmp(server_response, "Digite o id do cadastro a ser registrado: "))
            {
                bzero(buffer, 1024);

                fgets(cadastro, 512, stdin);
                cadastro[strlen(cadastro) - 1] = '\0';
                strcat(buffer, cadastro);

                printf("Digite o nome: \n");
                fgets(cadastro, 512, stdin);
                cadastro[strlen(cadastro) - 1] = '\0';
                strcat(buffer, "|");
                strcat(buffer, cadastro);

                printf("Digite o sobrenome: \n");
                fgets(cadastro, 512, stdin);
                cadastro[strlen(cadastro) - 1] = '\0';
                strcat(buffer, "|");
                strcat(buffer, cadastro);
                
                if (send(server_socket, buffer, strlen(buffer), 0) < 0)
                {
                    perror("Falha ao enviar requisicao\n");
                    exit(EXIT_FAILURE);
                }

                bzero(server_response, 1024);
                if (recv(server_socket, server_response, 1024, 0) < 0)
                {
                    printf("Falha ao receber resposta\n");
                    break;
                }

                printf("%s\n", server_response);

                bzero(buffer, 1024);
                bzero(server_response, 1024);
            }
        }
        else if (op == 3)
        {
            printf("Saindo\n");
        }
        else
        {
            printf("Opcao Invalida\n");
        }
    } while (op != 3);
}