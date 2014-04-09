#ifndef IGOR_PARSE
#define IGOR_PARSE

#include "../lib/mpc.h"

struct ival;
struct ienv;
typedef struct ival ival;
typedef struct ienv ienv;

enum { IVAL_ERR, IVAL_NUM, IVAL_SYM, IVAL_FUN, IVAL_SEXPR, IVAL_QEXPR };

typedef ival*(*ibuiltin)(ienv*, ival*);

struct ival {
  int type;

  long num;

  /* Error and Symbol types have some string data */
  char* err;
  char* sym;
  ibuiltin fun;

  /* Count and Pointer to a list of "ival*" */
  int count;
  struct ival** cell;

};

struct ienv {
  int count;
  char** syms;
  ival** vals;
};

ienv* ienv_new(void);
void ienv_del(ienv* e);
void ienv_add_builtins(ienv* e);
ival* ival_eval(ienv* e, ival* v);
ival* ival_read(mpc_ast_t* t);
void ival_println(ival* v);
void ival_del(ival* v);
void ienv_del(ienv* e);
ival* ival_num(long x);
ival* ival_read_num(mpc_ast_t* t);

#endif
