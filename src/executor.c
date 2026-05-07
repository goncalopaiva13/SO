#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "executor.h"

// Inicializa a struct de um comando.
// usados nos redirecionamentos (<, > e 2>).
static void limpar_comando(SimpleCommand *cmd) {
    int i = 0;

    while (i < MAX_ARGS) {
        cmd->argv[i] = NULL;
        i++;
    }

    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->error_file = NULL;
}

// Remove o '\n' do fim da string, se existir.
static void tirar_newline(char *s) {
    int i = 0;

    while (s[i] != '\0') {
        if (s[i] == '\n') {
            s[i] = '\0';
            return;
        }
        i++;
    }
}

// Faz o parsing de um comando simples, sem pipes.
// Separa os argumentos do comando e deteta redirecionamentos:
// <  -> ficheiro de entrada
// >  -> ficheiro de saída
// 2> -> ficheiro de erros
// Os operadores não ficam no argv, só o comando e argumentos.
static int parse_comando_simples(char *texto, SimpleCommand *cmd) {
    char *token;
    int i = 0;

    limpar_comando(cmd);

    token = strtok(texto, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) return -1;
            cmd->input_file = token;
        }
        else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) return -1;
            cmd->output_file = token;
        }
        else if (strcmp(token, "2>") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) return -1;
            cmd->error_file = token;
        }
        else {
            if (i >= MAX_ARGS - 1) return -1;
            cmd->argv[i] = token;
            i++;
        }

        token = strtok(NULL, " ");
    }

    cmd->argv[i] = NULL;

    if (cmd->argv[0] == NULL) return -1;

    return 0;
}

// Aplica os redirecionamentos do comando no processo filho.
// Abre os ficheiros indicados e usa dup2 para associar:
// stdin  ao ficheiro de input
// stdout ao ficheiro de output
// stderr ao ficheiro de erros
static int aplicar_redirecionamentos(SimpleCommand *cmd) {
    int fd;

    if (cmd->input_file != NULL) {
        fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) return -1;

        if (dup2(fd, STDIN_FILENO) < 0) {
            close(fd);
            return -1;
        }

        close(fd);
    }

    if (cmd->output_file != NULL) {
        fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) return -1;

        if (dup2(fd, STDOUT_FILENO) < 0) {
            close(fd);
            return -1;
        }

        close(fd);
    }

    if (cmd->error_file != NULL) {
        fd = open(cmd->error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) return -1;

        if (dup2(fd, STDERR_FILENO) < 0) {
            close(fd);
            return -1;
        }

        close(fd);
    }

    return 0;
}

// Executa um comando simples (sem pipes).
// Cria um filho com fork, aplica os redirecionamentos no filho
// e depois executa o comando com execvp.
// O processo pai espera que o filho termine.
static int executar_comando_simples(SimpleCommand *cmd) {
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0) {
        return -1;
    }

    if (pid == 0) {
        if (aplicar_redirecionamentos(cmd) < 0) {
            _exit(1);
        }

        execvp(cmd->argv[0], cmd->argv);
        _exit(1);
    }

    if (waitpid(pid, &status, 0) < 0) {
        return -1;
    }

    return 0;
}

// Divide uma linha de comando em várias partes usando o operador |
static int dividir_pipeline(char *linha, char *partes[]) {
    char *token;
    int n = 0;

    token = strtok(linha, "|");

    while (token != NULL) {
        if (n >= MAX_CMDS) return -1;
        partes[n] = token;
        n++;
        token = strtok(NULL, "|");
    }

    return n;
}

// Executa vários comandos ligados por pipes.
// Cria os pipes necessários, cria um processo para cada comando
// e liga a saída de um comando à entrada do seguinte.
// Também aplica redirecionamentos em cada comando, se existirem.
static int executar_pipeline(SimpleCommand comandos[], int n) {
    int pipes[MAX_CMDS][2];
    pid_t pids[MAX_CMDS];
    int i;
    int status;

    i = 0;
    while (i < n - 1) {
        if (pipe(pipes[i]) < 0) return -1;
        i++;
    }

    i = 0;
    while (i < n) {
        pids[i] = fork();

        if (pids[i] < 0) {
            return -1;
        }

        if (pids[i] == 0) {
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            if (i < n - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            int j = 0;
            while (j < n - 1) {
                close(pipes[j][0]);
                close(pipes[j][1]);
                j++;
            }

            if (aplicar_redirecionamentos(&comandos[i]) < 0) {
                _exit(1);
            }

            execvp(comandos[i].argv[0], comandos[i].argv);
            _exit(1);
        }

        i++;
    }

    i = 0;
    while (i < n - 1) {
        close(pipes[i][0]);
        close(pipes[i][1]);
        i++;
    }

    i = 0;
    while (i < n) {
        waitpid(pids[i], &status, 0);
        i++;
    }

    return 0;
}

// Recebe a linha de comando completa, remove o newline,
// divide por pipes, faz o parsing de cada comando
// e decide se executa um comando simples ou um pipeline.
int executar_linha_comando(char *linha) {
    char buffer[1024];
    char *partes[MAX_CMDS];
    SimpleCommand comandos[MAX_CMDS];
    int n;
    int i;

    strncpy(buffer, linha, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    tirar_newline(buffer);

    n = dividir_pipeline(buffer, partes);
    if (n <= 0) return -1;

    i = 0;
    while (i < n) {
        if (parse_comando_simples(partes[i], &comandos[i]) < 0) {
            return -1;
        }
        i++;
    }

    if (n == 1) {
        return executar_comando_simples(&comandos[0]);
    }

    return executar_pipeline(comandos, n);
}