#include <Etk.h>
#include "Entrance_Widgets.h"

#include "_ew_list.h"

static void _ew_tree_cb_row_clicked(Etk_Object *, Etk_Tree_Row *, Etk_Event_Mouse_Up_Down *, void *);

Entrance_Widget
ew_textlist_new(const char *title, int w, int h, int r_h, int c_w)
{
   Entrance_Widget ew = _ew_list_new(title, w, h, r_h);
   if(!ew) {
	   return NULL;
   }

   ew->list_col = etk_tree_col_new(ETK_TREE(ew->owner), NULL, etk_tree_model_text_new(ETK_TREE(ew->owner)), c_w);

   return _ew_list_buildtree(ew);
}

void
ew_textlist_add(Entrance_Widget ew, const char *label, void *data, size_t size,
                              void (*func) (void))
{
   Etk_Tree_Row *row;
   
   etk_tree_freeze(ETK_TREE(ew->owner));

   row = etk_tree_append(ETK_TREE(ew->owner), etk_tree_nth_col_get(ETK_TREE(ew->owner), 0),
                         label, NULL);
   
   Entrance_List_Data ewld = ew_listdata_new();
   if(ewld)
   {
	   ewld->func = func;
	   if(data)
	   {
		   char *s = data;
		   int c = strlen(s);
		   memcpy(ewld->data, data, size);
	   }
   }

   etk_tree_row_data_set(row, ewld);

   etk_tree_thaw(ETK_TREE(ew->owner));
}
