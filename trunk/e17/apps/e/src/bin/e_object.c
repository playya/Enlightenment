#include "e.h"

/* TODO List:
 * 
 * * fix a lot of parts of e17's code to use e_object_del NOT e_object_unref.
 *   there is a subtle difference. unref means u had a reference and you stop
 *   referencing the object - thats ALL. if you created it and now literally
 *   want to destroy it - del is the way to go. there is a separate handler for
 *   this so on del it can go and clean up objects that may reference this one
 *   etc.
 * 
 */

/* yes - i know. glibc specific... but i like being able to do my own */
/* backtraces! NB: you need CFLAGS="-rdynamic -g" LDFLAGS="-rdynamic -g" */
#ifdef OBJECT_PARANOIA_CHECK
#include <execinfo.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

/* local subsystem functions */
static void _e_object_segv(int sig);

/* local subsystem globals */
static sigjmp_buf _e_object_segv_buf;
#endif

/* externally accessible functions */
void *
e_object_alloc(int size, E_Object_Cleanup_Func cleanup_func)
{
   E_Object *obj;
   
   obj = calloc(1, size);
   if (!obj) return NULL;
   obj->magic = E_OBJECT_MAGIC;
   obj->references   = 1;
   obj->cleanup_func = cleanup_func;
   return obj;
}

void
e_object_del(E_Object *obj)
{
   E_OBJECT_CHECK(obj);
   obj->deleted = 1;
   if (obj->del_func) obj->del_func(obj);
   e_object_unref(obj);
}

int
e_object_del_get(E_Object *obj)
{
   E_OBJECT_CHECK_RETURN(obj, 1);
   return obj->deleted;
}

void
e_object_del_func_set(E_Object *obj, E_Object_Cleanup_Func del_func)
{
   E_OBJECT_CHECK(obj);
   obj->del_func = del_func;
}

void
e_object_free(E_Object *obj)
{
   E_OBJECT_CHECK(obj);
   if (obj->func) obj->func(obj);
   obj->magic = E_OBJECT_MAGIC_FREED;
   obj->cleanup_func(obj);
}

int
e_object_ref(E_Object *obj)
{
   E_OBJECT_CHECK(obj);
   obj->references++;
   return obj->references;
}

int
e_object_unref(E_Object *obj)
{
   int ref;
   
   E_OBJECT_CHECK(obj);
   obj->references--;
   ref = obj->references;
   if (obj->references <= 0) e_object_free(obj);
   return ref;
}

int
e_object_ref_get(E_Object *obj)
{
   E_OBJECT_CHECK_RETURN(obj, 0);
   return obj->references;
}

int
e_object_error(E_Object *obj)
{
#ifdef OBJECT_PARANOIA_CHECK   
   char buf[4096];
   char bt[8192];
   void *trace[128];
   char **messages = NULL;
   int i, trace_num;

   /* fetch stacktrace */
   trace_num = backtrace(trace, 128);
   messages = backtrace_symbols(trace, trace_num);

   /* build stacktrace */
   bt[0] = 0;
   if (messages)
     {
	for (i = 1; i < trace_num; i++)
	  {
	     strcat(bt, messages[i]);
	     strcat(bt, "\n");
	  }
	free(messages);
     }
   /* if NULL obj then dump stacktrace */
   if (!obj)
     {
	snprintf(buf, sizeof(buf),
		 "Object is NULL.\n"
		 "%s",
		 bt);
     }
   /* if obj pointer is non NULL, actually try an access and see if we segv */
   else if (obj)
     {
	struct sigaction act, oact;
	int magic = 0, segv = 0;
	
	/* setup segv handler */
	act.sa_handler = _e_object_segv;
	act.sa_flags   = SA_RESETHAND;
	sigemptyset(&act.sa_mask);
	sigaction(SIGSEGV, &act, &oact);
	/* set a longjump to be within this if statement. only called if we */
	/* segfault */
	if (sigsetjmp(_e_object_segv_buf, 1))
	  {
	     sigaction(SIGSEGV, &oact, NULL);
	     segv = 1;
	  }
	else
	  {
	     /* try access magic value */
	     magic = obj->magic;
	     /* if pointer is bogus we'd segv and so jump to the if() above */
	     /* contents, and thus reset segv handler and set segv flag. */
	     /* if not we just continue moving along here and reset handler */
	     sigaction(SIGSEGV, &oact, NULL);
	  }
	/* if we segfaulted above... */
	if (segv)
	  snprintf(buf, sizeof(buf),
		   "Object [%p] is an invalid/garbage pointer.\n"
		   "Segfault successfully avoided.\n"
		   "%s",
		   obj,
		   bt);
	else
	  {
	     /* valid ram then... if we freed this object before */
	     if (magic == E_OBJECT_MAGIC_FREED)
	       snprintf(buf, sizeof(buf),
			"Object [%p] is already freed.\n"
			"%s",
			obj,
			bt);
	     /* garbage magic value - pointer to non object */
	     else if (magic != E_OBJECT_MAGIC)
	       snprintf(buf, sizeof(buf),
			"Object [%p] has garbage magic (%x).\n"
			"%s",
			obj, magic,
			bt);
	     /* it's all ok! */
	     else
	       {
		  return 0;
	       }
	  }
     }
   /* display actual error message */
   e_error_message_show("%s", buf);
//   abort();
   return 1;
#else
   return 0;
#endif   
}

void
e_object_data_set(E_Object *obj, void *data)
{
   E_OBJECT_CHECK(obj);
   obj->data = data;
}

void *
e_object_data_get(E_Object *obj)
{
   E_OBJECT_CHECK_RETURN(obj, NULL);
   return obj->data;
}

void
e_object_free_attach_func_set(E_Object *obj, void (*func) (void *obj))
{
   E_OBJECT_CHECK(obj);
   obj->func = func;
}

#ifdef OBJECT_PARANOIA_CHECK
/* local subsystem functions */
static void
_e_object_segv(int sig)
{
   siglongjmp(_e_object_segv_buf, 1);
}
#endif
