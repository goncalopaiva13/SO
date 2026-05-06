#include "defs.h"
#include <sys/wait.h>

/* ===================== PROTÓTIPOS ===================== */

void modo_exec(char *argv[]);
void modo_consulta();
void modo_terminar();

void enviar_pedido(Msg msg);
void esperar_autorizacao(Msg msg, char *mypipe);
void correr_comando(char *comando);
void notificar_concluido(Msg msg);

Msg fazer_Msg(char operacao, int runner_pid, char *user_id, char *comando);

/* ===================== MAIN ===================== */

int main(int argc, char* argv[]){

    if(argc == 4 && strcmp(argv[1],"-e")==0)
        modo_exec(argv);
    else if (argc == 2 && strcmp(argv[1], "-c") == 0)
        modo_consulta();
    else if (argc == 2 && strcmp(argv[1], "-s") == 0)
        modo_terminar();
    else {
        write(2,"Invalid option or arguments\n",15);
        return 1;
    }
    
    return 0;
}




void modo_exec(char *argv[]){

    int runner_pid = getpid();
    char* user_id = argv[2];
    char* comando = argv[3];
    char mypipe[50];

    sprintf(mypipe, FIFO_RUNNER, runner_pid);

    unlink(mypipe);
    mkfifo(mypipe, 0666);

    Msg msg = fazer_Msg('e', runner_pid, user_id, comando);

    enviar_pedido(msg);
    esperar_autorizacao(msg, mypipe);
    correr_comando(msg.comando);
    notificar_concluido(msg);

    char tmsg[100];
    sprintf(tmsg, "[runner] command %d finished\n", runner_pid);
    write(1, tmsg, strlen(tmsg));
}

void enviar_pedido(Msg msg) {
    int fd = open(FIFO_CONTROLLER, O_WRONLY);

    write(fd, &msg, sizeof(msg));
    close(fd);

    char tmsg[100];
    sprintf(tmsg, "[runner] command %d submitted\n", msg.runner_pid);
    write(1, tmsg, strlen(tmsg));
}

void esperar_autorizacao(Msg msg, char *mypipe) {
    int fd = open(mypipe, O_RDONLY);

    Resposta r;
    read(fd, &r, sizeof(r));

    if (r.tipo == 1 && r.autorizado) {
        char tmsg[100];
        sprintf(tmsg, "[runner] executing command %d...\n", msg.runner_pid);
        write(1, tmsg, strlen(tmsg));
    }

    close(fd);
}

void correr_comando(char *comando) {
    char cmd_copy[256];
    strcpy(cmd_copy, comando);

    char *args[100];
    int i = 0;

    char *token = strtok(cmd_copy, " ");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    pid_t pid = fork();

    if (pid == 0) {
        execvp(args[0], args);

        write(2, "exec failed\n", 12);
        _exit(1);
    } 
    else {
        wait(NULL);
    }
}

void notificar_concluido(Msg msg) {
    int fd = open(FIFO_CONTROLLER, O_WRONLY);

    if (fd < 0) {
        write(2, "Error opening controller pipe\n", 30);
        return;
    }
    msg.operacao = 'f';

    write(fd, &msg, sizeof(msg));
    close(fd);
}




void modo_consulta() {
    char mypipe[50];
    sprintf(mypipe, FIFO_RUNNER, getpid());

    unlink(mypipe);
    mkfifo(mypipe, 0666);

    int fd = open(FIFO_CONTROLLER, O_WRONLY);
    
    int runner_pid = getpid();

    Msg msg = fazer_Msg('c', runner_pid, "", "");

    write(fd, &msg, sizeof(msg));
    close(fd);

    int fd2 = open(mypipe, O_RDONLY);

    char buffer[1024];
    int n;

    while ((n = read(fd2, buffer, sizeof(buffer))) > 0) {
        write(1, buffer, n);  
    }

    close(fd2);
}





void modo_terminar() {
    char mypipe[50];
    sprintf(mypipe, FIFO_RUNNER, getpid());

    unlink(mypipe);
    mkfifo(mypipe, 0666);

    write(1, "[runner] sent shutdown notification\n", 36);

    int fd = open(FIFO_CONTROLLER, O_WRONLY);

    int runner_pid = getpid();
    Msg msg = fazer_Msg('s', runner_pid, "", "");

    write(fd, &msg, sizeof(msg));
    close(fd);

    write(1, "[runner] waiting for controller to shutdown...\n", 48);

    int fd2 = open(mypipe, O_RDONLY);

    Resposta r;
    read(fd2, &r, sizeof(r));

    if (r.tipo == 2) {
        write(1, "[runner] controller exited.\n", 28);
    }

    close(fd2);
}

Msg fazer_Msg (char operacao, int runner_pid, char* user_id, char* comando){
    Msg msg;
    msg.operacao = operacao;
    strcpy(msg.comando, comando);
    msg.runner_pid = runner_pid;
    strcpy(msg.user_id, user_id);
    return msg;
}