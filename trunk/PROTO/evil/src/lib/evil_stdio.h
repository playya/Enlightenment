#ifndef __EVIL_STDIO_H__
#define __EVIL_STDIO_H__


/**
 * @file evil_stdio.h
 * @brief The file that provides functions ported from Unix in stdio.h.
 * @defgroup Evil_Stdio_Group Stdio.h functions
 *
 * This header provides functions ported from Unix in stdio.h.
 *
 * @{
 */


#if defined (_WIN32_WCE) && ! defined (__CEGCC__)

/*
 * Error related functions
 *
 */

EAPI void perror (const char *s);

/*
 * Stream related functions
 *
 */

EAPI FILE *evil_fopen(const char *path, const char *mode);

# define fopen(path, mode) evil_fopen(path, mode)

EAPI void evil_rewind(FILE *stream);

# define rewind(f) evil_rewind(f)

EAPI int evil_remove(const char *path);

# define remove(p) evil_remove(p)

#endif /* _WIN32_WCE && ! __CEGCC__ */


#ifdef _WIN32_WCE

EAPI FILE *evil_fopen_native(const char *path, const char *mode);

EAPI size_t evil_fread_native(void* buffer, size_t size, size_t count, FILE* stream);

EAPI int evil_fclose_native(FILE *stream);

#endif /* _WIN32_WCE */


/**
 * @}
 */


#endif /* __EVIL_STDIO_H__ */
