
enum { TVAL_NUM, TVAL_ERR, TVAL_SYM, TVAL_SEXPR, TVAL_QEXPR };
enum { TERR_DIV_ZERO, TERR_BAD_OP, TERR_BAD_NUM };

typedef struct tval {
  int type;
  long num;
  char* sym;
  char* err;
  int count;
  struct tval** cell;
} tval;

//tval eval(mpc_ast_t*);

void tval_println(tval*);
void tval_print(tval*);
tval* tval_read(mpc_ast_t*);
void tval_del(tval*);
tval* tval_eval(tval*);
