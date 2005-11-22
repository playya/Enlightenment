/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef E_H
#define E_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <dlfcn.h>
#include <math.h>
#include <fnmatch.h>
#include <limits.h>
#include <ctype.h>

#include <Evas.h>
#include <Evas_Engine_Buffer.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Evas.h>
#include <Ecore_Con.h>
#include <Ecore_Ipc.h>
#include <Ecore_Job.h>
#include <Ecore_Txt.h>
#include <Ecore_Config.h>
#include <Ecore_File.h>
#include <Ecore_X_Atoms.h>
#include <Ecore_X_Cursor.h>
#include <Eet.h>
#include <Edje.h>

#ifdef USE_E_CONFIG_H
#include "config.h"
#endif

#if HAVE___ATTRIBUTE__
#define __UNUSED__ __attribute__((unused))
#else
#define __UNUSED__
#endif

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

typedef struct _E_Before_Idler E_Before_Idler;
typedef struct _E_Rect E_Rect;

/* convenience macro to compress code and avoid typos */
#define E_FN_DEL(_fn, _h) if (_h) { _fn(_h); _h = NULL; }
#define E_INTERSECTS(x, y, w, h, xx, yy, ww, hh) (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && (((x) + (w)) > (xx)) && (((y) + (h)) > (yy)))
#define E_INSIDE(x, y, xx, yy, ww, hh) (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && ((x) >= (xx)) && ((y) >= (yy)))
#define E_CONTAINS(x, y, w, h, xx, yy, ww, hh) (((xx) >= (x)) && (((x) + (w)) >= ((xx) + (ww))) && ((yy) >= (y)) && (((y) + (h)) >= ((yy) + (hh))))
#define E_SPANS_COMMON(x1, w1, x2, w2) (!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))
#define E_REALLOC(p, s, n) p = (s *)realloc(p, sizeof(s) * n)
#define E_NEW(s, n) (s *)calloc(n, sizeof(s))
#define E_NEW_BIG(s, n) (s *)malloc(n * sizeof(s))
#define E_FREE(p) { if (p) {free(p); p = NULL;} }

#define E_REMOTE_OPTIONS 1
#define E_REMOTE_OUT     2
#define E_WM_IN          3
#define E_REMOTE_IN      4
#define E_ENUM           5
#define E_LIB_IN         6

#define E_TYPEDEFS 1
#include "e_includes.h"
#undef E_TYPEDEFS
#include "e_includes.h"

EAPI E_Before_Idler *e_main_idler_before_add(int (*func) (void *data), void *data, int once);
EAPI void            e_main_idler_before_del(E_Before_Idler *eb);


struct _E_Before_Idler
{
   int          (*func) (void *data);
   void          *data;
   unsigned char  once : 1;
   unsigned char  delete_me : 1;
};

struct _E_Rect
{
   int x, y, w, h;
};

extern EAPI E_Path *path_data;
extern EAPI E_Path *path_images;
extern EAPI E_Path *path_fonts;
extern EAPI E_Path *path_themes;
extern EAPI E_Path *path_icons;
extern EAPI E_Path *path_init;
extern EAPI E_Path *path_modules;
extern EAPI E_Path *path_backgrounds;
extern EAPI E_Path *path_input_methods;
extern EAPI E_Path *path_messages;
extern EAPI int     restart;
extern EAPI int     good;
extern EAPI int     evil;
extern EAPI int     starting;

#endif
