/* Test for Focus Chain Linear*/
#include <Elementary.h>

static void
_on_key_down(void *data, Evas *e, Evas_Object *obj, void *einfo)
{
   //Evas_Event_Key_Down *event = einfo;
   //printf("%s %p Key %s Parent %p\n", evas_object_type_get(obj),
   //       obj, event->keyname, evas_object_smart_parent_get(obj));
}

static inline void
my_show(Evas_Object *obj)
{
   evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_DOWN,
                                  _on_key_down, NULL);
   evas_object_show(obj);
}

void
test_focus(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win;
   unsigned int i, j;

   win = elm_win_add(NULL, "focus", ELM_WIN_BASIC);
   elm_win_title_set(win, "Focus");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   evas_object_resize(win, 800, 600);
   elm_win_autodel_set(win, EINA_TRUE);
   my_show(win);

     {
        Evas_Object *bg;
        bg = elm_bg_add(win);
        evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        elm_win_resize_object_add(win, bg);
        my_show(bg);
     }

   Evas_Object *mainbx = elm_box_add(win);
   elm_box_horizontal_set(mainbx, EINA_TRUE);
   elm_win_resize_object_add(win, mainbx);
   evas_object_size_hint_weight_set(mainbx, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   my_show(mainbx);

     { //First Col
        Evas_Object *bx = elm_box_add(win);
        evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        elm_box_pack_end(mainbx, bx);
        my_show(bx);

          {
             Evas_Object *lb = elm_label_add(win);
             elm_label_label_set(lb,
                                 "<b>Use Tab and Shift+Tab</b>"
                                );
             evas_object_size_hint_weight_set(lb, 0.0, 0.0);
             evas_object_size_hint_align_set(lb, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             elm_box_pack_end(bx, lb);
             my_show(lb);
          }

          {
             Evas_Object *tg = elm_toggle_add(win);
             elm_toggle_states_labels_set(tg, "Yes", "No");
             elm_box_pack_end(bx, tg);
             my_show(tg);
          }

          {
             Evas_Object *en = elm_scrolled_entry_add(win);
             evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
             evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
             elm_scrolled_entry_entry_set(en, "This is a single line");
             elm_scrolled_entry_single_line_set(en, 1);
             elm_box_pack_end(bx, en);
             my_show(en);
          }

          {
             Evas_Object *bx2 = elm_box_add(win);
             elm_box_horizontal_set(bx2, EINA_TRUE);
             evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(bx2, 0.0, 0.0);
             elm_box_pack_end(bx, bx2);

             for (i = 2; i; i--)
               {
                  Evas_Object *bt;
                  bt = elm_button_add(win);
                  elm_button_label_set(bt, "Box");
                  evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                  if (i%2)
                    elm_object_disabled_set(bt, EINA_TRUE);
                  elm_box_pack_end(bx2, bt);
                  my_show(bt);
               }

               {
                  Evas_Object *sc = elm_scroller_add(win);
                  evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND,
                                                   EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set(sc, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  elm_scroller_bounce_set(sc, 1, 1);
                  elm_scroller_content_min_limit(sc, 1, 1);
                  elm_box_pack_end(bx2, sc);
                  my_show(sc);

                    {
                       Evas_Object *bt;
                       bt = elm_button_add(win);
                       elm_button_label_set(bt, "Scroller");
                       evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                       EVAS_HINT_FILL);
                       evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                       elm_scroller_content_set(sc, bt);
                       my_show(bt);
                    }
               }

             my_show(bx2);
          }

          {
             Evas_Object *bt;
             bt = elm_button_add(win);
             elm_button_label_set(bt, "Box");
             evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(bt, 0.0, 0.0);
             elm_box_pack_end(bx, bt);
             my_show(bt);
          }

          {
             Evas_Object *bx2 = elm_box_add(win);
             elm_box_horizontal_set(bx2, EINA_TRUE);
             evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(bx2, 0.0, 0.0);
             elm_box_pack_end(bx, bx2);
             my_show(bx2);

             for (i = 2; i; i--)
               {
                  Evas_Object *bx3 = elm_box_add(win);
                  evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bx3, 0.0, 0.0);
                  elm_box_pack_end(bx2, bx3);
                  my_show(bx3);

                  for (j = 3; j; j--)
                    {
                       Evas_Object *bt;
                       bt = elm_button_add(win);
                       elm_button_label_set(bt, "Box");
                       evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                       EVAS_HINT_FILL);
                       evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                       elm_box_pack_end(bx3, bt);
                       my_show(bt);
                    }
               }

               {
                  Evas_Object *sc = elm_scroller_add(win);
                  evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND,
                                                   EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set(sc, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  elm_scroller_bounce_set(sc, 0, 1);
                  elm_scroller_content_min_limit(sc, 1, 0);
                  elm_box_pack_end(bx2, sc);
                  my_show(sc);

                  Evas_Object *bx3 = elm_box_add(win);
                  evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bx3, 0.0, 0.0);
                  elm_scroller_content_set(sc, bx3);
                  my_show(bx3);

                  for (i = 5; i; i--)
                    {
                       Evas_Object *bt;
                       bt = elm_button_add(win);
                       elm_button_label_set(bt, "BX Scroller");
                       evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                       EVAS_HINT_FILL);
                       evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                       elm_box_pack_end(bx3, bt);
                       my_show(bt);
                    }
               }
          }
     }

     {//Second Col
        char buf[PATH_MAX];
        Evas_Object *ly = elm_layout_add(win);
        snprintf(buf, sizeof(buf), "%s/objects/test.edj", PACKAGE_DATA_DIR);
        elm_layout_file_set(ly, buf, "twolines");
        evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        elm_box_pack_end(mainbx, ly);
        my_show(ly);

          {
             Evas_Object *bx2 = elm_box_add(win);
             elm_box_horizontal_set(bx2, EINA_TRUE);
             evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(bx2, 0.0, 0.0);
             elm_layout_content_set(ly, "element1", bx2);
             my_show(bx2);

             for (i = 3; i; i--)
               {
                  Evas_Object *bt;
                  bt = elm_button_add(win);
                  elm_button_label_set(bt, "Layout");
                  evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                  elm_box_pack_end(bx2, bt);
                  my_show(bt);
               }
          }

          {
             Evas_Object *bx2 = elm_box_add(win);
             evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(bx2, 0.0, 0.0);
             elm_layout_content_set(ly, "element2", bx2);
             my_show(bx2);

             for (i = 3; i; i--)
               {
                  Evas_Object *bt;
                  bt = elm_button_add(win);
                  elm_button_label_set(bt, "Box inside Layout");
                  evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                  elm_box_pack_end(bx2, bt);
                  my_show(bt);
               }
          }
     }

     {//Third Col
        Evas_Object *bx = elm_box_add(win);
        evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        elm_box_pack_end(mainbx, bx);
        my_show(bx);

          {
             Evas_Object *fr = elm_frame_add(win);
             elm_frame_label_set(fr, "Frame");
             elm_box_pack_end(bx, fr);
             evas_object_show(fr);

               {
                  Evas_Object *tb = elm_table_add(win);
                  evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                  elm_frame_content_set(fr, tb);
                  my_show(tb);
                  for (j = 0; j < 3; j++)
                    for (i = 0; i < 3; i++)
                      {
                         Evas_Object *bt;
                         bt = elm_button_add(win);
                         elm_button_label_set(bt, "Table");
                         evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                         EVAS_HINT_FILL);
                         evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                         elm_table_pack(tb, bt, i, j, 1, 1);
                         my_show(bt);
                      }
               }
          }
     }

     { //Panel
        Evas_Object *panel = elm_panel_add(win);
        elm_panel_orient_set(panel, ELM_PANEL_ORIENT_LEFT);
        evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_win_resize_object_add(win, panel);
        my_show(panel);
        elm_panel_hidden_set(panel, EINA_TRUE);

          {
             Evas_Object *bx2 = elm_box_add(win);
             evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND,
                                              EVAS_HINT_EXPAND);
             elm_panel_content_set(panel, bx2);
             my_show(bx2);

             for (i = 3; i; i--)
               {
                  Evas_Object *bt;
                  bt = elm_button_add(win);
                  elm_button_label_set(bt, "Panel");
                  evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND,
                                                   EVAS_HINT_EXPAND);
                  elm_box_pack_end(bx2, bt);
                  my_show(bt);
               }
          }
     }
}

