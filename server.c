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

void *connection_handler(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    int read_size;
    int a;
    int found = 0;
    char *insercao_ok = "Novo cadastro registrado\n";
    char *requsicao_desconhecida = "Comando desconhecido\n";
    char *solicita_cadastro = "Entre com o id a ser cadastrado: \0";
    char *solicita_busca = "Entre com o id a ser buscado: \0";
    char *nenhum_cadastro = "Nenhum cadastro recebido\n";
    char *message, client_message[1024], save_buffer[1024];
    char new_register[1024];
    char *field;
    char *myfifo = "/tmp/myfifo";
    FILE *fp;
    int fd1, fd2;
    pid_t childpid;

    mkfifo(myfifo, 0666);

    while ((read_size = recv(sock, client_message, 1024, 0)) > 0)
    {

        sem_wait(&semaphore_server);
        if (!strcmp(client_message, "cadastrar"))
        {
            bzero(client_message, 1024);
            // memset(client_message, '\0', strlen(client_message));
            printf("|%s|", solicita_cadastro);
            write(sock, solicita_cadastro, strlen(solicita_cadastro));
            if ((read_size = recv(sock, client_message, 1024, 0)) > 0)
            {
                printf("fon17\n");
                if ((childpid = fork()) == -1)
                {
                    perror("FORK FAIL\n");
                    exit(EXIT_FAILURE);
                }

                if (childpid > 0)
                {

                    client_message[strlen(client_message) - 1] = '\0';
                    fd1 = open(myfifo, O_WRONLY);
                    write(fd1, client_message, strlen(client_message));
                    close(fd1);
                }
                if (childpid == 0)
                {
                    fd2 = open(myfifo, O_RDONLY);
                    read(fd2, new_register, 1024);
                    close(fd2);
                    int size_register = strlen(new_register);
                    fp = fopen("memoriacompartilhada.txt", "a+");
                    fwrite(&size_register, sizeof(int), 1, fp);
                    fwrite(new_register, strlen(new_register), 1, fp);
                    fclose(fp);
                    memset(new_register, '\0', strlen(new_register));
                    write(sock, insercao_ok, strlen(insercao_ok));
                    exit(0);
                }
                memset(client_message, '\0', strlen(client_message));
            }
            else
            {
                printf("fon13\n");
            }
        }
        else if (!strcmp(client_message, "buscar"))
        {
            printf("fon1\n");
            sem_getvalue(&semaphore_server, &a);
            memset(client_message, '\0', strlen(client_message));
            write(sock, solicita_busca, strlen(solicita_busca));
            if ((read_size = recv(sock, client_message, 1024, 0)) > 0)
            {
                int tam;
                char read_buffer[1024];
                client_message[strlen(client_message) - 1] = '\0';
                fp = fopen("memoriacompartilhada.txt", "r");
                rewind(fp);
                found = 0;
                while (fread(&tam, sizeof(int), 1, fp) && !found)
                {
                    fread(read_buffer, tam, 1, fp);
                    strcpy(save_buffer, read_buffer);
                    field = strtok(save_buffer, "|");
                    if (!strcmp(client_message, field))
                    {
                        // printf("%s\n", read_buffer);
                        found = 1;
                        write(sock, read_buffer, strlen(read_buffer));
                    }
                    memset(read_buffer, '\0', strlen(read_buffer));
                    memset(save_buffer, '\0', strlen(save_buffer));
                }
                if (!found)
                {
                    write(sock, nenhum_cadastro, strlen(nenhum_cadastro));
                }
                memset(client_message, '\0', strlen(client_message));
                fclose(fp);
            }
            else
            {
                write(sock, nenhum_cadastro, strlen(nenhum_cadastro));
            }
        }
        else
        {
            write(sock, requsicao_desconhecida, strlen(requsicao_desconhecida));
        }
        sem_post(&semaphore_server);
    }

    if (read_size == 0)
    {
        puts("Cliente desconectado\n");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("recv falhou\n");
    }

    free(socket_desc);

    return 0;
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

        // // receive client message
        // bzero(buffer, 1024); // zera o buffer
        // recv(client_socket, buffer, sizeof(buffer), 0);
        // printf("Client message: %s\n", buffer);

        // // send response
        // bzero(buffer, 1024);
        // strcpy(buffer, "fon");
        // printf("Server Response: %s\n", buffer);
        // send(client_socket, buffer, strlen(buffer), 0);

        // close(client_socket);
        // printf("client disconnected\n");
    }

    return 0;
}