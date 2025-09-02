#include <stdio.h>
#include <stdlib.h>

#include "eval.h"

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
  strncpy(e->syms[e->count -1], k->sym, len);
}

void tenv_def(tenv *e, tval* k, tval* v) {
  while(e->par) {
    e = e->par;
  }

  tenv_put(e, k, v);
}

tenv* tenv_copy(tenv* e) {
  tenv* n = malloc(sizeof(tenv));
  n->par = e->par;
  n->count = e->count;
  n->syms = malloc(sizeof(char *) * n->count);
  n->vals = malloc(sizeof(tval *) * n->count);
  for (int i = 0; i < n->count; i++) {
    size_t len = strlen(e->syms[i]) + 1;
    n->syms[i] = malloc(len);
    strncpy(n->syms[i], e->syms[i], len);
    n->vals[i] = tval_copy(e->vals[i]);
  }
  return n;
}

tenv* tenv_new(void) {
  tenv* e = malloc(sizeof(tenv));
  e->par = NULL;
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

  if (e->par) {
    return tenv_get(e->par, k);
  } else {
    return tval_err("Unbound symbol '%s'!", k->sym);
  }
}
