#ifndef __EYESIGHT_PRIVATE_H__
#define __EYESIGHT_PRIVATE_H__


typedef struct _Eyesight_Module Eyesight_Module;
typedef struct _Eyesight_Plugin Eyesight_Plugin;


typedef Eina_Bool (*Eyesight_Module_Open)(Evas *evas, Evas_Object **obj, const Eyesight_Module **module, void **backend);
typedef void      (*Eyesight_Module_Close)(Eyesight_Module *module, void *backend);


struct _Eyesight_Index_Item
{
  char                     *title;
  Eyesight_Link_Action_Kind action;
  int                       page;
  Eina_List                *children;
};

struct _Eyesight_Plugin
{
  Eyesight_Module_Open m_open;
  Eyesight_Module_Close m_close;
};

struct _Eyesight_Module
{
  Eina_Bool            (*init)(Evas *evas, Evas_Object **obj, void **backend);
  void                 (*shutdown)(void *backend);
  void                *(*file_open)(void *backend, const char *filename);
  void                 (*file_close)(void *backend);
  Eina_List           *(*toc_get)(void *backend);
  int                  (*page_count)(void *backend);

  void                 (*page_set)(void *backend, int page);
  int                  (*page_get)(void *backend);
  void                 (*page_scale_set)(void *backend, double hscale, double vscale);
  void                 (*page_scale_get)(void *backend, double *hscale, double *vscale);
  void                 (*page_orientation_set)(void *backend, Eyesight_Orientation orientation);
  Eyesight_Orientation (*page_orientation_get)(void *backend);
  void                 (*page_size_get)(void *backend, int *width, int *height);
  void                 (*page_render)(void *backend);
  char                *(*page_text_get)(void *backend, Eina_Rectangle rect);
  Eina_List           *(*page_text_find)(void *eb, const char *text, Eina_Bool is_case_sensitive, Eina_Bool backward);
  Eina_List           *(*page_links_get)(void *eb);

  Eyesight_Plugin *plugin;
};

#ifdef __cplusplus
extern "C" {
#endif

EAPI Eina_Bool _eyesight_module_register(const char *name, Eyesight_Module_Open open, Eyesight_Module_Close close);
EAPI Eina_Bool _eyesight_module_unregister(const char *name);

EAPI Eyesight_Link *_eyesight_link_new(Eyesight_Link_Action_Kind action);
EAPI void           _eyesight_link_free(Eyesight_Link *link);

#ifdef __cplusplus
}
#endif


#endif /* __EYESIGHT_PRIVATE_H__ */
