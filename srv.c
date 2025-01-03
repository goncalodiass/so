#include "srvfile.h"
#include <signal.h>
#include <sys/wait.h>

// Função para lidar com o sinal SIGINT (Ctrl+C)
void handle_sigint(int signo) {
    unlink(SERVER_FIFO);
    exit(EXIT_SUCCESS);
}

// Função para lidar com a coleta de processos filhos zumbis
void reap_children(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void send_file_to_client(const char *file_name, int client_fd) {
    int file_fd = open(file_name, O_RDONLY);
    if (file_fd == -1) {
        char error_message[MAX_BUFFER];
        snprintf(error_message, sizeof(error_message), "Erro: Arquivo [%s] não encontrado.\n", file_name);
        write(client_fd, error_message, strlen(error_message));
        close(client_fd);
        return;
    }

    char buffer[MAX_BUFFER];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, MAX_BUFFER)) > 0) {
        write(client_fd, buffer, bytes_read);
    }

    close(file_fd);
    close(client_fd);
}



int main() {
    // Remover FIFO existente de execuções passadas mal sucedidas
    unlink(SERVER_FIFO); // ADD

    // Create server FIFO
    exit_on_error(mkfifo(SERVER_FIFO, 0666), "Erro ao criar FIFO do servidor");

    // Configurar sinais para lidar com interrupções e processos filhos
    signal(SIGINT, handle_sigint);
    signal(SIGCHLD, reap_children);

    // Abrir FIFO do servidor em modo de leitura
    int server_fd = open(SERVER_FIFO, O_RDONLY);
    exit_on_error(server_fd, "Erro ao abrir FIFO do servidor");


    struct t_request request;

    while (1) {
        // Ler requisição do FIFO do servidor
        if (read(server_fd, &request, sizeof(request)) > 0) {
            printf("PID:%d File: [%s]\n", request.pid, request.n_file);
            
            // Criar processo filho para tratar a requisição
            pid_t pid = fork();
            if (pid == 0) { // Processo filho
                char client_fifo[20];
                snprintf(client_fifo, sizeof(client_fifo), "/tmp/FIFO_%d", request.pid);
                
                // Abrir FIFO do cliente em modo de escrita
                int client_fd = open(client_fifo, O_WRONLY);
                if (client_fd == -1) {
                    perror("Erro ao abrir FIFO do cliente");
                    exit(EXIT_FAILURE);
                }


/*
                // Abrir arquivo solicitado pelo cliente
                int file_fd = open(request.n_file, O_RDONLY);
                if (file_fd == -1) {
                    //perror("open requested file");
                    //write(client_fd, "perror("open requested file");", 58);
                    //char error_message[256];
                    //snprintf(error_message, sizeof(error_message), "Erro: %s\n", strerror(errno));
                    perror("Erro");
                    //write(client_fd, error_message, strlen(error_message));
                    //write(client_fd, "ERRO: Arquivo não encontrado.\n", 31);
                    //fprintf(stderr, "Erro: Arquivo [%s] não encontrado.\n", request.n_file);
                    //write(client_fd, "Erro: Arquivo não encontrado.\n", 31);

                    close(client_fd);
                    exit(EXIT_FAILURE);
                }

                // Ler o conteúdo do arquivo e enviar para o cliente
                char buffer[MAX_BUFFER];
                ssize_t bytes_read;
                while ((bytes_read = read(file_fd, buffer, MAX_BUFFER)) > 0) {
                    write(client_fd, buffer, bytes_read);
                }

                // Fechar descritores de arquivo e encerrar o processo filho
                close(file_fd);
                close(client_fd);*/

                send_file_to_client(request.n_file, client_fd);

                exit(EXIT_SUCCESS);
            } 

       /* } else { // ADD
            if (errno == EINTR) {
                continue; // Interrompido por um sinal, continuar
            } else {
                perror("Erro ao ler do FIFO do servidor");
                break;
            }
        */
	}
    }

    // Fechar descritor de arquivo do servidor e remover FIFO do servidor
    close(server_fd);
    unlink(SERVER_FIFO);
    return 0;
}

