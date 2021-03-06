#include "token.h"

/**
 *  Tokenizes the input and returns the size of tokens
**/
int tokenize(char* line, Token** tokens, char** error) {
    Token* first = (Token*)malloc(sizeof(Token));
    // fill with zeros
    memset(first, 0, sizeof(Token));
    *tokens = first;

    int size = 0;
    Token* cur = first;
    bool sawRedirect = false;
    bool sawRedirectArg = false;

    // Tokenize the line with space and tab delimiters
    char* token = strtok(line, " \t");
    while (token) {
        size++;
        cur->text = token;
        bool isRedirect = strcmp(token, ">") == 0;

        // Error messages also help understanding the code

        if (cur == first) {
            if (isRedirect) {
                *error = "first token cannot be a redirect";
                return 0;
            }
            cur->type = COMMAND;
        } else if (isRedirect) {
            if (sawRedirect) {
                *error = "cannot have more than one redirect";
                return 0;
            }
            cur->type = REDIRECT;
            sawRedirect = true;
        } else {
            if (sawRedirect) {
                if (!sawRedirectArg) {
                    cur->type = REDIRECT_FILE;
                    sawRedirectArg = true;
                } else {
                    *error = "Can't have more than one redirect file";
                    return 0;
                }
            } else {
                cur->type = ARG;
            }
        }
        token = strtok(NULL, " \t");
        // Only allocate a new Token if it's needed
        if (token) {
            cur->next = (Token*)malloc(sizeof(Token));
            memset(cur->next, 0, sizeof(Token));
            cur = cur->next;
        }
    }
    *error = NULL;
    cur->next = NULL;

    return size;
}