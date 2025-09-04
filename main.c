#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "mpc.h"
#include "eval.h"

mpc_parser_t* Torino = NULL;

int main(int argc, char** argv) {
  mpc_parser_t* Comment = mpc_new("comment");
  mpc_parser_t* Boolean = mpc_new("boolean");
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* String = mpc_new("string");
  mpc_parser_t* Sexpr = mpc_new("sexpr");
  mpc_parser_t* Qexpr = mpc_new("qexpr");
  mpc_parser_t* Expr = mpc_new("expr");
  Torino = mpc_new("torino");

  mpca_lang(MPCA_LANG_DEFAULT,
      "									\
      comment	:  /;[^\\r\\n]*/ ;					\
      boolean	: \"true\" | \"false\";					\
      number	: /-?[0-9]+/ ;						\
      string	: /\"(\\\\.|[^\"])*\"/ ;				\
      symbol	: /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;			\
      sexpr	: '(' <expr>* ')';					\
      qexpr	: '{' <expr>* '}';					\
      expr	: <boolean> | <number> | <symbol> | <string> |		\
        <sexpr> | <qexpr> | <comment>;					\
      torino	: /^/ <expr>* /$/ ;               			\
      ",
      Comment, Boolean, Number, Symbol, String, Sexpr, Qexpr, Expr, Torino);

  tenv* e = tenv_new();
  tenv_add_builtins(e);

  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      tval* args = tval_add(tval_sexpr(), tval_str(argv[i]));
      tval* x = builtin_load(e, args);
      if (x->type == TVAL_ERR) {
	tval_print(x);
      }
      tval_del(x);
    }
  } else {
    puts("Torino Version 0.0.0.0.1");
    puts("Press Ctrl+c or type 'exit' to Exit\n");

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
  }

  tenv_del(e);

  mpc_cleanup(9, Comment,Boolean, Number, Symbol, String, Sexpr, Qexpr,
    Expr, Torino);

  return 0;
}
