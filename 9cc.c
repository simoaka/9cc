#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "number of input parameters is incorrect.\n");
        return 1;
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    char *p = argv[1];
    printf("  mov rax, %ld\n", strtol(p, &p, 10));
    while (*p != '\0') {
        if (*p == '+') {
            printf("  add rax, %ld\n", strtol(p+1, &p, 10));
            continue;
        }

        if (*p == '-') {
            printf("  sub rax, %ld\n", strtol(p+1, &p, 10));
            continue;
        }

        fprintf(stderr, "unexpectd charctor!: %c\n", *p);
        return 2;
    }
    printf("  ret\n");
    return 0;
}
