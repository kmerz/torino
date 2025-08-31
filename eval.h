struct tval;
struct tenv;

typedef struct tval tval;
typedef struct tenv tenv;

enum { TVAL_NUM, TVAL_ERR, TVAL_SYM, TVAL_SEXPR, TVAL_QEXPR, TVAL_FUN };

typedef tval*(*tbuiltin)(tenv*, tval*);

struct tval {
  int type;

  long num;
  char* sym;
  char* err;

  tbuiltin fun;
 
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
