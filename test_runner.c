#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "defs.h"

int main(){

    // cria o FIFO privado antes de enviar o pedido
    char fifo_runner_name[64];
    sprintf(fifo_runner_name, FIFO_RUNNER, getpid());
    mkfifo(fifo_runner_name, 0600);

    // envia pedido de execucao ao controller
    int fd = open(FIFO_CONTROLLER, O_WRONLY);
    Msg msg;
    msg.operacao = 'e';
    msg.runner_pid = getpid();
    write(fd, &msg, sizeof(Msg));
    close(fd);

    // espera resposta(autorização) do controller
    int fd_runner = open(fifo_runner_name, O_RDONLY);
    Resposta r;
    read(fd_runner, &r, sizeof(Resposta));
    printf("Resposta do controller: %d\n", r.autorizado);
    close(fd_runner);


    // simula execução
    sleep(10);

    // avisa controller que terminou
    msg.operacao = 'f';
    int fd2 = open(FIFO_CONTROLLER, O_WRONLY);
    write(fd2, &msg, sizeof(Msg));
    close(fd2);

    unlink(fifo_runner_name);
    return 0;
}



/* Cenário de teste

Terminal 1: ./controller 1
Terminal 2: ./test_runner
Terminal 3: ./test_runner (logo a seguir)

O segundo runner deve ficar bloqueado à espera até o primeiro fazer o sleep(3) e enviar o 'f'

*/