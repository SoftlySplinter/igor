#include <stdlib.h>
#include "../lib/mpc.h"
#include "parse.h"

ival* ival_num(long x) {
  ival* v = malloc(sizeof(ival));
  v->type = IVAL_NUM;
  v->num = x;
  return v;
}

ival* ival_err(char* fmt, ...) {
  ival* v = malloc(sizeof(ival));
  v->type = IVAL_ERR;
  
  /* Create a va list and initialize it */
  va_list va;
  va_start(va, fmt);
  
  /* Allocate 512 bytes of space */
  v->err = malloc(512);
  
  /* printf into the error string with a maximum of 511 characters */
  vsnprintf(v->err, 511, fmt, va);
  
  /* Reallocate to number of bytes actually used */
  v->err = realloc(v->err, strlen(v->err)+1);
  
  /* Cleanup our va list */
  va_end(va);
  
  return v;
}

ival* ival_sym(char* s) {
  ival* v = malloc(sizeof(ival));
  v->type = IVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

ival* ival_fun(ibuiltin func) {
  ival* v = malloc(sizeof(ival));
  v->type = IVAL_FUN;
  v->fun = func;
  return v;
}

ival* ival_sexpr(void) {
  ival* v = malloc(sizeof(ival));
  v->type = IVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

ival* ival_qexpr(void) {
  ival* v = malloc(sizeof(ival));
  v->type = IVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

void ival_del(ival* v) {

  switch (v->type) {
    case IVAL_NUM: break;
    case IVAL_FUN: break;
    case IVAL_ERR: free(v->err); break;
    case IVAL_SYM: free(v->sym); break;
    case IVAL_QEXPR:
    case IVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
        ival_del(v->cell[i]);
      }
      free(v->cell);
    break;
  }
  
  free(v);
}

ival* ival_copy(ival* v) {

  ival* x = malloc(sizeof(ival));
  x->type = v->type;
  
  switch (v->type) {
    
    /* Copy Functions and Numbers Directly */
    case IVAL_FUN: x->fun = v->fun; break;
    case IVAL_NUM: x->num = v->num; break;
    
    /* Copy Strings using malloc and strcpy */
    case IVAL_ERR: x->err = malloc(strlen(v->err) + 1); strcpy(x->err, v->err); break;
    case IVAL_SYM: x->sym = malloc(strlen(v->sym) + 1); strcpy(x->sym, v->sym); break;
    
    /* Copy Lists by copying each sub-expression */
    case IVAL_SEXPR:
    case IVAL_QEXPR:
      x->count = v->count;
      x->cell = malloc(sizeof(ival*) * x->count);
      for (int i = 0; i < x->count; i++) {
        x->cell[i] = ival_copy(v->cell[i]);
      }
    break;
  }
  
  return x;
}

ival* ival_add(ival* v, ival* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(ival*) * v->count);
  v->cell[v->count-1] = x;
  return v;
}

ival* ival_join(ival* x, ival* y) {  
  for (int i = 0; i < y->count; i++) {
    x = ival_add(x, y->cell[i]);
  }
  free(y->cell);
  free(y);  
  return x;
}

ival* ival_pop(ival* v, int i) {
  ival* x = v->cell[i];  
  memmove(&v->cell[i], &v->cell[i+1], sizeof(ival*) * (v->count-i-1));  
  v->count--;  
  v->cell = realloc(v->cell, sizeof(ival*) * v->count);
  return x;
}

ival* ival_take(ival* v, int i) {
  ival* x = ival_pop(v, i);
  ival_del(v);
  return x;
}

void ival_print(ival* v);

void ival_print_expr(ival* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    ival_print(v->cell[i]);    
    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void ival_print(ival* v) {
  switch (v->type) {
    case IVAL_FUN:   printf("<function>"); break;
    case IVAL_NUM:   printf("%li", v->num); break;
    case IVAL_ERR:   printf("Error: %s", v->err); break;
    case IVAL_SYM:   printf("%s", v->sym); break;
    case IVAL_SEXPR: ival_print_expr(v, '(', ')'); break;
    case IVAL_QEXPR: ival_print_expr(v, '{', '}'); break;
  }
}

void ival_println(ival* v) { ival_print(v); putchar('\n'); }

char* ltype_name(int t) {
  switch(t) {
    case IVAL_FUN: return "Function";
    case IVAL_NUM: return "Number";
    case IVAL_ERR: return "Error";
    case IVAL_SYM: return "Symbol";
    case IVAL_SEXPR: return "S-Expression";
    case IVAL_QEXPR: return "Q-Expression";
    default: return "Unknown";
  }
}

/* Lisp Environment */
ienv* ienv_new(void) {

  /* Initialize struct */
  ienv* e = malloc(sizeof(ienv));
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
  
}

void ienv_del(ienv* e) {
  
  /* Iterate over all items in environment deleting them */
  for (int i = 0; i < e->count; i++) {
    free(e->syms[i]);
    ival_del(e->vals[i]);
  }
  
  /* Free allocated memory for lists */
  free(e->syms);
  free(e->vals);
  free(e);
}

ival* ienv_get(ienv* e, ival* k) {
  
  /* Iterate over all items in environment */
  for (int i = 0; i < e->count; i++) {
    /* Check if the stored string matches the symbol string */
    /* If it does, return a copy of the value */
    if (strcmp(e->syms[i], k->sym) == 0) { return ival_copy(e->vals[i]); }
  }
  /* If no symbol found return error */
  return ival_err("Unbound Symbol '%s'", k->sym);
}

void ienv_put(ienv* e, ival* k, ival* v) {
  
  /* Iterate over all items in environment */
  /* This is to see if variable already exists */
  for (int i = 0; i < e->count; i++) {
  
    /* If variable is found delete item at that position */
    /* And replace with variable supplied by user */
    if (strcmp(e->syms[i], k->sym) == 0) {
      ival_del(e->vals[i]);
      e->vals[i] = ival_copy(v);
      return;
    }
  }
  
  /* If no existing entry found then allocate space for new entry */
  e->count++;
  e->vals = realloc(e->vals, sizeof(ival*) * e->count);
  e->syms = realloc(e->syms, sizeof(char*) * e->count);
  
  /* Copy contents of ival and symbol string into new location */
  e->vals[e->count-1] = ival_copy(v);
  e->syms[e->count-1] = malloc(strlen(k->sym)+1);
  strcpy(e->syms[e->count-1], k->sym);
}

/* Builtins */

#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { ival* err = ival_err(fmt, ##__VA_ARGS__); ival_del(args); return err; }

#define LASSERT_TYPE(func, args, index, expect) \
  LASSERT(args, args->cell[index]->type == expect, \
    "Function '%s' passed incorrect type for argument %i. Got %s, Expected %s.", \
    func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define LASSERT_NUM(func, args, num) \
  LASSERT(args, args->count == num, \
    "Function '%s' passed incorrect number of arguments. Got %i, Expected %i.", \
    func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index) \
  LASSERT(args, args->cell[index]->count != 0, \
    "Function '%s' passed {} for argument %i.", func, index);


ival* ival_eval(ienv* e, ival* v);

ival* builtin_list(ienv* e, ival* a) {
  a->type = IVAL_QEXPR;
  return a;
}

ival* builtin_head(ienv* e, ival* a) {
  LASSERT_NUM("head", a, 1);
  LASSERT_TYPE("head", a, 0, IVAL_QEXPR);
  LASSERT_NOT_EMPTY("head", a, 0);
  
  ival* v = ival_take(a, 0);  
  while (v->count > 1) { ival_del(ival_pop(v, 1)); }
  return v;
}

ival* builtin_tail(ienv* e, ival* a) {
  LASSERT_NUM("tail", a, 1);
  LASSERT_TYPE("tail", a, 0, IVAL_QEXPR);
  LASSERT_NOT_EMPTY("tail", a, 0);

  ival* v = ival_take(a, 0);  
  ival_del(ival_pop(v, 0));
  return v;
}

ival* builtin_eval(ienv* e, ival* a) {
  LASSERT_NUM("eval", a, 1);
  LASSERT_TYPE("tail", a, 0, IVAL_QEXPR);
  
  ival* x = ival_take(a, 0);
  x->type = IVAL_SEXPR;
  return ival_eval(e, x);
}

ival* builtin_join(ienv* e, ival* a) {
  
  for (int i = 0; i < a->count; i++) { LASSERT_TYPE("join", a, i, IVAL_QEXPR); }
  
  ival* x = ival_pop(a, 0);
  
  while (a->count) {
    ival* y = ival_pop(a, 0);
    x = ival_join(x, y);
  }
  
  ival_del(a);
  return x;
}

ival* builtin_op(ienv* e, ival* a, char* op) {
  
  for (int i = 0; i < a->count; i++) { LASSERT_TYPE(op, a, i, IVAL_NUM); }
  
  ival* x = ival_pop(a, 0);
  
  if ((strcmp(op, "-") == 0) && a->count == 0) { x->num = -x->num; }
  
  while (a->count > 0) {  
    ival* y = ival_pop(a, 0);
    
    if (strcmp(op, "+") == 0) { x->num += y->num; }
    if (strcmp(op, "-") == 0) { x->num -= y->num; }
    if (strcmp(op, "*") == 0) { x->num *= y->num; }
    if (strcmp(op, "/") == 0) {
      if (y->num != 0) {
        ival_del(x); ival_del(y);
        return ival_err("Division By Zero.");
      }
      x->num /= y->num;
    }
    
    ival_del(y);
  }
  
  ival_del(a);
  return x;
}

ival* builtin_add(ienv* e, ival* a) { return builtin_op(e, a, "+"); }
ival* builtin_sub(ienv* e, ival* a) { return builtin_op(e, a, "-"); }
ival* builtin_mul(ienv* e, ival* a) { return builtin_op(e, a, "*"); }
ival* builtin_div(ienv* e, ival* a) { return builtin_op(e, a, "/"); }

ival* builtin_def(ienv* e, ival* a) {

  LASSERT_TYPE("def", a, 0, IVAL_QEXPR);
  
  /* First argument is symbol list */
  ival* syms = a->cell[0];
  
  /* Ensure all elements of first list are symbols */
  for (int i = 0; i < syms->count; i++) {
    LASSERT(a, (syms->cell[i]->type == IVAL_SYM),
      "Function 'def' cannot define non-symbol. Got %s, Expected %s.",
      ltype_name(syms->cell[i]->type), ltype_name(IVAL_SYM));
  }
  
  /* Check correct number of symbols and values */
  LASSERT(a, (syms->count == a->count-1),
    "Function 'def' passed too many arguments for symbols. Got %i, Expected %i.",
    syms->count, a->count-1);
  
  /* Assign copies of values to symbols */
  for (int i = 0; i < syms->count; i++) {
    ienv_put(e, syms->cell[i], a->cell[i+1]);
  }
  
  ival_del(a);
  return ival_sexpr();
}

void ienv_add_builtin(ienv* e, char* name, ibuiltin func) {
  ival* k = ival_sym(name);
  ival* v = ival_fun(func);
  ienv_put(e, k, v);
  ival_del(k); ival_del(v);
}

void ienv_add_builtins(ienv* e) {
  /* Variable Functions */
  ienv_add_builtin(e, "def",  builtin_def);
  
  /* List Functions */
  ienv_add_builtin(e, "list", builtin_list);
  ienv_add_builtin(e, "head", builtin_head); ienv_add_builtin(e, "tail",  builtin_tail);
  ienv_add_builtin(e, "eval", builtin_eval); ienv_add_builtin(e, "join",  builtin_join);
  
  /* Mathematical Functions */
  ienv_add_builtin(e, "+",    builtin_add); ienv_add_builtin(e, "-",     builtin_sub);
  ienv_add_builtin(e, "*",    builtin_mul); ienv_add_builtin(e, "/",     builtin_div);
}

/* Evaluation */

ival* ival_eval_sexpr(ienv* e, ival* v) {
  
  for (int i = 0; i < v->count; i++) { v->cell[i] = ival_eval(e, v->cell[i]); }
  for (int i = 0; i < v->count; i++) { if (v->cell[i]->type == IVAL_ERR) { return ival_take(v, i); } }
  
  if (v->count == 0) { return v; }  
  if (v->count == 1) { return ival_take(v, 0); }
  
  /* Ensure first element is a function after evaluation */
  ival* f = ival_pop(v, 0);
  if (f->type != IVAL_FUN) {
    ival* err = ival_err(
      "S-Expression starts with incorrect type. Got %s, Expected %s.",
      ltype_name(f->type), ltype_name(IVAL_FUN));
    ival_del(f); ival_del(v);
    return err;
  }
  
  /* If so call function to get result */
  ival* result = f->fun(e, v);
  ival_del(f);
  return result;
}

ival* ival_eval(ienv* e, ival* v) {
  if (v->type == IVAL_SYM) {
    ival* x = ienv_get(e, v);
    ival_del(v);
    return x;
  }
  if (v->type == IVAL_SEXPR) { return ival_eval_sexpr(e, v); }
  return v;
}

/* Reading */

ival* ival_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ? ival_num(x) : ival_err("Invalid Number.");
}

ival* ival_read(mpc_ast_t* t) {
  
  if (strstr(t->tag, "number")) { return ival_read_num(t); }
  if (strstr(t->tag, "symbol")) { return ival_sym(t->contents); }
  
  ival* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = ival_sexpr(); } 
  if (strstr(t->tag, "sexpr"))  { x = ival_sexpr(); }
  if (strstr(t->tag, "qexpr"))  { x = ival_qexpr(); }
  
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = ival_add(x, ival_read(t->children[i]));
  }
  
  return x;
}
