#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind   kind;   /* kind of token */
    Token       *next;  /* next token */
    int         val;    /* number if kind of token is number. */
    char        *str;   /* token string */
};

/* current token */
Token *token;

/* input program */
char *user_input;

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); /* output pos number of spaces. */
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

/* if next token is match to expected symbol, go ahead token and return true.
 * otherwise, return false.
 */
bool consume(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

/* if next token is match to expected symbol, go ahead token.
 * otherwise, report error and exit.
 */
bool expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error_at(token->str, "expected '%c'.", op);
    token = token->next;
}

/* if next token is match to expected symbol, go ahead token and return number.
 * otherwise, report error and exit.
 */
int expect_number()
{
    if (token->kind != TK_NUM)
        error_at(token->str, "expected a number.\n");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return (token->kind == TK_EOF);
}

/* create new token and chain  to current token.
 */
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *tokenize(void)
{
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "expected a number.\n");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind    kind;
    Node        *lhs; /* left hand side */
    Node        *rhs; /* right hand side */
    int         val;  /* it will be used when kind is number */
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *expr();
Node *mul();
Node *num();

Node *expr()
{
    Node *node = mul();

    for (;;) {
        if (consume('+'))
            node = new_node(ND_ADD, node, mul());
        else if (consume('-'))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul()
{
    Node *node = num();

    for (;;) {
        if (consume('*'))
            node = new_node(ND_MUL, node, num());
        else if (consume('/'))
            node = new_node(ND_DIV, node, num());
        else
            return node;
    }
}

Node *num()
{
    return new_node_num(expect_number());
}

void gen(Node *node)
{
    if (node->kind == ND_NUM) {
        printf(" push %d\n", node->val);
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
        printf("  cpo\n");
        printf("  idiv rdi\n");
        break;
    }
    printf("  push rax\n");
}

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
