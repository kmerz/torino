#include <stdio.h>
#include <bsd/bsd.h>

#include "eval.h"

char* ttype_name(int i) {
  switch(i) {
    case TVAL_NUM: return "Number";
    case TVAL_ERR: return "Error";
    case TVAL_FUN: return "Function";
    case TVAL_SYM: return "Symbol";
    case TVAL_STR: return "String";
    case TVAL_SEXPR: return "S-Expresion";
    case TVAL_QEXPR: return "Q-Expresion";
    case TVAL_EXIT: return "Exit";
    default: return "Unknwon";
  }
}

tval* tval_call(tenv* e, tval* f, tval* a) {
  if (f->builtin) {
    return f->builtin(e, a);
  }

  int given = a->count;
  int total = f->formals->count;

  while(a->count) {
    if (f->formals->count == 0) {
      tval_del(a);
      return tval_err("Function passed too many arguments"
	  "Got %i, expected %i!", given, total);
    }

    tval* sym = tval_pop(f->formals, 0);
    if (strcmp(sym->sym, "&") == 0) {
      if (f->formals->count != 1) {
	tval_del(a);
	return tval_err("Function format invalid. "
	    "Symbol '&' not followed by single symbol.");
      }

      tval* nsym = tval_pop(f->formals, 0);
      tenv_put(f->env, nsym, builtin_list(e, a));
      tval_del(sym);
      tval_del(nsym);
      break;
    }

    tval* val = tval_pop(a, 0);

    tenv_put(f->env, sym, val);

    tval_del(sym);
    tval_del(val);
  }

  tval_del(a);

  if (f->formals->count > 0 &&
      strcmp(f->formals->cell[0]->sym, "&") == 0) {

    if (f->formals->count != 2) {
      return tval_err("Function format invalid. "
	  "Symbol '&' not followed by single symbol.");
    }

    tval_del(tval_pop(f->formals, 0));
    tval* sym = tval_pop(f->formals, 0);
    tval* val = tval_qexpr();
    tenv_put(f->env, sym, val);

    tval_del(sym);
    tval_del(val);
  }

  if (f->formals->count == 0) {
    f->env->par = e;
    return builtin_eval(f->env, tval_add(tval_sexpr(), tval_copy(f->body)));
  } else {
    return tval_copy(f);
  }
}

tval* builtin_if(tenv* e, tval* a) {
  TASSERT_NUM("if", a, 3);
  TASSERT_TYPE("if", a, 0, TVAL_SEXPR);
  TASSERT_TYPE("if", a, 1, TVAL_QEXPR);
  TASSERT_TYPE("if", a, 2, TVAL_QEXPR);

  tval* if_expr = tval_pop(a, 0);
  tval* result = tval_if(if_expr, a);
  return builtin_eval(e, tval_add(tval_sexpr(), result));
}

tval* builtin_cmp(tenv* e, tval* a, char* op) {
  TASSERT(a, a->count >= 2,
      "For comparision we need more then one argument Got %d.",
      a->count);

  tval* result;
  tval* left = tval_pop(a, 0);
  while(a->count) {
    tval* right = tval_pop(a, 0);
    result = tval_eq(left, right);  
    if (result->boolean == TVAL_FALSE) {
      tval_del(right);
      tval_del(left);
      if (strcmp(op, "!=") == 0) {
	result->boolean = TVAL_TRUE;
      }
      return result;
    }
    tval* old_left = left;
    left = right;
    tval_del(old_left);
  }
  tval_del(left);

  if (strcmp(op, "!=") == 0) {
    result->boolean = TVAL_FALSE;
  }
  return result;
}

tval* builtin_ord(tenv* e, tval* a, char* op) {
  TASSERT_NUM(op, a, 2);
  for (int i = 0; i < a->count; i++) {
    TASSERT(a, a->cell[i]->type == TVAL_NUM,
	"Cannot operat on non numeral value! Got %s.",
	ttype_name(a->cell[i]->type));
  }

  tval* l = tval_pop(a, 0);
  tval* r = tval_pop(a, 0);

  if (strcmp(op, "<") == 0) { return tval_num(l->num < r->num); }
  if (strcmp(op, "<=") == 0) { return tval_num(l->num <= r->num); }
  if (strcmp(op, ">") == 0) { return tval_num(l->num > r->num); }
  if (strcmp(op, ">=") == 0) { return tval_num(l->num >= r->num); }

  return tval_err("Unknown ordering operator '%s'", op);
}

tval* builtin_less(tenv* e, tval* a) {
  return builtin_ord(e, a, "<");
}

tval* builtin_less_equal(tenv* e, tval* a) {
  return builtin_ord(e, a, "<=");
}

tval* builtin_greater(tenv* e, tval* a) {
  return builtin_ord(e, a, ">");
}

tval* builtin_greater_equal(tenv* e, tval* a) {
  return builtin_ord(e, a, ">=");
}

tval* builtin_equal(tenv* e, tval* a) {
  return builtin_cmp(e, a, "==");
}

tval* builtin_unequal(tenv* e, tval* a) {
  return builtin_cmp(e, a, "!=");
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

tval* builtin_var(tenv* e, tval* a, char* func) {
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
    if (strcmp(func, "def") == 0) {
      tenv_def(e, syms->cell[i], a->cell[i+1]);
    }

    if (strcmp(func, "=") == 0) {
      tenv_put(e, syms->cell[i], a->cell[i+1]);
    }
  }

  tval_del(a);

  return tval_sexpr();
}

tval* builtin_def(tenv* e, tval* a) {
  return builtin_var(e, a, "def");
}

tval* builtin_put(tenv* e, tval* a) {
  return builtin_var(e, a, "=");
}

tval* builtin_fun(tenv* e, tval* a) {
  TASSERT_NUM("fun", a, 2);
  TASSERT_TYPE("fun", a, 0, TVAL_QEXPR);
  TASSERT_TYPE("fun", a, 1, TVAL_QEXPR);

  for (int i = 0; i < a->cell[0]->count; i++) {
    TASSERT(a, a->cell[0]->cell[i]->type == TVAL_SYM,
	"'fun': Only can only define symbols, got %s.",
	ttype_name(a->cell[0]->cell[i]->type));
  }

  tval* formals = tval_pop(a, 0);
  tval* f_sym = tval_pop(formals, 0);
  tval* body = tval_pop(a, 0);
  tval* lambda = tval_lambda(formals, body);
  tenv_def(e, f_sym, lambda);
  return lambda;

}

tval* builtin_lambda(tenv* e, tval* a) {
  TASSERT_NUM("\\", a, 2);
  TASSERT_TYPE("\\", a, 0, TVAL_QEXPR);
  TASSERT_TYPE("\\", a, 1, TVAL_QEXPR);

  for (int i = 0; i < a->cell[0]->count; i++) {
    TASSERT(a, a->cell[0]->cell[i]->type == TVAL_SYM,
	"'\\': Only can only define symbols, got %s.",
	ttype_name(a->cell[0]->cell[i]->type));
  }

  tval* formals = tval_pop(a, 0);
  tval* body = tval_pop(a, 0);
  tval_del(a);

  return tval_lambda(formals, body);
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

tval* builtin_load(tenv* e, tval* a) {
  TASSERT_NUM("load", a, 1);
  TASSERT_TYPE("load", a, 0, TVAL_STR);

  mpc_result_t r;
  if (mpc_parse_contents(a->cell[0]->str, Torino, &r)) {
    tval* expr = tval_read(r.output);
    mpc_ast_delete(r.output);

    while(expr->count) {
      tval* x = tval_eval(e, tval_pop(expr, 0));
      if (x->type == TVAL_ERR) {
	tval_println(x);
      }
      tval_del(x);
    }

    tval_del(expr);
    tval_del(a);
    return tval_sexpr();
  } else {
    char* err_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);

    tval* err = tval_err("Could not load Library %s", err_msg);
    free(err_msg);
    tval_del(a);

    return err;
  }
}

tval* builtin_print(tenv* e, tval* a) {
  for (int i = 0; i < a->count; i++) {
    tval_print(a->cell[i]);
    putchar(' ');
  }

  putchar('\n');
  tval_del(a);

  return tval_sexpr();
}

tval* builtin_error(tenv* e, tval* a) {
  TASSERT_NUM("error", a, 1);
  TASSERT_TYPE("error", a, 0, TVAL_STR);

  tval* err = tval_err(a->cell[0]->str);

  tval_del(a);
  return err;
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
  tenv_add_builtin(e, "=", builtin_put);
  tenv_add_builtin(e, "\\", builtin_lambda);
  tenv_add_builtin(e, "fun", builtin_fun);

  tenv_add_builtin(e, "+", builtin_add);
  tenv_add_builtin(e, "-", builtin_sub);
  tenv_add_builtin(e, "*", builtin_mul);
  tenv_add_builtin(e, "/", builtin_div);

  tenv_add_builtin(e, "<", builtin_less);
  tenv_add_builtin(e, "<=", builtin_less_equal);
  tenv_add_builtin(e, ">", builtin_greater);
  tenv_add_builtin(e, ">=", builtin_greater_equal);

  tenv_add_builtin(e, "==", builtin_equal);
  tenv_add_builtin(e, "!=", builtin_unequal);

  tenv_add_builtin(e, "if", builtin_if);
  tenv_add_builtin(e, "load", builtin_load);
  tenv_add_builtin(e, "print", builtin_print);
  tenv_add_builtin(e, "error", builtin_error);

  tenv_put(e, tval_sym("exit"), tval_exit());
}

tval* tval_sexpr_eval(tenv* e, tval *v) {
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = tval_eval(e, v->cell[i]);
  }

  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == TVAL_ERR || v->cell[i]->type == TVAL_EXIT) {
      return tval_take(v, i);
    }
  }

  if (v->count == 0) {
    return v;
  }

  if (v->count == 1) {
    return tval_take(v, 0);
  }

  tval* f = tval_pop(v, 0);
  if (f->type != TVAL_FUN) {
    tval* err = tval_err(
	"S-Expression starts with a incorrect type!"
	"Got %s, expected %s.", ttype_name(f->type), ttype_name(TVAL_FUN));
    tval_del(f);
    tval_del(v);
    return tval_err("f element is not a function!");
  }

  tval* result = tval_call(e, f, v);
  free(f);
  return result;
}

tval* tval_eval(tenv* e, tval* v) {
  if (v->type == TVAL_EXIT) {
    return v;
  }
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

