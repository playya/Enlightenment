%{
#include <math.h>  /* For math functions, cos(), sin(), etc. */
#include "calc.h"  /* Contains definition of `symrec'        */

#define YYERROR_VERBOSE

void
yyerror (const char *s);

double _result;

%}
%union {
double     val;  /* For returning numbers.                   */
symrec  *tptr;   /* For returning symbol-table pointers      */
}

%token <val>  NUM        /* Simple double precision number   */
%token <tptr> VAR FNCT  /* Variable and Function            */
%type  <val>  exp

%token OBRAK
%token CBRAK

%right '='
%left '-' '+'
%left '*' '/'
%left NEG     /* Negation--unary minus */
%right '^'    /* Exponentiation        */

/* Grammar follows */

%%

input:    /* empty string */
        | exp { printf ("result:\t%.10g\n", $1); _result = $1;}
;

exp:      NUM                  { $$ = $1;                         }
        | VAR                  { $$ = $1->value.var;              }
        | VAR '=' exp          { $$ = $3; $1->value.var = $3;     }
        | FNCT OBRAK exp CBRAK { $$ = (*($1->value.fnctptr))($3); }
        | exp '+' exp          { $$ = $1 + $3;                    }
        | exp '-' exp          { $$ = $1 - $3;                    }
        | exp '*' exp          { $$ = $1 * $3;                    }
        | exp '/' exp          { $$ = $1 / $3;                    }
        | '-' exp  %prec NEG   { $$ = -$2;                        }
        | exp '^' exp          { $$ = pow ($1, $3);               }
        | OBRAK exp CBRAK      { $$ = $2;                         }
;
/* End of grammar */
%%


#include <stdio.h>
#include "lex.yy.c"

void
yyerror (const char *s)  /* Called by yyparse on error */
{
  printf ("%s\n", s);
}

double yyresult (void)
{
  return _result;
}

struct init
{
  char *fname;
  double (*fnct)(double);
};

struct init arith_fncts[] =
{
  "sin",  sin,
  "cos",  cos,
  "tan", tan,
  "ln",   log,
  "exp",  exp,
  "sqrt", sqrt,
  0, 0
};

/* The symbol table: a chain of `struct symrec'.  */
symrec *sym_table = (symrec *) 0;

/* Put arithmetic functions in table. */
void
init_table (void)
{
  int i;
  symrec *ptr;
  for (i = 0; arith_fncts[i].fname != 0; i++)
    {
      ptr = putsym (arith_fncts[i].fname, FNCT);
      ptr->value.fnctptr = arith_fncts[i].fnct;
    }
}

symrec *
putsym (const char *sym_name, int sym_type)
{
  symrec *ptr;
  ptr = (symrec *) malloc (sizeof (symrec));
  ptr->name = (char *) malloc (strlen (sym_name) + 1);
  strcpy (ptr->name,sym_name);
  ptr->type = sym_type;
  ptr->value.var = 0; /* set value to 0 even if fctn.  */
  ptr->next = (struct symrec *)sym_table;
  sym_table = ptr;
  return ptr;
}

symrec *
getsym (const char *sym_name)
{
  symrec *ptr;
  for (ptr = sym_table; ptr != (symrec *) 0;
       ptr = (symrec *)ptr->next)
    if (strcmp (ptr->name,sym_name) == 0)
      return ptr;
  return 0;
}

