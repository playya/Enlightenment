#include <limits.h>
#include <Ecore_File.h>
#include <Ecore_Data.h>
#include <Entrance_Widgets.h>
#include <Entrance_Edit.h>

static void _theme_cb_selected(void);
static void _theme_cb_ok(void *, void *);
static void _theme_cb_apply(void *, void *);
static void _theme_cb_close(void *, void *);

static char* _theme_get_path(const char *);
static void _theme_close(void);
static void _theme_apply(void);

static Entrance_Dialog win;
static Entrance_Widget img_preview;
static Entrance_List list_thumbs;
static Entrance_Widget group_themes;
static Entrance_Widget group_preview;
static Entrance_Widget group_options;

void
egui_theme_dialog_show()
{
   Ecore_List *themes;
   char *theme;
   
   win = ew_dialog_new(_("Entrance - Theme Chooser"), EW_FALSE);

   group_preview = ew_dialog_group_add(win, _("Preview"));

   img_preview = ew_image_new(320, 240);
   ew_group_add(group_preview, img_preview);

   group_themes = ew_dialog_group_add(win, _("Themes"));

   list_thumbs = ew_textlist_new(NULL, 320, 140, 20, 90);
   
   /* scan for themes and add them to the list */
   themes = ecore_file_ls("/usr/share/entrance/themes");
   if(!themes || ecore_list_is_empty(themes))
     return;

   theme = ecore_list_first(themes);
   char *theme_file = _theme_get_path(theme);
   ew_image_edje_load(img_preview, theme_file, "Preview");
   free(theme_file);
   /*FIXME: selecting the first row doesn't work - maybe we select first row while adding elements to the list:(*/
   ew_list_first_row_select(list_thumbs);

   ecore_list_goto_first(themes);
   while((theme = ecore_list_next(themes)))
     {
        char *theme_no_ext = ecore_file_strip_ext(theme);
		ew_textlist_add(list_thumbs, theme_no_ext, theme, strlen(theme) + 1, _theme_cb_selected);

        free(theme_no_ext);
     }
   ecore_list_destroy(themes);
   
   ew_group_add(group_themes, list_thumbs);

   /*Entrance_Widget group_options = ew_dialog_group_add(win, _("Options"));*/
   
   ew_dialog_close_button_add(win, _theme_cb_close, NULL);
   ew_dialog_apply_button_add(win, _theme_cb_apply, NULL);
   ew_dialog_ok_button_add(win, _theme_cb_ok, NULL);
   
   ew_dialog_show(win);
}

static void
_theme_cb_selected()
{
	char* theme = ew_list_selected_data_get(list_thumbs);
	char* theme_path = _theme_get_path(theme);
	ew_image_edje_load(img_preview, theme_path, "Preview");

	free(theme_path);
}


static void
_theme_cb_ok(void *object, void *data)
{
   _theme_apply();
   _theme_close();
}

static void
_theme_cb_apply(void *object, void *data)
{
   _theme_apply();
}

static void
_theme_cb_close(void *object, void *data)
{
	_theme_close();
}

static char*
_theme_get_path(const char *t)
{
	char *path = calloc(PATH_MAX, sizeof(*path));
	snprintf(path, PATH_MAX, "/usr/share/entrance/themes/%s", t);

	return path;
}

static void 
_theme_apply(void)
{
    char *theme = ew_list_selected_data_get(list_thumbs);
	if(!theme) 
	{
		printf("Error. Please select a theme first\n");
		return;
	}

	entrance_edit_string_set(ENTRANCE_EDIT_KEY_CLIENT_THEME_STR, theme);
	if(!entrance_edit_save())
	{
		printf("Error setting theme - %s. Please check your permissions\n", theme);
	}
}

static void
_theme_close(void)
{
	ew_dialog_destroy(win);
}
