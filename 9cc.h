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


extern Token *token;
extern char *user_input;

extern Token *tokenize(void);
extern Node *expr();

extern void gen(Node *node);

extern void error_at(char *loc, char *fmt, ...);
