
#ifndef __EWL_MACROS_H__
#define __EWL_MACROS_H__

/**
 * @file ewl_macros.h
 * @defgroup Ewl_Macros Macros: Useful Macros Used Internally and Available Externally
 * Defines a variety of utility macros.
 */

#undef NEW
/**
 * @def NEW(type, num)
 * Allocates memory of @a num elements of sizeof(@a type).
 */
#define NEW(type, num) calloc(num, sizeof(type));

#undef REALLOC
/**
 * @def REALLOC(dat, type, num)
 * Reallocates memory pointed to by @a dat to @a num elements of sizeof(@a
 * type).
 */
#define REALLOC(dat, type, num) \
{ \
	if (dat) \
	  { \
		dat = realloc(dat, sizeof(type) * num); \
	  } \
}

#undef FREE
/**
 * @def FREE(dat)
 * Free the data pointed to by @a dat and it to NULL.
 */
#define FREE(dat) \
{ \
	free(dat); dat = NULL; \
}


#undef IF_FREE
/**
 * @def IF_FREE(dat)
 * If @a dat is non-NULL, free @a dat and assign it to NULL.
 */
#define IF_FREE(dat) \
{ \
	if (dat) FREE(dat); \
}

#undef ZERO
/**
 * @def ZERO(ptr, type, num)
 * Set the first @a num elements of sizeof(@a type) pointed to by @a ptr to
 * zero.
 */
#define ZERO(ptr, type, num) ptr = memset(ptr, 0, sizeof(type) * (num))

#endif				/* __EWL_MACROS_H__ */
