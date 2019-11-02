#ifndef SHELL_H
#define SHELL_H

typedef struct shell {
    char* cwd;
} Shell;

#include <stdbool.h>
#include <stdio.h>
#include "commands.h"
#include "token.h"

#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define RED "\033[1;31m"
#define NO_COLOR "\033[0m"

void start_shell(Shell*);

#endif