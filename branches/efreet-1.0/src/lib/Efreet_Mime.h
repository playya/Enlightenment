#ifndef EFREET_MIME_H
#define EFREET_MIME_H

/**
 * @file Efreet_Mime.h
 * @brief The file that must be included by any project wishing to use
 * @addtogroup Efreet_Mime Efreet_Mime: The XDG Shared Mime Info standard
 * Efreet Mime is a library designed to help apps work with the
 * Freedesktop.org Shared Mime Info standard.
 * Efreet_Mime.h provides all of the necessary headers and
 * includes to work with Efreet_Mime.
 * @{
 */

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EFREET_MIME_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EFREET_MIME_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif


EAPI int         efreet_mime_init(void);
EAPI int         efreet_mime_shutdown(void);

EAPI const char *efreet_mime_type_get(const char *file);
EAPI const char *efreet_mime_magic_type_get(const char *file);
EAPI const char *efreet_mime_globs_type_get(const char *file);
EAPI const char *efreet_mime_special_type_get(const char *file);
EAPI const char *efreet_mime_fallback_type_get(const char *file);

EAPI const char *efreet_mime_type_icon_get(const char *mime, const char *theme,
                                                          unsigned int size);

EAPI void efreet_mime_type_cache_clear(void);
EAPI void efreet_mime_type_cache_flush(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
