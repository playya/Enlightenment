#include <Ecore.h>
#include "ecore_x_private.h"
#include <Ecore_X.h>
#include <Ecore_Txt.h>

/* FIXME: Initialize! */
static Ecore_X_Selection_Data selections[3];
static Ecore_X_Selection_Data request_data[3];
static Ecore_X_Selection_Converter *converters;

static int _ecore_x_selection_converter_text(char *target, void *data, int size, void **data_ret, int *size_ret);

void
_ecore_x_selection_data_initialize(void)
{
   memset(selections, 0, sizeof(selections));
   memset(request_data, 0, sizeof(request_data));

   /* Initialize converters */
   converters = NULL;
   ecore_x_selection_converter_atom_add(_ecore_x_atom_text, 
         _ecore_x_selection_converter_text);
   ecore_x_selection_converter_atom_add(_ecore_x_atom_compound_text,
         _ecore_x_selection_converter_text);
   ecore_x_selection_converter_atom_add(_ecore_x_atom_string,
         _ecore_x_selection_converter_text);
   
}

static void
_ecore_x_selection_request_data_get(Ecore_X_Atom selection, void **buf, int *len)
{
   int i;
   char *data;
   if (selection == _ecore_x_atom_selection_primary)
      i = 0;
   else if (selection == _ecore_x_atom_selection_secondary)
      i = 1;
   else if (selection == _ecore_x_atom_selection_clipboard)
      i = 2;
   else
      return;

   if (!request_data[i].data || !request_data[i].length)
   {
      *len = 0;
      *buf = NULL;
      return;
   }

   data = malloc(request_data[i].length);
   memcpy(data, request_data[i].data, request_data[i].length);
   *len = request_data[i].length;
   *buf = data;
   return;
}

void
ecore_x_selection_primary_request_data_get(void **buf, int *len)
{
   _ecore_x_selection_request_data_get(_ecore_x_atom_selection_primary,
                                       buf, len);
}

void
ecore_x_selection_secondary_request_data_get(void **buf, int *len)
{
   _ecore_x_selection_request_data_get(_ecore_x_atom_selection_secondary,
                                       buf, len);
}

void
ecore_x_selection_clipboard_request_data_get(void **buf, int *len)
{
   _ecore_x_selection_request_data_get(_ecore_x_atom_selection_clipboard,
                                       buf, len);
}

void 
_ecore_x_selection_request_data_set(Ecore_X_Selection_Data data)
{
   int i;
   if (data.selection == _ecore_x_atom_selection_primary)
      i = 0;
   else if (data.selection == _ecore_x_atom_selection_secondary)
      i = 1;
   else if (data.selection == _ecore_x_atom_selection_clipboard)
      i = 2;
   else
      return;

   request_data[i] = data;
}

Ecore_X_Selection_Data *
_ecore_x_selection_get(Atom selection)
{
   if (selection == _ecore_x_atom_selection_primary)
      return &selections[0];
   else if (selection == _ecore_x_atom_selection_secondary)
      return &selections[1];
   else if (selection == _ecore_x_atom_selection_clipboard)
      return &selections[2];
   else
      return NULL;
}

int 
_ecore_x_selection_set(Window w, char *data, int len, Atom selection)
{
   int in;
   char *buf = NULL;
   
   XSetSelectionOwner(_ecore_x_disp, selection, w, _ecore_x_event_last_time);
   if (XGetSelectionOwner(_ecore_x_disp, selection) != w)
      return 0;
   
   if (selection == _ecore_x_atom_selection_primary)
      in = 0;
   else if (selection == _ecore_x_atom_selection_secondary)
      in = 1;
   else
      in = 2;
   
   if (data)
   {
      selections[in].win = w;
      selections[in].selection = selection;
      selections[in].length = len;
      selections[in].time = _ecore_x_event_last_time;
      
      buf = malloc(sizeof(char) * len);
      memcpy(buf, data, sizeof(char) * len);
      selections[in].data = buf;
   }
   else
   {
      if (selections[in].data)
      {
         free(selections[in].data);
         memset(&selections[in], 0, sizeof(Ecore_X_Selection_Data));
      }
   }
   
   /* ecore_x_window_prop_property_set(_ecore_x_disp, w, selection, 
         XA_STRING, 8, data, len); */
   return 1;
}

int 
ecore_x_selection_primary_set(Ecore_X_Window w, char *data, int len)
{
   return _ecore_x_selection_set(w, data, len, _ecore_x_atom_selection_primary);
}

int 
ecore_x_selection_primary_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, _ecore_x_atom_selection_primary);
}

int 
ecore_x_selection_secondary_set(Ecore_X_Window w, char *data, int len)
{
   return _ecore_x_selection_set(w, data, len, _ecore_x_atom_selection_secondary);
}

int 
ecore_x_selection_secondary_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, _ecore_x_atom_selection_secondary);
}

int 
ecore_x_selection_clipboard_set(Ecore_X_Window w, char *data, int len)
{
   return _ecore_x_selection_set(w, data, len, _ecore_x_atom_selection_clipboard);
}

int 
ecore_x_selection_clipboard_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, _ecore_x_atom_selection_clipboard);
}

Atom
_ecore_x_selection_target_atom_get(char *target)
{
   Atom x_target;
   
   if (!strcmp(target, ECORE_X_SELECTION_TARGET_TEXT))
      x_target = _ecore_x_atom_text;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_COMPOUND_TEXT))
      x_target = _ecore_x_atom_compound_text;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_STRING))
      x_target = _ecore_x_atom_string;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_UTF8_STRING))
      x_target = _ecore_x_atom_utf8_string;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_FILENAME))
      x_target = _ecore_x_atom_file_name;
   else
   {
      char *atom_name;
      atom_name = malloc(strlen(target) + 4);
      sprintf(atom_name, "_E_%s", target);
      x_target = XInternAtom(_ecore_x_disp, atom_name, False);
      free(atom_name);
   }

   return x_target;
}

char *
_ecore_x_selection_target_get(Atom target)
{
   if (target == _ecore_x_atom_file_name)
      return strdup(ECORE_X_SELECTION_TARGET_FILENAME);
   else if (target == _ecore_x_atom_string)
      return strdup(ECORE_X_SELECTION_TARGET_STRING);
   else if (target == _ecore_x_atom_utf8_string)
      return strdup(ECORE_X_SELECTION_TARGET_UTF8_STRING);
   else if (target == _ecore_x_atom_text)
      return strdup(ECORE_X_SELECTION_TARGET_TEXT);
   else
      return strdup(ECORE_X_SELECTION_TARGET_TEXT);
}

static void 
_ecore_x_selection_request(Ecore_X_Window w, Ecore_X_Atom selection, char *target_str) 
{
   Ecore_X_Atom target, prop;

   target = _ecore_x_selection_target_atom_get(target_str);
   
   if (selection == _ecore_x_atom_selection_primary)
      prop = _ecore_x_atom_selection_prop_primary;
   else if (selection == _ecore_x_atom_selection_secondary)
      prop = _ecore_x_atom_selection_prop_secondary;
   else
      prop = _ecore_x_atom_selection_prop_clipboard;

   XConvertSelection(_ecore_x_disp, selection, target, prop,
                     w, _ecore_x_event_last_time);
}

void 
ecore_x_selection_primary_request(Ecore_X_Window w, char *target)
{
   _ecore_x_selection_request(w, _ecore_x_atom_selection_primary, target);
}

void 
ecore_x_selection_secondary_request(Ecore_X_Window w, char *target)
{
   _ecore_x_selection_request(w, _ecore_x_atom_selection_secondary, target);
}

void 
ecore_x_selection_clipboard_request(Ecore_X_Window w, char *target)
{
   _ecore_x_selection_request(w, _ecore_x_atom_selection_clipboard, target);
}

void
ecore_x_selection_converter_atom_add(Ecore_X_Atom target,
      int (*func)(char *target, void *data, int size, void **data_ret, int *size_ret))
{
   Ecore_X_Selection_Converter *cnv;

   cnv = converters;
   if (converters) 
   {
      while (1)
      {
         if (cnv->target == target)
         {
            cnv->convert = func;
            return;
         }
         if (cnv->next)
            cnv = cnv->next;
         else
            break;
      }
   
      cnv->next = calloc(1, sizeof(Ecore_X_Selection_Converter));
      cnv = cnv->next;
   } else {
      converters = calloc(1, sizeof(Ecore_X_Selection_Converter));
      cnv = converters;
   }
   cnv->target = target;
   cnv->convert = func;
}


void
ecore_x_selection_converter_add(char *target, 
      int (*func)(char *target, void *data, int size, void **data_ret, int *size_ret))
{
   Ecore_X_Atom x_target;
   
   if (!func || !target)
      return;

   x_target = _ecore_x_selection_target_atom_get(target);
   
   ecore_x_selection_converter_atom_add(x_target, func);
}

void
ecore_x_selection_converter_atom_del(Ecore_X_Atom target)
{
   Ecore_X_Selection_Converter *cnv, *prev_cnv;
   
   prev_cnv = NULL;
   cnv = converters;
   
   while (cnv)
   {
      if (cnv->target == target)
      {
         if (target == _ecore_x_atom_text ||
             target == _ecore_x_atom_compound_text ||
             target == _ecore_x_atom_string)
         {
            cnv->convert = _ecore_x_selection_converter_text;
         }
         else
         {
            if(prev_cnv)
               prev_cnv->next = cnv->next;
            else
               converters = NULL; /* This was the only converter */
            free(cnv);
         }
         
         return;
      }
      prev_cnv = cnv;
      cnv = cnv->next;
   }
}

void
ecore_x_selection_converter_del(char *target)
{
   Ecore_X_Atom x_target;
   
   if (!target)
      return;
   
   x_target = _ecore_x_selection_target_atom_get(target);
   ecore_x_selection_converter_atom_del(x_target);
}


/* Locate and run conversion callback for specified selection target */
int
_ecore_x_selection_convert(Atom selection, Atom target, void **data_ret)
{
   Ecore_X_Selection_Data *sel;
   Ecore_X_Selection_Converter *cnv;
   void *data;
   int size;
   char *tgt_str;
   
   sel = _ecore_x_selection_get(selection);
   tgt_str = _ecore_x_selection_target_get(target);
   
   for (cnv = converters; cnv; cnv = cnv->next)
   {
      if (cnv->target == target)
      {
         int r;
         r = cnv->convert(tgt_str, sel->data, sel->length, &data, &size);
         if (r)
         {
            *data_ret = data;
            return r;
         }
         else
            return -1;
      }
   }

   free(tgt_str);

   return -1;
}

/* Converter for standard non-utf8 text targets */
static int _ecore_x_selection_converter_text(char *target, void *data, int size, void **data_ret, int *size_ret)
{
   XTextProperty text_prop;
   char *mystr;
   XICCEncodingStyle style;
   
   if (!data || !size)
      return 0;

   if (!strcmp(target, ECORE_X_SELECTION_TARGET_TEXT))
      style = XTextStyle;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_COMPOUND_TEXT))
      style = XCompoundTextStyle;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_STRING))
      style = XStringStyle;
   else
      return 0;
   
   mystr = strdup(data);
   
   if (XmbTextListToTextProperty(_ecore_x_disp, &mystr, 1, style, &text_prop) == Success)
   {
      int bufsize = strlen(text_prop.value) + 1;
      *data_ret = malloc(bufsize);
      memcpy(*data_ret, text_prop.value, bufsize);
      *size_ret = bufsize;
      XFree(text_prop.value);
      return 1;
   }
   else
      return 0;
}

