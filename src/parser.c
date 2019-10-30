#include "parser.h"

void parse_input(Token* tokens) {
    Token* t = tokens;
    while (t->next) {
        printf("Token %s, type %d\n", t->text, t->type);
        t = t->next;
    }
}