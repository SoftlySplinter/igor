#include <stdlib.h>
#include "../lib/mpc.h"
#include "parse.h"
/* Construct a pointer to a new Number ival */ 
ival* ival_num(long x) {
  ival* v = malloc(sizeof(ival));
  v->type = IVAL_NUM;
  v->num = x;
  return v;
}

/* Construct a pointer to a new Error ival */ 
ival* ival_err(char* m) {
  ival* v = malloc(sizeof(ival));
  v->type = IVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}

/* Construct a pointer to a new Symbol ival */ 
ival* ival_sym(char* s) {
  ival* v = malloc(sizeof(ival));
  v->type = IVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

/* A pointer to a new empty Sexpr ival */
ival* ival_sexpr(void) {
  ival* v = malloc(sizeof(ival));
  v->type = IVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

void ival_del(ival* v) {

  switch (v->type) {
    /* Do nothing special for number type */
    case IVAL_NUM: break;

    /* For Err or Sym free the string data */
    case IVAL_ERR: free(v->err); break;
    case IVAL_SYM: free(v->sym); break;

    /* If Sexpr then delete all elements inside */
    case IVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
        ival_del(v->cell[i]);
      }
      /* Also free the memory allocated to contain the pointers */
      free(v->cell);
    break;
  }

  /* Finally free the memory allocated for the "ival" struct itself */
  free(v);
}

ival* ival_add(ival* v, ival* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(ival*) * v->count);
  v->cell[v->count-1] = x;
  return v;
}

ival* ival_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ? ival_num(x) : ival_err("invalid number");
}

ival* ival_read(mpc_ast_t* t) {

  /* If Symbol or Number return conversion to that type */
  if (strstr(t->tag, "number")) { return ival_read_num(t); }
  if (strstr(t->tag, "symbol")) { return ival_sym(t->contents); }

  /* If root (>) or sexpr then create empty list */
  ival* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = ival_sexpr(); } 
  if (strstr(t->tag, "sexpr"))  { x = ival_sexpr(); }

  /* Fill this list with any valid expression contained within */
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

ival* ival_eval_sexpr(ival* v) {

  /* Evaluate Children */
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = ival_eval(v->cell[i]);
  }

  /* Error Checking */
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == IVAL_ERR) { return ival_take(v, i); }
  }

  /* Empty Expression */
  if (v->count == 0) { return v; }

  /* Single Expression */
  if (v->count == 1) { return ival_take(v, 0); }

  /* Ensure First Element is Symbol */
  ival* f = ival_pop(v, 0);
  if (f->type != IVAL_SYM) {
    ival_del(f); ival_del(v);
    return ival_err("S-expression Does not start with symbol!");
  }

  /* Call builtin with operator */
  ival* result = builtin_op(v, f->sym);
  ival_del(f);
  return result;
}

ival* ival_eval(ival* v) {
  /* Evaluate Sexpressions */
  if (v->type == IVAL_SEXPR) { return ival_eval_sexpr(v); }
  /* All other ival types remain the same */
  return v;
}

ival* ival_pop(ival* v, int i) {
  /* Find the item at "i" */
  ival* x = v->cell[i];

  /* Shift the memory following the item at "i" over the top of it */
  memmove(&v->cell[i], &v->cell[i+1], sizeof(ival*) * (v->count-i-1));

  /* Decrease the count of items in the list */
  v->count--;

  /* Reallocate the memory used */
  v->cell = realloc(v->cell, sizeof(ival*) * v->count);
  return x;
}

ival* ival_take(ival* v, int i) {
  ival* x = ival_pop(v, i);
  ival_del(v);
  return x;
}

ival* builtin_op(ival* a, char* op) {
  
  /* Ensure all arguments are numbers */
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != IVAL_NUM) {
      ival_del(a);
      return ival_err("Cannot operator on non number!");
    }
  }
  
  /* Pop the first element */
  ival* x = ival_pop(a, 0);

  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && a->count == 0) { x->num = -x->num; }

  /* While there are still elements remaining */
  while (a->count > 0) {

    /* Pop the next element */
    ival* y = ival_pop(a, 0);

    /* Perform operation */
    if (strcmp(op, "+") == 0) { x->num += y->num; }
    if (strcmp(op, "-") == 0) { x->num -= y->num; }
    if (strcmp(op, "*") == 0) { x->num *= y->num; }
    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
        ival_del(x); ival_del(y);
        x = ival_err("Division By Zero!"); break;
      } else {
        x->num /= y->num;
      }
    }

    /* Delete element now finished with */
    ival_del(y);
  }

  /* Delete input expression and return result */
  ival_del(a);
  return x;
}

void ival_expr_print(ival* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {

    /* Print Value contained within */
    ival_print(v->cell[i]);

    /* Don't print trailing space if last element */
    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void ival_print(ival* v) {
  switch (v->type) {
    case IVAL_NUM:   printf("%li", v->num); break;
    case IVAL_ERR:   printf("Error: %s", v->err); break;
    case IVAL_SYM:   printf("%s", v->sym); break;
    case IVAL_SEXPR: ival_expr_print(v, '(', ')'); break;
  }
}

void ival_println(ival* v) { ival_print(v); putchar('\n'); }
