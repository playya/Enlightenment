/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "exhibit.h"
#include "exhibit_file.h"

pid_t pid = -1;
Evas_List *thumb_list;

int
_ex_thumb_exe_exit(void *data, int type, void *event)
{
   Ecore_Exe_Event_Del  *ev;
   Ex_Thumb             *thumb;
   char                 *ext;

   ev = event;
   if (ev->pid != pid) return 1;
   if (!thumb_list) return 1;

   thumb = thumb_list->data;
   thumb_list = evas_list_remove_list(thumb_list, thumb_list);

   ext = strrchr(thumb->name, '.');

   if (ext)
     {
	Etk_Tree_Row *row;
	
	thumb->image = (char*)epsilon_thumb_file_get(thumb->ep);
	row = etk_tree_row_append(ETK_TREE(thumb->e->cur_tab->itree),
				   NULL,
				   thumb->e->cur_tab->icol, 
				   thumb->image, NULL,
				   thumb->name, NULL);
	if(thumb->selected)
	  {
	     etk_tree_row_select(row);
	     etk_tree_row_scroll_to(row, ETK_TRUE);
	  }
	E_FREE(thumb->image);
	E_FREE(thumb->name);
	epsilon_free(thumb->ep);
	E_FREE(thumb);
     }

   pid = -1;
   _ex_thumb_generate();
   return 1;
}

void
_ex_thumb_generate()
{
   Ex_Thumb *thumb;

   if ((!thumb_list) || (pid != -1)) return;

   pid = fork();
   if (pid == 0)
     {
	/* reset signal handlers for the child */
	signal(SIGSEGV, SIG_DFL);
	signal(SIGILL, SIG_DFL);
	signal(SIGFPE, SIG_DFL);
	signal(SIGBUS, SIG_DFL);

	thumb = thumb_list->data;
	if(_ex_file_is_ebg(thumb->name))
	  epsilon_key_set(thumb->ep, "desktop/background");
	if(epsilon_generate(thumb->ep))
	  {
	     thumb->image = (char*)epsilon_thumb_file_get(thumb->ep);
	  }
	exit(0);
     }
}
