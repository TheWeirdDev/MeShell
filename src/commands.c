#include "commands.h"

// Commands return true if everything goes well
typedef bool (*CmdFunc)(Token* args, Shell* sh);

typedef struct cmd_item {
    char* name;
    CmdFunc func;
} CmdItem;

static bool is_valid_name(char* name) {
    char invalid_chars[] = {'\\', '|', '<', '>'};
    for (int i = 0; i < 4; i++) {
        if (strchr(name, invalid_chars[i]) != NULL) {
            return false;
        }
    }
    return true;
}

static bool cd(Token* args, Shell* sh) {
    Token* t = args;
    char* dir = NULL;

    // There's no arg token or maybe there's a redirect
    if ((!t || t->type != ARG) || strcmp(t->text, "/") == 0) {
        // We always use heap for cwd
        sh->cwd = (char*)realloc(sh->cwd, sizeof(char) * 2);
        strcpy(sh->cwd, "/");
        sh->cwd_id = 0;
        return true;
    }

    if (strcmp(t->text, ".") == 0) {
        return true;
    }

    if (strcmp(t->text, "..") == 0) {
        if (sh->cwd_id == 0) {
            sh->last_cmd_error = "Root directory has no parent";
            return false;
        }
        int cwd_new;
        char* new = db_get_dir_parent(sh->sqldb->db, sh->cwd_id, &cwd_new);

        if (new != NULL) {
            free(sh->cwd);
            sh->cwd = new;
            sh->cwd_id = cwd_new;
        }
        return true;
    }

    if (t->type == ARG) {
        dir = t->text;
    }

    if (t->next != NULL && t->next->type == ARG) {
        sh->last_cmd_error = "Too many args for cd";
        return false;
    }

    if (dir != NULL) {
        if (!is_valid_name(dir)) {
            sh->last_cmd_error = "Invalid character found in directory name";
            return false;
        }

        int dir_len = strlen(dir);
        // Remove trailing '/' if exists
        if (dir_len > 1 && *(dir + dir_len - 1) == '/') {
            *(dir + dir_len - 1) = '\0';
            dir_len--;
        }

        // If dir doesn't start with '/' then it must be a relative path
        if (dir[0] != '/') {
            if (!check_dir_exists(sh->sqldb->db, sh->cwd_id, dir)) {
                sh->last_cmd_error = "Directory not found";
                return false;
            }
            int cwd_len = strlen(sh->cwd);
            // length + 2 for null terminated string and the '/'
            sh->cwd = (char*)realloc(sh->cwd, sizeof(char) * (dir_len + cwd_len + 2));
            // if cwd is root, no '/' is needed
            if (cwd_len != 1)
                strcat(sh->cwd, "/");
            strcat(sh->cwd, dir);
        } else {
            if (!check_full_path_exists(sh->sqldb->db, dir)) {
                sh->last_cmd_error = "Directory not found";
                return false;
            }
            // length + 1 for null terminated string
            sh->cwd = (char*)realloc(sh->cwd, sizeof(char) * (dir_len + 1));
            strcpy(sh->cwd, dir);
        }
        sh->cwd_id = db_get_dir_id(sh->sqldb->db, sh->cwd);
    } else {
        sh->last_cmd_error = "Unknown error happend.";
        return false;
    }

    return true;
}

static bool pwd(Token* args, Shell* sh) {
    if (args != NULL && args[0].type == ARG) {
        sh->last_cmd_error = "pwd does not accept arguments";
        return false;
    }
    sh->last_cmd_allocated = true;
    sh->last_cmd_output = (char*)malloc(sizeof(char) * (strlen(sh->cwd) + 1));
    strcpy(sh->last_cmd_output, sh->cwd);
    return true;
}

static bool exit_shell(Token* args, Shell* sh) {
    if (args != NULL && args[0].type == ARG) {
        sh->exit_code = atoi(args[0].text);
    }
    sh->closed = true;
    return true;
}

static bool echo(Token* args, Shell* sh) {
    if (args != NULL && args[0].type == ARG) {
        sh->last_cmd_output = (char*)malloc(sizeof(char) * (strlen(args[0].text) + 2));
        strcpy(sh->last_cmd_output, args[0].text);
        strcat(sh->last_cmd_output, " ");
    }
    Token* tok = args[0].next;
    while (tok && tok->type == ARG) {
        sh->last_cmd_output = (char*)realloc(sh->last_cmd_output,
                                             sizeof(char) * (strlen(sh->last_cmd_output) + strlen(tok->text) + 2));
        strcat(sh->last_cmd_output, tok->text);
        strcat(sh->last_cmd_output, " ");
        tok = tok->next;
    }
    sh->last_cmd_allocated = true;
    return true;
}

static bool mkdir(Token* args, Shell* sh) {
    if (args != NULL && args[0].type == ARG) {
        if (!is_valid_name(args[0].text) || args[0].text[0] == '/') {
            sh->last_cmd_error = "Invalid character found in directory name";
            return false;
        }
        if (check_dir_exists(sh->sqldb->db, sh->cwd_id, args[0].text) ||
            check_file_exists(sh->sqldb->db, sh->cwd_id, args[0].text)) {
            sh->last_cmd_error = "file or directory already exists";
            return false;
        }
        db_make_directory(sh->sqldb->db, sh->cwd_id, sh->cwd, args[0].text);
        return true;
    } else {
        sh->last_cmd_error = "mkdir needs one argument: 'directory name'";
        return false;
    }
}

static bool touch(Token* args, Shell* sh) {
    if (args != NULL && args[0].type == ARG) {
        if (!is_valid_name(args[0].text) || args[0].text[0] == '/') {
            sh->last_cmd_error = "Invalid character found in file name";
            return false;
        }
        if (check_dir_exists(sh->sqldb->db, sh->cwd_id, args[0].text) ||
            check_file_exists(sh->sqldb->db, sh->cwd_id, args[0].text)) {
            sh->last_cmd_error = "file or directory already exists";
            return false;
        }
        db_make_file(sh->sqldb->db, sh->cwd_id, sh->cwd, args[0].text);
        return true;
    } else {
        sh->last_cmd_error = "touch needs one argument: 'file name'";
        return false;
    }
}

static bool ls(Token* args, Shell* sh) {
    sh->last_cmd_allocated = true;
    if (!args || args[0].type != ARG) {
        sh->last_cmd_output = db_list_dir_contents(sh->sqldb->db, sh->cwd_id);

    } else {
        char* arg = args[0].text;
        if (arg[0] == '/') {
            if (check_file_exists(sh->sqldb->db, 0, arg + 1)) {
                sh->last_cmd_output = (char*)malloc(sizeof(char) * strlen(arg + 1));
                strcpy(sh->last_cmd_output, arg + 1);
            } else if (check_dir_path_exists(sh->sqldb->db, arg)) {
                sh->last_cmd_output = db_list_dir_contents(sh->sqldb->db, db_get_dir_id(sh->sqldb->db, arg));
            } else {
                sh->last_cmd_error = "No such file or directory";
                sh->last_cmd_allocated = false;
                return false;
            }
        } else {
            if (check_file_exists(sh->sqldb->db, sh->cwd_id, arg)) {
                sh->last_cmd_output = (char*)malloc(sizeof(char) * strlen(arg));
                strcpy(sh->last_cmd_output, arg);
            } else if (check_dir_exists(sh->sqldb->db, sh->cwd_id, arg)) {
                char* path = (char*)malloc(sizeof(char) * strlen(sh->cwd) + strlen(arg) + 2);
                strcpy(path, sh->cwd);
                if (strlen(sh->cwd) > 1)
                    strcat(path, "/");
                strcat(path, arg);

                sh->last_cmd_output = db_list_dir_contents(sh->sqldb->db, db_get_dir_id(sh->sqldb->db, path));
                free(path);

            } else {
                sh->last_cmd_error = "No such file or directory";
                sh->last_cmd_allocated = false;
                return false;
            }
        }
    }
    return true;
}

static void redirect_to_file(Shell* sh, int parent_id, char* name, char* contents) {
    if (check_dir_exists(sh->sqldb->db, sh->cwd_id, name)) {
        puts(RED "Error: can't redirect output to directory" NO_COLOR);
        return;
    }
    if (!check_file_exists(sh->sqldb->db, sh->cwd_id, name)) {
        if (!is_valid_name(name)) {
            puts(RED "Error: Invalid redirect file name" NO_COLOR);
            return;
        }
        db_make_file(sh->sqldb->db, sh->cwd_id, sh->cwd, name);
    }
    db_write_file_contents(sh->sqldb->db, sh->cwd_id, name, contents);
}

#define TOTAL_COMMANDS 7
static const CmdItem cmd_lookup_table[] = {{"cd", &cd},
                                           {"pwd", &pwd},
                                           {"exit", &exit_shell},
                                           {"mkdir", &mkdir},
                                           {"echo", &echo},
                                           {"touch", &touch},
                                           {"ls", &ls}};

static CmdFunc find_command(Token cmd) {
    for (int i = 0; i < TOTAL_COMMANDS; ++i) {
        CmdItem ci = cmd_lookup_table[i];
        if (strcmp(cmd.text, ci.name) == 0) {
            return ci.func;
        }
    }
    return NULL;
}

void run_command(Token* tokens, Shell* sh) {
    if (tokens == NULL || tokens[0].type != COMMAND) {
        printf(RED "Error: %s\n" NO_COLOR, "Error parsing the command");
        return;
    }

    CmdFunc func = find_command(tokens[0]);
    if (func == NULL) {
        puts(RED "Error: Command not found" NO_COLOR);
        return;
    }

    // Reset shell status
    sh->last_cmd_allocated = false;
    sh->last_cmd_output = NULL;
    sh->last_cmd_error = NULL;
    bool result = func(tokens->next, sh);

    if (!result && sh->last_cmd_error) {
        printf(RED "Error: %s\n" NO_COLOR, sh->last_cmd_error);
        return;
    }

    if (sh->last_cmd_output) {
        // Handle redirects
        Token* t = tokens->next;
        while (t != NULL && t->type != REDIRECT_FILE) {
            t = t->next;
        }
        // Did we find a redirect file?
        if (t != NULL) {
            redirect_to_file(sh, sh->cwd_id, t->text, sh->last_cmd_output);
        } else {
            printf("%s\n", sh->last_cmd_output);
        }
        if (sh->last_cmd_allocated)
            free(sh->last_cmd_output);
    }
}