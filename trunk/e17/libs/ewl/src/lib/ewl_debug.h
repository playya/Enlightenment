#ifndef __EWL_DEBUG_H__
#define __EWL_DEBUG_H__

#include "ewl-config.h"

#define DLEVEL_UNSTABLE 1
#define DLEVEL_TESTING 10
#define DLEVEL_STABLE 20

inline void     ewl_print_warning(void);
inline void	ewl_segv(void);

#define DEBUG 1

#ifdef DEBUG

#define DENTER_FUNCTION(lvl) \
{ \
	if (ewl_config.debug.enable && (ewl_config.debug.level >= lvl)) \
	  { \
		fprintf(stderr, " --> %s:%i\tEntering %s();\n", \
			__FILE__, __LINE__, __FUNCTION__); \
	  } \
}

#define DLEAVE_FUNCTION(lvl) \
{ \
	if (ewl_config.debug.enable && ewl_config.debug.level >= lvl) \
	  { \
		fprintf(stderr, "<--  %s:%i\tLeaving  %s();\n", \
			__FILE__, __LINE__, __FUNCTION__); \
	  } \
}

#define DRETURN(lvl) \
{ \
	DLEAVE_FUNCTION(lvl); \
	if (ewl_config.debug.enable && ewl_config.debug.level >= lvl) \
	  { \
		fprintf(stderr, "<--  %s:%i\tReturn in %s();\n", \
			__FILE__, __LINE__, __FUNCTION__); \
	  } \
	return; \
}

#define DRETURN_PTR(ptr, lvl) \
{ \
	DLEAVE_FUNCTION(lvl); \
	if (ewl_config.debug.enable && ewl_config.debug.level >= lvl) \
	  { \
		fprintf(stderr, "<--  %s:%i\tReturning %p in %s();\n", \
			__FILE__, __LINE__, (void *)ptr, __FUNCTION__); \
	  } \
	return (void *)ptr; \
}

#define DRETURN_FLOAT(num, lvl) \
{ \
	DLEAVE_FUNCTION(lvl); \
	if (ewl_config.debug.enable && ewl_config.debug.level >= lvl) \
	  { \
		fprintf(stderr, "<--  %s:%i\tReturning %f in %s();\n", \
			__FILE__, __LINE__, (float) num, __FUNCTION__); \
	  } \
	return num; \
}

#define DRETURN_INT(num, lvl) \
{ \
	DLEAVE_FUNCTION(lvl); \
	if (ewl_config.debug.enable && ewl_config.debug.level >= lvl) \
	  { \
		fprintf(stderr, "<--  %s:%i\tReturning %i in %s();\n", \
			__FILE__, __LINE__, (int) num, __FUNCTION__); \
	  } \
	return num; \
}

#define DWARNING(fmt) \
{ \
	ewl_print_warning(); \
	fprintf(stderr, "\tIn function:\n\n" \
			"\t%s();\n\n", __FUNCTION__); \
	fprintf(stderr, fmt); \
	ewl_segv(); \
}

#define DCHECK_PARAM_PTR(str, ptr) \
{ \
	if (!ptr) \
	  { \
		ewl_print_warning(); \
		fprintf(stderr, "\tThis program is calling:\n\n" \
				"\t%s();\n\n" \
				"\tWith the parameter:\n\n" \
				"\t%s\n\n" \
				"\tbeing NULL. Please fix your program.\n", \
				__FUNCTION__, str); \
		ewl_segv(); \
		return; \
	  } \
}

#define DCHECK_PARAM_PTR_RET(str, ptr, ret) \
{ \
	if (!ptr) \
	  { \
		ewl_print_warning(); \
		fprintf(stderr, "\tThis program is calling:\n\n" \
				"\t%s();\n\n" \
				"\tWith the parameter:\n\n" \
				"\t%s\n\n" \
				"\tbeing NULL. Please fix your program.\n", \
				__FUNCTION__, str); \
		ewl_segv(); \
		return ret; \
	  } \
}

#define DCHECK_TYPE(str, ptr, type) \
{ \
	if (!ewl_widget_type_is(EWL_WIDGET(ptr), type)) \
	{ \
		ewl_print_warning(); \
		fprintf(stderr, "\tThis program is calling:\n\n" \
				"\t%s();\n\n" \
				"\tWith the paramter:\n\n" \
				"\t%s\n\n" \
				"\tas the wrong type. (%s) instead of (%s).\n" \
				"\tPlease fix your program.\n", \
				__FUNCTION__, str, EWL_WIDGET(ptr)->inheritance, type); \
		ewl_segv(); \
	} \
}

#define DCHECK_TYPE_RET(str, ptr, type, ret) \
{ \
	if (!ewl_widget_type_is(EWL_WIDGET(ptr), type)) \
	{ \
		ewl_print_warning(); \
		fprintf(stderr, "\tThis program is calling:\n\n" \
				"\t%s();\n\n" \
				"\tWith the paramter:\n\n" \
				"\t%s\n\n" \
				"\tas the wrong type. (%s) instead of (%s).\n" \
				"\tPlease fix your program.\n", \
				__FUNCTION__, str, EWL_WIDGET(ptr)->inheritance, type); \
		ewl_segv(); \
		return ret; \
	} \
}

#else

#define DENTER_FUNCTION(lvl) {}
#define DLEAVE_FUNCTION(lvl) {}
#define DRETURN(lvl) return
#define DRETURN_PTR(ptr, lvl) return (void *)ptr
#define DRETURN_FLOAT(num, lvl) return num
#define DRETURN_INT(num, lvl) return num
#define DWARNING(fmt) {}
#define DCHECK_PARAM_PTR(str, ptr) \
{ \
	if (!ptr) { \
		return; \
	} \
}
#define DCHECK_PARAM_PTR_RET(str, ptr, ret) \
{ \
	if (!ptr) { \
		return ret; \
	} \
}
#define DCHECK_TYPE(str, ptr, type) {}
#define DCHECK_TYPE_RET(str, ptr, type, ret) {}
#endif

#define DERROR(fmt) \
{ \
	ewl_print_warning(); \
	fprintf(stderr, "\tIn function:\n\n" \
			"\t%s();\n\n", __FUNCTION__); \
	fprintf(stderr, fmt); \
}

#endif				/* __EWL_DEBUG_H__ */

