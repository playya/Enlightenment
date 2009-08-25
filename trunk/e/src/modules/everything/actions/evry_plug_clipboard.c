#include "Evry.h"


static Evry_Action *act;
static Ecore_X_Window clipboard_win = 0;

static int
_action(Evry_Action *act)
{
   const Evry_Item *it = act->item1;
   
   ecore_x_selection_primary_set(clipboard_win, it->label, strlen(it->label));
   ecore_x_selection_clipboard_set(clipboard_win, it->label, strlen(it->label));

   return 1;
}

static int
_check_item(Evry_Action *act __UNUSED__, const Evry_Item *it)
{
   return (it && it->label && (strlen(it->label) > 0));
}

static Eina_Bool
_init(void)
{
   Ecore_X_Window win = ecore_x_window_new(0, 0, 0, 1, 1);
   if (!win) return EINA_FALSE;

   act = evry_action_new("Copy to Clipboard", "TEXT", NULL, NULL, "edit-copy",
			 _action, _check_item, NULL, NULL, NULL);

   evry_action_register(act);

   clipboard_win = win;

   return EINA_TRUE;
}

static void
_shutdown(void)
{
   ecore_x_window_free(clipboard_win);
   evry_action_free(act);
}

EINA_MODULE_INIT(_init);
EINA_MODULE_SHUTDOWN(_shutdown);

