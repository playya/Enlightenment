#ifndef _ECORE_TXT_H
#define _ECORE_TXT_H

#ifdef EAPI
#undef EAPI
#endif
#ifdef WIN32
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef GCC_HASCLASSVISIBILITY
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#endif

/**
 * @file Ecore_Txt.h
 * @brief Provides a text encoding conversion function.
 */

#ifdef __cplusplus
extern "C" {
#endif

EAPI char *ecore_txt_convert(char *enc_from, char *enc_to, char *text);

#ifdef __cplusplus
}
#endif

#endif
