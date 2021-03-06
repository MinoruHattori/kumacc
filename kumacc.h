#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//
// tokenize.c
//

typedef enum {
  TK_RESERVED,
  TK_IDENT,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  long val;
  char *loc;
  int len;
};

void error(const char *fmt, ...);
void error_tok(const Token *tok, const char *fmt, ...);
bool equal(const Token *tok, const char *op);
Token *skip(const Token *tok, const char *op);
Token *tokenize(char *input);

//
// parse.c
//

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ, // ==
  ND_NE, // !=
  ND_LT, // <
  ND_LE, // <=
  ND_ASSIGN, // =
  ND_RETURN, // "return"
  ND_EXPR_STMT, // Expression statement
  ND_VAR, // Variable
  ND_NUM, // Integer
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *next;
  Node *lhs;
  Node *rhs;
  char name;
  long val;
};

Node *parse(Token *tok);

//
// codegen.c
//

void codegen(Node *node);
