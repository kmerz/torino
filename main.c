#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "mpc.h"
#include "eval.h"

int main(int argc, char** argv) {
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* Sexpr = mpc_new("sexpr");
  mpc_parser_t* Qexpr = mpc_new("qexpr");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Torino = mpc_new("torino");

  mpca_lang(MPCA_LANG_DEFAULT,
      "									\
      number	: /-?[0-9]+/ ;						\
      symbol	: /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;			\
      sexpr	: '(' <expr>* ')';					\
      qexpr	: '{' <expr>* '}';					\
      expr	: <number> | <symbol> | <sexpr> | <qexpr> ;		\
      torino	: /^/ <expr>* /$/ ;               			\
      ",
      Number, Symbol, Sexpr, Qexpr, Expr, Torino);

  puts("Torino Version 0.0.0.0.1");
  puts("Press Ctrl+c or type 'exit' to Exit\n");

  tenv* e = tenv_new();
  tenv_add_builtins(e);

  while(1) {
    char* input = readline("torino> ");

    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Torino, &r)) {
      tval* result = tval_eval(e, tval_read(r.output));
      tval_println(result);
      mpc_ast_delete(r.output);
      if (result->type == TVAL_EXIT) {
        tval_del(result);
        free(input);
        break;
      }
      tval_del(result);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  tenv_del(e);

  mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Torino);

  return 0;
}
