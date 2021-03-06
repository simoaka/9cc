#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

/* return true if this character is available for making token string.
 * return true if it's alphabet, number of under score.
 */
static int is_alpha_digit(char c)
{
    return (isalpha(c) || isdigit(c) || c == '_');
}

/* return local variables which has the same name as token.
 * if token is not found, return NULL.
 */
static LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals; var != NULL; var = var->next)
        if (var->len == tok->len && !strncmp(var->name, tok->str, var->len))
            return var;
    return NULL;
}

/* return number of local variables */
int count_lvar(void)
{
    int cnt = 0;
    for (LVar *var = locals; var; var = var->next)
        cnt++;
    return cnt;
}

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

/* if next token is expected keyword, go ahead token and return true.
 * otherwise, return false.
 */
static bool consume_keyword(TokenKind kind)
{
    if (token->kind != kind)
        return false;
    token = token->next;
    return true;
}

/* if next token is identifier, go ahead token and return identifier token.
 * otherwise, return NULL.
 */
static Token * consume_ident(void)
{
    Token *tok = token;
    if (tok->kind != TK_IDENT)
        return NULL;
    token = token->next;
    return tok;
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


typedef struct Keyword Keyword;

struct Keyword {
    TokenKind   kind;
    char        *str;
    int         len;
};

#define entry(kind, literal)    { (kind), (literal), strlen(literal) }

Keyword keywords[] = {
    entry(TK_RETURN, "return"),
    entry(TK_IF,     "if"),
    entry(TK_ELSE,   "else"),
    entry(TK_WHILE,  "while"),
    entry(TK_FOR,    "for"),
    entry(TK_EOF,    ""),
};

Keyword* find_keyword(char *p)
{
    for (Keyword *key = keywords; key->kind != TK_EOF; key++) {
        if (!strncmp(p, key->str, key->len) && !is_alpha_digit(p[key->len]))
            return key;
    }
    return NULL;
}

char token_kind_string[][12] = {
    "TK_RESERVED",
    "TK_RETURN",
    "TK_IF",
    "TK_ELSE",
    "TK_WHILE",
    "TK_FOR",
    "TK_IDENT",
    "TK_NUM",
    "TK_EOF",
};

void show_token(void)
{
    for (Token *tok = token; tok; tok = tok->next) {
        printf("{%s:", token_kind_string[tok->kind]);
        for (int i = 0; i < tok->len; i++)
            printf("%c", tok->str[i]);
        printf("}, ");
    }
    printf("\n");
}

Token *tokenize(void)
{
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;
    Keyword *key;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        key = find_keyword(p);
        if (key) {
            cur = new_token(key->kind, cur, p, key->len);
            p += key->len;
            continue;
        }

        if (!strncmp(p, "==", 2) || !strncmp(p, "!=", 2) ||
            !strncmp(p, "<=", 2) || !strncmp(p, ">=", 2)) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
            *p == '(' || *p == ')' || *p == '<' || *p == '>' ||
            *p == '=' || *p == ';' || *p == '{' || *p == '}') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isalpha(*p) || (*p == '_')) {
            char *q = p;
            cur = new_token(TK_IDENT, cur, p, 0);
            while (is_alpha_digit(*p))
                p++;
            cur->len = p - q;
            continue;
        }

        if (isdigit(*p)) {
            char *q = p;
            cur = new_token(TK_NUM, cur, p, 0);
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

/* new node of vector */
static Node *new_vector(Vector *vector)
{
    Node *node = new_node(ND_BLOCK);
    node->vector = vector;
    return node;
}

/* 1. == !=
 * 2. < <= > >=
 * 3. + -
 * 4. * /
 * 5. +(unary) -(unary)
 * 6. ()
 */
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

Node *code[100];

/* EBNF: program = stmt* */
void *program()
{
    int i = 0;
    while (!at_eof())
        code[i++] = stmt();
    code[i] = NULL;
}

/* EBNF: stmt = expr ";"
 *            | "{" stmt* "}"
 *            | "return" expr ";"
 *            | "if" "(" expr ")" stmt ("else" stmt)?
 *            | "while" "(" expr ")" stmt
 *            | "for" "(" expr? ";" expr? ; expr? ")" stmt
 * pending - parse isolated ';'
 */
Node *stmt()
{
    Node *node;

    if (consume_keyword(TK_RETURN)) {
        node = new_binary(ND_RETURN, expr(), NULL);
    } else if (consume_keyword(TK_IF)) {
        expect("(");
        Node *ex = expr();
        expect(")");
        Node *st = stmt();
        if (consume_keyword(TK_ELSE)) {
            node = new_binary(ND_ELSE, st, stmt());
            return new_binary(ND_IF, ex, node);
        } else {
            return new_binary(ND_IF, ex, st);
        }
    } else if (consume_keyword(TK_WHILE)) {
        expect("(");
        Node *ex = expr();
        expect(")");
        return new_binary(ND_WHILE, ex, stmt());
    } else if (consume_keyword(TK_FOR)) {
        Node *ini, *cnd, *nxt;
        ini = cnd = nxt = NULL;

        expect("(");
        if (!consume(";")) {
            ini = expr();
            expect(";");
        } else {
            ini = new_num(1);
        }

        if (!consume(";")) {
            cnd = expr();
            expect(";");
        } else {
            cnd = new_num(1);
        }

        if (!consume(")")) {
            nxt = expr();
            expect(")");
        } else {
            nxt = new_num(1);
        }

        return new_binary(ND_FOR,
            new_binary(ND_FOR1, ini, cnd),
            new_binary(ND_FOR2, nxt, stmt()));
    } else if (consume("{")) {
        Vector *vec = calloc(1, sizeof(Vector));
        while (!consume("}"))
            enque(vec, stmt());
        return new_vector(vec);
    } else {
        node = expr();
    }

    expect(";");
    return node;
}

/* EBNF: expr = assign */
Node *expr()
{
    return assign();
}

/* EBNF: assign = eauality ("=" assign)? */
Node *assign()
{
    Node *node = equality();
    if (consume("="))
        node = new_binary(ND_ASSIGN, node, assign());
    return node;
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

/* EBNF: primary = num
 *               | ident ("(" ")")?
 *               | "(" expr ")"
 */
static Node *primary()
{
    /* if next token is "(", expression will be expanded. */
    if (consume("(")) {
        Node *node = expr();
        expect(")"); /* expression must be closed with ')' */
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        if (consume("(")) {
            expect(")");
            node->kind = ND_FUNC;
            node->func = tok->str;
            node->len = tok->len;
        } else {
            node->kind = ND_LVAR;
            LVar *lvar = find_lvar(tok);
            if (lvar) {
                node->offset = lvar->offset;
            } else {
                lvar = calloc(1, sizeof(LVar));
                lvar->next = locals;
                lvar->name = tok->str;
                lvar->len = tok->len;
                lvar->offset = (locals) ? (locals->offset + 8) : 0;
                node->offset = lvar->offset;
                locals = lvar;
            }
        }
        return node;
    }

    return new_num(expect_number());
}
