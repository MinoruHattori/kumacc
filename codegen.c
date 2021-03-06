#include "kumacc.h"

static char *reg(int idx) {
  static char *r[] = {"r10", "r11", "r12", "r13", "r14", "r15"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx) {
    error("register out of range: %d", idx);
  }
  return r[idx];
}

static int top;

static void gen_addr(Node *node) {
  if (node->kind == ND_VAR) {
    int offset = (node->name - 'a' + 1) * 8;
    offset += 32;
    printf("  lea %s, [rbp-%d]\n", reg(top++), offset);
  }
}

static void load(void) {
  printf(" mov %s, [%s]\n", reg(top - 1), reg(top - 1));
}

static void store(void) {
  printf("  mov [%s], %s\n", reg(top - 1), reg(top - 2));
  top--;
}

static void gen_expr(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  mov %s, %lu\n", reg(top++), node->val);
    return;
  case ND_VAR:
    gen_addr(node);
    load();
    return;
  case ND_ASSIGN:
    gen_expr(node->rhs);
    gen_addr(node->lhs);
    store();
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
  case ND_EQ:
    printf("  cmp %s, %s\n", rd, rs);
    printf("  sete al\n");
    printf("  movzb %s, al\n", rd);
    return;
  case ND_NE:
    printf("  cmp %s, %s\n", rd, rs);
    printf("  setne al\n");
    printf("  movzb %s, al\n", rd);
    return;
  case ND_LT:
    printf("  cmp %s, %s\n", rd, rs);
    printf("  setl al\n");
    printf("  movzb %s, al\n", rd);
    return;
  case ND_LE:
    printf("  cmp %s, %s\n", rd, rs);
    printf("  setle al\n");
    printf("  movzb %s, al\n", rd);
    return;
  default:
    error("invalid expression");
  }
}

static void gen_stmt(Node *node) {
  switch (node->kind) {
  case ND_RETURN:
    gen_expr(node->lhs);
    printf("  mov rax, %s\n", reg(--top));
    printf("  jmp .L.return\n");
    return;
  case ND_EXPR_STMT:
    gen_expr(node->lhs);
    printf("  mov rax, %s\n", reg(--top));
    return;
  default:
    error("invalid statement");
  }
}

void codegen(Node *node) {
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 240\n");
  printf("  mov [rbp-8], r12\n");
  printf("  mov [rbp-16], r13\n");
  printf("  mov [rbp-24], r14\n");
  printf("  mov [rbp-32], r15\n");

  for (Node *n = node; n; n = n->next) {
    gen_stmt(n);
    assert(top == 0);
  }

  printf(".L.return:\n");
  printf("  mov r12, [rbp-8]\n");
  printf("  mov r13, [rbp-16]\n");
  printf("  mov r14, [rbp-24]\n");
  printf("  mov r15, [rbp-32]\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}