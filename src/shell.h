#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include "parser.h"
#include "token.h"

#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define RED "\033[0;31m"
#define NO_COLOR "\033[0m"

typedef struct shell {
    char* curdir;
} Shell;

void start_shell();

#endif