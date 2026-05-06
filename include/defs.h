#define FIFO_CONTROLLER "/tmp/controller_fifo"
#define FIFO_RUNNER "/tmp/runner_%d"  // %d = PID do runner

#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct msg{ // msg é o que o runner envia ao controller via FIFO
    char operacao;    // 'e' executar, 'c' consultar, 's' terminar, 'f' avisar controller que terminou
    int runner_pid;
    char user_id[32];
    char comando[256];
} Msg;


typedef struct resposta{
    int tipo; // 1 = executar comando 2 = executar terminar
    int autorizado;  // 1 = podes executar
} Resposta;



typedef struct comando{ // comando é o que o controller guarda internamente para gerir a fila de espera, a lista de comandos e o log
    int runner_pid;
    char user_id[32];
    char comando[256];
    struct timeval inicio;
} Comando;

