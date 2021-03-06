#include <stdio.h>
#include <stdlib.h>

#include "../lib/mpc.h"
#include "parse.h"

#ifdef _WIN32
#include <string.h>
static char buffer[2048];
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  ctrcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char* unused) {}
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif // _WIN32

int main(int argc, char** argv) {
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Symbol   = mpc_new("symbol");
  mpc_parser_t* Sexpr    = mpc_new("sexpr");
  mpc_parser_t* Qexpr    = mpc_new("qexpr");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Igor     = mpc_new("igor");

  mpca_lang(MPC_LANG_DEFAULT,
    "                                                   \
    number : /-?[0-9]+/ ;                               \
    symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;         \
    sexpr  : '(' <expr>* ')' ;                          \
    qexpr  : '{' <expr>* '}' ;                          \
    expr   : <number> | <symbol> | <sexpr> | <qexpr> ;  \
    igor   : /^/ <expr>* /$/ ; \
    ",
    Number, Symbol, Sexpr, Qexpr, Expr, Igor);

  puts("Igor Version 0.0.1");

  ienv* e = ienv_new();
  ienv_add_builtins(e);

  while(1) {
    char* input = readline("igor> ");
    if(strstr(input, "exit")) break;
    if(strstr(input, "help")) {
      printf("Igor current support reverse poslish notation with integer numbers\n");
      continue;
    }
    add_history(input);
    mpc_result_t r;
    if(mpc_parse("<stdin>", input, Igor, &r)) {
      ival* x = ival_eval(e, ival_read(r.output));
      ival_println(x);
      ival_del(x);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    free(input);
  }
  ienv_del(e);

  mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Igor);
  return 0;
}
