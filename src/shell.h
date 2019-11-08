#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct shell {
    char* cwd;
    bool closed;
    int exit_code;
} Shell;

#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define RED "\033[1;31m"
#define NO_COLOR "\033[0m"

void start_shell(Shell*);

#endif