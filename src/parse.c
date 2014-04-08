#include "../lib/mpc.h"
#include "parse.h"

ival ival_num(long x) {
  ival v;
  v.type = IVAL_LONG;
  v.num = x;
  return v;
}

ival ival_err(int x) {
  ival v;
  v.type = IVAL_ERR;
  v.err = x;
  return v;
}

void ival_print(ival v) {
  switch (v.type) {
  case IVAL_LONG: 
    printf("%li", v.num);
    break;
  case IVAL_ERR:
    switch(v.err) {
    case IERR_DIV_ZERO:
      printf("Error: Division by zero");
      break;
    case IERR_BAD_OP:
      printf("Error: Invalid operator");
      break;
    case IERR_BAD_NUM:
      printf("Error: Inavalid number");
      break;
    }
  }
}

void ival_println(ival v) {
  ival_print(v);
  putchar('\n');
}

ival eval_op(ival x, char* op, ival y) {
  if(x.type == IVAL_ERR) { return x; }
  if(y.type == IVAL_ERR) { return y; }
  if(strcmp(op, "+") == 0) { return ival_num(x.num + y.num); }
  if(strcmp(op, "-") == 0) { return ival_num(x.num - y.num); }
  if(strcmp(op, "*") == 0) { return ival_num(x.num * y.num); }
  if(strcmp(op, "/") == 0) { return y.num == 0 ? ival_err(IERR_DIV_ZERO) : ival_num(x.num / y.num); }
  if(strcmp(op, "%") == 0) { return y.num == 0 ? ival_err(IERR_DIV_ZERO) : ival_num(x.num % y.num); }
  if(strcmp(op, "max") == 0) { return x.num > y.num ? ival_num(x.num) : ival_num(y.num); }
  if(strcmp(op, "min") == 0) { return x.num < y.num ? ival_num(x.num) : ival_num(y.num); }
  return ival_err(IERR_BAD_OP);
}

ival eval(mpc_ast_t* t) {
  if(strstr(t->tag, "number")) {
    long x = strtol(t->contents, NULL ,10);
    return errno != ERANGE ? ival_num(x) : ival_err(IERR_BAD_NUM);
  }

  char *op = t->children[1]->contents;
  ival x = eval(t->children[2]);

  int i = 3;
  while(strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  return x;
}
