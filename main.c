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
    int         len;    /* length of str */
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
bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(op, token->str, token->len))
        return false;
    token = token->next;
    return true;
}

/* if next token is match to expected symbol, go ahead token.
 * otherwise, report error and exit.
 */
bool expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(op, token->str, token->len))
        error_at(token->str, "expected \"%s\".", op);
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
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

        if (!strncmp(p, "==", 2) || !strncmp(p, "!=", 2) ||
            !strncmp(p, "<=", 2) || !strncmp(p, ">=", 2)) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
            *p == '(' || *p == ')' || *p == '<' || *p == '>') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "expected a number.\n");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

typedef enum {
    ND_ADD, /* + */
    ND_SUB, /* - */
    ND_MUL, /* * */
    ND_DIV, /* / */
    ND_EQU, /* == */
    ND_NEQ, /* != */
    ND_LES, /* <  */
    ND_LEQ, /* <= */
    ND_NUM, /* number */
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind    kind; /* kine of token */
    Node        *lhs; /* left hand side */
    Node        *rhs; /* right hand side */
    int         val;  /* it will be used when kind is number */
};

Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

/* new node of binary tree */
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

/* new number of binary tree */
Node *new_num(int val)
{
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

/* 1. == !=
 * 2. < <= > >=
 * 3. + -
 * 4. * /
 * 5. +(unary) -(unary)
 * 6. ()
 */
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

/* EBNF: expr = equality */
Node *expr()
{
    return equality();
}

/* EBNF: equality = rerational ("==" relational | "!=" relational)* */
Node *equality()
{
    Node *node = relational();

    for (;;) {
        if (consume("=="))
            node = new_binary(ND_EQU, node, relational());
        else if (consume("!="))
            node = new_binary(ND_NEQ, node, relational());
        else
            return node;
    }
}

/* EBNF: rerational = add ("<" add | "<=" add | ">" add | ">=" add)* */
Node *relational()
{
    Node *node = add();

    for (;;) {
        if (consume("<"))
            node = new_binary(ND_LES, node, add());
        else if (consume("<="))
            node = new_binary(ND_LEQ, node, add());
        else if (consume(">"))
            node = new_binary(ND_LES, add(), node);
        else if (consume(">="))
            node = new_binary(ND_LEQ, add(), node);
        else
            return node;
    }
}

/* EBNF: add = mul ("*" mul | "/" mul)* */
Node *add()
{
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_binary(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_binary(ND_SUB, node, mul());
        else
            return node;
    }
}

/* EBNF: mul = unary ("*" unary | "/" unary )* */
Node *mul()
{
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_binary(ND_DIV, node, unary());
        else
            return node;
    }
}

/* EBNF: unary = ("+" | "-")? primary */
Node *unary()
{
    if (consume("+"))
        return unary();
    else if (consume("-"))
        /* -num = 0-num */
        return new_binary(ND_SUB, new_num(0), unary());
    else
        return primary();
}

/* EBNF: primary = num | "(" expr ")" */
Node *primary()
{
    /* if next token is "(", expression will be expanded. */
    if (consume("(")) {
        Node *node = expr();
        expect(")"); /* expression must be closed with ')' */
        return node;
    }

    return new_num(expect_number());
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
