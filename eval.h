struct tval;
struct tenv;

typedef struct tval tval;
typedef struct tenv tenv;

enum { TVAL_NUM, TVAL_ERR, TVAL_SYM, TVAL_SEXPR, TVAL_QEXPR, TVAL_FUN,
  TVAL_EXIT };

typedef tval*(*tbuiltin)(tenv*, tval*);

struct tval {
  int type;

  /* basic */
  long num;
  char* sym;
  char* err;

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
  int count;
  char** syms;
  tval** vals;
};

void tval_println(tval*);
void tval_print(tval*);
tval* tval_read(mpc_ast_t*);
void tval_del(tval*);
tval* tval_eval(tenv*, tval*);

tenv* tenv_new(void);
void tenv_del(tenv*);
void tenv_add_builtins(tenv*);
