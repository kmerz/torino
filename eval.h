
enum { TVAL_NUM, TVAL_ERR };
enum { TERR_DIV_ZERO, TERR_BAD_OP, TERR_BAD_NUM };

typedef union {
  int type;
  long num;
  int err;
} tval;

tval eval(mpc_ast_t*);

void tval_println(tval);
void tval_print(tval);
