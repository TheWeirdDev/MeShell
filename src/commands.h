#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include "shell.h"
#include "token.h"

void run_command(Token* tokens, Shell* sh, char** err);

#endif