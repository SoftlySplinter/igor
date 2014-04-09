#ifndef IGOR_PARSE
#define IGOR_PARSE

#include "../lib/mpc.h"

typedef struct ival {
  int type;

  long num;

  /* Error and Symbol types have some string data */
  char* err;
  char* sym;

  /* Count and Pointer to a list of "ival*" */
  int count;
  struct ival** cell;

} ival;

enum { IVAL_ERR, IVAL_NUM, IVAL_SYM, IVAL_SEXPR, IVAL_QEXPR };

enum { IERR_DIV_ZERO, IERR_BAD_OP, IERR_BAD_NUM };

/* Construct a pointer to a new Number ival */ 
ival* ival_num(long x);

/* Construct a pointer to a new Error ival */ 
ival* ival_err(char *m);

/* Construct a pointer to a new Symbol ival */ 
ival* ival_sym(char *s);

/* A pointer to a new empty Sexpr ival */
ival* ival_sexpr(void);

/* A pointer to a new empty Qexpr ival */
ival* ival_qexpr(void);

/* Free an ival */
void ival_del(ival* v);

ival* ival_add(ival* v, ival* x);

ival* ival_read_num(mpc_ast_t* t);

ival* ival_read(mpc_ast_t* t);

ival* ival_eval_sexpr(ival* v);

ival* ival_eval(ival* v);

ival* ival_pop(ival* v, int i);

ival* ival_take(ival* v, int i);

ival* builtin(ival* a, char* op);

ival* builtin_op(ival* a, char* op);

ival* builtin_head(ival* a);

ival* builtin_tail(ival* a);

ival* builtin_list(ival* a);

ival* builtin_eval(ival* a);

ival* builtin_join(ival* a);

ival* ival_join(ival* x, ival* y);

void ival_expr_print(ival* v, char open, char close);

/* Print an ival */
void ival_print(ival* v);

/* Println an ival */
void ival_println(ival* v);

#endif
