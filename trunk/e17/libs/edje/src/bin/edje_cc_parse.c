#include "edje_cc.h"

static void  new_object(void);
static void  new_statement(void);
static int   isdelim(char c);
static char *next_token(char *p, char *end, char **new_p, int *delim);
static char *stack_id(void);
static void  stack_chop_top(void);
static void  parse(char *data, off_t size);

int        line = 0;
Evas_List *stack = NULL;
Evas_List *params = NULL;

static void
new_object(void)
{
   char *id;
   int i;
   int handled = 0;
   
   id = stack_id();
   for (i = 0; i < object_handler_num(); i++)
     {
	if (!strcmp(object_handlers[i].type, id))
	  {
	     handled = 1;
	     if (object_handlers[i].func)
	       {
		  object_handlers[i].func();
	       }
	     break;
	  }
     }
   if (!handled)
     {
	fprintf(stderr, "%s: Error. %s:%i unhandled keyword %s\n",
		progname, file_in, line, evas_list_data(evas_list_last(stack)));
	exit(-1);
     }
   free(id);
}

static void
new_statement(void)
{
   char *id;
   int i;
   int handled = 0;
   
   id = stack_id();
   for (i = 0; i < statement_handler_num(); i++)
     {
	if (!strcmp(statement_handlers[i].type, id))
	  {
	     handled = 1;
	     if (statement_handlers[i].func)
	       {
		  statement_handlers[i].func();
	       }
	     break;
	  }
     }
   if (!handled)
     {
	fprintf(stderr, "%s: Error. %s:%i unhandled keyword %s\n",
		progname, file_in, line, evas_list_data(evas_list_last(stack)));
	exit(-1);
     }
   free(id);
}

static int
isdelim(char c)
{
   const char *delims = "{},;";
   char *d;
		  
   d = (char *)delims;
   while (*d)
     {
	if (c == *d) return 1;
	d++;
     }
   return 0;
}

static char *
next_token(char *p, char *end, char **new_p, int *delim)
{
   char *tok_start = NULL, *tok_end = NULL, *tok = NULL, *sa_start = NULL;
   int in_tok = 0;
   int in_quote = 0;
   int in_comment_ss = 0;
   int in_comment_sa = 0;
   int had_quote = 0;

   *delim = 0;
   if (p >= end) return NULL;
   while (p < end)
     {
	if (*p == '\n')
	  {
	     in_comment_ss = 0;
	     line++;
	  }
	if ((!in_comment_ss) && (!in_comment_sa))
	  {
	     if ((!in_quote) && (*p == '/') && (p < (end - 1)) && (*(p + 1) == '/'))
	       in_comment_ss = 1;
	     if ((!in_quote) && (*p == '/') && (p < (end - 1)) && (*(p + 1) == '*'))
	       {
		  in_comment_sa = 1;
		  sa_start = p;
	       }
	  }
	if ((!in_comment_ss) && (!in_comment_sa))
	  {
	     if (!in_tok)
	       {
		  if (!in_quote)
		    {
		       if (!isspace(*p))
			 {
			    if (*p == '"')
			 {
			    in_quote = 1;
			    had_quote = 1;
			 }
			    in_tok = 1;
			    tok_start = p;
			    if (isdelim(*p)) *delim = 1;
			 }
		    }
	       }
	     else
	       {
		  if (in_quote)
		    {
		       if (((*p) == '"') && (*(p - 1) != '\\'))
		    {
		       in_quote = 0;
		       had_quote = 1;
		    }
		    }
		  else
		    {
		       if (*p == '"')
			 {
			    in_quote = 1;
			    had_quote = 1;
			 }
		       if (
			   (isspace(*p)) ||
			   ((*delim) && (!isdelim(*p))) ||
			   (isdelim(*p))
			   )
			 {
			    in_tok = 0;
			    tok_end = p - 1;
			    if (*p == '\n') line--;
			    goto done;
			 }
		    }
	       }
	  }
	if (in_comment_sa)
	  {
	     if ((*p == '/') && (*(p - 1) == '*') && ((p - sa_start) > 2))
	       in_comment_sa = 0;
	  }
	p++;
     }
   if (!in_tok) return NULL;
   tok_end = p - 1;
   
   done:
   *new_p = p;
   
   tok = mem_alloc(tok_end - tok_start + 2);
   strncpy(tok, tok_start, tok_end - tok_start + 1);
   tok[tok_end - tok_start + 1] = 0;
   
   if (had_quote)
     {
	p = tok;
	
	while (*p)
	  {
	     if (*p == '"')
	       strcpy(p, p + 1);
	     else if ((*p == '\\') && (*(p + 1) == '"'))
	       strcpy(p, p + 1);
	     else if ((*p == '\\') && (*(p + 1) == '\\'))
	       strcpy(p, p + 1);
	     p++;
	  }
     }
   return tok;
}

static char *
stack_id(void)
{
   char *id;
   int len;
   Evas_List *l;

   len = 0;
   for (l = stack; l; l = l->next)
     len += strlen(l->data) + 1;
   id = mem_alloc(len);
   id[0] = 0;
   for (l = stack; l; l = l->next)
     {
	strcat(id, l->data);
	if (l->next) strcat(id, ".");
     }
   return id;
}

static void
stack_chop_top(void)
{
   char *top;
   
   /* remove top from stack */
   top = evas_list_data(evas_list_last(stack));
   if (top)
     {
	free(top);
	stack = evas_list_remove(stack, top);
     }
   else
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. } marker without matching { marker\n",
		progname, file_in, line);
	exit(-1);
     }
}

static void
parse(char *data, off_t size)
{
   char *p, *end, *token;
   int delim = 0;
   int do_params = 0;

   if (verbose)
     {
	printf("%s: Parsing input file\n",
	       progname);
     }
   p = data;
   end = data + size;
   line = 1;
   while ((token = next_token(p, end, &p, &delim)) != NULL)
     {
	if (delim)
	  {
	     if (!strcmp(token, ",")) do_params = 1;
	     else if (!strcmp(token, "}"))
	       {
		  if (do_params)
		    {
		       fprintf(stderr, "%s: Error. parse error %s:%i. } marker before ; marker\n",
			       progname, file_in, line);
		       exit(-1);
		    }
		  else
		    stack_chop_top();
	       }
	     else if (!strcmp(token, ";"))
	       {
		  if (do_params)
		    {
		       do_params = 0;
		       new_statement();
		       /* clear out params */
		       while (params)
			 {
			    free(params->data);
			    params = evas_list_remove(params, params->data);
			 }
		       /* remove top from stack */
		       stack_chop_top();
		    }
	       }
	     else if (!strcmp(token, "{"))
	       {
		  if (do_params)
		    {
		       fprintf(stderr, "%s: Error. parse error %s:%i. { marker before ; marker\n",
			       progname, file_in, line);
		       exit(-1);		       
		    }
	       }
	     free(token);
	  }
	else
	  {
	     if (do_params)
	       params = evas_list_append(params, token);
	     else
	       {
		  stack = evas_list_append(stack, token);
		  new_object();
	       }
	  }
     }
   if (verbose)
     {
	printf("%s: Parsing done\n",
	       progname);
     }
}

void
compile(void)
{
   int fd;
   off_t size;
   char *data;
   
   fd = open(file_in, O_RDONLY);
   if (fd < 0)
     {
	fprintf(stderr, "%s: Error. cannot open file \"%s\" for input. %s\n",
		progname, file_in, strerror(errno));
	exit(-1);
     }
   if (verbose)
     {
	printf("%s: Opening \"%s\" for input\n",
	       progname, file_in);
     }
	
   size = lseek(fd, 0, SEEK_END);
   lseek(fd, 0, SEEK_SET);
   data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
   if (data)
     {
	parse(data, size);
	munmap(data, size);
     }
   else
     {
	fprintf(stderr, "%s: Error. cannot mmap file \"%s\" for input. %s\n",
		progname, file_in, strerror(errno));
	exit(-1);
     }
   close(fd);
}

char *
parse_str(int n)
{
   char *str;
   char *s;
   
   str = evas_list_nth(params, n);
   if (!str)
     {
	fprintf(stderr, "%s: Error. %s:%i no parameter supplied as argument %i\n",
		progname, file_in, line, n + 1);
	exit(-1);	
     }
   s = mem_strdup(str);
   return s;
}

int
parse_enum(int n, ...)
{
   char *str;
   va_list va;
   
   str = evas_list_nth(params, n);   
   if (!str)
     {
	fprintf(stderr, "%s: Error. %s:%i no parameter supplied as argument %i\n",
		progname, file_in, line, n + 1);
	exit(-1);	
     }
   va_start(va, n);
   for (;;)
     {
	char *s;
	int   v;
	
	s = va_arg(va, char *);
	if (!s)
	  {
	     fprintf(stderr, "%s: Error. %s:%i token %s not one of:",
		     progname, file_in, line, str);
	     va_start(va, n);
	     s = va_arg(va, char *);
	     while (s)
	       {
		  v = va_arg(va, int);
		  fprintf(stderr, " %s", s);
		  s = va_arg(va, char *);
		  if (!s) break;
	       }
	     fprintf(stderr, "\n");
	     va_end(va);
	     exit(-1);	     
	  }
	v = va_arg(va, int);
	if (!strcmp(s, str))
	  {
	     va_end(va);
	     return v;
	  }
     }
   va_end(va);
   return 0;
}

int
parse_int(int n)
{
   char *str;
   int i;
   
   str = evas_list_nth(params, n);
   if (!str)
     {
	fprintf(stderr, "%s: Error. %s:%i no parameter supplied as argument %i\n",
		progname, file_in, line, n + 1);
	exit(-1);	
     }
   i = atoi(str);
   return i;
}

int
parse_int_range(int n, int f, int t)
{
   char *str;
   int i;
   
   str = evas_list_nth(params, n);
   if (!str)
     {
	fprintf(stderr, "%s: Error. %s:%i no parameter supplied as argument %i\n",
		progname, file_in, line, n + 1);
	exit(-1);	
     }
   i = atoi(str);
   if ((i < f) || (i > t))
     {
	fprintf(stderr, "%s: Error. %s:%i integer %i out of range of %i to %i inclusive\n",
		progname, file_in, line, i, f, t);
	exit(-1);
     }
   return i;
}

double
parse_float(int n)
{
   char *str;
   double i;
   
   str = evas_list_nth(params, n);
   if (!str)
     {
	fprintf(stderr, "%s: Error. %s:%i no parameter supplied as argument %i\n",
		progname, file_in, line, n + 1);
	exit(-1);	
     }
   i = atof(str);
   return i;
}

double
parse_float_range(int n, int f, int t)
{
   char *str;
   double i;
   
   str = evas_list_nth(params, n);
   if (!str)
     {
	fprintf(stderr, "%s: Error. %s:%i no parameter supplied as argument %i\n",
		progname, file_in, line, n + 1);
	exit(-1);	
     }
   i = atoi(str);
   if ((i < f) || (i > t))
     {
	fprintf(stderr, "%s: Error. %s:%i integer %i out of range of %i to %i inclusive\n",
		progname, file_in, line, i, f, t);
	exit(-1);
     }
   return i;
}
