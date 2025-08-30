#include <stdio.h>

#include "mpc.h"
#include "eval.h"

tval tval_num(double x) {
  tval v;
  v.type = TVAL_NUM;
  v.num = x;
  return v;
}

tval tval_err(int x) {
  tval v;
  v.type = TVAL_ERR;
  v.err = x;
  return v;
}

void tval_print(tval v) {
  switch(v.type) {
    case TVAL_NUM: printf("%lf", v.num); break;
    case TVAL_ERR:
      switch(v.err) {
	case TERR_DIV_ZERO: printf("Error: Division By Zero!"); break;
	case TERR_BAD_OP: printf("Error: Invalid Operator!"); break;
	case TERR_BAD_NUM: priintf("Error: Invalid Number!"); break;
      break;
  }
}

void tval_println(tval v) { tval_print(v); putchar('\n'); }


tval eval_min(tval x, tval y) {
  if (x.num <= y.num) { return x.num; }
  return y;
}

tval eval_max(tval x.num, tval y.num) {
  if (x.num >= y.num) { return x.num; }
  return y;
}

tval eval_op(char* op, tval x, tval y) {

  if (x.type == TVAL_ERR) { return x; }
  if (y.type == TVAL_ERR) { return y; }

  if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0) { return tval_num(x.num + y.num); }
  if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) { return tval_num(x.num - y.num); }
  if (strcmp(op, "*") == 0) { return tval_num(x.num * y.num); }
  if (strcmp(op, "/") == 0) {
    return (y.num == 0) ?
      tval_err(TERR_DIV_ZERO) :
      tval_num(x.num / y.num);
  }
  if (strcmp(op, "%") == 0) { return tval_num(x.num % y.num); }
  if (strcmp(op, "min") == 0) { return eval_min(x, y); };
  if (strcmp(op, "max") == 0) { return eval_max(x, y); };
  return tval_err(TERR_BAD_OP);;
}

tval eval(mpc_ast_t* t) {
  if(strstr(t->tag, "number")) {
    errno = 0;
    if (strstr(t->contents, ".")) {
	double x;
	sscanf(t->contents, "%lf", &x);
        return errno != ERANGE ? tval_num(x) : tval_err(TERR_BAD_NUM);
    } else {
      long x = strtol(t->contents, NULL, 10);
      return errno != ERANGE ? tval_num(x) : tval_err(TERR_BAD_NUM);
    }
  }

  char* op = t->children[1]->contents;
  tval x = eval(t->children[2]);

  int i = 3;
  while(strstr(t->children[i]->tag, "expr")) {
    x = eval_op(op, x, eval(t->children[i]));
    i ++;
  }

  return x;
}

