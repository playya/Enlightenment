#include "etk_test.h"
#include "../../config.h"

static Etk_Bool _etk_test_tree_window_deleted_cb(void *data)
{
   etk_widget_hide(ETK_WIDGET(data));
   return 1;
}

static void _etk_test_tree_row_selected(Etk_Object *object, Etk_Tree_Row *row, void *data)
{
   Etk_Tree *tree;
   char *col1_string, *col3_path;
   int col2_value;

   tree = ETK_TREE(object);
   printf(_("Row selected %p %p\n"), object, row);
   etk_tree_row_fields_get(row, etk_tree_nth_col_get(tree, 0), NULL, &col1_string, etk_tree_nth_col_get(tree, 1), &col2_value, etk_tree_nth_col_get(tree, 2), &col3_path, NULL);
   printf("\"%s\" %d %s\n\n", col1_string, col2_value, col3_path);
}

static void _etk_test_tree_row_unselected(Etk_Object *object, Etk_Tree_Row *row, void *data)
{
   printf(_("Row unselected %p %p\n"), object, row);
}

void etk_test_tree_window_create(void *data)
{
   static Etk_Widget *win = NULL;
   Etk_Widget *tree;
   Etk_Tree_Row *row;
   Etk_Tree_Col *col1, *col2, *col3;
   Etk_Widget *hpaned;
   Etk_Widget *vbox;
   Etk_Widget *label;
   int i;

   if (win)
	{
		etk_widget_show(ETK_WIDGET(win));
		return;
	}

   win = etk_window_new();
   etk_window_title_set(ETK_WINDOW(win), _("Etk Tree Test"));
   etk_signal_connect("delete_event", ETK_OBJECT(win), ETK_CALLBACK(_etk_test_tree_window_deleted_cb), win);	
	
   hpaned = etk_hpaned_new();
   etk_container_add(ETK_CONTAINER(win), hpaned);

   /* The tree: */
   vbox = etk_vbox_new(FALSE, 0);
   etk_paned_add1(ETK_PANED(hpaned), vbox);

   label = etk_label_new(_("<h1>Tree:</h1>"));
   etk_box_pack_start(ETK_BOX(vbox), label, FALSE, TRUE, 0);

   tree = etk_tree_new();
   etk_widget_size_request_set(tree, 320, 400);
   //etk_container_add(ETK_CONTAINER(win), tree);
   etk_box_pack_start(ETK_BOX(vbox), tree, TRUE, TRUE, 0);

   etk_tree_mode_set(ETK_TREE(tree), ETK_TREE_MODE_TREE);
   col1 = etk_tree_col_new(ETK_TREE(tree), _("Column 1"), ETK_TREE_COL_ICON_TEXT, 100);
   col2 = etk_tree_col_new(ETK_TREE(tree), _("Column 2"), ETK_TREE_COL_INT, 100);
   col3 = etk_tree_col_new(ETK_TREE(tree), _("Column 3"), ETK_TREE_COL_IMAGE, 100);
   etk_tree_build(ETK_TREE(tree));

   etk_tree_freeze(ETK_TREE(tree));
   for (i = 0; i < 1000; i++)
   {
      row = etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/open.png", _("Row1"),
         col2, 1, col3, PACKAGE_DATA_DIR "/images/1star.png", NULL);
      row = etk_tree_append_to_row(row, col1, PACKAGE_DATA_DIR "/images/open.png", _("Row2"),
         col2, 2, col3, PACKAGE_DATA_DIR "/images/2stars.png", NULL);
      etk_tree_append_to_row(row, col1, PACKAGE_DATA_DIR "/images/open.png", _("Row3"),
         col2, 3, col3, PACKAGE_DATA_DIR "/images/3stars.png", NULL);
   }
   etk_tree_thaw(ETK_TREE(tree));

   etk_signal_connect("row_selected", ETK_OBJECT(tree), ETK_CALLBACK(_etk_test_tree_row_selected), NULL);
   etk_signal_connect("row_unselected", ETK_OBJECT(tree), ETK_CALLBACK(_etk_test_tree_row_unselected), NULL);

   /* The list: */
   vbox = etk_vbox_new(FALSE, 0);
   etk_paned_add2(ETK_PANED(hpaned), vbox);

   label = etk_label_new(_("<h1>List:</h1>"));
   etk_box_pack_start(ETK_BOX(vbox), label, FALSE, TRUE, 0);

   tree = etk_tree_new();
   etk_widget_size_request_set(tree, 320, 400);
   etk_box_pack_start(ETK_BOX(vbox), tree, TRUE, TRUE, 0);

   etk_tree_multiple_select_set(ETK_TREE(tree), TRUE);
   col1 = etk_tree_col_new(ETK_TREE(tree), _("Column 1"), ETK_TREE_COL_ICON_TEXT, 100);
   col2 = etk_tree_col_new(ETK_TREE(tree), _("Column 2"), ETK_TREE_COL_INT, 100);
   col3 = etk_tree_col_new(ETK_TREE(tree), _("Column 3"), ETK_TREE_COL_IMAGE, 100);
   etk_tree_build(ETK_TREE(tree));

   etk_tree_freeze(ETK_TREE(tree));
   for (i = 0; i < 300; i++)
   {
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row1"), col2, 1, col3, PACKAGE_DATA_DIR "/images/1star.png", NULL);
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row2"), col2, 2, col3, PACKAGE_DATA_DIR "/images/2stars.png", NULL);
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row3"), col2, 3, col3, PACKAGE_DATA_DIR "/images/3stars.png", NULL);
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row4"), col2, 1, col3, PACKAGE_DATA_DIR "/images/3stars.png", NULL);
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row5"), col2, 2, col3, PACKAGE_DATA_DIR "/images/2stars.png", NULL);
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row6"), col2, 3, col3, PACKAGE_DATA_DIR "/images/1star.png", NULL);
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row7"), col2, 1, col3, PACKAGE_DATA_DIR "/images/1star.png", NULL);
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row8"), col2, 2, col3, PACKAGE_DATA_DIR "/images/2stars.png", NULL);
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row9"), col2, 3, col3, PACKAGE_DATA_DIR "/images/3stars.png", NULL);
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row10"), col2, 1, col3, PACKAGE_DATA_DIR "/images/3stars.png", NULL);
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row11"), col2, 2, col3, PACKAGE_DATA_DIR "/images/2stars.png", NULL);
      etk_tree_append(ETK_TREE(tree), col1, PACKAGE_DATA_DIR "/images/1star.png", _("Row12"), col2, 3, col3, PACKAGE_DATA_DIR "/images/1star.png", NULL);
   }
   etk_tree_thaw(ETK_TREE(tree));

   etk_signal_connect("row_selected", ETK_OBJECT(tree), ETK_CALLBACK(_etk_test_tree_row_selected), NULL);
   etk_signal_connect("row_unselected", ETK_OBJECT(tree), ETK_CALLBACK(_etk_test_tree_row_unselected), NULL);
   
   etk_widget_show_all(win);
}
