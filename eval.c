#include <stdio.h>

#include "mpc.h"
#include "eval.h"

#define TASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    tval* err = tval_err(fmt, ##__VA_ARGS__); \
    tval_del(args); \
    return err; \
  }

char* ttype_name(int i) {
  switch(i) {
    case TVAL_NUM: return "Number";
    case TVAL_ERR: return "Error";
    case TVAL_FUN: return "Function";
    case TVAL_SYM: return "Symbol";
    case TVAL_SEXPR: return "S-Expresion";
    case TVAL_QEXPR: return "Q-Expresion";
    default: return "Unknwon";
  }
}

tval* tval_num(long x) {
  tval* v = malloc(sizeof(tval));
  v->count = 0;
  v->type = TVAL_NUM;
  v->num = x;
  return v;
}

tval* tval_err(char* fmt, ...) {
  tval* v = malloc(sizeof(tval));
  v->type = TVAL_ERR;

  va_list va;
  va_start(va, fmt);

  v->err = malloc(512);

  vsnprintf(v->err, 511, fmt, va);

  v->err = realloc(v->err, strlen(v->err) + 1);

  va_end(va);

  return v;
}

tval* tval_sym(char* sym) {
  tval* v = malloc(sizeof(tval));
  v->count = 0;
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
  v->sym = NULL;
  v->err = NULL;
  return v;
}

tval* tval_qexpr(void) {
  tval* v = malloc(sizeof(tval));
  v->type = TVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  v->sym = NULL;
  v->err = NULL;
  return v;
}

tval* tval_fun(tbuiltin func) {
  tval* v = malloc(sizeof(tval));
  v->type = TVAL_FUN;
  v->fun = func;
  return v;
}

void tval_del(tval *v) {
  switch(v->type) {
    case TVAL_NUM: break;
    case TVAL_FUN: break;
    case TVAL_ERR: free(v->err); break;
    case TVAL_SYM: free(v->sym); break;
    case TVAL_QEXPR:
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
  }
  if (strstr(t->tag, "qexpr")) {
    x = tval_qexpr();
  }

  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0 ||
        strcmp(t->children[i]->contents, ")") == 0 ||
        strcmp(t->children[i]->contents, "{") == 0 ||
        strcmp(t->children[i]->contents, "}") == 0 ||
        strcmp(t->children[i]->tag, "regex") == 0) {
      continue;
    }
    x = tval_add(x, tval_read(t->children[i]));
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
    case TVAL_FUN: printf("<function>"); break;
    case TVAL_SEXPR: tval_print_expr('(', ')', v); break;
    case TVAL_QEXPR: tval_print_expr('{', '}', v); break;
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

tval* tval_copy(tval* v) {
  tval* c = malloc(sizeof(tval));

  c->type = v->type;
  size_t len;

  switch(v->type) {
    case TVAL_NUM:
      c->num = v->num;
      break;
    case TVAL_FUN:
      c->fun = v->fun;
      break;
    case TVAL_ERR:
      len = strlen(v->err) + 1;
      c->err = malloc(len);
      strlcpy(c->err, v->err, len);
      break;
    case TVAL_SYM:
      len = strlen(v->sym) + 1;
      c->sym = malloc(len);
      strlcpy(c->sym, v->sym, len);
      break;
    case TVAL_SEXPR:
    case TVAL_QEXPR:
      c->count = v->count;
      c->cell = malloc(sizeof(tval*) * v->count);
      for (int i = 0; i < v->count; i++) {
	c->cell[i] = tval_copy(v->cell[i]);
      }
      break;
  }

  return c;
}

tval* tval_take(tval* v, int i) {
  tval* x = tval_pop(v, i);
  tval_del(v);

  return x;
}

tenv* tenv_new(void) {
  tenv* e = malloc(sizeof(tenv));
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;

  return e;
}

void tenv_del(tenv* e) {
  for(int i = 0; i < e->count; i++) {
    free(e->syms[i]);
    tval_del(e->vals[i]);
  }
  free(e->syms);
  free(e->vals);
  free(e);
}

tval* tenv_get(tenv* e, tval* k) {
  if (k->type != TVAL_SYM) {
    return tval_err("tenv_get: Wrong argument type!");
  }

  for(int i = 0; i < e->count; i++) {
    if (strcmp(e->syms[i], k->sym) == 0) {
      return tval_copy(e->vals[i]);
    }
  }

  return tval_err("Unbound symbol '%s'!", k->sym);
}

void tenv_put(tenv* e, tval* k, tval* v) {
  for (int i = 0; i < e->count; i++) {
    if (strcmp(e->syms[i], k->sym) == 0) {
      tval_del(e->vals[i]);
      e->vals[i] = tval_copy(v);
    }
  }

  e->count++;
  e->syms = realloc(e->syms, sizeof(char*) * e->count);
  e->vals = realloc(e->vals, sizeof(tval*) * e->count);

  e->vals[e->count - 1] = tval_copy(v);
  size_t len = strlen(k->sym) + 1;
  e->syms[e->count - 1] = malloc(len);
  strlcpy(e->syms[e->count -1], k->sym, len);
}

tval* builtin_op(tenv* e, tval* a, char* op) {
  for (int i = 0; i < a->count; i++) {
    TASSERT(a, a->cell[i]->type == TVAL_NUM,
      "Cannot operat on non numeral value! Got %s.",
      ttype_name(a->cell[i]->type));
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


tval* builtin_add(tenv* e, tval* a) {
  return builtin_op(e, a, "+");
}

tval* builtin_sub(tenv* e, tval* a) {
  return builtin_op(e, a, "-");
}

tval* builtin_mul(tenv* e, tval* a) {
  return builtin_op(e, a, "*");
}

tval* builtin_div(tenv* e, tval* a) {
  return builtin_op(e, a, "/");
}

void tenv_add_builtin(tenv* e, char* sym, tbuiltin func) {
  tval* k = tval_sym(sym);
  tval* v = tval_fun(func);
  tenv_put(e, k, v);
  tval_del(k);
  tval_del(v);
}

tval* builtin_def(tenv* e, tval* a) {
  TASSERT(a, a->cell[0]->type == TVAL_QEXPR,
      "Function 'def' passed incorrect type "
      "Expected %s, got %s.", ttype_name(TVAL_QEXPR),
      ttype_name(a->cell[0]->type));

  tval* syms = a->cell[0];

  for (int i = 0; i < syms->count; i++) {
    TASSERT(a, syms->cell[i]->type == TVAL_SYM,
      "Function 'def' cannot define non symbol. Got %s.",
      ttype_name(syms->cell[i]->type));
  }

  TASSERT(a, syms->count == a->count -1,
      "Function 'def' cannot define incorrect number of symbols to values");

  for (int i = 0; i < syms->count; i++) {
    tenv_put(e, syms->cell[i], a->cell[i+1]);
  }

  tval_del(a);

  return tval_sexpr();
}

tval* builtin_head(tenv* e, tval* a) {
  TASSERT(a, a->count == 1, "Function 'head' passed to many arguments."
      "Expected %d, got %d", 1, a->count);
  TASSERT(a, a->cell[0]->type == TVAL_QEXPR,
      "Function 'head' passed invalid argument type."
      "Expected %s, got %s", ttype_name(TVAL_QEXPR),
      ttype_name(a->cell[0]->type));
  TASSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}");

  tval* v = tval_take(a, 0);
  while (v->count > 1) {
    tval_del(tval_pop(v, 1));
  }

  return v;
}

tval* builtin_tail(tenv* e, tval* a) {
  TASSERT(a, a->count == 1, "Function 'tail' passed to many arguments."
      "Expected %d, got %d", 1, a->count);
  TASSERT(a, a->cell[0]->type == TVAL_QEXPR,
      "Function 'tail' passed invalid argument type. "
      "Expected %s, got %s.", ttype_name(TVAL_QEXPR),
      ttype_name(a->cell[0]->type));
  TASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}");

  tval* v = tval_take(a, 0);
  tval_del(tval_pop(v, 0));

  return v;
}

tval* builtin_list(tenv* e, tval* a) {
  a->type = TVAL_QEXPR;
  return a;
}

tval* builtin_eval(tenv* e, tval* a) {
  TASSERT(a, a->count == 1, "Function 'eval' passed to many arguments."
      "Expected %d, got %d", 1, a->count);
  TASSERT(a, a->cell[0]->type == TVAL_QEXPR,
      "Function 'eval' passed invalid argument type. "
      "Expected %s, got %s", ttype_name(TVAL_QEXPR),
      ttype_name(a->cell[0]->type));

  tval* x = tval_take(a, 0);
  x->type = TVAL_SEXPR;

  return tval_eval(e, x);
}

tval* tval_join(tval* x, tval* y) {
  while(y->count) {
    x = tval_add(x, tval_pop(y, 0));
  }

  tval_del(y);
  return x;
}

tval* builtin_join(tenv* e, tval* a) {
  for(int i; i < a->count; i++) {
  TASSERT(a, a->cell[i]->type == TVAL_QEXPR,
      "Function 'eval' passed invalid argument type. "
      "Expected %s, got %s", ttype_name(TVAL_QEXPR),
      ttype_name(a->cell[i]->type));
  }

  tval* x = tval_pop(a, 0);
  while(a->count) {
    x = tval_join(x, tval_pop(a, 0));
  }

  tval_del(a);

  return x;
}

void tenv_add_builtins(tenv* e) {
  tenv_add_builtin(e, "head", builtin_head);
  tenv_add_builtin(e, "tail", builtin_tail);
  tenv_add_builtin(e, "join", builtin_join);
  tenv_add_builtin(e, "list", builtin_list);
  tenv_add_builtin(e, "eval", builtin_eval);

  tenv_add_builtin(e, "def", builtin_def);

  tenv_add_builtin(e, "+", builtin_add);
  tenv_add_builtin(e, "-", builtin_sub);
  tenv_add_builtin(e, "*", builtin_mul);
  tenv_add_builtin(e, "/", builtin_div);
}

tval* tval_sexpr_eval(tenv* e, tval *v) {
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = tval_eval(e, v->cell[i]);
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
  if (first->type != TVAL_FUN) {
    tval_del(first);
    tval_del(v);
    return tval_err("first element is not a function!");
  }

  tval* result = first->fun(e, v);
  free(first);
  return result;
}

tval* tval_eval(tenv* e, tval* v) {
  if (v->type == TVAL_SYM) {
    tval* x = tenv_get(e, v);
    tval_del(v);
    return x;
  }
  if (v->type == TVAL_SEXPR) {
    return tval_sexpr_eval(e, v);
  }
  return v;
}

