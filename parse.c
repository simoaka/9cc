#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

/* if next token is match to expected symbol, go ahead token and return true.
 * otherwise, return false.
 */
static bool consume(char *op)
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
static bool expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(op, token->str, token->len))
        error_at(token->str, "expected \"%s\".", op);
    token = token->next;
}

/* if next token is match to expected symbol, go ahead token and return number.
 * otherwise, report error and exit.
 */
static int expect_number()
{
    if (token->kind != TK_NUM)
        error_at(token->str, "expected a number.\n");
    int val = token->val;
    token = token->next;
    return val;
}

static bool at_eof()
{
    return (token->kind == TK_EOF);
}

/* create new token and chain  to current token.
 */
static Token *new_token(TokenKind kind, Token *cur, char *str, int len)
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

static Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

/* new node of binary tree */
static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

/* new number of binary tree */
static Node *new_num(int val)
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
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

/* EBNF: expr = equality */
Node *expr()
{
    return equality();
}

/* EBNF: equality = rerational ("==" relational | "!=" relational)* */
static Node *equality()
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
static Node *relational()
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
static Node *add()
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
static Node *mul()
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
static Node *unary()
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
static Node *primary()
{
    /* if next token is "(", expression will be expanded. */
    if (consume("(")) {
        Node *node = expr();
        expect(")"); /* expression must be closed with ')' */
        return node;
    }

    return new_num(expect_number());
}
