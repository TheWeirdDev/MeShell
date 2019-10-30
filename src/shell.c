#include "shell.h"

void start_shell() {
    Shell sh;
    sh.curdir = "/";
    while (1) {
        char line[100];
        printf(YELLOW "MeShell " NO_COLOR "%s" GREEN " > " NO_COLOR, sh.curdir);
        char* readl = fgets(line, 100, stdin);

        // Check for EOF (Ctrl-D)
        if (readl == NULL) break;
        // Empty input
        if (*readl == '\n') continue;

        if (strchr(readl, '%') != NULL) {
            puts(RED "Error: Character '%' is not allowed" NO_COLOR);
            continue;
        }
        // Remove trailing newline
        readl[strcspn(readl, "\r\n")] = 0;

        Token* tokens = NULL;
        char* err = NULL;

        int size = tokenize(readl, &tokens, &err);
        if (size == 0) {
            if (err) {
                printf(RED "%s\n" NO_COLOR, err);
                free(err);
            }
            continue;
        }

        parse_input(tokens);

        Token* t = tokens;
        while (t->next) {
            Token* temp = t->next;
            free(t->text);
            free(t);
            t = temp;
        }
    }
}