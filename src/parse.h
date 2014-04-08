#ifndef IGOR_PARSE
#define IGOR_PARSE

typedef struct _ival {
  int type;
  long num;
  int err;
} ival;


enum { IVAL_LONG, IVAL_DOUBLE, IVAL_ERR };

enum { IERR_DIV_ZERO, IERR_BAD_OP, IERR_BAD_NUM};

void ival_print(ival v);
void ival_println(ival v);
ival eval(mpc_ast_t* t);

#endif
