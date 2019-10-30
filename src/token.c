#include "token.h"

/**
 *  Tokenizes the input and returns the size of tokens
**/
int tokenize(char* line, Token** tokens, char** error) {
    Token* first = (Token*)malloc(sizeof(Token));
    // fill 'first' whit zeros
    memset(first, 0, sizeof(Token));

    int size = 0;
    Token* cur = first;
    bool sawRedirect = false;

    // Tokenize the line with space and tab delimiters
    char* token = strtok(line, " \t");
    while (token) {
        size++;
        cur->text = (char*)malloc(strlen(token) * sizeof(char));
        strcpy(cur->text, token);
        bool isRedirect = strcmp(token, ">") == 0;

        if (cur == first) {
            if (isRedirect) {
                *error = (char*)malloc(40 * sizeof(char));
                strcpy(*error, "Error: first token cannot be a redirect");
                return 0;
            }
            cur->type = COMMAND;
        } else if (isRedirect) {
            if (sawRedirect) {
                *error = (char*)malloc(40 * sizeof(char));
                strcpy(*error, "Error: cannot have more than one redirect");
                return 0;
            }
            cur->type = REDIRECT;
            sawRedirect = true;
        } else {
            cur->type = (sawRedirect) ? REDIRECT_FILE : ARG;
        }
        cur->next = (Token*)malloc(sizeof(Token));
        cur = cur->next;
        token = strtok(NULL, " \t");
    }
    *error = NULL;

    // The last one is empty and should be freed
    if (cur->next)
        free(cur->next);
    cur->next = NULL;

    *tokens = first;
    return size;
}