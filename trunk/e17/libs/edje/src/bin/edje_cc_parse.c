/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_cc.h"

static void  new_object(void);
static void  new_statement(void);
static int   isdelim(char c);
static char *next_token(char *p, char *end, char **new_p, int *delim);
static char *stack_id(void);
static void  stack_chop_top(void);
static void  parse(char *data, off_t size);

/* simple expression parsing protos */
static int my_atoi(const char * s);
static char * _alphai(char *s, int * val);
static char * _betai(char *s, int * val);
static char * _gammai(char *s, int * val);
static char * _deltai(char *s, int * val);
static char * _get_numi(char *s, int * val);
static int _is_numi(char c);
static int _is_op1i(char c);
static int _is_op2i(char c);
static int _calci(char op, int a, int b);

static double my_atof(const char * s);
static char * _alphaf(char *s, double * val);
static char * _betaf(char *s, double * val);
static char * _gammaf(char *s, double * val);
static char * _deltaf(char *s, double * val);
static char * _get_numf(char *s, double * val);
static int _is_numf(char c);
static int _is_op1f(char c);
static int _is_op2f(char c);
static double _calcf(char op, double a, double b);
static int strstrip(const char *in, char *out, size_t size);


int        line = 0;
Evas_List *stack = NULL;
Evas_List *params = NULL;

static char  file_buf[4096];
static int   verbatim = 0;
static int   verbatim_line1 = 0;
static int   verbatim_line2 = 0;
static char *verbatim_str = NULL;

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
	for (i = 0; i < statement_handler_num(); i++)
	  {
	     if (!strcmp(statement_handlers[i].type, id))
	       {
		  free(id);
		  return;
	       }
	  }
     }
   if (!handled)
     {
	fprintf(stderr, "%s: Error. %s:%i unhandled keyword %s\n",
		progname, file_in, line,
		(char *)evas_list_data(evas_list_last(stack)));
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
		progname, file_in, line,
		(char *)evas_list_data(evas_list_last(stack)));
	exit(-1);
     }
   free(id);
}

static int
isdelim(char c)
{
   const char *delims = "{},;:";
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
   int in_comment_ss  = 0;
   int in_comment_cpp = 0;
   int in_comment_sa  = 0;
   int had_quote = 0;
   char *cpp_token_line = NULL;
   char *cpp_token_file = NULL;

   *delim = 0;
   if (p >= end) return NULL;
   while (p < end)
     {
	if (*p == '\n')
	  {
	     in_comment_ss = 0;
	     in_comment_cpp = 0;
	     cpp_token_line = NULL;
	     cpp_token_file = NULL;
	     line++;
	  }
	if ((!in_comment_ss) && (!in_comment_sa))
	  {
	     if ((!in_quote) && (*p == '/') && (p < (end - 1)) && (*(p + 1) == '/'))
	       in_comment_ss = 1;
	     if ((!in_quote) && (*p == '#'))
	       in_comment_cpp = 1;
	     if ((!in_quote) && (*p == '/') && (p < (end - 1)) && (*(p + 1) == '*'))
	       {
		  in_comment_sa = 1;
		  sa_start = p;
	       }
	  }
	if ((in_comment_cpp) && (*p == '#'))
	  {
	     char *pp, fl[4096];
	     char *tmpstr = NULL;
	     int   l, nm;
	     
	     /* handle cpp comments */
	     /* their line format is
	      * # <line no. of next line> <filename from next line on> [??]
	      */
	     cpp_token_line = NULL;
	     cpp_token_file = NULL;
	     
	     pp = p;
	     while ((pp < end) && (*pp != '\n'))
	       {
		  pp++;
	       }
	     l = pp - p;
	     tmpstr = malloc(l + 1);
	     if (!tmpstr)
	       {
		  fprintf(stderr, "%s: Error. %s:%i malloc %i bytes failed\n",
			  progname, file_in, line, l + 1);
		  exit(-1);
	       }
	     strncpy(tmpstr, p, l);
	     tmpstr[l] = 0;
	     l = sscanf(tmpstr, "%*s %i \"%[^\"]\"", &nm, fl);
	     if (l == 2)
	       {
		  strcpy(file_buf, fl);
		  line = nm;
		  file_in = file_buf;
	       }
	     free(tmpstr);
	  }
	else if ((!in_comment_ss) && (!in_comment_sa) && (!in_comment_cpp))
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
	       memmove(p, p + 1, strlen(p));
	     else if ((*p == '\\') && (*(p + 1) == '"'))
	       memmove(p, p + 1, strlen(p));
	     else if ((*p == '\\') && (*(p + 1) == '\\'))
	       memmove(p, p + 1, strlen(p));
	     else
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
	     if ((!strcmp(token, ",")) || (!strcmp(token, ":"))) do_params = 1;
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
		  if ((verbatim == 1) && (p < (end - 2)))
		    {
		       int escaped = 0;
		       int inquotes = 0;
		       int insquotes = 0;
		       int squigglie = 1;
		       int l1 = 0, l2 = 0;
		       char *verbatim_1;
		       char *verbatim_2;
		       
		       l1 = line;
		       while ((p[0] != '{') && (p < end))
			 {
			    if (*p == '\n') line++;
			    p++;
			 }
		       p++;
		       verbatim_1 = p;
		       verbatim_2 = NULL;
		       for (; p < end; p++)
			 {
			    if (*p == '\n') line++;
			    if (escaped) escaped = 0;
			    if (!escaped)
			      {
				 if (p[0] == '\\') escaped = 1;
				 else if (p[0] == '\"')
				   {
				      if (!insquotes)
					{
					   if (inquotes) inquotes = 0;
					   else inquotes = 1;
					}
				   }
				 else if (p[0] == '\'')
				   {
				      if (!inquotes)
					{
					   if (insquotes) insquotes = 0;
					   else insquotes = 1;
					}
				   }
				 else if ((!inquotes) && (!insquotes))
				   {
				      if      (p[0] == '{') squigglie++;
				      else if (p[0] == '}') squigglie--;
				      if (squigglie == 0)
					{
					   verbatim_2 = p - 1;
					   l2 = line;
					   break;
					}
				   }
			      }
			 }
		       if (verbatim_2 > verbatim_1)
			 {
			    int l;
			    char *v;
			    
			    l = verbatim_2 - verbatim_1 + 1;
			    v = malloc(l + 1);
			    strncpy(v, verbatim_1, l);
			    v[l] = 0;
			    set_verbatim(v, l1, l2);
			 }
		       else
			 {
			    fprintf(stderr, "%s: Error. parse error %s:%i. { marker does not have matching } marker\n",
				    progname, file_in, line);
			    exit(-1);
			 }
		       new_object();
		       verbatim = 0;
		    }
	       }
	  }
     }
   if (verbose)
     {
	printf("%s: Parsing done\n",
	       progname);
     }
}

static char *clean_file = NULL;
static void
clean_tmp_file(void)
{
   if (clean_file) unlink(clean_file);
}

int
is_verbatim(void)
{
   return verbatim;
}

void
track_verbatim(int on)
{
   verbatim = on;
}

void
set_verbatim(char *s, int l1, int l2)
{
   verbatim_line1 = l1;
   verbatim_line2 = l2;
   verbatim_str = s;
}

char *
get_verbatim(void)
{
   return verbatim_str;
}

int
get_verbatim_line1(void)
{
   return verbatim_line1;
}

int
get_verbatim_line2(void)
{
   return verbatim_line2;
}

void
compile(void)
{
   int fd;
   off_t size;
   char *data;
   char buf[4096];
   static char tmpn[4096];
   
   strcpy(tmpn, "/tmp/edje_cc.edc-tmp-XXXXXX");
   fd = mkstemp(tmpn);
   if (fd >= 0)
     {
	int ret;
        
	clean_file = tmpn;
	close(fd);
	atexit(clean_tmp_file);
	snprintf(buf, sizeof(buf), "cat %s | cpp -E -o %s", file_in, tmpn);
	ret = system(buf);
	if (ret < 0)
	  {
	     snprintf(buf, sizeof(buf), "gcc -E -o %s %s", tmpn, file_in);
	     ret = system(buf);
	  }
	if (ret >= 0) file_in = tmpn;
     }
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
   data = malloc(size);
   if (data && read(fd, data, size) == size)
	parse(data, size);
   else
     {
	fprintf(stderr, "%s: Error. cannot read file \"%s\". %s\n",
		progname, file_in, strerror(errno));
	exit(-1);
     }
   free(data);
   close(fd);
}

int
is_param(int n)
{
   char *str;
   
   str = evas_list_nth(params, n);
   if (str) return 1;
   return 0;
}

int
is_num(int n)
{
   char *str;
   long int ret;
   char *end;
   
   str = evas_list_nth(params, n);
   if (!str)
     {
	fprintf(stderr, "%s: Error. %s:%i no parameter supplied as argument %i\n",
		progname, file_in, line, n + 1);
	exit(-1);	
     }
   if (str[0] == 0) return 0;
   end = str;
   ret = strtol(str, &end, 0);
   if ((end != str) && (end[0] == 0)) return 1;
   return 0;
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
   i = my_atoi(str);
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
   i = my_atoi(str);
   if ((i < f) || (i > t))
     {
	fprintf(stderr, "%s: Error. %s:%i integer %i out of range of %i to %i inclusive\n",
		progname, file_in, line, i, f, t);
	exit(-1);
     }
   return i;
}

int
parse_bool(int n)
{
   char *str, buf[4096];
   int i;

   str = evas_list_nth(params, n);
   if (!str)
     {
	fprintf(stderr, "%s: Error. %s:%i no parameter supplied as argument %i\n",
		progname, file_in, line, n + 1);
	exit(-1);
     }

   if (!strstrip(str, buf, sizeof (buf)))
     {
	fprintf(stderr, "%s: Error. %s:%i expression is too long\n",
		progname, file_in, line);
	return 0;
     }

   if (!strcasecmp(buf, "false") || !strcasecmp(buf, "off"))
      return 0;
   if (!strcasecmp(buf, "true") || !strcasecmp(buf, "on"))
      return 1;

   i = my_atoi(str);
   if ((i < 0) || (i > 1))
     {
	fprintf(stderr, "%s: Error. %s:%i integer %i out of range of 0 to 1 inclusive\n",
		progname, file_in, line, i);
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
   i = my_atof(str);
   return i;
}

double
parse_float_range(int n, double f, double t)
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
   i = my_atof(str);
   if ((i < f) || (i > t))
     {
	fprintf(stderr, "%s: Error. %s:%i float %3.3f out of range of %3.3f to %3.3f inclusive\n",
		progname, file_in, line, i, f, t);
	exit(-1);
     }
   return i;
}

void
check_arg_count(int required_args)
{
   int num_args = evas_list_count (params);

   if (num_args != required_args)
     {
	fprintf(stderr, "%s: Error. %s:%i got %i arguments, but expected %i\n",
	      progname, file_in, line, num_args, required_args);
	exit(-1);
     }
}

/* simple expression parsing stuff */

/*
 * alpha ::= beta + beta || beta
 * beta  ::= gamma + gamma || gamma
 * gamma ::= num || delta
 * delta ::= '(' alpha ')'
 * 
 */

/* int set of function */

static int
my_atoi(const char * s)
{
   int res = 0;
   char buf[4096];
   
   if (!s)
     return 0;
   
   if (!strstrip(s, buf, sizeof (buf)))
     {
	fprintf(stderr, "%s: Error. %s:%i expression is too long\n",
		progname, file_in, line);
	return 0;
     }
   
   _alphai(buf, &res);
   return res;
}

static char *
_deltai(char *s, int * val)
{
   if (!val) return NULL;
   
   if ('(' != s[0])
     { 
	fprintf(stderr, "%s: Error. %s:%i unexpected character at %s\n",
		progname, file_in, line, s);
	return s;
     }
   else
     {
	s++;
	s = _alphai(s, val);
	s++;
	return s;
     }
   
   return s;
}

static char *
_gammai(char *s, int * val)
{
   if (!val) return NULL;
   
   if (_is_numi(s[0]))
     {
	s = _get_numi(s, val);
	return s;
     }
   else if ('(' == s[0])
     {
	s = _deltai(s, val);
	return s;
     }
   else
     fprintf(stderr, "%s: Error. %s:%i unexpected character at %s\n",
	     progname, file_in, line, s);
   return s;
}

static char *
_betai(char *s, int * val)
{
   int a1, a2;
   char op;
   
   if (!val)
     return NULL;
   
   s = _gammai(s, &a1);
   
   while (_is_op1i(s[0]))
     {
	op = s[0];
	s++;
	s = _gammai(s, &a2);
	a1 = _calci(op, a1, a2);
     }
   
   (*val) = a1;
   
   return s;
}

static char *
_alphai(char *s, int * val)
{
   int a1, a2;
   char op;
   
   if (!val)
     return NULL;
   
   s = _betai(s, &a1);
   
   while (_is_op2i(s[0]))
     {
	op = s[0];
	s++;
	s = _betai(s, &a2);
	a1 = _calci(op, a1, a2);
     }
   
   (*val) = a1;
   return s;
}

char *
_get_numi(char *s, int * val)
{
   char buf[4096];
   int pos = 0;
   
   if (!val)
     return s;   
   
   while (
	  (('0' <= s[pos]) && ('9' >= s[pos])) ||
	  ((0 == pos) && ('-' == s[pos]))
	  )
     {
	buf[pos] = s[pos];
	pos++;
     }
   
   buf[pos] = '\0';
   (*val) = atoi(buf);
   return (s+pos);
}

int
_is_numi(char c)
{
   if (((c >= '0') && (c <= '9')) || ('-' == c) || ('+' == c))
     return 1;
   else
     return 0;
}

int
_is_op1i(char c)
{
   switch(c)
     {
      case '*':;
      case '/': return 1;
      default: return 0;
     }
   return 0;
}

int
_is_op2i(char c)
{
   switch(c)
     {
      case '+':;
      case '-': return 1;
      default: return 0;
     }
   return 0;
}

int
_calci(char op, int a, int b)
{
   switch(op)
     {
      case '+':
	a += b;
	return a;
      case '-': 
	a -= b;
	return a;
      case '/':
	if(0 != b)
	  a /= b;
	else
	  fprintf(stderr, "%s: Error. %s:%i divide by zero\n",
		  progname, file_in, line);
	return a;
      case '*':
	a *= b;
	return a;
      default:
	fprintf(stderr, "%s: Error. %s:%i unexpected character '%c'\n",
		progname, file_in, line, op);
	return a;
     }
}

/* float set of functoins */

double
my_atof(const char * s)
{
   double res = 0;
   char buf[4096];
   
   if (!s)
     return 0;
   
   if (!strstrip(s, buf, sizeof (buf)))
     {
	fprintf(stderr, "%s: Error. %s:%i expression is too long\n",
		progname, file_in, line);
	return 0;
     }
   
   _alphaf(buf, &res);
   return res;
}

static char *
_deltaf(char *s, double * val)
{
   if (!val) return NULL;
   
   if ('(' != s[0])
     {
	fprintf(stderr, "%s: Error. %s:%i unexpected character at %s\n",
		progname, file_in, line, s);
	return s;
     }
   else
     {
	s++;
	s = _alphaf(s, val);
	s++;
	return s;
     }
   
   return s;
}

static char *
_gammaf(char *s, double * val)
{
   if (!val) return NULL;
   
   if (_is_numf(s[0]))
     {
	s = _get_numf(s, val);
	return s;
     }
   else if ('(' == s[0])
     {
	s = _deltaf(s, val);
	return s;
     }
   else
     fprintf(stderr, "%s: Error. %s:%i unexpected character at %s\n",
	     progname, file_in, line, s);
   return s;
}

static char *
_betaf(char *s, double * val)
{
   double a1=0, a2=0;
   char op;
   
   if (!val)
     return NULL;
   
   s = _gammaf(s, &a1);
   
   while (_is_op1f(s[0]))
     {
	op = s[0];
	s++;
	s = _gammaf(s, &a2);
	a1 = _calcf(op, a1, a2);
     }
   
   (*val) = a1;
   
   return s;
}

static char *
_alphaf(char *s, double * val)
{
   double a1=0, a2=0;
   char op;
   
   if (!val)
     return NULL;
   
   s = _betaf(s, &a1);
   
   while (_is_op2f(s[0]))
     {
	op = s[0];
	s++;
	s = _betaf(s, &a2);
	a1 = _calcf(op, a1, a2);
     }
   
   (*val) = a1;
   
   return s;
}

static char *
_get_numf(char *s, double * val)
{
   char buf[4096];
   int pos = 0;
   
   if (!val)
     return s;   
   
   while (
	  (('0' <= s[pos]) && ('9' >= s[pos])) ||
	  ('.' == s[pos]) ||
	  ((0 == pos) && ('-' == s[pos]))
	  )
     {
	buf[pos] = s[pos];
	pos++;
     }
   
   buf[pos] = '\0';
   (*val) = atof(buf);
   return (s+pos);
}

static int
_is_numf(char c)
{
   if (((c >= '0') && (c <= '9')) 
       || ('-' == c) 
       || ('.' == c)
       || ('+' == c))
     return 1;
   else
     return 0;
}

static int
_is_op1f(char c)
{
   switch(c)
     {
      case '*':;
      case '/': return 1;
      default: return 0;
     }
   return 0;
}

static int
_is_op2f(char c)
{
   switch(c)
     {
      case '+':;
      case '-': return 1;
      default: return 0;
     }
   return 0;
}

static double
_calcf(char op, double a, double b)
{
   switch(op)
     {
      case '+':
	a += b;
	return a;
      case '-': 
	a -= b;
	return a;
      case '/':
	if (b != 0) a /= b;
	else
	  fprintf(stderr, "%s: Error. %s:%i divide by zero\n",
		  progname, file_in, line);
	return a;
      case '*':
	a *= b;
	return a;
      default:
	fprintf(stderr, "%s: Error. %s:%i unexpected character '%c'\n",
		progname, file_in, line, op);
	return a;
     }
}

static int
strstrip(const char *in, char *out, size_t size)
{
   if ((size -1 ) < strlen(in))
     {
	fprintf(stderr, "%s: Error. %s:%i expression is too long\n",
		progname, file_in, line);
	return 0;
     }

   /* remove spaces and tabs */
   while (*in)
     {
	if ((0x20 != *in) && (0x09 != *in))
	  {
	     *out = *in;
	     out++;
	  }
	in++;
     }

   *out = '\0';

   return 1;
}
