#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

sem_t semaphore_server;

void *connection_handler(void *sock)
{
    int a;
    int client_socket = *(int *)sock;
    int read_size;
    char *await_cadastro = "Digite o id do cadastro a ser registrado: \0";
    char *await_busca = "Digite o id do cadastro a ser buscado: \0";
    char *success = "Cadastro feito com sucesso\0";
    char *not_found = "Cadastro nao encontrado\0";
    char *opcao_invalida = "Opcao invalida\0";
    char buffer[1024];
    char *myfifo = "/tmp/myfifo";
    FILE *shared_memory;
    int wfd, rfd;
    pid_t childpid;

    mkfifo(myfifo, 0666);

    bzero(buffer, 1024);
    while ((read_size = recv(client_socket, buffer, 1024, 0)) > 0)
    {
        sem_wait(&semaphore_server);
        if (!strcmp(buffer, "cadastrar"))
        {
            bzero(buffer, 1024);
            write(client_socket, await_cadastro, strlen(await_cadastro));
            if ((read_size = recv(client_socket, buffer, 1024, 0)) > 0)
            {
                if ((childpid = fork()) == -1)
                {
                    perror("Nao possivel criar o fork\n");
                    exit(EXIT_FAILURE);
                }
                if (childpid > 0)
                {
                    wfd = open(myfifo, O_WRONLY);
                    write(wfd, buffer, strlen(buffer));
                    close(wfd);
                }
                if (childpid == 0)
                {
                    char cadastro[1024];
                    rfd = open(myfifo, O_RDONLY);
                    
                    bzero(cadastro, 1024);
                    read(rfd, cadastro, 1024);
                    close(rfd);

                    int tamanho_cadastro = strlen(cadastro);
                    shared_memory = fopen("memoriacompartilhada.txt", "a+");

                    fwrite(&tamanho_cadastro, sizeof(int), 1, shared_memory);
                    fwrite(cadastro, strlen(cadastro), 1, shared_memory);
                    fclose(shared_memory);

                    write(client_socket, success, strlen(success));
                    exit(EXIT_SUCCESS);
                }
            }
        }
        else if (!strcmp(buffer, "buscar"))
        {
            char *field;
            int found = 0;
            sem_getvalue(&semaphore_server, &a);
            bzero(buffer, 1024);
            write(client_socket, await_busca, strlen(await_busca));
            if ((read_size = recv(client_socket, buffer, 1024, 0)) > 0)
            {
                int tam;
                char read_stream[1024], registro[1024];
                buffer[strlen(buffer) - 1] = '\0';
                shared_memory = fopen("memoriacompartilhada.txt", "r");
                rewind(shared_memory);
                
                while (fread(&tam, sizeof(int), 1, shared_memory) && !found)
                {
                    bzero(read_stream, 1024);
                    bzero(registro, 1024);
                    fread(read_stream, tam, 1, shared_memory);
                    strcpy(registro, read_stream);
                    field = strtok(registro, "|");
                    if (!strcmp(buffer, field))
                    {
                        found = 1;
                        write(client_socket, read_stream, strlen(read_stream));
                    }
                }
                if (!found)
                {
                    write(client_socket, not_found, strlen(not_found));
                }
                fclose(shared_memory);
            }
            else
            {
                write(client_socket, not_found, strlen(not_found));
            }
        }
        else
        {
            write(client_socket, opcao_invalida, strlen(opcao_invalida));
        }
        sem_post(&semaphore_server);
        bzero(buffer, 1024);
    }

    if (read_size == 0)
    {
        printf("Cliente desconectado\n");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("Falha ao receber resposta\n");
    }

    free(sock);
}

int main()
{
    int server_socket, client_socket, *new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    int fd[2];
    char buffer[1024];
    char *ip = "127.0.0.1";
    int port = 5566;
    sem_init(&semaphore_server, 0, 1);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Nao foi possivel fazer o socket do servidor\n");
        exit(EXIT_FAILURE);
    }
    printf("Socket de servidor criado com sucesso\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    int n = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        perror("Erro de conexao\n");
        exit(EXIT_FAILURE);
    }
    printf("Conexao com a porta: %d estabelecia com sucesso\n", port);

    listen(server_socket, 3); // acho que isso são os 3 servidores
    pipe(fd);
    printf("Listening...\n");

    while (1)
    {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        printf("Client conectado\n");
        pthread_t sniffer_thread;
        new_socket = malloc(1);
        *new_socket = client_socket;
        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_socket) < 0)
        {
            perror("Nao foi possivel criar thread\n");
            exit(EXIT_FAILURE);
        }
        printf("Conexao estabelecida\n");
    }

    return 0;
}