#include "common.h"

static void on_recent(void *data, Evas_Object *obj, void *event_info);

static Evas_Object *
_create_message(Evas_Object *win, Data_Message *msg)
{
   Data_Contact *ctc;
   Evas_Object *msgui;
   char *title = NULL;
   const char *when;
   char *icon;
   time_t t, tnow;
   struct tm *tm, tmnow;
   char tbuf[256], fbuf[512], tobuf[512];
   
   tnow = time(NULL);
   tm = localtime(&tnow);
   memcpy(&tmnow, tm, sizeof(struct tm));
   
   title = "???";
   if (msg->from_to) title = (char *)msg->from_to;
   ctc = data_contact_by_tel_find(msg->from_to);
   if (ctc)
     title = data_contact_name_get(ctc);
   else
     title = strdup(title);
   if (msg->flags & DATA_MESSAGE_SENT)
     {
        snprintf(tobuf, sizeof(tobuf), "To: %s", title);
        if (title) free(title);
        title = strdup(tobuf);
     }
   when = "???";
   t = msg->timestamp;
   tm = localtime(&t);
   if (t > tnow)
     {
        snprintf(tbuf, sizeof(tbuf), "Future");
        when = tbuf;
     }
   else if ((tnow - t) < (24 * 60 * 60))
     {
        if (tmnow.tm_yday != tm->tm_yday)
          {
             strftime(tbuf, sizeof(tbuf) - 1, "Yesterday %H:%M", tm);
             when = tbuf;
          }
        else
          {
             strftime(tbuf, sizeof(tbuf) - 1, "%H:%M:%S", tm);
             when = tbuf;
          }
     }
   else if ((tnow - t) < (6 * 24 * 60 * 60))
     {
        strftime(tbuf, sizeof(tbuf) - 1, "%a %H:%M", tm);
        when = tbuf;
     }
   else
     {
        strftime(tbuf, sizeof(tbuf) - 1, "%d/%m/%y %H:%M", tm);
        when = tbuf;
     }
   icon = NULL;
   if (ctc) icon = data_contact_photo_file_get(ctc);
   msgui = create_message
     (win, title, when, icon,
      (msg->flags & DATA_MESSAGE_SENT) ? 1 : 0,
      (ctc ? 1 : 0),
      msg->body, 
      msg);
   if (icon) free(icon);
   if (title) free(title);
   return msgui;
}

static Evas_Object *window = NULL, *box = NULL, *content = NULL;
static Evas_Object *inwin = NULL, *inwin2 = NULL;
static Evas_Object *number_entry = NULL;
static Evas_Object *sms_entry = NULL;
static Evas_Object *write_ph = NULL, *write_lb = NULL;
static char *number = NULL;

static void
on_win_del_req(void *data, Evas_Object *obj, void *event_info)
{
   elm_exit();
}

static void
on_select_ok(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_del(inwin);
   inwin = NULL;
}

static void
on_number_ok(void *data, Evas_Object *obj, void *event_info)
{
   const char *n = elm_entry_entry_get(number_entry);
   if (number) free(number);
   if (n) number = strdup(n);
   else number = NULL;
   elm_photo_file_set(write_ph, NULL);
   elm_label_label_set(write_lb, number);
   evas_object_del(inwin2);
   inwin2 = NULL;
}

static void
on_number_list_ok(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_del(inwin2);
   inwin2 = NULL;
}

static void
on_number_select(void *data, Evas_Object *obj, void *event_info)
{
   Data_Contact_Tel *tel = data;
   if (number) free(number);
   number = strdup(tel->number);
}

static void
on_contact_select(void *data, Evas_Object *obj, void *event_info)
{
   Data_Contact *ctc = data;
   if (!ctc)
     {
        Evas_Object *win, *bx, *bt, *sc, *en;

        win = window;
        inwin2 = elm_win_inwin_add(win);

        bx = elm_box_add(win);
        evas_object_size_hint_weight_set(bx, 1.0, 0.0);
        evas_object_size_hint_align_set(bx, -1.0, -1.0);
        
        sc = elm_scroller_add(win);
        elm_scroller_content_min_limit(sc, 0, 1);
        evas_object_size_hint_weight_set(sc, 1.0, 0.0);
        evas_object_size_hint_align_set(sc, -1.0, -1.0);
        elm_box_pack_end(bx, sc);
        
        en = elm_entry_add(win);
        elm_entry_single_line_set(en, 1);
        elm_entry_entry_set(en, "Enter number...");
        evas_object_size_hint_weight_set(en, 1.0, 0.0);
        evas_object_size_hint_align_set(en, -1.0, 0.0);
        elm_entry_select_all(en);
        elm_scroller_content_set(sc, en);
        evas_object_show(en);
        evas_object_show(sc);
        number_entry = en;
   
        bt = elm_button_add(window);
        elm_button_label_set(bt, "OK");
        evas_object_size_hint_weight_set(bt, 1.0, 0.0);
        evas_object_size_hint_align_set(bt, -1.0, -1.0);
        elm_box_pack_end(bx, bt);
        evas_object_smart_callback_add(bt, "clicked", on_number_ok, NULL);
        evas_object_show(bt);   
        
        elm_win_inwin_content_set(inwin2, bx);
        evas_object_show(bx);

        elm_win_inwin_activate(inwin2);
        // FIXME: this is not exported. bad!
        elm_widget_focus_set(en, 1);
     }
   else if (eina_list_count(ctc->tel.numbers) > 1)
     {
        Evas_Object *win, *bx, *bx2, *bt, *li, *fr, *ph, *lb;
        Eina_List *l;
        char *name, *file;
        
        win = window;
        inwin2 = elm_win_inwin_add(win);

        bx = elm_box_add(win);
        evas_object_size_hint_weight_set(bx, 1.0, 0.0);
        evas_object_size_hint_align_set(bx, -1.0, -1.0);

        fr = elm_frame_add(win);
        name = data_contact_name_get(ctc);
        if (name)
          {
             elm_label_label_set(write_lb, name);
             elm_frame_label_set(fr, name);
             free(name);
          }
        evas_object_size_hint_weight_set(fr, 1.0, 0.0);
        evas_object_size_hint_align_set(fr, -1.0, -1.0);
        elm_box_pack_end(bx, fr);        
        evas_object_show(fr);

        bx2 = elm_box_add(win);
        elm_box_horizontal_set(bx2, 1);
        elm_frame_content_set(fr, bx2);
        evas_object_show(bx2);
        
        ph = elm_photo_add(win);
        file = data_contact_photo_file_get(ctc);
        if (file)
          {
             elm_photo_file_set(write_ph, file);
             elm_photo_file_set(ph, file);
             free(file);
          }
        evas_object_size_hint_weight_set(ph, 0.0, 0.0);
        evas_object_size_hint_align_set(ph, -1.0, -1.0);
        elm_box_pack_end(bx2, ph);
        evas_object_show(ph);
        
        lb = elm_label_add(win);
        elm_label_label_set(lb, 
                            "This contact has multiple<br>"
                            "numbers. Select one.");
        evas_object_size_hint_weight_set(lb, 1.0, 0.0);
        evas_object_size_hint_align_set(lb, -1.0, -1.0);
        elm_box_pack_end(bx2, lb);
        evas_object_show(lb);
        
        li = elm_list_add(win);
        evas_object_size_hint_weight_set(li, 1.0, 1.0);
        evas_object_size_hint_align_set(li,  -1.0, -1.0);
        elm_box_pack_end(bx, li);

        for (l = (Eina_List *)(ctc->tel.numbers); l; l = l->next)
          {
             Data_Contact_Tel *tel = l->data;
             // FIXME:
             // tel->flags can be 0 or more of:
             //    DATA_CONTACT_TEL_HOME = (1 << 0),
             //    DATA_CONTACT_TEL_MSG = (1 << 1),
             //    DATA_CONTACT_TEL_WORK = (1 << 2),
             //    DATA_CONTACT_TEL_PREF = (1 << 3),
             //    DATA_CONTACT_TEL_VOICE = (1 << 4),
             //    DATA_CONTACT_TEL_FAX = (1 << 5),
             //    DATA_CONTACT_TEL_CELL = (1 << 6),
             //    DATA_CONTACT_TEL_VIDEO = (1 << 7),
             //    DATA_CONTACT_TEL_PAGER = (1 << 8),
             //    DATA_CONTACT_TEL_BBS = (1 << 9),
             //    DATA_CONTACT_TEL_MODEM = (1 << 10),
             //    DATA_CONTACT_TEL_CAR = (1 << 11),
             //    DATA_CONTACT_TEL_ISDN = (1 << 12),
             //    DATA_CONTACT_TEL_PCS = (1 << 13)
             elm_list_item_append(li, tel->number, NULL, NULL, on_number_select, tel);
          }
        elm_list_go(li);
        
        evas_object_show(li);
        
        bt = elm_button_add(window);
        elm_button_label_set(bt, "OK");
        evas_object_size_hint_weight_set(bt, 1.0, 0.0);
        evas_object_size_hint_align_set(bt, -1.0, -1.0);
        elm_box_pack_end(bx, bt);
        evas_object_smart_callback_add(bt, "clicked", on_number_list_ok, NULL);
        evas_object_show(bt);   
        
        elm_win_inwin_content_set(inwin2, bx);
        evas_object_show(bx);

        elm_win_inwin_activate(inwin2);
        
     }
   else
     {
        Data_Contact_Tel *tel;
        char *name, *file;
        
        tel = ctc->tel.numbers->data;
        if (number) free(number);
        number = strdup(tel->number);
        elm_label_label_set(write_lb, number);
        name = data_contact_name_get(ctc);
        if (name)
          {
             elm_label_label_set(write_lb, name);
             free(name);
          }
        file = data_contact_photo_file_get(ctc);
        if (file)
          {
             elm_photo_file_set(write_ph, file);
             free(file);
          }
     }
}

static void
on_to_select(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *li, *bx, *bt;
   Eina_List *l;
   
   win = window;
   inwin = elm_win_inwin_add(win);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 0.0);
   evas_object_size_hint_align_set(bx, -1.0, -1.0);
   
   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, 1.0, 1.0);
   evas_object_size_hint_align_set(li,  -1.0, -1.0);
   elm_box_pack_end(bx, li);

   for (l = (Eina_List *)data_contacts_all_list(); l; l = l->next)
     {
        Data_Contact *ctc = l->data;
        Evas_Object *ph;
        char buf[1024];
        char *file, *name;

        if (!ctc->tel.numbers) continue;
        name = data_contact_name_get(ctc);
        if (!name) continue;
        ph = elm_photo_add(win);
        elm_photo_size_set(ph, 20);
        file = data_contact_photo_file_get(ctc);
        if (file)
          {
             elm_photo_file_set(ph, file);
             free(file);
          }
        elm_list_item_append(li, name, ph, NULL, on_contact_select, ctc);
        evas_object_show(ph);
        free(name);
     }

   elm_list_item_append(li, "... Other", NULL, NULL, on_contact_select, NULL);
   
   elm_list_go(li);
   
   evas_object_show(li);
   
   bt = elm_button_add(window);
   elm_button_label_set(bt, "OK");
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   elm_box_pack_end(bx, bt);
   evas_object_smart_callback_add(bt, "clicked", on_select_ok, NULL);
   evas_object_show(bt);   
   
   elm_win_inwin_content_set(inwin, bx);
   evas_object_show(bx);
   
   elm_win_inwin_activate(inwin);
}

static void
on_send(void *data, Evas_Object *obj, void *event_info)
{
   const char *text;
   char *text2;
   int len;
   
   if (!number)
     {
        printf("NO number! tell user\n");
        return;
     }
   text = elm_entry_entry_get(sms_entry);
   if (!text)
     {
        printf("NO text - warn user\n");
     }
   text2 = elm_entry_markup_to_utf8(text);
   if (!text2)
     {
        printf("ERROR: converting to plain utf8 failed\n");
        return;
     }
   len = strlen(text2);
   if (len > 0)
     {
        if (text2[len - 1] == '\n') text2[len - 1] = 0;
     }
   printf("TO: <%s>\n", number);
   printf("TEXT...\n");
   printf("%s\n", text2);
   free(text2);
   on_recent(NULL, NULL, NULL);
}

static void
on_write(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *bx, *bx2, *ph, *lb, *bt, *sc, *en;
   
   if (number)
     {
        free(number);
        number = NULL;
     }
   /* FIXME: content -> editor + to who display + select "who" */
   if (content) evas_object_del(content);
   
   bx = elm_box_add(window);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_size_hint_align_set(bx, -1.0, -1.0);
   
   content = bx;
   
   bx2 = elm_box_add(window);
   elm_box_horizontal_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);
   
   ph = elm_photo_add(window);
   evas_object_smart_callback_add(ph, "clicked", on_to_select, NULL);
   evas_object_size_hint_weight_set(ph, 0.0, 1.0);
   evas_object_size_hint_align_set(ph, -1.0, -1.0);
   elm_box_pack_end(bx2, ph);
   evas_object_show(ph);
   write_ph = ph;
   
   lb = elm_label_add(window);
   elm_label_label_set(lb, "Select...");
   evas_object_size_hint_weight_set(lb, 1.0, 1.0);
   evas_object_size_hint_align_set(lb, -1.0, 0.5);
   elm_box_pack_end(bx2, lb);
   evas_object_show(lb);
   write_lb = lb;
   
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);
   
   sc = elm_scroller_add(window);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(bx, sc);
   
   en = elm_entry_add(window);
   elm_entry_entry_set(en, "");
   evas_object_size_hint_weight_set(en, 1.0, 1.0);
   evas_object_size_hint_align_set(en, -1.0, -1.0);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);
   
   evas_object_show(sc);
   sms_entry = en;
   
   bt = elm_button_add(window);
   elm_button_label_set(bt, "Send");
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   elm_box_pack_end(bx, bt);
   evas_object_smart_callback_add(bt, "clicked", on_send, NULL);
   evas_object_show(bt);   

   elm_box_pack_end(box, bx);
   evas_object_show(bx);
}

static int
_sort_msg_newest(const void *data1, const void *data2)
{
   const Data_Message *msg1 = data1;
   const Data_Message *msg2 = data2;
   if (msg1->timestamp > msg2->timestamp) return -1;
   else if (msg1->timestamp < msg2->timestamp) return 1;
   return 0;
}

static void
on_recent(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *sc, *bx2;
   Eina_List *l, *mlist = NULL;
   int from, num, i;
   
   if (content) evas_object_del(content);
   
   sc = elm_scroller_add(window);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(box, sc);
   evas_object_show(sc);
   
   content = sc;

   bx2 = elm_box_add(window);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);
   elm_scroller_content_set(sc, bx2);
   evas_object_show(bx2);

   tzset();

   for (l = (Eina_List *)data_message_all_list(); l; l = l->next)
     {
        // FIXME: use filter
        mlist = eina_list_append(mlist, l->data);
     }

   // sort newest first
   mlist = eina_list_sort(mlist, eina_list_count(mlist), _sort_msg_newest);
   // FIXME: from and num are inputs
   from = 0; // from message # 0
   num = 50; // 50 messages max;
   
   for (l = mlist, i = 0; l; l = l->next, i++)
     {
        Data_Message *msg = l->data;
        if ((msg->flags & DATA_MESSAGE_TRASH))
          continue;
        if (i >= from)
          {
             if (i == from)
               {
                  if (l->prev)
                    {
                       // FIXME: add a "newer" button
                    }
               }
             Evas_Object *msgui = _create_message(window, msg);
             elm_box_pack_end(bx2, msgui);
             evas_object_show(msgui);
          }
        if (i >= (from + num - 1))
          {
             if (l->next)
               {
                  // FIXME: add a "older" button
               }
             break;
          }
     }
   
   eina_list_free(mlist);
}

static void
on_chats(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *sc, *bx2;
   
   if (content) evas_object_del(content);
   
   sc = elm_scroller_add(window);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(box, sc);
   evas_object_show(sc);
   
   content = sc;

   bx2 = elm_box_add(window);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);
   elm_scroller_content_set(sc, bx2);
   evas_object_show(bx2);

   /* FIXME: delete content and add a scrolled list of everyone who has sent
    * you and sms or that you have sent one to - in order of most recent */
}

static void
on_trash(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *sc, *bx2;
   Eina_List *l, *mlist = NULL;
   int from, num, i;
   
   if (content) evas_object_del(content);
   
   sc = elm_scroller_add(window);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(box, sc);
   evas_object_show(sc);
   
   content = sc;

   bx2 = elm_box_add(window);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);
   elm_scroller_content_set(sc, bx2);
   evas_object_show(bx2);

   tzset();

   for (l = (Eina_List *)data_message_all_list(); l; l = l->next)
     {
        // FIXME: use filter
        mlist = eina_list_append(mlist, l->data);
     }

   // sort newest first
   mlist = eina_list_sort(mlist, eina_list_count(mlist), _sort_msg_newest);
   // FIXME: from and num are inputs
   from = 0; // from message # 0
   num = 50; // 50 messages max;
   
   for (l = mlist, i = 0; l; l = l->next, i++)
     {
        Data_Message *msg = l->data;
        if (!(msg->flags & DATA_MESSAGE_TRASH))
          continue;
        if (i >= from)
          {
             if (i == from)
               {
                  if (l->prev)
                    {
                       // FIXME: add a "newer" button
                    }
               }
             Evas_Object *msgui = _create_message(window, msg);
             elm_box_pack_end(bx2, msgui);
             evas_object_show(msgui);
          }
        if (i >= (from + num - 1))
          {
             if (l->next)
               {
                  // FIXME: add a "older" button
               }
             break;
          }
     }
   
   eina_list_free(mlist);
}

void
create_main_win(void)
{
   Evas_Object *win, *bg, *bx, *sc, *tb, *ic;
   Elm_Toolbar_Item *tbi;

   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   elm_win_title_set(win, "Messages");
   evas_object_smart_callback_add(win, "delete-request", on_win_del_req, NULL);
   
   window = win;
   
   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);
   
   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_size_hint_align_set(bx, -1.0, -1.0);
   elm_win_resize_object_add(win, bx);
   
   box = bx;

   tb = elm_toolbar_add(win);
   evas_object_size_hint_weight_set(tb, 1.0, 0.0);
   evas_object_size_hint_align_set(tb, -1.0, -1.0);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_toolbar_item_add(tb, ic, "Write", on_write, NULL);
   evas_object_show(ic);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "clock");
   tbi = elm_toolbar_item_add(tb, ic, "Recent", on_recent, NULL);
   evas_object_show(ic);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "chat");
   elm_toolbar_item_add(tb, ic, "Chats", on_chats, NULL);
   evas_object_show(ic);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_toolbar_item_add(tb, ic, "Trash", on_trash, NULL);
   evas_object_show(ic);
   
   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   elm_toolbar_item_select(tbi);
   
   evas_object_show(bx);
   
   evas_object_resize(win, 240, 280);
   
   evas_object_show(win);
}
