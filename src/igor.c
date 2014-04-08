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
  mpc_parser_t* Double   = mpc_new("double");
  mpc_parser_t* Integer  = mpc_new("integer");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Igor     = mpc_new("igor");

  mpca_lang(MPC_LANG_DEFAULT,
    "                                                                    \
      integer  : /[0-9]+/ ;                                              \
      double   : /[0-9]+\\.[0-9]*/ ;                                     \
      number   : <double> | <integer> ;                                  \
      operator : '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\" ; \
      expr     : <number> | '(' <operator> <expr>+ ')' ;                 \
      igor     : /^/ <operator> <expr>+ /$/ ;                            \
    ",
    Integer, Double, Number, Operator, Expr, Igor);


  puts("Igor Version 0.0.0.0.1");

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
      ival result = eval(r.output);
      ival_println(result);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    free(input);
  }

  mpc_cleanup(6, Integer, Double, Number, Operator, Expr, Igor);
  return 0;
}
