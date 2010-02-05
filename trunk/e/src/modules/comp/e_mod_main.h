#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Config        Config;
typedef struct _Mod           Mod;


struct _Config
{
   unsigned char    use_shadow;
   const char      *shadow_file;
   int              engine;
   unsigned char    texture_from_pixmap;
   unsigned char    lock_fps;
   unsigned char    loose_sync;
   unsigned char    efl_sync;
   unsigned char    grab;
   unsigned char    keep_unmapped;
   unsigned char    send_flush;
   unsigned char    send_dump;
   int              max_unmapped_pixels;
   int              max_unmapped_time;
   int              min_unmapped_time;
   unsigned char    effect_fade;
   unsigned char    effect_zoom;
   unsigned char    effect_wobble;
   unsigned char    effect_rotate;
};

#define EFFECT_NONE 0
#define EFFECT_FADE 1
#define EFFECT_ZOOM 0
#define EFFECT_ZOOM 0

struct _Mod
{
   E_Module        *module;
   
   E_Config_DD     *conf_edd;
   Config          *conf;
   
   E_Config_Dialog *config_dialog;
};

extern Mod *_comp_mod;

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);
EAPI int   e_modapi_info     (E_Module *m);

#endif
