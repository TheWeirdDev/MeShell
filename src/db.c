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
        "id INT PRIMARY KEY     NOT NULL,"
        "name           TEXT    NOT NULL,"
        "parent         INT,"
        "full_path      TEXT,"
        "FOREIGN KEY(parent) REFERENCES Directory(id));";

    char* data_sql =
        "CREATE TABLE IF NOT EXISTS Data("
        "id INT PRIMARY KEY     NOT NULL,"
        "contents       TEXT"
        ");";

    char* file_sql =
        "CREATE TABLE IF NOT EXISTS File("
        "id INT PRIMARY KEY     NOT NULL,"
        "name           TEXT    NOT NULL,"
        "parent         INT,"
        "full_path      TEXT,"
        "contents       INT NOT NULL,"
        "FOREIGN KEY(parent) REFERENCES Directory(id),"
        "FOREIGN KEY(contents) REFERENCES Data(id));";

    execute_query(db, data_sql);
    execute_query(db, dir_sql);
    execute_query(db, file_sql);
}

static void ensure_root(sqlite3* db) {
    char* root_sql =
        "insert into Directory (id, name, full_path)"
        "select 0,'/','/' where not exists (select id from Directory where id=0);";
    execute_query(db, root_sql);
}

bool check_dir_exists(sqlite3* db, char* full_path) {
    char sql[200];
    sprintf(sql, "select exists (select id from Directory where full_path='%s');", full_path);
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

void init_db(database* sqldb) {
    open_db(sqldb->name, &sqldb->db);
    create_db_tables(sqldb->db);
    ensure_root(sqldb->db);
}

void close_db(database* sqldb) {
    sqlite3_close(sqldb->db);
}