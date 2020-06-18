#include "kumacc.h"

static char *current_input;

void error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void verror_at(const char *loc, const char *fmt, va_list ap) {
  int pos = loc - current_input;
  fprintf(stderr, "%s\n", current_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(const char *loc, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
}

void error_tok(const Token *tok, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}

bool equal(const Token *tok, const char *s) {
  return strlen(s) == tok->len &&
         !strncmp(tok->loc, s, tok->len);
}

Token *skip(const Token *tok, const char *s) {
  if (!equal(tok, s)) {
    error_tok(tok, "expected '%s'", s);
  }
  return tok->next;
}


static Token *new_token(TokenKind kind, char *str, const int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = str;
  tok->len = len;
  return tok;
}

static bool startswith(const char *p, const char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

static bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_alnum(char c) {
  return is_alpha(c) || ('0' <= c && c <= '9');
}

Token *tokenize(char *p) {
  current_input = p;
  Token head = {};
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (startswith(p, "==") || startswith(p, "!=") ||
        startswith(p, "<=") || startswith(p, ">=")) {
      Token *new;
      new = new_token(TK_RESERVED, p, sizeof(char) * 2);
      cur->next = new;
      cur = new;
      p += 2;
      continue;
    }

    if (isdigit(*p)) {
      Token *new;
      new = new_token(TK_NUM, p, 0);
      cur->next = new;
      cur = new;
      char *q = p;
      cur->val = strtoul(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    if (startswith(p, "return") && !is_alnum(p[6])) {
      Token *new;
      new = new_token(TK_RESERVED, p, 6);
      cur->next = new;
      cur = new;
      p += 6;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      Token *new;
      new = new_token(TK_IDENT, p++, 1);
      cur->next = new;
      cur = new;
      continue;
    }
 
    if (ispunct(*p)) {
      Token *new;
      new = new_token(TK_RESERVED, p++, sizeof(char));
      cur->next = new;
      cur = new;
      continue;
    }

    error_at(p, "invalid token");
  }

  Token *new;
  new = new_token(TK_EOF, p, 0);
  cur->next = new;
  cur = new;
  return head.next;
}