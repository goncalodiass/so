#include "srvfile.h"

int main() {

    // Solicitar o nome do arquivo ao usuário
    char filename[MAX_FILE];
    printf("File? ");
    if (fgets(filename, MAX_FILE, stdin) == NULL) {
        fprintf(stderr, "Erro ao ler o nome do arquivo.\n");
        exit(EXIT_FAILURE);
    }
    // Remover o caractere de nova linha do input
    filename[strcspn(filename, "\n")] = '\0';
    
    // Verificar se o nome do arquivo é inválido
    if (strlen(filename) == 0 || strstr(filename, "..") || strchr(filename, '/')) {
        fprintf(stderr, "Erro: Nome de arquivo inválido.\n");
        exit(EXIT_FAILURE);
    }


    // Criar o FIFO do cliente
    char client_fifo[20];
    snprintf(client_fifo, sizeof(client_fifo), "/tmp/FIFO_%d", getpid());
    if (mkfifo(client_fifo, 0666) == -1) { 
        if (errno != EEXIST) {
            perror("Erro ao criar FIFO do cliente");
            exit(EXIT_FAILURE);
        }
    }
    
    
    // Registrar o FIFO para remoção no encerramento
//    atexit(() -> { unlink(client_fifo); });
    

    // Enviar requisição ao servidor
    int server_fd = open(SERVER_FIFO, O_WRONLY);
    if (server_fd == -1) {
        fprintf(stderr, "Erro: Não foi possível conectar ao servidor. Certifique-se de que ele está em execução.\n");
        unlink(client_fifo);
        exit(EXIT_FAILURE);
    }

    struct t_request request;
    request.pid = getpid();
    strncpy(request.n_file, filename, MAX_FILE);
    exit_on_error(write(server_fd, &request, sizeof(request)), "Erro ao escrever no FIFO do servidor");
    close(server_fd);


    // Ler a resposta do servidor
    int client_fd = open(client_fifo, O_RDONLY);
    if (client_fd == -1) {
        perror("Erro ao abrir FIFO do cliente");
        unlink(client_fifo);
        exit(EXIT_FAILURE);
	}

    char buffer[MAX_BUFFER];
    ssize_t bytes_read;
    

    while ((bytes_read = read(client_fd, buffer, MAX_BUFFER)) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
/*
    if (file_found == 0) {
        printf("O arquivo solicitado não foi encontrado ou está vazio.\n");
    }
*/
    close(client_fd);
    unlink(client_fifo);
    return 0;
}

