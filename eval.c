#include "mpc.h"

long eval_min(long x, long y) {
  if (x <= y) { return x; }
  return y;
}

long eval_max(long x, long y) {
  if (x >= y) { return x; }
  return y;
}


long eval_op(char* op, long x, long y) {
  if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0) { return x + y; }
  if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) { return x - y; }
  if (strcmp(op, "*") == 0) { return x * y; }
  if (strcmp(op, "/") == 0) { return x / y; }
  if (strcmp(op, "%") == 0) { return x % y; }
  if (strcmp(op, "min") == 0) { return eval_min(x, y); };
  if (strcmp(op, "max") == 0) { return eval_max(x, y); };
  return 0;
}

long eval(mpc_ast_t* t) {
  if(strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  char* op = t->children[1]->contents;

  long x = eval(t->children[2]);

  int i = 3;
  while(strstr(t->children[i]->tag, "expr")) {
    x = eval_op(op, x, eval(t->children[i]));
    i ++;
  }

  return x;
}

