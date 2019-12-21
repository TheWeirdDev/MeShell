#include "db.h"

static void open_db(char* name, sqlite3** db) {
    if (sqlite3_open(name, db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
        exit(1);
    }
}

static void execute_query(sqlite3* db, char* sql) {
    char* error_msg = 0;

    if (sqlite3_exec(db, sql, NULL, NULL, &error_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        exit(1);
    }
}

static void create_db_tables(sqlite3* db) {
    char* dir_sql =
        "CREATE TABLE IF NOT EXISTS Directory("
        "id INTEGER PRIMARY KEY     AUTOINCREMENT,"
        "name           TEXT    NOT NULL,"
        "parent         INTEGER,"
        "full_path      TEXT,"
        "FOREIGN KEY(parent) REFERENCES Directory(id) ON DELETE CASCADE);";

    char* file_sql =
        "CREATE TABLE IF NOT EXISTS File("
        "id INTEGER PRIMARY KEY     AUTOINCREMENT,"
        "name           TEXT    NOT NULL,"
        "parent         INTEGER,"
        "full_path      TEXT,"
        "contents       TEXT,"
        "FOREIGN KEY(parent) REFERENCES Directory(id) ON DELETE CASCADE);";

    char* enable_foreign_key = "PRAGMA foreign_keys = ON;";

    execute_query(db, enable_foreign_key);
    execute_query(db, dir_sql);
    execute_query(db, file_sql);
}

static void ensure_root(sqlite3* db) {
    char* root_sql =
        "insert into Directory (id, name, full_path)"
        "select 0,'/','/' where not exists (select id from Directory where id=0);";
    execute_query(db, root_sql);
}

bool check_dir_exists(sqlite3* db, int parent_id, char* name) {
    char sql[200];
    sprintf(sql, "select exists (select id from Directory where name='%s' and parent=%d);", name, parent_id);
    struct sqlite3_stmt* selectstmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    bool found = false;
    if (result == SQLITE_OK) {
        if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            found = sqlite3_column_int(selectstmt, 0);
        }
    }
    sqlite3_finalize(selectstmt);
    return found;
}

bool check_dir_path_exists(sqlite3* db, char* path) {
    char sql[200];
    sprintf(sql, "select exists (select id from Directory where full_path='%s');", path);
    struct sqlite3_stmt* selectstmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    bool found = false;
    if (result == SQLITE_OK) {
        if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            found = sqlite3_column_int(selectstmt, 0);
        }
    }
    sqlite3_finalize(selectstmt);
    return found;
}

bool check_file_exists(sqlite3* db, int parent_id, char* name) {
    char sql[200];
    sprintf(sql, "select exists (select id from File where name='%s' and parent=%d);", name, parent_id);
    struct sqlite3_stmt* selectstmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    bool found = false;
    if (result == SQLITE_OK) {
        if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            found = sqlite3_column_int(selectstmt, 0);
        }
    }
    sqlite3_finalize(selectstmt);
    return found;
}

bool check_full_path_exists(sqlite3* db, char* full_path) {
    char sql[200];
    sprintf(sql, "select exists (select id from Directory where full_path='%s') or (select id from File where full_path='%s');", full_path, full_path);
    struct sqlite3_stmt* selectstmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    bool found = false;
    if (result == SQLITE_OK) {
        if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            found = sqlite3_column_int(selectstmt, 0);
        }
    }
    sqlite3_finalize(selectstmt);
    return found;
}

void db_make_directory(sqlite3* db, int parent_id, char* parent_path, char* dir) {
    char sql[200];
    int parent_len = strlen(parent_path);
    char* full_path = (char*)malloc(sizeof(char) * (strlen(dir) + parent_len + 2));
    strcpy(full_path, parent_path);
    if (parent_len != 1)
        strcat(full_path, "/");
    strcat(full_path, dir);
    sprintf(sql, "insert into Directory(name, parent, full_path) values ('%s', %d, '%s');",
            dir, parent_id, full_path);
    execute_query(db, sql);
    free(full_path);
}

void db_make_file(sqlite3* db, int parent_id, char* parent_path, char* file) {
    char sql[200];
    int parent_len = strlen(parent_path);
    char* full_path = (char*)malloc(sizeof(char) * (strlen(file) + parent_len + 2));
    strcpy(full_path, parent_path);
    if (parent_len != 1)
        strcat(full_path, "/");
    strcat(full_path, file);
    sprintf(sql, "insert into File(name, parent, full_path) values ('%s', %d, '%s');",
            file, parent_id, full_path);
    execute_query(db, sql);
    free(full_path);
}

int db_get_dir_id(sqlite3* db, char* path) {
    char sql[200];
    sprintf(sql, "select id from Directory where full_path='%s';", path);
    struct sqlite3_stmt* selectstmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    int found_id = -1;
    if (result == SQLITE_OK) {
        if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            found_id = sqlite3_column_int(selectstmt, 0);
        }
    }
    sqlite3_finalize(selectstmt);
    return found_id;
}

char* db_list_dir_contents(sqlite3* db, int id) {
    char sql[200];
    sprintf(sql, "select name from Directory where parent=%d;", id);
    struct sqlite3_stmt* selectstmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    const char* name;
    int size = 1;
    char* contents = (char*)malloc(sizeof(char) * size);
    *contents = '\0';

    if (result == SQLITE_OK) {
        while (sqlite3_step(selectstmt) == SQLITE_ROW) {
            name = (char*)sqlite3_column_text(selectstmt, 0);
            size += strlen(name) + 2;
            contents = (char*)realloc(contents, sizeof(char) * size);
            strcat(contents, name);
            strcat(contents, "/\n");
        }
    }
    sqlite3_finalize(selectstmt);

    sprintf(sql, "select name from File where parent=%d;", id);
    struct sqlite3_stmt* selectstmt2;
    result = sqlite3_prepare_v2(db, sql, -1, &selectstmt2, NULL);
    if (result == SQLITE_OK) {
        while (sqlite3_step(selectstmt2) == SQLITE_ROW) {
            name = (char*)sqlite3_column_text(selectstmt2, 0);
            size += strlen(name) + 1;
            contents = (char*)realloc(contents, sizeof(char) * size);
            strcat(contents, name);
            strcat(contents, "\n");
        }
    }
    if (size > 1) {
        contents[strlen(contents) - 1] = '\0';
    }
    sqlite3_finalize(selectstmt2);
    return contents;
}

char* db_get_dir_parent(sqlite3* db, int id, int* parent_id) {
    char sql[200];
    sprintf(sql, "select full_path, id from Directory where id=(select parent from Directory where id=%d);", id);
    struct sqlite3_stmt* selectstmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    const char* name;
    char* parent_name = NULL;
    if (result == SQLITE_OK) {
        if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            name = (char*)sqlite3_column_text(selectstmt, 0);
            parent_name = (char*)malloc(sizeof(char) * (strlen(name) + 1));
            strcpy(parent_name, name);
            *parent_id = sqlite3_column_int(selectstmt, 1);
        }
    }
    sqlite3_finalize(selectstmt);
    return parent_name;
}

char* db_read_file_contents(sqlite3* db, int parent_id, char* name) {
    char sql[200];
    sprintf(sql, "select contents from File where parent=%d and name='%s';", parent_id, name);
    struct sqlite3_stmt* selectstmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    char* contents = NULL;
    if (result == SQLITE_OK) {
        if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            char* c = (char*)sqlite3_column_text(selectstmt, 0);
            if (c != NULL) {
                int len = strlen(c);
                contents = (char*)malloc(sizeof(char) * (len + 1));
                strcpy(contents, c);
            } else {
                contents = (char*)malloc(sizeof(char) * 1);
                *contents = '\0';
            }
        }
    }
    sqlite3_finalize(selectstmt);
    return contents;
}

void db_write_file_contents(sqlite3* db, int parent_id, char* name, char* contents) {
    char sql[512];
    sprintf(sql, "update File set contents='%s' where parent=%d and name='%s';",
            contents, parent_id, name);
    execute_query(db, sql);
}

void db_remove_file(sqlite3* db, int parent_id, char* name) {
    char sql[200];
    sprintf(sql, "delete from File where parent=%d and name='%s';", parent_id, name);
    execute_query(db, sql);
}

void db_remove_directory_recursive(sqlite3* db, int parent_id, char* name) {
    char sql[200];
    sprintf(sql, "delete from Directory where parent=%d and name='%s';", parent_id, name);
    execute_query(db, sql);
}

void init_db(database* sqldb) {
    open_db(sqldb->name, &sqldb->db);
    create_db_tables(sqldb->db);
    ensure_root(sqldb->db);
}

void close_db(database* sqldb) {
    sqlite3_close(sqldb->db);
}