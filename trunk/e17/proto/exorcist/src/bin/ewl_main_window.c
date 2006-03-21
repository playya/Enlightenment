#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Ewl.h>

#include <Epdf.h>
#include <ewl_pdf.h>


#define __UNUSED__ __attribute__((unused))


typedef struct Exo_Ewl_ Exo_Ewl;
struct Exo_Ewl_
{
  char       *path;
  char       *filename;
  Ewl_Widget *pdf;
  Ewl_Widget *list_pages;
  Ewl_Widget *list_index;
  Ewl_Widget *win_index;

  Ewl_Widget *popup;

  Ewl_Widget *search_text;
  Ewl_Widget *search_is_case_sensitive;
  Ewl_Widget *search_circular;
};


static void _exo_ewl_change_page_cb    (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);
static void _exo_ewl_document_open_cb  (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);
static void _exo_ewl_filedialog_cb     (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);
static void _exo_ewl_row_data_free_cb  (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);
static void _exo_ewl_index_show_cb     (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);
static void _exo_ewl_search_cb         (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);
static void _exo_ewl_exit_cb           (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);
static void _exo_ewl_search_destroy_cb (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);
static void _exo_ewl_search_next_cb    (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);
static void _exo_ewl_popup_cb          (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);
static void _exo_ewl_info_cb           (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);
static void _exo_ewl_info_destroy_cb   (Ewl_Widget *widget,
                                        void       *ev_data,
                                        void       *user_data);

void
_exo_ewl_tree_fill (Exo_Ewl *data, Ewl_Row *row, Ecore_List *items)
{
  Ewl_Widget              *prow;
  Evas_Poppler_Index_Item *item;

  if (!items)
    return;

  ecore_list_goto_first (items);
  while ((item = ecore_list_next (items)))
    {
      int         page;
      int        *num;
      char       *buf;
      Ecore_List *c;

      buf = strdup (evas_poppler_index_item_title_get (item));
      prow = ewl_tree_text_row_add (EWL_TREE (data->list_index), row,
                                    &buf);
      page = evas_poppler_index_item_page_get (ewl_pdf_pdf_document_get (EWL_PDF (data->pdf)), item);
      if (page >= 0)
        {
          num = (int *)malloc (sizeof (int));
          *num = page;
          ewl_widget_data_set (prow, "row-number", num);
          ewl_callback_append (EWL_WIDGET (prow),
                               EWL_CALLBACK_CLICKED,
                               EWL_CALLBACK_FUNCTION (_exo_ewl_change_page_cb),
                               data);
          ewl_callback_prepend (EWL_WIDGET (prow),
                                EWL_CALLBACK_DESTROY,
                                EWL_CALLBACK_FUNCTION (_exo_ewl_row_data_free_cb),
                                NULL);
        }
      free (buf);
      c = evas_poppler_index_item_children_get (item);
      if (c)
        {
          _exo_ewl_tree_fill (data, EWL_ROW (prow), c);
        }
    }
}

static void
_exo_ewl_update_document (Exo_Ewl *data)
{
  Evas_Poppler_Document   *document;
  Ecore_List              *index;
  int                      page_count;
  int                      i;

  if (!data || !data->filename) return;

  ewl_container_reset (EWL_CONTAINER (data->list_pages));
  ewl_container_reset (EWL_CONTAINER (data->list_index));

  printf ("page : %d %s\n", ewl_pdf_page_get (EWL_PDF (data->pdf)), data->filename);
  ewl_pdf_file_set (EWL_PDF (data->pdf), data->filename);
  document = ewl_pdf_pdf_document_get (EWL_PDF (data->pdf));
  if (!document) return;

  page_count = evas_poppler_document_page_count_get (document);
  for (i = 0; i < page_count; i++) {
    char        row_text[64];
    char       *txt;
    Ewl_Widget *row;
    int        *num;

    txt = row_text;
    snprintf (row_text, 64, "%d", i + 1);
    row = ewl_tree_text_row_add (EWL_TREE (data->list_pages), NULL, &txt);
    num = (int *)malloc (sizeof (int));
    *num = i;
    ewl_widget_data_set (row, "row-number", num);
    ewl_callback_append (EWL_WIDGET (row),
                         EWL_CALLBACK_CLICKED,
                         EWL_CALLBACK_FUNCTION (_exo_ewl_change_page_cb),
                         data);
    ewl_callback_prepend (EWL_WIDGET (row),
                          EWL_CALLBACK_DESTROY,
                          EWL_CALLBACK_FUNCTION (_exo_ewl_row_data_free_cb),
                          NULL);
  }

  index = ewl_pdf_pdf_index_get (EWL_PDF (data->pdf));
  _exo_ewl_tree_fill (data, NULL, index);
}

static Ewl_Widget *
_exo_ewl_menu_options (Exo_Ewl *data)
{
  Ewl_Widget *menu;
  Ewl_Widget *item;
  Ewl_Widget *check;

  menu = ewl_menu_new ();

  item = ewl_menu_item_new ();
  ewl_button_label_set (EWL_BUTTON (item), "Zoom in");
  ewl_container_child_append (EWL_CONTAINER (menu), item);
  ewl_widget_show (item);

  item = ewl_menu_item_new ();
  ewl_button_label_set (EWL_BUTTON (item), "Zoom out");
  ewl_container_child_append (EWL_CONTAINER (menu), item);
  ewl_widget_show (item);

  item = ewl_menu_item_new ();
  ewl_button_label_set (EWL_BUTTON (item), "Search");
  ewl_callback_append (item,
                       EWL_CALLBACK_CLICKED,
                       _exo_ewl_search_cb, data);
  ewl_container_child_append (EWL_CONTAINER (menu), item);
  ewl_widget_show (item);

  item = ewl_menu_item_new ();
  ewl_container_child_append (EWL_CONTAINER (menu), item);
  ewl_widget_show (item);

  check = ewl_checkbutton_new ();
  ewl_button_label_set (EWL_BUTTON (check), "Index");
  ewl_checkbutton_checked_set (EWL_CHECKBUTTON (check), FALSE);
  ewl_callback_append (check,
                       EWL_CALLBACK_CLICKED,
                       _exo_ewl_index_show_cb, data);
  ewl_container_child_append (EWL_CONTAINER (item), check);
  ewl_widget_show (check);

  return menu;
}

static Ewl_Widget *
_exo_ewl_menu_bar (Exo_Ewl *data)
{
  Ewl_Widget *menu_bar;
  Ewl_Widget *menu;
  Ewl_Widget *spacer;
  Ewl_Widget *item;

  menu_bar = ewl_hmenubar_new ();

  /* File menu */
  menu = ewl_menu_new ();
  ewl_button_label_set (EWL_BUTTON (menu), "File");
  ewl_object_fill_policy_set(EWL_OBJECT(menu), EWL_FLAG_FILL_HSHRINK);
  ewl_container_child_append (EWL_CONTAINER(menu_bar), menu);
  ewl_widget_show (menu);

  item = ewl_menu_item_new ();
  ewl_button_stock_type_set (EWL_BUTTON (item), EWL_STOCK_OPEN);
  ewl_container_child_append (EWL_CONTAINER (menu), item);
  ewl_callback_append (item,
                       EWL_CALLBACK_CLICKED,
                       _exo_ewl_document_open_cb, data);
  ewl_widget_show (item);

  item = ewl_menu_item_new ();
  ewl_button_label_set (EWL_BUTTON (item), "Info");
  ewl_container_child_append (EWL_CONTAINER (menu), item);
  ewl_callback_append (item,
                       EWL_CALLBACK_CLICKED,
                       _exo_ewl_info_cb, data);
  ewl_widget_show (item);

  item = ewl_hseparator_new ();
  ewl_container_child_append (EWL_CONTAINER (menu), item);
  ewl_widget_show (item);

  item = ewl_menu_item_new ();
  ewl_button_stock_type_set (EWL_BUTTON (item), EWL_STOCK_QUIT);
  ewl_container_child_append (EWL_CONTAINER (menu), item);
  ewl_callback_append (item,
                       EWL_CALLBACK_CLICKED,
                       _exo_ewl_exit_cb, data);
  ewl_widget_show (item);

  /* options menu */
  menu = _exo_ewl_menu_options (data);
  ewl_button_label_set (EWL_BUTTON (menu), "Options");
  ewl_object_fill_policy_set(EWL_OBJECT(menu), EWL_FLAG_FILL_HSHRINK);
  ewl_container_child_append (EWL_CONTAINER(menu_bar), menu);
  ewl_widget_show (menu);

  spacer = ewl_spacer_new ();
  ewl_object_fill_policy_set(EWL_OBJECT(spacer), EWL_FLAG_FILL_HFILL);
  ewl_container_child_append (EWL_CONTAINER (menu_bar), spacer);
  ewl_widget_show (spacer);

  /* help menu */
  menu = ewl_menu_new ();
  ewl_button_label_set (EWL_BUTTON (menu), "Help");
  ewl_object_fill_policy_set(EWL_OBJECT(menu), EWL_FLAG_FILL_HSHRINK);
  ewl_container_child_append (EWL_CONTAINER(menu_bar), menu);
  ewl_widget_show (menu);

  item = ewl_menu_item_new ();
  ewl_button_label_set (EWL_BUTTON (item), "About");
  ewl_container_child_append (EWL_CONTAINER (menu), item);
  ewl_widget_show (item);

  return menu_bar;
}

void
_exo_ewl_index_window (Exo_Ewl *data)
{
  char        title[4096];

  snprintf (title, 4096, "Exorcist - %s - Index", data->filename);

  data->win_index = ewl_window_new ();
  ewl_window_title_set (EWL_WINDOW (data->win_index), title);
  ewl_window_name_set (EWL_WINDOW (data->win_index), title);
  ewl_window_class_set (EWL_WINDOW (data->win_index), title);

  ewl_tree_headers_visible_set (EWL_TREE (data->list_index), FALSE);
  ewl_container_child_append (EWL_CONTAINER (data->win_index), data->list_index);
  ewl_widget_show (data->list_index);
}

void
exo_ewl_main_window (char *filename)
{
  Ewl_Widget *window;
  Ewl_Widget *vbox;
  Ewl_Widget *menu_bar;
  Ewl_Widget *hbox;
  Exo_Ewl    *data;

  data = (Exo_Ewl *)calloc (sizeof (Exo_Ewl), 1);
  if (!data) return;

  if (filename)
    data->filename = strdup (filename);
  data->pdf = ewl_pdf_new ();
  data->list_pages = ewl_tree_new (1);
  ewl_tree_expandable_rows_set (EWL_TREE (data->list_pages), 0);
  data->list_index = ewl_tree_new (1);

  window = ewl_window_new ();
  ewl_object_size_request(EWL_OBJECT(window), 200, 200);
  ewl_window_title_set (EWL_WINDOW (window), "Exorcist");
  ewl_window_name_set (EWL_WINDOW (window), "Exorcist");
  ewl_window_class_set (EWL_WINDOW (window), "Exorcist");
  ewl_callback_append (window,
                       EWL_CALLBACK_DELETE_WINDOW,
                       _exo_ewl_exit_cb, data);
  ewl_callback_append (window,
                       EWL_CALLBACK_CLICKED,
                       _exo_ewl_popup_cb, data);

  vbox = ewl_vbox_new ();
  ewl_container_child_append (EWL_CONTAINER (window), vbox);
  ewl_box_homogeneous_set (EWL_BOX (vbox), FALSE);
  ewl_widget_show (vbox);

  menu_bar = _exo_ewl_menu_bar (data);
  ewl_container_child_append (EWL_CONTAINER (vbox), menu_bar);
  ewl_widget_show (menu_bar);

  hbox = ewl_hbox_new ();
  ewl_box_homogeneous_set (EWL_BOX (hbox), FALSE);
  ewl_container_child_append (EWL_CONTAINER (vbox), hbox);
  ewl_widget_show (hbox);

  ewl_tree_headers_visible_set (EWL_TREE (data->list_pages), FALSE);
  ewl_container_child_append (EWL_CONTAINER (hbox), data->list_pages);
  ewl_widget_show (data->list_pages);

  ewl_container_child_append (EWL_CONTAINER (hbox), data->pdf);
  ewl_widget_show (data->pdf);

  data->popup = _exo_ewl_menu_options (data);
  ewl_container_child_append (EWL_CONTAINER (window), data->popup);
  ewl_widget_hide (data->popup);

  if (data->filename)
    _exo_ewl_update_document (data);

  ewl_widget_show (window);

  _exo_ewl_index_window (data);
}

static void
_exo_ewl_index_show_cb (Ewl_Widget *widget,
                        void       *ev_data __UNUSED__,
                        void       *user_data)
{
  Exo_Ewl *data;

  data = (Exo_Ewl *)user_data;

  if (ewl_checkbutton_is_checked (EWL_CHECKBUTTON (widget)))
    ewl_widget_show (data->win_index);
  else
    ewl_widget_hide (data->win_index);
}

static void
_exo_ewl_search_cb (Ewl_Widget *widget __UNUSED__,
                    void       *ev_data __UNUSED__,
                    void       *user_data)
{
  Exo_Ewl    *data;
  Ewl_Widget *dialog_search;
  Ewl_Widget *button;
  Ewl_Widget *table;
  Ewl_Widget *label;

  data = (Exo_Ewl *)user_data;
  dialog_search = ewl_dialog_new ();
  ewl_window_title_set (EWL_WINDOW (dialog_search), "Exorcist - Search");
  ewl_window_name_set (EWL_WINDOW (dialog_search), "Exorcist - Search");
  ewl_window_class_set (EWL_WINDOW (dialog_search), "Exorcist - Search");
  ewl_callback_append (dialog_search,
                       EWL_CALLBACK_DELETE_WINDOW,
                       _exo_ewl_search_destroy_cb, NULL);

  button = ewl_button_new ();
  ewl_button_stock_type_set (EWL_BUTTON (button), EWL_STOCK_ARROW_LEFT);
  ewl_container_child_append (EWL_CONTAINER (dialog_search), button);
  ewl_widget_show (button);

  button = ewl_button_new ();
  ewl_button_stock_type_set (EWL_BUTTON (button), EWL_STOCK_ARROW_RIGHT);
  ewl_callback_append (EWL_WIDGET (button),
                       EWL_CALLBACK_CLICKED,
                       EWL_CALLBACK_FUNCTION (_exo_ewl_search_next_cb),
                       data);
  ewl_container_child_append (EWL_CONTAINER (dialog_search), button);
  ewl_widget_show (button);

  ewl_dialog_active_area_set (EWL_DIALOG (dialog_search), EWL_POSITION_TOP);

  table = ewl_table_new (2, 3, NULL);
  ewl_container_child_append (EWL_CONTAINER (dialog_search), table);
  ewl_widget_show (table);

  label = ewl_label_new ();
  ewl_label_text_set (EWL_LABEL (label), "Word:");
  ewl_object_alignment_set (EWL_OBJECT (label), EWL_FLAG_ALIGN_LEFT | EWL_FLAG_ALIGN_CENTER);
  ewl_table_add (EWL_TABLE (table), label, 1, 1, 1, 1);
  ewl_widget_show (label);

  data->search_text = ewl_entry_new ();
  ewl_table_add (EWL_TABLE (table), data->search_text, 2, 2, 1, 1);
  ewl_widget_show (data->search_text);

  data->search_is_case_sensitive = ewl_checkbutton_new ();
  ewl_button_label_set (EWL_BUTTON (data->search_is_case_sensitive), "Case sensitive");
  ewl_table_add (EWL_TABLE (table), data->search_is_case_sensitive, 1, 2, 2, 2);
  ewl_widget_show (data->search_is_case_sensitive);

  data->search_circular = ewl_checkbutton_new ();
  ewl_button_label_set (EWL_BUTTON (data->search_circular), "Circular search");
  ewl_table_add (EWL_TABLE (table), data->search_circular, 1, 2, 3, 3);
  ewl_widget_show (data->search_circular);

  ewl_widget_show (dialog_search);

}

static void
_exo_ewl_exit_cb (Ewl_Widget *widget,
                  void       *ev_data __UNUSED__,
                  void       *user_data)
{
  Exo_Ewl *data;

  data = (Exo_Ewl *)user_data;
  ewl_widget_destroy (widget);
  ewl_main_quit ();
  if (data->filename)
    free (data->filename);
  free (data);
}

static void
_exo_ewl_change_page_cb (Ewl_Widget *widget,
                         void       *ev_data __UNUSED__,
                         void       *user_data)
{
  Ewl_Pdf *pdf;
  int      row_number;

  pdf =  EWL_PDF (((Exo_Ewl *)user_data)->pdf);

  row_number = *(int *)ewl_widget_data_get (widget, "row-number");
  if (row_number != ewl_pdf_page_get (pdf))
    {
      ewl_pdf_page_set (pdf, row_number);
      ewl_callback_call (EWL_WIDGET (pdf), EWL_CALLBACK_REVEAL);
    }
}

static void
_exo_ewl_document_open_cb (Ewl_Widget *widget __UNUSED__,
                           void       *ev_data __UNUSED__,
                           void       *user_data)
{
  static Ewl_Widget *filedialog = NULL;
  Exo_Ewl           *data;

  if (filedialog) {
    /* raise the dialog */
    return;
  }
  data = (Exo_Ewl *)user_data;
  filedialog = ewl_filedialog_new ();
  ewl_filedialog_type_set (EWL_FILEDIALOG (filedialog),
                           EWL_FILEDIALOG_TYPE_OPEN);
  if (data->path)
    ewl_filedialog_path_set (EWL_FILEDIALOG (filedialog), data->path);
  ewl_callback_append (filedialog,
                       EWL_CALLBACK_VALUE_CHANGED,
                       EWL_CALLBACK_FUNCTION (_exo_ewl_filedialog_cb),
                       data);
  ewl_widget_show (filedialog);
}

static void
_exo_ewl_filedialog_cb (Ewl_Widget *widget,
                        void       *ev_data,
                        void       *user_data)
{
  Ewl_Filedialog *filedialog;
  Exo_Ewl        *data;
  unsigned int    resp;

  filedialog = EWL_FILEDIALOG (widget);
  data = (Exo_Ewl *)user_data;
  resp = *(unsigned int *)ev_data;
  switch (resp) {
  case EWL_STOCK_OPEN:
    if (data->path)
      free (data->path);
    if (data->filename)
      free (data->filename);
    data->path = ewl_filedialog_path_get (filedialog);
    data->filename = ewl_filedialog_file_get (filedialog);
    _exo_ewl_update_document (data);
    ewl_widget_destroy (widget);
    break;
  case EWL_STOCK_CANCEL:
  default:
    ewl_widget_destroy (widget);
    break;
  }
}

static void
_exo_ewl_row_data_free_cb (Ewl_Widget *widget,
                           void       *ev_data __UNUSED__,
                           void       *user_data __UNUSED__)
{
  int *row_number;

  if (!widget)
    return;
  row_number = (int *)ewl_widget_data_get (EWL_WIDGET (widget), "row-number");
  if (row_number) free (row_number);
}

static void
_exo_ewl_search_destroy_cb (Ewl_Widget *widget,
                            void       *ev_data __UNUSED__,
                            void       *user_data __UNUSED__)
{
  ewl_widget_destroy (widget);
}

static void
_exo_ewl_search_next_cb (Ewl_Widget *widget __UNUSED__,
                         void       *ev_data __UNUSED__,
                         void       *user_data)
{
  Exo_Ewl *data;
  Ewl_Pdf *pdf;
  char    *text;
  int      is_case_sensitive;
  int      is_circular;
  int      res;

  data = (Exo_Ewl *)user_data;
  pdf =  EWL_PDF (((Exo_Ewl *)user_data)->pdf);

  text = ewl_text_text_get (EWL_TEXT (data->search_text));
  if (!text || (text[0] == '\0'))
    return;
  is_case_sensitive = ewl_checkbutton_is_checked (EWL_CHECKBUTTON (data->search_is_case_sensitive));
  is_circular = ewl_checkbutton_is_checked (EWL_CHECKBUTTON (data->search_circular));
  ewl_pdf_search_text_set (pdf, text);
  ewl_pdf_search_is_case_sensitive (pdf, is_case_sensitive);
  printf ("we search...\n");
  res = ewl_pdf_search_next (pdf);

  if (!res)
    printf ("FIN\n");
}

static void
_exo_ewl_popup_cb (Ewl_Widget *widget,
                   void       *ev_data,
                   void       *user_data)
{
  Exo_Ewl            *data;
  Ewl_Event_Mouse_Up *ev;
  Ewl_Embed          *emb;

  data = (Exo_Ewl *)user_data;
  ev = (Ewl_Event_Mouse_Up *)ev_data;
  emb = ewl_embed_widget_find (widget);

  printf ("popup, %d\n", ev->button);

  if ((ev->x >= CURRENT_X (data->pdf)) &&
      (ev->y >= CURRENT_Y (data->pdf)) &&
      (ev->x <= (CURRENT_X (data->pdf) + CURRENT_W (data->pdf))) &&
      (ev->y <= (CURRENT_Y (data->pdf) + CURRENT_H (data->pdf)))) {
    printf ("dans pdf\n");
    if (ev->button == 3) {
      ewl_object_position_request (EWL_OBJECT (data->popup), ev->x, ev->y);
/*       if (!HIDDEN (data->popup)) */
/*         return; */

      ewl_widget_show (data->popup);
    }
    else
      ewl_widget_hide (data->popup);
  }
  else
    ewl_widget_hide (data->popup);
}

static void
_exo_ewl_info_cb (Ewl_Widget *widget __UNUSED__,
                  void       *ev_data __UNUSED__,
                  void       *user_data)
{
  Exo_Ewl                *data;
  Ewl_Widget             *dialog_info;
  Ewl_Widget             *button;
  Ewl_Widget             *table;
  Ewl_Widget             *notebook;
  Ewl_Widget             *label;
  Ewl_Widget             *entry;
  Ewl_Widget             *list;
  Ecore_List             *fonts;
  Evas_Poppler_Document  *doc;
  Evas_Poppler_Font_Info *font;

  data = (Exo_Ewl *)user_data;
  doc = ewl_pdf_pdf_document_get (EWL_PDF (data->pdf));

  if (!doc) return;

  dialog_info = ewl_dialog_new ();
  ewl_window_title_set (EWL_WINDOW (dialog_info), "Exorcist - Info");
  ewl_window_name_set (EWL_WINDOW (dialog_info), "Exorcist - Info");
  ewl_window_class_set (EWL_WINDOW (dialog_info), "Exorcist - Info");
  ewl_callback_append (dialog_info,
                       EWL_CALLBACK_DELETE_WINDOW,
                       _exo_ewl_info_destroy_cb, dialog_info);

  button = ewl_button_new ();
  ewl_button_stock_type_set (EWL_BUTTON (button), EWL_STOCK_CANCEL);
  ewl_callback_append (button,
                       EWL_CALLBACK_CLICKED,
                       _exo_ewl_info_destroy_cb, dialog_info);
  ewl_container_child_append (EWL_CONTAINER (dialog_info), button);
  ewl_widget_show (button);

  ewl_dialog_active_area_set (EWL_DIALOG (dialog_info), EWL_POSITION_TOP);

  table = ewl_table_new (2, 2, NULL);
  ewl_container_child_append (EWL_CONTAINER (dialog_info), table);
  ewl_widget_show (table);

  label = ewl_label_new ();
  ewl_label_text_set (EWL_LABEL (label), "Poppler version:");
  ewl_object_alignment_set (EWL_OBJECT (label), EWL_FLAG_ALIGN_LEFT | EWL_FLAG_ALIGN_CENTER);
  ewl_table_add (EWL_TABLE (table), label, 1, 1, 1, 1);
  ewl_widget_show (label);

  entry = ewl_entry_new ();
  ewl_text_text_set (EWL_TEXT (entry), evas_poppler_version_get ());
  ewl_table_add (EWL_TABLE (table), entry, 2, 2, 1, 1);
  ewl_widget_show (entry);

  notebook = ewl_notebook_new ();
  ewl_table_add (EWL_TABLE (table), notebook, 1, 2, 2, 2);
  ewl_widget_show (notebook);

  {
    char  buf[16];
    char *row_text[4];
    char *text;

    list = ewl_tree_new (2);
/*     ewl_tree_headers_visible_set (EWL_TREE (list), FALSE); */
    ewl_widget_show (list);

    row_text[0] = "File name";
    row_text[1] = evas_poppler_document_filename_get (doc);
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);

    row_text[0] = "Title";
    text = evas_poppler_document_title_get (doc);
    if (text)
      row_text[1] = text;
    else
      row_text[1] = "unknown";
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);
    if (text)
      free (text);

    row_text[0] = "Format";
    snprintf (buf, 16, "PDF-%.1f", evas_poppler_document_pdf_version_get (doc));
    row_text[1] = buf;
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);

    row_text[0] = "Author";
    text = evas_poppler_document_author_get (doc);
    if (text)
      row_text[1] = text;
    else
      row_text[1] = "unknown";
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);
    if (text)
      free (text);

    row_text[0] = "Subject";
    text = evas_poppler_document_subject_get (doc);
    if (text)
      row_text[1] = text;
    else
      row_text[1] = "unknown";
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);
    if (text)
      free (text);

    row_text[0] = "Keywords";
    text = evas_poppler_document_keywords_get (doc);
    if (text)
      row_text[1] = text;
    else
      row_text[1] = "unknown";
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);
    if (text)
      free (text);

    row_text[0] = "Creator";
    text = evas_poppler_document_creator_get (doc);
    if (text)
      row_text[1] = text;
    else
      row_text[1] = "unknown";
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);
    if (text)
      free (text);

    row_text[0] = "Producer";
    text = evas_poppler_document_producer_get (doc);
    if (text)
      row_text[1] = text;
    else
      row_text[1] = "unknown";
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);
    if (text)
      free (text);

    row_text[0] = "Linearized";
    if (evas_poppler_document_is_linearized (doc))
      row_text[1] = "yes";
    else
      row_text[1] = "no";
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);

    row_text[0] = "Page mode";
    row_text[1] = (char *)evas_poppler_document_page_mode_string_get (doc);
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);

    row_text[0] = "Page layout";
    row_text[1] = (char *)evas_poppler_document_page_layout_string_get (doc);
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);

    row_text[0] = "Creation date";
    text = evas_poppler_document_creation_date_get (doc);
    if (text)
      row_text[1] = text;
    else
      row_text[1] = "unknown";
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);
    if (text)
      free (text);

    row_text[0] = "Modification date";
    text = evas_poppler_document_mod_date_get (doc);
    if (text)
      row_text[1] = text;
    else
      row_text[1] = "unknown";
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);
    if (text)
      free (text);

    row_text[0] = "Page count";
    snprintf (buf, 16, "%d", evas_poppler_document_page_count_get (doc));
    row_text[1] = buf;
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);

    ewl_notebook_page_tab_text_set (EWL_NOTEBOOK (notebook), list, "Document");
    ewl_container_child_append (EWL_CONTAINER (notebook), list);
  }

  {
    char *header[4];

    header[0] = "Name";
    header[1] = "Type";
    header[2] = "Embedded";
    header[3] = "Subset";
    list = ewl_tree_new (4);
    ewl_tree_headers_set (EWL_TREE (list), header);
    ewl_widget_show (list);

    fonts = evas_poppler_document_fonts_get (doc);
    ecore_list_goto_first (fonts);
    while ((font = ecore_list_next (fonts))) {
      char *row_text[4];

      row_text[0] = (char *)evas_poppler_font_info_font_name_get (font);
      row_text[1] = (char *)evas_poppler_font_info_type_name_get (font);
      row_text[2] = evas_poppler_font_info_is_embedded_get (font) ? "yes" : "no";
      row_text[3] = evas_poppler_font_info_is_subset_get (font) ? "yes" : "no";
      ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);
    }
    ecore_list_destroy (fonts);

    ewl_notebook_page_tab_text_set (EWL_NOTEBOOK (notebook), list, "Fonts");
    ewl_container_child_append (EWL_CONTAINER (notebook), list);
  }

  {
    char  buf[16];
    char *row_text[2];
    Evas_Poppler_Page *page;
    Evas_Poppler_Page_Orientation o;

    page = ewl_pdf_pdf_page_get (EWL_PDF (data->pdf));

    list = ewl_tree_new (2);
    ewl_widget_show (list);

    row_text[0] = "Page number";
    snprintf (buf, 16, "%d", ewl_pdf_page_get (EWL_PDF (data->pdf)));
    row_text[1] = buf;
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);

    row_text[0] = "Size (pixels)";
    snprintf (buf, 16, "%d x %d", evas_poppler_page_width_get (page), evas_poppler_page_height_get (page));
    row_text[1] = buf;
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);

    row_text[0] = "Orientation";
    o = evas_poppler_page_orientation_get (page);
    switch (o) {
    case EVAS_POPPLER_PAGE_ORIENTATION_LANDSCAPE:
      row_text[1] = "landscape";
      break;
    case EVAS_POPPLER_PAGE_ORIENTATION_UPSIDEDOWN:
      row_text[1] = "upside down";
      break;
    case EVAS_POPPLER_PAGE_ORIENTATION_SEASCAPE:
      row_text[1] = "seascape";
      break;
    case EVAS_POPPLER_PAGE_ORIENTATION_PORTRAIT:
      row_text[1] = "portrait";
      break;
    }
    ewl_tree_text_row_add (EWL_TREE (list), NULL, row_text);

    ewl_notebook_page_tab_text_set (EWL_NOTEBOOK (notebook), list, "Page properties");
    ewl_container_child_append (EWL_CONTAINER (notebook), list);
  }

  ewl_widget_show (dialog_info);

}

static void
_exo_ewl_info_destroy_cb (Ewl_Widget *widget __UNUSED__,
                          void       *ev_data __UNUSED__,
                          void       *user_data)
{
  ewl_widget_destroy (EWL_WIDGET (user_data));
}
