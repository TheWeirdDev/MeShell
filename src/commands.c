#include "commands.h"

typedef int (*CmdFunc)(Token* args, Shell* sh, char* output);

typedef struct cmd_item {
    char* name;
    CmdFunc func;
} CmdItem;

static int cd(Token* args, Shell* sh, char* output) {
    // Temporary test
    sh->cwd = "Ok";
    return 0;
}

#define TOTAL_COMMANDS 1
static const CmdItem cmd_lookup_table[] = {{"cd", cd}};

static CmdFunc find_command(Token* cmd) {
    for (int i = 0; i < TOTAL_COMMANDS; ++i) {
        CmdItem ci = cmd_lookup_table[i];
        if (strcmp(cmd->text, ci.name) == 0) {
            return ci.func;
        }
    };
    return NULL;
}

void run_command(Token* tokens, Shell* sh, char** err) {
    if (tokens[0].type != COMMAND) {
        *err = "Error parsing the command";
        return;
    }

    CmdFunc func = find_command(tokens);
    if (func == NULL) {
        *err = "Command not found";
        return;
    }
    char out[50];
    func(tokens->next, sh, out);
}