#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"
#include "eval.h"

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

tval* tval_exit(void) {
  tval* v = malloc(sizeof(tval));
  v->type = TVAL_EXIT;
  return v;
}

tval* tval_sym(char* sym) {
  tval* v = malloc(sizeof(tval));
  v->count = 0;
  v->type = TVAL_SYM;
  size_t len = strlen(sym) + 1;
  v->sym = malloc(len);
  strncpy(v->sym, sym, len);
  return v;
}

tval* tval_str(char* str) {
  tval* v = malloc(sizeof(tval));
  v->type = TVAL_STR;
  v->str = malloc(strlen(str) + 1);
  strcpy(v->str, str);
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
  v->cell = NULL;
  v->sym = NULL;
  v->err = NULL;
  v->type = TVAL_FUN;
  v->builtin = func;
  return v;
}

tval* tval_lambda(tval* formals, tval* body) {
  tval* v = malloc(sizeof(tval));
  v->type = TVAL_FUN;
  v->builtin = NULL;

  v->env = tenv_new();

  v->formals = formals;
  v->body = body;
}

tval* tval_bool(int val) {
  tval* v = malloc(sizeof(tval));
  v->type = TVAL_BOOL;
  v->boolean = val;
  v->num = 0;
}

tval* tval_eq(tval* l, tval* r) {
  if (r->type != l->type) {
    return tval_bool(TVAL_FALSE);;
  }

  switch(r->type) {
    case TVAL_BOOL: return r->boolean == l->boolean ?
      tval_bool(TVAL_TRUE) : tval_bool(TVAL_FALSE);
    case TVAL_NUM: return r->num == l->num ?
      tval_bool(TVAL_TRUE) : tval_bool(TVAL_FALSE);
    case TVAL_SYM: return strcmp(r->sym, l->sym) == 0 ?
      tval_bool(TVAL_TRUE) : tval_bool(TVAL_FALSE);
    case TVAL_STR: return strcmp(r->str, l->str) == 0 ?
      tval_bool(TVAL_TRUE) : tval_bool(TVAL_FALSE);
    case TVAL_ERR: return strcmp(r->err, l->err) == 0 ?
      tval_bool(TVAL_TRUE) : tval_bool(TVAL_FALSE);
    case TVAL_QEXPR:
    case TVAL_SEXPR:
      if (l->count != r->count) {
	return tval_bool(TVAL_FALSE);
      }
      for (int i = 0; i < l->count; i++) {
	if (!tval_eq(l->cell[i], r->cell[i])) {
	  return tval_bool(TVAL_FALSE);
	}
      }
      return tval_bool(TVAL_TRUE);
    case TVAL_FUN:
      if (l->formals->count != r->formals->count) {
	return tval_bool(TVAL_FALSE);
      }
      if (l->body->count != r->body->count) {
	return tval_bool(TVAL_FALSE);
      }
      for (int i = 0; i < l->formals->count; i++) {
	if (!tval_eq(l->formals->cell[i], r->formals->cell[i])) {
	  return tval_bool(TVAL_FALSE);
	}
      }
      for (int i = 0; i < l->body->count; i++) {
	if (!tval_eq(l->body->cell[i], r->body->cell[i])) {
	  return tval_bool(TVAL_FALSE);
	}
      }
      return tval_bool(TVAL_TRUE);
    case TVAL_EXIT: return tval_bool(TVAL_TRUE);
  }
}

tval* tval_if(tval *result, tval* a) {
  switch(result->type) {
    case TVAL_BOOL: return result->boolean == TVAL_TRUE ? tval_take(a, 0) : tval_take(a, 1);
    case TVAL_NUM: return result->num > 0 ? tval_take(a, 0) : tval_take(a, 1);
    case TVAL_EXIT:
    case TVAL_ERR: return result;
    case TVAL_SEXPR:
    case TVAL_QEXPR:
    case TVAL_FUN: return tval_take(a, 0);
  } 
  return tval_take(a, 1);
}

void tval_del(tval *v) {
  switch(v->type) {
    case TVAL_EXIT: break;
    case TVAL_BOOL: break;
    case TVAL_NUM: break;
    case TVAL_FUN: 
      if(!v->builtin) {
        tenv_del(v->env);
	tval_del(v->formals);
	tval_del(v->body);
      }
      break;
    case TVAL_ERR: free(v->err); break;
    case TVAL_SYM: free(v->sym); break;
    case TVAL_STR: free(v->str); break;
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

tval* tval_read_bool(mpc_ast_t* t) {
  if (strcmp(t->contents, "true") == 0) {
    return tval_bool(TVAL_TRUE);
  } else {
    return tval_bool(TVAL_FALSE);
  }
}

tval* tval_read_str(char* str) {
  str[strlen(str) - 1] = '\0';
  char* unescaped = malloc(strlen(str+1)+1);
  strcpy(unescaped, str+1);

  unescaped = mpcf_unescape(unescaped);
  tval* tstr = tval_str(unescaped);
  
  free(unescaped);
  return tstr;
}

tval* tval_add(tval* v, tval* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(tval*) * v->count);
  v->cell[v->count - 1] = x;
  return v;
}

tval* tval_read(mpc_ast_t* t) {
  if (strstr(t->tag, "boolean")) { return tval_read_bool(t); }
  if (strstr(t->tag, "number")) { return tval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return tval_sym(t->contents); }
  if (strstr(t->tag, "string")) { return tval_read_str(t->contents); }

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
	strstr(t->children[i]->tag, "comment") ||
        strcmp(t->children[i]->tag, "regex") == 0) {
      continue;
    }
    x = tval_add(x, tval_read(t->children[i]));
  }

  return x;
}

void tval_print_str(tval* v) {
  char* escaped = malloc(strlen(v->str) + 1);
  strcpy(escaped, v->str);
  escaped = mpcf_escape(escaped);
  printf("\"%s\"", escaped);
  free(escaped);
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
    case TVAL_EXIT: printf("exit"); break;
    case TVAL_BOOL: v->boolean ? printf("true") : printf("false"); break;
    case TVAL_NUM: printf("%li", v->num); break;
    case TVAL_ERR: printf("%s", v->err); break;
    case TVAL_STR: tval_print_str(v); break;
    case TVAL_SYM: printf("%s", v->sym); break;
    case TVAL_FUN: 
      if(v->builtin) {
        printf("<builtin>");
      } else {
	printf("(\\ "); tval_print(v->formals);
	putchar(' '); tval_print(v->body); putchar(')');
      }
      break;
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
    case TVAL_BOOL:
      c->boolean = v->boolean;
      break;
    case TVAL_NUM:
      c->num = v->num;
      break;
    case TVAL_FUN:
      if (v->builtin) {
        c->builtin = v->builtin;
      } else {
	c->builtin = NULL;
	c->env = tenv_copy(v->env);
	c->formals = tval_copy(v->formals);
	c->body = tval_copy(v->body);
      }
      break;
    case TVAL_ERR:
      len = strlen(v->err) + 1;
      c->err = malloc(len);
      strncpy(c->err, v->err, len);
      break;
    case TVAL_SYM:
      len = strlen(v->sym) + 1;
      c->sym = malloc(len);
      strncpy(c->sym, v->sym, len);
      break;
    case TVAL_STR:
      len = strlen(v->str) + 1;
      c->str = malloc(len);
      strncpy(c->str, v->str, len);
      break;
    case TVAL_SEXPR:
    case TVAL_QEXPR:
      c->count = v->count;
      c->cell = malloc(sizeof(tval*) * v->count);
      for (int i = 0; i < v->count; i++) {
	c->cell[i] = tval_copy(v->cell[i]);
      }
      break;
    case TVAL_EXIT:
      break;
  }

  return c;
}

tval* tval_take(tval* v, int i) {
  tval* x = tval_pop(v, i);
  tval_del(v);

  return x;
}
