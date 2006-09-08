/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "exhibit.h"
#include <Edje.h>

static void _ex_file_download_dialog_response(Etk_Object *obj, int response_id, void *data);

char *viewables[] =
{
   ".jpg",
   ".jpeg",
   ".png",
   ".gif",
   ".tiff",
   ".edj",
   ".svg",
   ".svgz",
   NULL
};

static Etk_Widget *dialog;
static Etk_Widget *progressbar;


static void
_ex_file_download_dialog_response(Etk_Object *obj, int response_id, void *data)
{
   switch(response_id)
     {
      case ETK_RESPONSE_CANCEL:
	 etk_object_destroy(ETK_OBJECT(obj));
	 break;
      default:
	 break;
     }
}

void
_ex_file_download_dialog(char *url)
{
   char string[PATH_MAX + 50];

   sprintf(string, "Downloading %s", url);

   dialog = etk_message_dialog_new(ETK_MESSAGE_DIALOG_INFO,
	 ETK_MESSAGE_DIALOG_CANCEL,
	 string);
   etk_signal_connect("response", ETK_OBJECT(dialog),
	 ETK_CALLBACK(_ex_file_download_dialog_response), NULL);

   etk_container_border_width_set(ETK_CONTAINER(dialog), 4);
   etk_window_title_set(ETK_WINDOW(dialog),
	 _("Exhibit - Download progress"));

   progressbar = etk_progress_bar_new();
   etk_progress_bar_text_set(ETK_PROGRESS_BAR(progressbar), "0% done");
   etk_dialog_pack_in_main_area(ETK_DIALOG(dialog), progressbar, 
	 ETK_TRUE, ETK_TRUE, 2, ETK_TRUE);

   etk_widget_show_all(dialog);
}

int 
_ex_file_download_progress_cb(void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow)
{
   double fraction;
   char text[32];

   if (dlnow == 0 || dltotal == 0)
     return 0;

   fraction = (double) ((double) dlnow) / ((double) dltotal);
   snprintf(text, sizeof(text), "%d%% done", (int) (fraction * 100.0));
   etk_progress_bar_text_set(ETK_PROGRESS_BAR(progressbar), text);
   etk_progress_bar_fraction_set(ETK_PROGRESS_BAR(progressbar), fraction);
   
   return 0;
}

void 
_ex_file_download_complete_cb(void *data, const char *file, int status)
{
   etk_object_destroy(ETK_OBJECT(dialog));
   _ex_main_itree_add(file, file);
   D(("Download of file %s is done\n", file));

}

int
_ex_file_is_viewable(char *file)
{
   char *ext;
   int i = 0;

   ext = strrchr(file, '.');
   if(!ext) return 0;

   for(i = 0; viewables[i]; i++)
     {
	if(!strcasecmp(ext, viewables[i]))
	  return 1;
     }

   return 0;
}

int
_ex_file_is_ebg(char *file)
{
   int          val;
   char        *ext;
   Evas_List   *groups, *l;
   
   ext = strrchr(file, '.');
   if (!ext) return 0;
   
   if (strcasecmp(ext, ".edj"))
     return 0;
   
   val = 0;
   groups = edje_file_collection_list(file);
   if (!groups)
     return 0;
   
   for (l = groups; l; l = l->next)
     {
	if (!strcmp(l->data, "desktop/background"))
	  {
	     val = 1;
	     break;
	  }
     }
   edje_file_collection_list_free(groups);
   return val;
}

int
_ex_file_is_jpg(char *file)
{
   char        *ext;

   ext = strrchr(file, '.');
   if (!ext) return 0;

   if (strcasecmp(ext, ".jpg") && strcasecmp(ext, ".jpeg"))
      return 0;

   return 1;
}

char *
_ex_file_strip_extention(char *path) {
   char *ptr;
   char *c;
   char *file;
   int i;
   
   i = 0;
   ptr = path;
   c = strrchr(ptr,'.');
   file = malloc(strlen(path));
   
   while(ptr != c) {
      file[i] = *ptr;
      ptr++;
      i++;
   }
   
   file[i] = '\0';
   return file;
}

const char *
_ex_file_get(char *path)
{
   if(!path[0]) return NULL;
   
   if(path[strlen(path) - 1] == '/' && strlen(path) >= 2)
     {
	char *ret;
	
	ret = &path[strlen(path) - 2];
	while(ret != path)
	  {
	     if(*ret == '/')
	       return ++ret;
	     --ret;
	  }
	return path;	
     }
   else return ecore_file_get_file(path);
}
