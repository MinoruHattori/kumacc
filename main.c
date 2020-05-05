#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

typedef enum {
  TK_RESERVED,
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

static char *current_input;

static void error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

static void verror_at(char *loc, char *fmt, va_list ap) {
  int pos = loc - current_input;
  fprintf(stderr, "%s\n", current_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

static void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
}

static void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}

static bool equal(const Token *tok, const char *s) {
  return strlen(s) == tok->len &&
         !strncmp(tok->loc, s, tok->len);
}

static Token *skip(Token *tok, const char *s) {
  if (!equal(tok, s)) {
    error_tok(tok, "expected '%s'", s);
  }
  return tok->next;
}

static long get_number(Token *tok) {
  if (tok->kind != TK_NUM) {
    error_tok(tok, "expected a number");
  }
  return tok->val;
}

static Token *new_token(TokenKind kind, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = str;
  tok->len = len;
  return tok;
}

static Token *tokenize(void) {
  char *p = current_input;
  Token head = {};
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (ispunct(*p)) {
      Token *nxt;
      nxt = new_token(TK_RESERVED, p++, sizeof(char));
      cur->next = nxt;
      cur = nxt;
      continue;
    }

    if (isdigit(*p)) {
      Token *nxt;
      nxt = new_token(TK_NUM, p, 0);
      cur->next = nxt;
      cur = nxt;
      char *q = p;
      cur->val = strtoul(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "invalid token");
  }

  Token *nxt;
  nxt = new_token(TK_EOF, p, 0);
  cur->next = nxt;
  cur = nxt;
  return head.next;
}

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // Integer
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  long val;
};

static Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *new_num(long val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

static Node *expr(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

// expr = mul ("+" mul | "-" mul)*
static Node *expr(Token **rest, Token *tok) {
  Node *node = mul(&tok, tok);

  for(;;) {
    if (equal(tok, "+")) {
      Node *rhs = mul(&tok, tok->next);
      node = new_binary(ND_ADD, node, rhs);
      continue;
    }

    if (equal(tok, "-")) {
      Node *rhs = mul(&tok, tok->next);
      node = new_binary(ND_SUB, node, rhs);
      continue;
    }

    *rest = tok;
    return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul(Token **rest, Token *tok) {
  Node *node = unary(&tok, tok);

  for (;;) {
    if (equal(tok, "*")) {
      Node *rhs = unary(&tok, tok->next);
      node = new_binary(ND_MUL, node, rhs);
      continue;
    }

    if (equal(tok, "/")) {
      Node *rhs = unary(&tok, tok->next);
      node = new_binary(ND_DIV, node, rhs);
      continue;
    }

    *rest = tok;
    return node;
  }
}

// unary = ("+" | "-") unary
//       | primary
static Node *unary(Token **rest, Token *tok) {
  if (equal(tok, "+")) {
    return unary(rest, tok->next);
  }

  if (equal(tok, "-")) {
    return new_binary(ND_SUB, new_num(0), unary(rest, tok->next));
  }

  return primary(rest, tok);
}

// primary = "(" expr ")" | num
static Node *primary(Token **rest, Token *tok) {
  if (equal(tok, "(")) {
    Node *node = expr(&tok, tok->next);
    *rest = skip(tok, ")");
    return node;
  }

  Node *node = new_num(get_number(tok));
  *rest = tok->next;
  return node;
}

static char *reg(int idx) {
  static char *r[] = {"r10", "r11", "r12", "r13", "r14", "r15"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx) {
    error("register out of range: %d", idx);
  }
  return r[idx];
}

static int top;

static void gen_expr(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  mov %s, %ld\n", reg(top++), node->val);
    return;
  }

  gen_expr(node->lhs);
  gen_expr(node->rhs);

  char *rd = reg(top - 2);
  char *rs = reg(top - 1);
  top--;

  switch (node->kind) {
  case ND_ADD:
    printf("  add %s, %s\n", rd, rs);
    return;
  case ND_SUB:
    printf("  sub %s, %s\n", rd, rs);
    return;
  case ND_MUL:
    printf("  imul %s, %s\n", rd, rs);
    return;
  case ND_DIV:
    printf("  mov rax, %s\n", rd);
    printf("  cqo\n");
    printf("  idiv %s\n", rs);
    printf("  mov %s, rax\n", rd);
    return;
  default:
    error("invalid expression");
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: invalid number of arguments\n", argv[0]);
  }

  current_input = argv[1];
  Token *tok = tokenize();

  Node *node = expr(&tok, tok);

  if (tok->kind != TK_EOF) {
    error_tok(tok, "extra token");
  }

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  printf("  push r12\n");
  printf("  push r13\n");
  printf("  push r14\n");
  printf("  push r15\n");

  gen_expr(node);

  printf("  mov rax, %s\n", reg(top - 1));

  printf("  pop r15\n");
  printf("  pop r14\n");
  printf("  pop r13\n");
  printf("  pop r12\n");
  printf("  ret\n");
  return 0;
}
