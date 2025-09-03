#include "mpc.h"

struct tval;
struct tenv;

typedef struct tval tval;
typedef struct tenv tenv;

enum { TVAL_FALSE, TVAL_TRUE };

enum { TVAL_NUM, TVAL_ERR, TVAL_SYM, TVAL_SEXPR, TVAL_QEXPR, TVAL_FUN,
  TVAL_EXIT, TVAL_BOOL };

typedef tval*(*tbuiltin)(tenv*, tval*);

struct tval {
  int type;

  /* basic */
  long num;
  char* sym;
  char* err;
  int boolean;

  /* function */
  tbuiltin builtin;
  tenv *env;
  tval* formals;
  tval* body; 
 
  /* Expressions */
  int count;
  struct tval** cell;
};

struct tenv {
  tenv* par;

  int count;
  char** syms;
  tval** vals;
};

void tval_println(tval*);
void tval_print(tval*);
void tval_print_expr(char, char, tval*);
tval* tval_read(mpc_ast_t*);
tval* tval_eval(tenv*, tval*);
void tval_del(tval*);
tval* tval_add(tval*, tval*);
tval* tval_eq(tval*, tval*);
tval* tval_if(tval*, tval*);

tval* tval_copy(tval*);
tval* tval_pop(tval*, int);
tval* tval_take(tval*, int);

tval* tval_err(char *, ...);
tval* tval_num(long x);
tval* tval_exit(void);
tval* tval_sym(char* sym);
tval* tval_sexpr(void);
tval* tval_qexpr(void);
tval* tval_fun(tbuiltin fun);
tval* tval_lambda(tval* formals, tval* body);
tval* tval_bool(int);

tval* tval_read_num(mpc_ast_t*);

tenv* tenv_new(void);
void tenv_del(tenv*);
void tenv_add_builtins(tenv*);
tenv* tenv_copy(tenv*);
void tenv_put(tenv*, tval*, tval*);
void tenv_def(tenv*, tval*, tval*);
tval* tenv_get(tenv*, tval*);

tval* builtin_eval(tenv*, tval*);
tval* builtin_list(tenv*, tval*);

#define TASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    tval* err = tval_err(fmt, ##__VA_ARGS__); \
    tval_del(args); \
    return err; \
  }

#define TASSERT_NUM(name, args, num) \
  if (!args->count == 2) { \
    tval* err = tval_err("%s: Wrong number of values. Expected %d, got %d",name, num, args->count); \
    tval_del(args); \
    return err; \
  }

#define TASSERT_TYPE(name, args, i, type_name) \
  if (!args->cell[i]->type == type_name) { \
    tval* err = tval_err("%s: Wrong type. Expected %s, got %s", name, ttype_name(type_name), \
	ttype_name(args->cell[i]->type)); \
    tval_del(args); \
    return err; \
  }
