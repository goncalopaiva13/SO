#define FIFO_CONTROLLER "/tmp/controller_fifo"
#define FIFO_RUNNER "/tmp/runner_%d"  // %d = PID do runner

#include <sys/time.h>

typedef struct msg{ // msg é o que o runner envia ao controller via FIFO
    char operacao;    // 'e' executar, 'c' consultar, 's' terminar, 'f' avisar controller que terminou
    int runner_pid;
    char user_id[32];
    char comando[256];
} Msg;


typedef struct resposta{
    int autorizado;  // 1 = podes executar
} Resposta;



typedef struct comando{ // comando é o que o controller guarda internamente para gerir a fila de espera, a lista de comandos e o log
    int runner_pid;
    char user_id[32];
    char comando[256];
    struct timeval inicio;
} Comando;

