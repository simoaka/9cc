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
    program();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    /* prologue - prepare memory for 26 variables. */
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    int i;
    for (i = 0; code[i]; i++) {
        gen(code[i]);

        /* total calculated value is stacked at top.
         * load it and return.
         */
        printf("  pop rax\n");
    }

    /* epilogue */
    /* total evaluated value is stored in rax. return rax. */
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}
