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
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Torino = mpc_new("torino");

  mpca_lang(MPCA_LANG_DEFAULT,
      "									\
      number	: /-?[0-9]+/ ;				\
      symbol	: '+' | '-' | '*' | '/' | '%' |				\
        \"add\" | \"sub\" | \"max\" | \"min\"; 				\
      sexpr	: '(' <expr>* ')';					\
      expr	: <number> | <symbol> | <sexpr> ;			\
      torino	: /^/ <expr>* /$/ ;               			\
      ",
      Number, Symbol, Sexpr, Expr, Torino);

  puts("Torino Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  while(1) {
    char* input = readline("torino> ");

    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Torino, &r)) {
      tval* result = tval_eval(tval_read(r.output));
      tval_println(result);
      tval_del(result);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  mpc_cleanup(4, Number, Symbol, Sexpr, Expr, Torino);

  return 0;
}
