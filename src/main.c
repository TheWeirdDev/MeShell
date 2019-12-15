#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "db.h"
#include "shell.h"

#define VERSION 0.1

static inline char *get_database_file_name();

int main(int argc, char const *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0) {
            printf("MeShell version %0.1f\n", VERSION);
            exit(0);
        } else {
            printf("Warning: uknown argument '%s'\n", argv[1]);
        }
    }
    char *db_name = get_database_file_name();
    database meshell_db = {
        .name = db_name,
        .db = NULL};
    init_db(&meshell_db);

    Shell shell = {.closed = false, .exit_code = 0, .sqldb = &meshell_db, .cwd_id = 0};
    start_shell(&shell);

    free(shell.cwd);
    free(db_name);
    close_db(&meshell_db);
    return 0;
}

static inline char *get_home_dir() {
    struct passwd *pw = getpwuid(getuid());
    return pw->pw_dir;
}

static inline char *get_database_file_name() {
    char *home = get_home_dir();
    // + 13 for '/me_shell.db'
    char *name = (char *)malloc(sizeof(char) * (strlen(home) + 13));
    strcpy(name, home);
    strcat(name, "/me_shell.db");
    return name;
}
