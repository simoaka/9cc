typedef enum {
    TK_RESERVED,    /* symbols */
    TK_IDENT,       /* identifier */
    TK_NUM,         /* number */
    TK_EOF,         /* end of file */
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
    ND_ADD,     /* + */
    ND_SUB,     /* - */
    ND_MUL,     /* * */
    ND_DIV,     /* / */
    ND_EQU,     /* == */
    ND_NEQ,     /* != */
    ND_LES,     /* <  */
    ND_LEQ,     /* <= */
    ND_ASSIGN,  /* = */
    ND_LVAR,    /* local variable */
    ND_NUM,     /* number */
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind    kind; /* kine of token */
    Node        *lhs; /* left hand side */
    Node        *rhs; /* right hand side */
    int         val;  /* it will be used when kind is number */
    int         offset; /* it will be used when kind is local variable */
};


extern char *user_input;
extern Token *token;
extern Node *code[100];

extern Token *tokenize(void);
extern void *program();

extern void gen(Node *node);

extern void error(char *fmt, ...);
extern void error_at(char *loc, char *fmt, ...);
