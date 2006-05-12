#ifndef E_MOD_LANG_H
#define E_MOD_LANG_H

#include <e.h>

typedef struct _Language	    Language;
typedef struct _Language_Def	    Language_Def;
typedef struct _Language_Kbd_Model  Language_Kbd_Model;

struct _Language_Kbd_Model
{
   const char *kbd_model;
   const char *kbd_model_desctiption;
};

struct _Language
{
   const char *lang_name;
   const char *lang_shortcut;
   const char *lang_flag;
   const char *kbd_model;
   const char *kbd_layout;
   const char *kbd_variant;
};

struct _Language_Def
{
   const char  *lang_name;
   const char  *lang_shortcut;
   const char  *lang_flag;
   Evas_List   *kbd_layout;
   Evas_List   *kbd_variant;
};

/*void  lang_register_language(const char *lang_name, const char *lang_shortcut,
			     const char *lang_flag, const char *kbd_layout,
			     const char *kbd_layout_variant);*/

/*void lang_register_language_layout(const char *lang_name, const char *kbd_layout);*/
/*void lang_register_language_layout_variant(const char *lang_name, const char *kbd_layout_variant);*/

/*void  lang_unregister_all_languages();*/

int   lang_load_kbd_models();
void  lang_free_kbd_models();

int   lang_load_xfree_languages();
void  lang_free_xfree_languages();

#endif
