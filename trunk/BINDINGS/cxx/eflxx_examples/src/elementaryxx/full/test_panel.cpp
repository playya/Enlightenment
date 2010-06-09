#include "test.h"

/*static Elm_Genlist_Item_Class itc;

static char *_label_get(const void *data, Evas_Object *obj, const char *source);
static Evas_Object *_icon_get(const void *data, Evas_Object *obj, const char *source);
static Eina_Bool _state_get(const void *data, Evas_Object *obj, const char *source);
static void _item_del(const void *data, Evas_Object *obj);
static void _fill_list(Evas_Object *obj);
static Eina_Bool _dir_has_subs(const char *path);*/

void test_panel (void *data, Evas_Object *obj, void *event_info) 
{
  Window *win = Window::factory ("panel", ELM_WIN_BASIC);
  win->setTitle ("Panel");
  win->setAutoDel (true);
  
  Background *bg = Background::factory (*win);
  win->addObjectResize (*bg);
  bg->setWeightHintSize (EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  bg->show ();
  
  Panel *panel = Panel::factory (*win);
  panel->setOrientation (ELM_PANEL_ORIENT_LEFT);
  panel->setWeightHintSize (EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  panel->setAlignHintSize (EVAS_HINT_FILL, EVAS_HINT_FILL);
  panel->show ();

  GenList *gl = GenList::factory (*win);
  gl->resize (Eflxx::Size (100, 100));
  gl->setWeightHintSize (EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  gl->setAlignHintSize (EVAS_HINT_FILL, EVAS_HINT_FILL);
  gl->show ();
  panel->setContent (*gl);

#warning TODO: GenList Wrapper
  cerr << "TODO: GenList Wrapper" << endl;

  win->resize (Size (300, 300));
  win->show ();
}

/*static char *
_label_get(const void *data, Evas_Object *obj, const char *source) 
{
   return strdup(ecore_file_file_get(data));
}

static Evas_Object *
_icon_get(const void *data, Evas_Object *obj, const char *source) 
{
   if (!strcmp(source, "elm.swallow.icon")) 
     {
        Evas_Object *ic;

        ic = elm_icon_add(obj);
        if (ecore_file_is_dir((char *)data))
          elm_icon_standard_set(ic, "folder");
        else
          elm_icon_standard_set(ic, "file");
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
        evas_object_show(ic);
        return ic;
     }
   return NULL;
}

static Eina_Bool 
_state_get(const void *data, Evas_Object *obj, const char *source) 
{
   return EINA_FALSE;
}

static void 
_item_del(const void *data, Evas_Object *obj) 
{
   eina_stringshare_del(data);
}

static void 
_fill_list(Evas_Object *obj) 
{
   DIR *d;
   struct dirent *de;
   Eina_List *dirs = NULL, *l;
   char *real;

   if (!(d = opendir(getenv("HOME")))) return;
   while ((de = readdir(d)) != NULL) 
     {
        char buff[PATH_MAX];

        if (de->d_name[0] == '.') continue;
        snprintf(buff, sizeof(buff), "%s/%s", getenv("HOME"), de->d_name);
        if (!ecore_file_is_dir(buff)) continue;
        real = ecore_file_realpath(buff);
        dirs = eina_list_append(dirs, real);
     }
   closedir(d);

   dirs = eina_list_sort(dirs, eina_list_count(dirs), EINA_COMPARE_CB(strcoll));

   EINA_LIST_FOREACH(dirs, l, real) 
     {
        Eina_Bool result = EINA_FALSE;

        result = _dir_has_subs(real);
        if (!result) 
          elm_genlist_item_append(obj, &itc, eina_stringshare_add(real), 
                                  NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
        else 
          elm_genlist_item_append(obj, &itc, eina_stringshare_add(real), 
                                  NULL, ELM_GENLIST_ITEM_SUBITEMS, 
                                  NULL, NULL);
        free(real);
     }
   eina_list_free(dirs);
}

static Eina_Bool 
_dir_has_subs(const char *path) 
{
   DIR *d;
   struct dirent *de;
   Eina_Bool result = EINA_FALSE;

   if (!path) return result;
   if (!(d = opendir(path))) return result;
   while ((de = readdir(d)) != NULL) 
     {
        char buff[PATH_MAX];

        if (de->d_name[0] == '.') continue;
        snprintf(buff, sizeof(buff), "%s/%s", path, de->d_name);
        if (ecore_file_is_dir(buff)) 
          {
             result = EINA_TRUE;
             break;
          }
     }
   closedir(d);
   return result;
}
*/