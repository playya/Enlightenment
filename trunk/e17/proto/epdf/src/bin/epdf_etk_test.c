#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Etk.h>

#include "etk_pdf.h"


static void _quit_cb(void *data);
static void _tree2_fill (Etk_Pdf *pdf, Etk_Tree2 *tree, Etk_Tree2_Col *col, Etk_Tree2_Row *row, Ecore_List *items);
static void _change_page_cb (Etk_Object *object, Etk_Tree2_Row *row, Etk_Event_Mouse_Up *event, void *data);

int
main (int argc, char *argv[])
{
  Etk_Widget     *window;
  Etk_Widget     *table;
  Etk_Widget     *tree;
  Etk_Widget     *list;
  Etk_Widget     *pdf;
  Etk_Tree2_Col  *col;
  Etk_Tree2_Row  *row;
  Ecore_List     *index;
  Epdf_Document  *document;
  int             page_count;
  int             i;

  etk_init (NULL,NULL);

  if (argc == 1) {
    printf ("Usage: %s pdf_file\n", argv[0]);
    etk_main_quit ();
    return -1;
  }

  /* We open the pdf file */
  pdf = etk_pdf_new ();
  etk_pdf_file_set (ETK_PDF (pdf), argv[1]);
  document = ETK_PDF (pdf)->pdf_document;
  if (!document) {
    printf ("The file %s can't be opened\n", argv[1]);
    etk_main_quit ();
    return -1;
  }

  index = etk_pdf_pdf_index_get (ETK_PDF (pdf));

  window = etk_window_new ();
  etk_window_title_set (ETK_WINDOW (window), "Etk Pdf Test Application");
  etk_signal_connect ("delete_event", ETK_OBJECT (window),
                      ETK_CALLBACK(_quit_cb), NULL);

  table = etk_table_new (2, 2, ETK_FALSE);
  etk_container_add (ETK_CONTAINER (window), table);
  etk_widget_show (table);

  if (index) {
    Etk_Tree2_Col *col;

    tree = etk_tree2_new ();
    etk_tree2_mode_set (ETK_TREE2 (tree), ETK_TREE2_MODE_TREE);
    etk_tree2_multiple_select_set (ETK_TREE2 (tree), ETK_FALSE);

    /* column */
    col = etk_tree2_col_new (ETK_TREE2 (tree), "Index", 130, 0.0);
    etk_tree2_col_model_add (col, etk_tree2_model_text_new());

    etk_tree2_build (ETK_TREE2 (tree));

    /* rows */
    _tree2_fill (ETK_PDF (pdf), ETK_TREE2 (tree), col, NULL, index);
    epdf_index_delete (index);

    /* change page */
    etk_signal_connect ("row_clicked", ETK_OBJECT (tree),
                        ETK_CALLBACK(_change_page_cb), pdf);

    /* we attach and show */
    etk_table_attach_default (ETK_TABLE (table), tree, 0, 0, 0, 0);
    etk_widget_show (tree);
  }

  list = etk_tree2_new ();
  etk_tree2_headers_visible_set (ETK_TREE2 (list), FALSE);
  etk_tree2_mode_set (ETK_TREE2 (list), ETK_TREE2_MODE_LIST);
  etk_tree2_multiple_select_set (ETK_TREE2 (list), ETK_FALSE);

  /* column */
  col = etk_tree2_col_new (ETK_TREE2 (list), "", 60, 0.0);
  etk_tree2_col_model_add (col, etk_tree2_model_int_new());

  etk_tree2_build (ETK_TREE2 (list));

  /* rows */
  page_count = epdf_document_page_count_get (ETK_PDF (pdf)->pdf_document);
  for (i = 0; i < page_count; i++) {
    int  *num;

    row = etk_tree2_row_append (ETK_TREE2 (list), NULL, col, i + 1, NULL);
    num = (int *)malloc (sizeof (int));
    *num = i;
    etk_tree2_row_data_set_full (row, num, free);
  }

  /* change page */
  etk_signal_connect ("row_clicked", ETK_OBJECT (list),
                      ETK_CALLBACK(_change_page_cb), pdf);

  /* we attach and show */
  if  (index)
    etk_table_attach_default (ETK_TABLE (table), list, 0, 0, 1, 1);
  else
    etk_table_attach_default (ETK_TABLE (table), list, 0, 0, 0, 1);
  etk_widget_show (list);

  etk_table_attach (ETK_TABLE (table), pdf,
                    1, 1, 0, 1,
                    0, 0, ETK_TABLE_NONE);
  etk_widget_show (pdf);
  
  etk_widget_show (window);

  etk_main ();

  etk_shutdown ();

  return 0;
}

static void
_quit_cb(void *data)
{
  etk_main_quit ();
}

static void
_tree2_fill (Etk_Pdf *pdf, Etk_Tree2 *tree, Etk_Tree2_Col *col, Etk_Tree2_Row *row, Ecore_List *items)
{
  Etk_Tree2_Row   *prow;
  Epdf_Index_Item *item;

  if (!items)
    return;
  
  ecore_list_goto_first (items);
  while ((item = ecore_list_next (items))) {
    char       *buf;
    Ecore_List *c;
    int        *num;

    buf = strdup (epdf_index_item_title_get (item));
    prow = etk_tree2_row_append (tree, row, col, buf, NULL);
      
    num = (int *)malloc (sizeof (int));
    *num = epdf_index_item_page_get (etk_pdf_pdf_document_get (pdf), item);
    etk_tree2_row_data_set_full (prow, num, free);
    free (buf);
    c = epdf_index_item_children_get (item);
    if (c) {
      _tree2_fill (pdf, tree, col, prow, c);
    }
  }
}

static void
_change_page_cb (Etk_Object *object, Etk_Tree2_Row *row, Etk_Event_Mouse_Up *event, void *data)
{
  Etk_Tree2 *tree;
  Etk_Pdf   *pdf;
  int        row_number;

  tree = ETK_TREE2 (object);
  pdf = ETK_PDF (data);
  row_number = *(int *)etk_tree2_row_data_get (row);
  etk_pdf_page_set (pdf, row_number);
}
