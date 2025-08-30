#include <stdio.h>

#include "mpc.h"
#include "eval.h"

tval* tval_num(long x) {
  tval* v = malloc(sizeof(tval));
  v->type = TVAL_NUM;
  v->num = x;
  return v;
}

tval* tval_err(char* msg) {
  tval* v = malloc(sizeof(tval));
  v->type = TVAL_ERR;
  size_t len = strlen(msg) + 1;
  v->err = malloc(len);
  strlcpy(v->err, msg, len);
  return v;
}

tval* tval_sym(char* sym) {
  tval* v = malloc(sizeof(tval));
  v->type = TVAL_SYM;
  size_t len = strlen(sym) + 1;
  v->sym = malloc(len);
  strlcpy(v->sym, sym, len);
  return v;
}

tval* tval_sexpr(void) {
  tval* v = malloc(sizeof(tval));
  v->type = TVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  v->err = NULL;
  v->sym = NULL;
  return v;
}

void tval_del(tval *v) {
  switch(v->type) {
    case TVAL_NUM: break;
    case TVAL_ERR: free(v->err); break;
    case TVAL_SYM: free(v->sym); break;
    case TVAL_SEXPR: 
      for (int i = 0; i < v->count; i++) {
	tval_del(v->cell[i]);
      }
      free(v->cell);
      break;
  }
  free(v);
}

tval* tval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
    tval_num(x) : tval_err("invalid number");
}

tval* tval_add(tval* v, tval* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(tval*) * v->count);
  v->cell[v->count - 1] = x;
  return v;
}

tval* tval_read(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) { return tval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return tval_sym(t->contents); }

  tval* x = NULL;
  if (strcmp(t->tag, ">") == 0 || strstr(t->tag, "sexpr")) {
    x = tval_sexpr();
    for (int i = 0; i < t->children_num; i++) {
      if (strcmp(t->children[i]->contents, "(") == 0 ||
	  strcmp(t->children[i]->contents, ")") == 0 ||
	  strcmp(t->children[i]->tag, "regex") == 0) {
	continue;
      }
      x = tval_add(x, tval_read(t->children[i]));
    }
  } 

  return x;
}

void tval_print_expr(char open, char close, tval *v) {
  putchar(open);

  for (int i = 0; i < v->count; i++) {
    tval_print(v->cell[i]);
    if (i != (v->count -1)) {
      putchar(' ');
    }
  }

  putchar(close);
}

void tval_print(tval* v) {
  switch (v->type) {
    case TVAL_NUM: printf("%li", v->num); break;
    case TVAL_ERR: printf("%s", v->err); break;
    case TVAL_SYM: printf("%s", v->sym); break;
    case TVAL_SEXPR: tval_print_expr('(', ')', v); break;
  }
}

void tval_println(tval* v) {
  tval_print(v);
  putchar('\n');
}

tval* tval_pop(tval* v, int i) {
  tval* x = v->cell[i];

  memmove(&v->cell[i], &v->cell[i+1], sizeof(tval*) * (v->count - i - 1));

  v->count--;

  v->cell = realloc(v->cell, sizeof(tval*) * v->count);

  return x;
}

tval* tval_take(tval* v, int i) {
  tval* x = tval_pop(v, i);
  tval_del(v);

  return x;
} 

tval* builtin_op(tval* a, char* op) {
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != TVAL_NUM) {
      tval_del(a);
      return tval_err("Cannot operat on non numeral value!");
    }
  } 

  tval* x = tval_pop(a, 0);

  if (strcmp(op, "-") == 0 && a->count == 0) {
    x->num = -x->num;
  }

  while (a->count > 0) {
    tval* y = tval_pop(a, 0);

    if (strcmp(op, "+") == 0) { x->num += y->num; }
    if (strcmp(op, "-") == 0) { x->num -= y->num; }
    if (strcmp(op, "*") == 0) { x->num *= y->num; }
    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
	tval_del(x);
	tval_del(y);
	x = tval_err("Division with zero!");
	break;
      }
      x->num /= y->num;
    }

    tval_del(y);
  }

  tval_del(a);

  return x;
}

tval* tval_sexpr_eval(tval *v) {
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = tval_eval(v->cell[i]);
  }

  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == TVAL_ERR) {
      return tval_take(v, i);
    }
  }

  if (v->count == 0) {
    return v;
  }

  if (v->count == 1) {
    return tval_take(v, 0);
  }

  tval* first = tval_pop(v, 0);
  if (first->type != TVAL_SYM) {
    tval_del(first);
    tval_del(v);
    return tval_err("S-Expression does not start with a symbol!");
  }

  tval* result = builtin_op(v, first->sym);
  free(first);
  return result;
}

tval* tval_eval(tval* v) {
  if (v->type == TVAL_SEXPR) {
    return tval_sexpr_eval(v);
  }
  return v;
}

