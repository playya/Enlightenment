#ifndef EDJE_CC_H
#define EDJE_CC_H

#include "edje_main.h"
/* Imlib2 stuff for loading up input images */
#define X_DISPLAY_MISSING
#include <Imlib2.h>
/* done Imlib2 stuff */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <locale.h>
#include <ctype.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#ifndef ABS
#define ABS(x) x < 0 ? -x : x
#endif

/* types */
typedef struct _New_Object_Handler    New_Object_Handler;
typedef struct _New_Statement_Handler New_Statement_Handler;
typedef struct _Font_List             Font_List;
typedef struct _Font                  Font;
typedef struct _Code                  Code;
typedef struct _Code_Program          Code_Program;
typedef struct _SrcFile               SrcFile;
typedef struct _SrcFile_List          SrcFile_List;

struct _New_Object_Handler
{
   char *type;
   void (*func)(void);
};

struct _New_Statement_Handler
{
   char *type;
   void (*func)(void);
};

struct _Font_List
{
   Evas_List *list;
};

struct _Font
{
   char *file;
   char *name;
};

struct _Code
{
   int       l1, l2;
   char      *shared;
   Evas_List *programs; 
};

struct _Code_Program
{
   int        l1, l2;
   int        id;
   char      *script;
};

struct _SrcFile
{
   char *name;
   char *file;
};

struct _SrcFile_List
{
   Evas_List *list;
};

/* global fn calls */
void    data_setup(void);
void    data_write(void);
void    data_queue_part_lookup(Edje_Part_Collection *pc, char *name, int *dest);
void    data_queue_program_lookup(Edje_Part_Collection *pc, char *name, int *dest);
void    data_queue_image_lookup(char *name, int *dest);
void    data_queue_part_slave_lookup(int *master, int *slave);
void    data_queue_image_slave_lookup(int *master, int *slave);
void    data_process_lookups(void);
void    data_process_scripts(void);
void    data_process_script_lookups(void);
    

int     is_verbatim(void);
void    track_verbatim(int on);
void    set_verbatim(char *s, int l1, int l2);
char   *get_verbatim(void);
int     get_verbatim_line1(void);    
int     get_verbatim_line2(void);    
void    compile(void);
int     is_param(int n);
int     is_num(int n);    
char   *parse_str(int n);
int     parse_enum(int n, ...);
int     parse_int(int n);
int     parse_int_range(int n, int f, int t);
int     parse_bool(int n);
double  parse_float(int n);
double  parse_float_range(int n, double f, double t);

int     object_handler_num(void);
int     statement_handler_num(void);

void    source_edd(void);
void    source_fetch(void);
int     source_append(Eet_File *ef);
SrcFile_List *source_load(Eet_File *ef);
int     source_fontmap_save(Eet_File *ef, Evas_List *fonts);
Font_List *source_fontmap_load(Eet_File *ef);
    
void   *mem_alloc(size_t size);
char   *mem_strdup(const char *s);
#define SZ sizeof

/* global vars */
extern Evas_List             *img_dirs;
extern Evas_List             *fnt_dirs;
extern char                  *file_in;
extern char                  *file_out;
extern char                  *progname;
extern int                    verbose;
extern int                    no_lossy;
extern int                    no_comp;
extern int                    no_raw;
extern int                    min_quality;
extern int                    max_quality;
extern int                    scale_lossy;
extern int                    scale_comp;
extern int                    scale_raw;
extern int                    line;
extern Evas_List             *stack;
extern Evas_List             *params;
extern Edje_File             *edje_file;
extern Evas_List             *edje_collections;
extern Evas_List             *fonts;
extern Evas_List             *codes;
extern New_Object_Handler     object_handlers[];
extern New_Statement_Handler  statement_handlers[];


#endif
