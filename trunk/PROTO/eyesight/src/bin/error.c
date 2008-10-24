#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <Evas.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#include "main_window.h"

#include "error.h"

void
append_startup_error(Eina_List **startup_errors, char *error_string,
                     char *argument)
{
   char *errstr;
   errstr = malloc(strlen(error_string) + strlen(argument) - 1);
   snprintf(errstr, (strlen(error_string) + strlen(argument) - 1), error_string,
            argument);
   *startup_errors = eina_list_append(*startup_errors, errstr);         
}

void
append_startup_error2(Eina_List **startup_errors, char *error_string,
                     char *argument1, char *argument2)
{
   char *errstr;
   errstr = malloc(strlen(error_string) + strlen(argument1) + strlen(argument2)
                   - 2);
   snprintf(errstr, (strlen(error_string) + strlen(argument1) + 
            strlen(argument2) - 2), error_string, argument1, argument2);
   *startup_errors = eina_list_append(*startup_errors, errstr);         
}

void
display_startup_error_dialog(Ecore_Evas *ee, Eina_List *startup_errors)
{
   char *str;
   Eina_List *tmp;
   unsigned int len = 0;
   Evas_Object *main_window;
   
   tmp = startup_errors;
   
   if (!startup_errors) return;
   
   // Calculating string size
   len = strlen(ERROR_STARTUP_BEGIN) + strlen("<br>");
   do
   {
      len += strlen("<i>> ") + strlen((char *)eina_list_data_get(tmp)) + strlen("<br>");
   } while ((tmp = eina_list_next(tmp)));
   
   str = calloc(sizeof(char), len);
   
   // Concatenate errors
   tmp = startup_errors;
   strncpy(str, ERROR_STARTUP_BEGIN, len);
   strncat(str, "<br>", len);
   
   do
   {
      strncat(str, "<i>> ", len);
      strncat(str, eina_list_data_get(tmp), len);
      strncat(str, "<br>", len);
   } while ((tmp = eina_list_next(tmp)));
   
   // Display error dialog (error_show signal will be emitted later)
   // TODO: error.c: Resize dialog to fit text
   main_window = ((Main_Window *)ecore_evas_data_get(ee, "main_window"))->controls;
   edje_object_part_text_set(main_window, 
                             "eyesight/main_window/controls/error_dialog_text", 
                             str);   
}


