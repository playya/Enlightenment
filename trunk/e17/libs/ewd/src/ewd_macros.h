
#ifndef __EWD_MACROS_H__
#define __EWD_MACROS_H__

/* Wrappers around free() that helps debug free() bugs such as freeing NULL
 * or accessing a pointer that has already been freed */
#ifndef IF_FREE
#define IF_FREE(ptr) if (ptr) free(ptr); ptr = NULL;
#endif

#ifndef FREE
#define FREE(ptr) free(ptr); ptr = NULL;
#endif

/* Debugging printf, basically a wrapper to fprintf that checks the level
 * of the message and checks that it is to be printed at the current debugging
 * level */
#ifndef DPRINTF
#define DPRINTF(debug, format, args...) \
if (debug >= DEBUG_LEVEL) \
	fprintf(stderr, format, args);
#endif

/* convenience macros for checking pointer parameters for non-NULL */
#ifndef CHECK_PARAM_POINTER_RETURN
#define   CHECK_PARAM_POINTER_RETURN(sparam, param, ret) \
if (!(param)) \
{ \
  fprintf(stderr, "***** Developer Warning ***** :\n" \
                  "\tThis program is calling:\n\n" \
                  "\t%s();\n\n" \
                  "\tWith the parameter:\n\n" \
                  "\t%s\n\n" \
                  "\tbeing NULL. Please fix your program.\n", __FUNCTION__,\
								  sparam); \
  fflush(stdout); \
  return ret; \
}
#endif

#ifndef CHECK_PARAM_POINTER
#define   CHECK_PARAM_POINTER(sparam, param) \
if (!(param)) \
{ \
  fprintf(stderr, "***** Developer Warning ***** :\n" \
                  "\tThis program is calling:\n\n" \
                  "\t%s();\n\n" \
                  "\tWith the parameter:\n\n" \
                  "\t%s\n\n" \
                  "\tbeing NULL. Please fix your program.\n", __FUNCTION__, \
								 sparam); \
  fflush(stdout); \
  return; \
}
#endif

/* Use the larger of a and b */
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

/* Use the smaller of a and b */
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#endif /* __EWL_MACROS_H__ */
