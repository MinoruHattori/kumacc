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

    if (*p == '+' || *p == '-') {
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

    error_at(p, "expected a number");
  }

  Token *nxt;
  nxt = new_token(TK_EOF, p, 0);
  cur->next = nxt;
  cur = nxt;
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: invalid number of arguments\n", argv[0]);
  }

  current_input = argv[1];
  Token *tok = tokenize();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  printf("  mov rax, %ld\n", get_number(tok));
  tok = tok->next;

  while (tok->kind != TK_EOF) {
    if (equal(tok, "+")) {
      printf("  add rax, %ld\n", get_number(tok->next));
      tok = tok->next->next;
      continue;
    }

    tok = skip(tok, "-");
    printf("    sub rax, %ld\n", get_number(tok));
    tok = tok->next;
  }

  printf("  ret\n");
  return 0;
}
