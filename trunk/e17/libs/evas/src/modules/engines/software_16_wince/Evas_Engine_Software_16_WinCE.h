#ifndef __EVAS_ENGINE_SOFTWARE_16_WINCE_H__
#define __EVAS_ENGINE_SOFTWARE_16_WINCE_H__


#include <windows.h>


typedef struct _Evas_Engine_Info_Software_16_WinCE Evas_Engine_Info_Software_16_WinCE;

struct _Evas_Engine_Info_Software_16_WinCE
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct {
      HWND  window;
      int   backend; /* 0: auto, 1: raw, 2: gapi, 3: ddraw */
      int   rotation;
   } info;
   /* engine specific function calls to query stuff about messages */
   struct {
      int   (*suspend) (int backend);
      int   (*resume)  (int backend);
      void *(*default_keys)  (int backend);
   } func;
};


#endif /* __EVAS_ENGINE_SOFTWARE_16_WINCE_H__ */
