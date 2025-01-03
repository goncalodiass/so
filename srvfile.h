#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // 
#include <errno.h> // 

#define exit_on_error(s,m) if (s < 0) { perror(m); exit(1); }

/*Nome do FIFO*/
#define SERVER_FIFO "SRVFIFO"
/*Tam. Máximo para nome de ficheiro*/
#define MAX_FILE 50
/*Tam. do buffer*/
#define MAX_BUFFER  100

/*Estrutura de dados para pedido ao servidor*/
struct t_request {
    int pid;
    char n_file[MAX_FILE];
};
