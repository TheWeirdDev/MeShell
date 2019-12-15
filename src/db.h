#ifndef DB_HELPER_H
#define DB_HELPER_H

#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct db_helper {
    char* name;
    sqlite3* db;

} database;

void init_db(database*);
void close_db(database*);
bool check_dir_exists(sqlite3*, int, char*);
bool check_full_path_exists(sqlite3*, char*);
void db_make_directory(sqlite3*, int, char*, char*);
int db_get_dir_id(sqlite3*, char*);
char* db_get_dir_parent(sqlite3*, int, int*);
#endif