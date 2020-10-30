#include <stdio.h>
#include "9cc.h"

Token *token;
char *user_input;

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "number of input parameters is incorrect.\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);

    /* total calculated value is stacked at top.
     * load it and return.
     */
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
