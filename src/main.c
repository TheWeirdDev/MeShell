#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

#define VERSION 0.1

int main(int argc, char const *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0) {
            printf("MeShell version %0.1f\n", VERSION);
            exit(0);
        }
    }
    Shell shell;
    start_shell(&shell);
    free(shell.cwd);
    return 0;
}