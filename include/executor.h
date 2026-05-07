#ifndef EXECUTOR_H
#define EXECUTOR_H

#define MAX_ARGS 64
#define MAX_CMDS 16

typedef struct simple_command {
    char *argv[MAX_ARGS];
    char *input_file;
    char *output_file;
    char *error_file;
} SimpleCommand;

int executar_linha_comando(char *linha);

#endif