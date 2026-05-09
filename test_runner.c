#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "include/defs.h"

 
//      ./test_runner user1 — runner do utilizador 1
//      ./test_runner user2 — runner do utilizador 2


int main(int argc, char* argv[]){

    // cria o FIFO privado antes de enviar o pedido
    char fifo_runner_name[64];
    sprintf(fifo_runner_name, FIFO_RUNNER, getpid());
    mkfifo(fifo_runner_name, 0600);

    // envia pedido de execucao ao controller
    sleep(5);//para assim no teste do RR(ver nota dos testes abaixo do codigo) haver tempo do  segundo runner do user 1 e o runner do user 2 entrarem ambos na fila de espera 
    int fd = open(FIFO_CONTROLLER, O_WRONLY);
    Msg msg;
    msg.operacao = 'e';
    msg.runner_pid = getpid();
    strcpy(msg.user_id, argv[1]); // adicionado para o teste de ROUND ROBIN
    write(fd, &msg, sizeof(Msg));
    close(fd);

    // espera resposta(autorização) do controller
    int fd_runner = open(fifo_runner_name, O_RDONLY);
    Resposta r;
    read(fd_runner, &r, sizeof(Resposta));
    printf("Resposta do controller: %d\n", r.autorizado);
    close(fd_runner);


    // simula execução
    sleep(20);

    // avisa controller que terminou
    msg.operacao = 'f';
    int fd2 = open(FIFO_CONTROLLER, O_WRONLY);

    write(fd2, &msg, sizeof(Msg));
    close(fd2);

    unlink(fifo_runner_name);
    return 0;
}



/* 


////Cenário de teste para o FIFO//////

Terminal 1: ./controller 1
Terminal 2: ./test_runner
Terminal 3: ./test_runner (logo a seguir)


O segundo runner deve ficar bloqueado à espera até o primeiro fazer o sleep(3) e enviar o 'f'



////Cenário de teste para o ROUND ROBIN/////// 

Tem de se ter max_paralelo = 1 
para o Round Robin ser visível.

Se fosse  max_paralelo = 3 e só 3 runners, todos são autorizados
imediatamente sem ir para a fila e o Round Robin nunca entra em ação.
Com max_paralelo = 1 só 1 executa de cada vez, os outros vão para a fila ,
e aí o Round Robin decide a ordem.


Terminal 1: ./controller 1 1 (Round Robin)
Terminal 2: ./test_runner user1
Terminal 3: ./test_runner user1
Terminal 4: ./test_runner user2


O segundo a ser autorizado deve ser o user2 em vez do segundo user1 
para confirmar que o Round Robin está a funcionar








*/