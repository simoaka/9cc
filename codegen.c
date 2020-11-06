#include <stdio.h>
#include "9cc.h"

void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
        error("left value is not variable\n");

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

static unsigned int lbegin_number = 1;
static unsigned int lend_number = 1;
static unsigned int lelse_number = 1;

void gen(Node *node)
{
    switch (node->kind) {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    case ND_RETURN:
        gen(node->lhs);

        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    case ND_IF:
        if (node->rhs->kind == ND_ELSE) {
            unsigned int m = lelse_number++;
            unsigned int n = lend_number++;
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lelse%u\n", m);
            gen(node->rhs->lhs);
            printf("  jmp .Lend%u\n", n);
            printf(".Lelse%u:\n", m);
            gen(node->rhs->rhs);
            printf(".Lend%u:\n", n);
        } else {
            unsigned int n = lend_number++;
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%u\n", n);
            gen(node->rhs);
            printf(".Lend%u:\n", n);
        }
        return;
    case ND_WHILE: {
        unsigned int m = lbegin_number++;
        unsigned int n = lend_number++;
        printf(".Lbegin%u:\n", m);
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%u\n", n);
        gen(node->rhs);
        printf("  jmp .Lbegin%u\n", m);
        printf(".Lend%u:\n", n);
        }
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop  rdi\n");
    printf("  pop  rax\n");
    switch (node->kind) {
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    case ND_EQU:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NEQ:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LES:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LEQ:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    }

    printf("  push rax\n");
}
