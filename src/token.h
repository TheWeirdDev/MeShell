#ifndef TOKEN_H
#define TOKEN_H

#include <ctype.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>

typedef enum type { COMMAND,
                    ARG,
                    REDIRECT,
                    REDIRECT_FILE } Type;

typedef struct token_ {
    char* text;
    Type type;
    struct token_* next;
} Token;

int tokenize(char*, Token**, char**);

#endif