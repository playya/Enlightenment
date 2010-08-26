#include "main.h"
#include "evas_object/slideshow_object.h"
#include "evas_object/photo_object.h"

static Elm_Genlist_Item_Class itc_exifs;
static char *_gl_exifs_label_get(const void *data, Evas_Object *obj, const char *part);

static Elm_Genlist_Item_Class itc_iptcs;
static char *_gl_iptcs_label_get(const void *data, Evas_Object *obj, const char *part);
static void _slideshow_selected_cb(void *data, Evas_Object *obj, void *event_info);

static Slideshow_Item_Class itc_slideshow;
static Evas_Object *_slideshow_icon_get(const void *data, Evas_Object *obj);


static void _panel_image_photo_set(Panel_Image *panel_image, Enlil_Photo *photo);

static void _entry_name_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _entry_description_changed_cb(void *data, Evas_Object *obj, void *event_info);

static void _bt_undo_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_redo_cb(void *data, Evas_Object *obj, void *event_info);

static void _bt_1_1_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_fit_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_fill_cb(void *data, Evas_Object *obj, void *event_info);
static void _photocam_mouse_wheel_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _photocam_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _slider_photocam_zoom_cb(void *data, Evas_Object *obj, void *event_info);
static void _photocam_move_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _bt_rotate_180_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_rotate_90_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_rotate_R90_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_flip_vertical_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_flip_horizontal_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_blur_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_sharpen_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_grayscale_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_sepia_cb(void *data, Evas_Object *obj, void *event_info);
static void _end_trans_cb(void *data, Enlil_Trans_Job *job, const char *file);

static void _notify_trans_item_add(Panel_Image *panel_image, Evas_Object *item);
static void _notify_trans_item_first_del(Panel_Image *panel_image);
static void _bt_notify_trans_cancel_cb(void *data, Evas_Object *obj, void *event_info);

static void _update_undo_redo(Panel_Image *panel_image);
static void _menu_history_cb(void *data, Evas_Object *obj, void *event_info);

static void _bt_save_as_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_save_as_done_cb(void *data, Evas_Object *obj, void *event_info);
static void _bt_save_cb(void *data, Evas_Object *obj, void *event_info);
static void _inwin_save_as_apply_cb(void *data);
static void _bt_close_cb(void *data, Evas_Object *obj, void *event_info);

static void _close_without_save_cb(void *data);
static void _close_save_cb(void *data);

void _panel_select_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item);

static Eina_Bool _save_description_name_timer(void *data);
static void _save_description_name(Panel_Image *panel_image);

static void _panes_clicked_double(void *data, Evas_Object *obj, void *event_info);
static void _panes_h_clicked_double(void *data, Evas_Object *obj, void *event_info);



Panel_Image *panel_image_new(Evas_Object *obj, Enlil_Photo *photo)
{
    Evas_Object *ph, *vbox, *vbox2, *bx, *bx2, *tb, *bt, *rect, *sl, *pb, *icon, *gl,
	       *fr, *entry, *sc, *lbl, *tabs, *panels, *panes, *panes_h, *hbox;
   Elm_Toolbar_Item *tb_item;
   Tabpanel_Item *tp_item;
   Eina_List *l;
   Enlil_Photo *_photo;

   Panel_Image *panel_image = calloc(1, sizeof(Panel_Image));
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   photo_data->panel_image = panel_image;
   panel_image->photo = photo;

   panes = elm_panes_add(obj);
   panel_image->panes = panes;
   evas_object_size_hint_weight_set(panes, 1.0, 1.0);
   evas_object_size_hint_align_set(panes, -1.0, -1.0);
   elm_panes_content_left_size_set(panes, 0.20);
   evas_object_smart_callback_add(panes, "clicked,double", _panes_clicked_double, panel_image);
   evas_object_show(panes);

   // left panel
   panes_h = elm_panes_add(obj);
   panel_image->panes_h = panes_h;
   elm_panes_horizontal_set(panes_h, EINA_TRUE);
   evas_object_size_hint_weight_set(panes_h, 1.0, 1.0);
   evas_object_size_hint_align_set(panes_h, -1.0, -1.0);
   elm_panes_content_left_size_set(panes_h, 0.40);
   evas_object_smart_callback_add(panes_h, "clicked,double", _panes_h_clicked_double, panel_image);
   evas_object_show(panes_h);
   elm_panes_content_left_set(panes, panes_h);


   vbox = elm_box_add(obj);
   evas_object_size_hint_weight_set(vbox, 0.0, 1.0);
   evas_object_size_hint_align_set(vbox, -1.0, -1.0);
   evas_object_show(vbox);
   elm_panes_content_left_set(panes_h, vbox);

   bt = elm_button_add(obj);
   elm_button_label_set(bt, D_("Close the photo"));
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   evas_object_size_hint_align_set(bt, -1.0, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _bt_close_cb, panel_image);
   evas_object_show(bt);
   elm_box_pack_end(vbox, bt);

   bt = elm_button_add(obj);
   elm_button_label_set(bt, D_("Save"));
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   evas_object_size_hint_align_set(bt, -1.0, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _bt_save_cb, panel_image);
   evas_object_show(bt);
   elm_box_pack_end(vbox, bt);

   bt = elm_button_add(obj);
   elm_button_label_set(bt, D_("Save as"));
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   evas_object_size_hint_align_set(bt, -1.0, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _bt_save_as_cb, panel_image);
   evas_object_show(bt);
   elm_box_pack_end(vbox, bt);

   fr = elm_frame_add(obj);
   elm_frame_label_set(fr, D_("Description"));
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   evas_object_size_hint_align_set(fr, -1.0, -1.0);
   evas_object_show(fr);
   elm_box_pack_end(vbox, fr);

   vbox2 = elm_box_add(obj);
   evas_object_size_hint_weight_set(vbox2, 1.0, 0.5);
   evas_object_size_hint_align_set(vbox2, -1.0, 0.0);
   evas_object_show(vbox2);
   elm_frame_content_set(fr, vbox2);

   sc = elm_scroller_add(obj);
   elm_scroller_content_min_limit(sc, 0, 1);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_scroller_bounce_set(sc, 0, 0);
   evas_object_size_hint_weight_set(sc, 1.0, 0.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   evas_object_show(sc);
   elm_box_pack_end(vbox2, sc);

   entry = elm_entry_add(obj);
   panel_image->entry_name = entry;
   elm_entry_single_line_set(entry, 1);
   evas_object_smart_callback_add(entry, "changed", _entry_name_changed_cb, panel_image);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, 1.0);
   evas_object_size_hint_align_set(entry, -1.0, 0.5);
   elm_scroller_content_set(sc,entry);
   evas_object_show(entry);

   sc = elm_scroller_add(obj);
   elm_scroller_bounce_set(sc, 0, 0);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   evas_object_show(sc);
   elm_box_pack_end(vbox2, sc);

   entry = elm_entry_add(obj);
   panel_image->entry_description = entry;
   evas_object_smart_callback_add(entry, "changed", _entry_description_changed_cb, panel_image);
   elm_entry_single_line_set(entry, 0);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, 0.0);
   elm_scroller_content_set(sc,entry);
   evas_object_show(entry);

   tb = elm_table_add(obj);
   elm_table_padding_set(tb, 5, 5);
   evas_object_size_hint_weight_set(tb, 1.0, 0.0);
   evas_object_size_hint_align_set(tb, 0.0, 1.0);
   evas_object_show(tb);
   elm_box_pack_end(vbox2, tb);

   lbl = elm_label_add(obj);
   elm_label_label_set(lbl, D_("File size : "));
   evas_object_size_hint_align_set(lbl, 0.0, 0.5);
   evas_object_show(lbl);
   elm_table_pack(tb, lbl, 0, 1, 1, 1);

   lbl = elm_label_add(obj);
   panel_image->lbl_file_size = lbl;
   evas_object_show(lbl);
   elm_table_pack(tb, lbl, 1, 1, 1, 1);

   lbl = elm_label_add(obj);
   evas_object_size_hint_align_set(lbl, 0.0, 0.5);
   elm_label_label_set(lbl, D_("Picture size : "));
   evas_object_show(lbl);
   elm_table_pack(tb, lbl, 0, 2, 1, 1);

   lbl = elm_label_add(obj);
   panel_image->exifs.size = lbl;
   elm_label_label_set(lbl, D_("Unknown"));
   evas_object_show(lbl);
   elm_table_pack(tb, lbl, 1, 2, 1, 1);

   //
   panel_image->tabpanel = tabpanel_add(obj);

   vbox = elm_box_add(obj);
   evas_object_size_hint_weight_set(vbox, 0.0, 1.0);
   evas_object_size_hint_align_set(vbox, -1.0, -1.0);
   evas_object_show(vbox);
   elm_panes_content_right_set(panes_h, vbox);


   tabs = tabpanel_tabs_obj_get(panel_image->tabpanel);
   evas_object_size_hint_weight_set(tabs, 0.0, 0.0);
   evas_object_size_hint_align_set(tabs, -1.0, 0.0);
   evas_object_show(tabs);
   elm_box_pack_end(vbox, tabs);

   panels = tabpanel_panels_obj_get(panel_image->tabpanel);
   evas_object_size_hint_weight_set(panels, 1.0, 1.0);
   evas_object_size_hint_align_set(panels, -1.0, -1.0);
   evas_object_show(panels);
   elm_box_pack_end(vbox, panels);

   gl = elm_genlist_add(obj);
   panel_image->exifs.gl = gl;
   elm_genlist_horizontal_mode_set(gl, ELM_LIST_SCROLL);
   evas_object_size_hint_weight_set(gl, 1.0, 1.0);
   evas_object_size_hint_align_set(gl, -1.0, -1.0);
   evas_object_show(gl);
   tp_item = tabpanel_item_add(panel_image->tabpanel, D_("Exifs"), gl, NULL, NULL);

   itc_exifs.item_style     = "default_style";
   itc_exifs.func.label_get = _gl_exifs_label_get;
   itc_exifs.func.icon_get  = NULL;
   itc_exifs.func.state_get = NULL;
   itc_exifs.func.del       = NULL;

   gl = elm_genlist_add(obj);
   panel_image->iptcs.gl = gl;
   elm_genlist_horizontal_mode_set(gl, ELM_LIST_SCROLL);
   evas_object_size_hint_weight_set(gl, 1.0, 1.0);
   evas_object_size_hint_align_set(gl, -1.0, -1.0);
   evas_object_show(gl);
   tabpanel_item_add(panel_image->tabpanel, D_("IPTCs"), gl, NULL, NULL);

   itc_iptcs.item_style     = "default_style";
   itc_iptcs.func.label_get = _gl_iptcs_label_get;
   itc_iptcs.func.icon_get  = NULL;
   itc_iptcs.func.state_get = NULL;
   itc_iptcs.func.del       = NULL;

   tabpanel_item_select(tp_item);
   //




   //right panel
   ph = elm_photocam_add(obj);
   panel_image->photocam = ph;

   panel_image->menu = menu_photo_new(photo_data->enlil_data->win->win);

   bx2 = elm_box_add(obj);
   evas_object_size_hint_weight_set(bx2, 1.0, 1.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);
   evas_object_show(bx2);
   elm_panes_content_right_set(panes, bx2);


   panel_image->tb = elm_toolbar_add(obj);
   elm_toolbar_menu_parent_set(panel_image->tb, photo_data->enlil_data->win->win);
   elm_toolbar_homogenous_set(panel_image->tb, 0);
   elm_box_pack_end(bx2, panel_image->tb);
   evas_object_size_hint_weight_set(panel_image->tb, 1.0, 0.0);
   evas_object_size_hint_align_set(panel_image->tb, -1.0, 0.0);
   evas_object_show(panel_image->tb);

   elm_toolbar_item_add(panel_image->tb, NULL, D_("Save"), _bt_save_cb, panel_image);
   elm_toolbar_item_add(panel_image->tb, NULL, D_("Save as"), _bt_save_as_cb, panel_image);

   tb_item = elm_toolbar_item_add(panel_image->tb, NULL, NULL, NULL, NULL);
   elm_toolbar_item_separator_set(tb_item, 1);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/undo");
   tb_item = elm_toolbar_item_add(panel_image->tb, icon, D_("Undo"), NULL, NULL);
   panel_image->undo.undo = elm_toolbar_item_menu_get(tb_item);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/undo");
   panel_image->undo.item_undo = elm_menu_item_add(panel_image->undo.undo, NULL, icon, D_("Undo"), _bt_undo_cb, panel_image);
   elm_menu_item_disabled_set(panel_image->undo.item_undo, 1);
   elm_menu_item_separator_add(panel_image->undo.undo, NULL);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/redo");
   tb_item = elm_toolbar_item_add(panel_image->tb, icon, D_("Redo"), NULL, NULL);
   panel_image->undo.redo = elm_toolbar_item_menu_get(tb_item);

   icon = elm_icon_add(obj);
   evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_icon_file_set(icon, THEME, "icons/redo");
   panel_image->undo.item_redo = elm_menu_item_add(panel_image->undo.redo, NULL, icon, D_("Redo"), _bt_redo_cb, panel_image);
   elm_menu_item_disabled_set(panel_image->undo.item_redo, 1);
   elm_menu_item_separator_add(panel_image->undo.redo, NULL);

   tb_item = elm_toolbar_item_add(panel_image->tb, NULL, NULL, NULL, NULL);
   elm_toolbar_item_separator_set(tb_item, 1);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/1_1");
   elm_toolbar_item_add(panel_image->tb, icon, D_("1:1"), _bt_1_1_cb, panel_image);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/fit");
   elm_toolbar_item_add(panel_image->tb, icon, D_("Fit"), _bt_fit_cb, panel_image);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/fill");
   elm_toolbar_item_add(panel_image->tb, icon, D_("Fill"), _bt_fill_cb, panel_image);

   tb_item = elm_toolbar_item_add(panel_image->tb, NULL, NULL, NULL, NULL);
   elm_toolbar_item_separator_set(tb_item, 1);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/rotate/90");
   elm_toolbar_item_add(panel_image->tb, icon, D_("90°"), _bt_rotate_90_cb, panel_image);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/rotate/90/reverse");
   elm_toolbar_item_add(panel_image->tb, icon, D_("-90°"), _bt_rotate_R90_cb, panel_image);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/rotate/180");
   elm_toolbar_item_add(panel_image->tb, icon, D_("180°"), _bt_rotate_180_cb, panel_image);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/flip/horizontal");
   elm_toolbar_item_add(panel_image->tb, icon, D_("Horizontal"), _bt_flip_horizontal_cb, panel_image);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/flip/vertical");
   elm_toolbar_item_add(panel_image->tb, icon, D_("Vertical"), _bt_flip_vertical_cb, panel_image);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/blur");
   elm_toolbar_item_add(panel_image->tb, icon, D_("Blur"), _bt_blur_cb, panel_image);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/sharpen");
   elm_toolbar_item_add(panel_image->tb, icon, D_("Sharpen"), _bt_sharpen_cb, panel_image);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/sepia");
   elm_toolbar_item_add(panel_image->tb, icon, D_("Sepia"), _bt_sepia_cb, panel_image);

   icon = elm_icon_add(obj);
   elm_icon_file_set(icon, THEME, "icons/grayscale");
   elm_toolbar_item_add(panel_image->tb, icon, D_("Grayscale"), _bt_grayscale_cb, panel_image);



   evas_object_size_hint_weight_set(ph, 1.0, 1.0);
   evas_object_size_hint_align_set(ph, -1.0, -1.0);
   evas_object_show(ph);
   elm_photocam_zoom_mode_set(ph, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
   elm_box_pack_end(bx2, ph);

   //rectangle on top of the photocam which retrieves the mouse wheel
   rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   panel_image->rect = rect;
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_repeat_events_set(rect,1);
   evas_object_show(rect);
   evas_object_smart_member_add(rect, ph);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_WHEEL, _photocam_mouse_wheel_cb, panel_image);
   evas_object_raise(rect);

   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, _photocam_mouse_up_cb, panel_image);
   //

   evas_object_event_callback_add(ph, EVAS_CALLBACK_RESIZE, _photocam_move_resize_cb, panel_image);
   evas_object_event_callback_add(ph, EVAS_CALLBACK_MOVE, _photocam_move_resize_cb, panel_image);

   hbox = elm_box_add(obj);
   evas_object_size_hint_weight_set(hbox, 1.0, 0.0);
   evas_object_size_hint_align_set(hbox, -1.0, 1.0);
   evas_object_show(hbox);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_pack_end(bx2, hbox);

   //slideshow
   Evas_Object *slideshow = slideshow_object_add(obj);
   panel_image->slideshow.slideshow = slideshow;
   evas_object_size_hint_weight_set(slideshow, 0.0, 1.0);
   evas_object_size_hint_align_set(slideshow, 0.0, -1.0);
   evas_object_size_hint_min_set(slideshow, 100, 50);
   slideshow_object_file_set(slideshow, THEME, "slideshow");
   evas_object_smart_callback_add(slideshow, "selected", _slideshow_selected_cb, panel_image);
   evas_object_show(slideshow);
   elm_box_pack_end(hbox, slideshow);

   itc_slideshow.icon_get = _slideshow_icon_get;
   EINA_LIST_FOREACH(enlil_album_photos_get(enlil_photo_album_get(photo)), l, _photo)
   {
	   Enlil_Photo_Data *_photo_data = enlil_photo_user_data_get(_photo);
	   Slideshow_Item *_item = slideshow_object_item_append(slideshow, &itc_slideshow,  _photo);
	   _photo_data->slideshow_object_items = eina_list_append(_photo_data->slideshow_object_items, _item);
   }

   Slideshow_Item *_item = eina_list_data_get(eina_list_last(photo_data->slideshow_object_items));
   slideshow_object_item_select(slideshow, _item);

   //zoom
   sl = elm_slider_add(obj);
   panel_image->sl = sl;
   evas_object_size_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, -1.0, -1.0);
   elm_slider_label_set(sl, "Zoom");
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 1, 10);
   elm_slider_value_set(sl, 50);
   elm_slider_unit_format_set(sl, "%4.0f");
   evas_object_smart_callback_add(sl, "delay,changed", _slider_photocam_zoom_cb, panel_image);
   evas_object_show(sl);
   elm_box_pack_end(hbox, sl);
   elm_slider_value_set(photo_data->panel_image->sl, elm_photocam_zoom_get(photo_data->panel_image->photocam));



   //transformations notification window
   panel_image->notify_trans = elm_notify_add(ph);
   elm_notify_orient_set(panel_image->notify_trans, ELM_NOTIFY_ORIENT_BOTTOM_RIGHT);
   evas_object_size_hint_weight_set(panel_image->notify_trans, -1.0, -1.0);
   evas_object_size_hint_align_set(panel_image->notify_trans, -1.0, -1.0);

   bx = elm_box_add(obj);
   panel_image->notify_trans_bx = bx;
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_size_hint_align_set(bx, -1.0, 0.0);
   evas_object_show(bx);
   elm_notify_content_set(panel_image->notify_trans, bx);

   bx2 = elm_box_add(obj);
   elm_box_horizontal_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 1.0);
   evas_object_size_hint_align_set(bx2, -1.0, 0.0);
   evas_object_show(bx2);
   elm_box_pack_end(bx, bx2);

   pb = elm_progressbar_add(obj);
   elm_object_style_set(pb, "wheel");
   elm_progressbar_label_set(pb, "");
   elm_progressbar_pulse(pb, EINA_TRUE);
   evas_object_size_hint_weight_set(pb, 1.0, 0.0);
   evas_object_size_hint_align_set(pb, -1.0, 0.5);
   evas_object_show(pb);
   elm_box_pack_end(bx2, pb);

   bt = elm_button_add(obj);
   elm_button_label_set(bt, D_("Cancel"));
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   evas_object_size_hint_align_set(bt, -1.0, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _bt_notify_trans_cancel_cb, panel_image);
   evas_object_show(bt);
   elm_box_pack_end(bx2, bt);
   //

   panel_image->tabpanel_item = tabpanel_item_add(photo_data->enlil_data->tabpanel,
	 enlil_photo_name_get(photo), panes, _panel_select_cb, photo);

   _panel_image_photo_set(panel_image, photo);

   return panel_image;
}

static void _panel_image_photo_set(Panel_Image *panel_image, Enlil_Photo *photo)
{
	Enlil_Trans_Job *job;
	Elm_Menu_Item *mi_item;
	Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
	char buf[PATH_MAX];

	if(panel_image->photo)
	{
		Enlil_Photo_Data *_data = enlil_photo_user_data_get(panel_image->photo);
		_data->panel_image = NULL;
	}

	panel_image->photo = photo;
	photo_data->panel_image = panel_image;

	tabpanel_item_label_set(panel_image->tabpanel_item, enlil_photo_name_get(photo));

	elm_entry_entry_set(panel_image->entry_name, enlil_photo_name_get(photo));
	elm_entry_entry_set(panel_image->entry_description, enlil_photo_description_get(photo));

	snprintf(buf, sizeof(buf), "%f mo", enlil_photo_size_get(photo) / 1024. / 1024.);
	elm_label_label_set(panel_image->lbl_file_size, buf);

	elm_label_label_set(panel_image->exifs.size, D_("Unknown"));

	snprintf(buf, sizeof(buf),"%s/%s", enlil_photo_path_get(photo), enlil_photo_file_name_get(photo));
	elm_photocam_file_set(panel_image->photocam, buf);

	panel_image->save.save = EINA_FALSE;

	EINA_LIST_FREE( panel_image->jobs_trans, job)
		enlil_trans_job_del(job);

	EINA_LIST_FREE( panel_image->undo.items_undo, mi_item)
	      ;
	EINA_LIST_FREE( panel_image->undo.items_redo, mi_item)
	      ;

	enlil_trans_history_free(panel_image->history);
	panel_image->history = enlil_trans_history_new(buf);

	panel_image_exifs_update(photo);
	panel_image_iptcs_update(photo);
}

void panel_image_free(Panel_Image **_panel_image)
{
   Enlil_Trans_Job *job;
   Eina_List *l;
   Slideshow_Item *item;
   Elm_Menu_Item *mi_item;
   Panel_Image *panel_image = *_panel_image;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get( panel_image->photo );

   photo_data->panel_image = NULL;

   EINA_LIST_FREE( panel_image->jobs_trans, job)
      enlil_trans_job_del(job);

   EINA_LIST_FREE( panel_image->undo.items_undo, mi_item)
      ;
   EINA_LIST_FREE( panel_image->undo.items_redo, mi_item)
      ;

   enlil_trans_history_free(panel_image->history);
   if(panel_image->tabpanel_item)
     {
	evas_object_event_callback_del_full(panel_image->photocam, EVAS_CALLBACK_RESIZE, _photocam_move_resize_cb, panel_image);
	evas_object_event_callback_del_full(panel_image->photocam, EVAS_CALLBACK_MOVE, _photocam_move_resize_cb, panel_image);
	tabpanel_item_del(panel_image->tabpanel_item);
	panel_image->tabpanel_item = NULL;
	evas_object_del(panel_image->rect);
     }

   if(panel_image->timer_description_name)
     ecore_timer_del(panel_image->timer_description_name);

   EINA_LIST_FOREACH(slideshow_object_items_get(panel_image->slideshow.slideshow), l, item)
   {
	   Enlil_Photo *_photo;
	   Eina_List *l2;
	   EINA_LIST_FOREACH(enlil_album_photos_get(enlil_photo_album_get(panel_image->photo)), l2, _photo)
	   {
		   Enlil_Photo_Data *_photo_data = enlil_photo_user_data_get(_photo);
		   _photo_data->slideshow_object_items = eina_list_remove( _photo_data->slideshow_object_items, item);
	   }
   }

   FREE(panel_image);
}

void panel_image_exifs_update(Enlil_Photo *photo)
{
   char buf[PATH_MAX];
   const Eina_List *l;
   Enlil_Exif *exif;
   ASSERT_RETURN_VOID(photo != NULL);

   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   if(!photo_data->panel_image) return;

   snprintf(buf, PATH_MAX, "%d x %d", enlil_photo_size_w_get(photo) , enlil_photo_size_h_get(photo));
   elm_label_label_set(photo_data->panel_image->exifs.size, buf);

   elm_genlist_clear(photo_data->panel_image->exifs.gl);

   if(!enlil_photo_exif_loaded_get(photo))
     {
	photo_data->clear_exif_data = EINA_FALSE;
	enlil_exif_job_prepend(photo, exif_load_done, photo);
     }
   else
     {
	EINA_LIST_FOREACH(enlil_photo_exifs_get(photo), l, exif)
	   elm_genlist_item_append(photo_data->panel_image->exifs.gl, &itc_exifs,
		 exif, NULL, ELM_GENLIST_ITEM_NONE, NULL, exif);
     }
}

static char *_gl_exifs_label_get(const void *data, Evas_Object *obj, const char *part)
{
   Enlil_Exif *exif = (Enlil_Exif *)data;
   char buf[PATH_MAX];
   snprintf(buf, PATH_MAX, "<b>%s</b>  :  %s", enlil_exif_tag_get(exif), enlil_exif_value_get(exif));
   return strdup(buf);
}

void panel_image_iptcs_update(Enlil_Photo *photo)
{
   const Eina_List *l;
   Enlil_IPTC *iptc;
   ASSERT_RETURN_VOID(photo != NULL);

   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   if(!photo_data->panel_image) return;

   elm_genlist_clear(photo_data->panel_image->iptcs.gl);

   elm_entry_entry_set(photo_data->panel_image->entry_name, enlil_photo_name_get(photo));
   elm_entry_entry_set(photo_data->panel_image->entry_description, enlil_photo_description_get(photo));

   if(!enlil_photo_iptc_loaded_get(photo))
     {
	photo_data->clear_iptc_data = EINA_FALSE;
	enlil_iptc_job_prepend(photo, iptc_load_done, photo);
     }
   else
     {
	EINA_LIST_FOREACH(enlil_photo_iptcs_get(photo), l, iptc)
	   elm_genlist_item_append(photo_data->panel_image->iptcs.gl, &itc_iptcs,
		 iptc, NULL, ELM_GENLIST_ITEM_NONE, NULL, iptc);
     }
}

static char *_gl_iptcs_label_get(const void *data, Evas_Object *obj, const char *part)
{
   Enlil_IPTC *iptc = (Enlil_IPTC *)data;
   char buf[PATH_MAX];
   snprintf(buf, PATH_MAX, "<b>%s</b>  :  %s", enlil_iptc_title_get(iptc), enlil_iptc_value_get(iptc));
   return strdup(buf);
}

void panel_image_1_1(Enlil_Photo *photo)
{
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   elm_slider_value_set(photo_data->panel_image->sl, 1);
   elm_photocam_zoom_mode_set(photo_data->panel_image->photocam, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
   elm_photocam_zoom_set(photo_data->panel_image->photocam, 1);
}

void panel_image_fit(Enlil_Photo *photo)
{
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   elm_photocam_zoom_mode_set(photo_data->panel_image->photocam, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
   elm_slider_value_set(photo_data->panel_image->sl, elm_photocam_zoom_get(photo_data->panel_image->photocam));
}


void panel_image_fill(Enlil_Photo *photo)
{
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   elm_photocam_zoom_mode_set(photo_data->panel_image->photocam, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FILL);
   elm_slider_value_set(photo_data->panel_image->sl, elm_photocam_zoom_get(photo_data->panel_image->photocam));
}

void panel_image_rotation_90(Enlil_Photo *photo)
{
   Evas_Object *item;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Enlil_Trans_Job *job;

   item = elm_label_add(photo_data->panel_image->photocam);
   elm_label_label_set(item, D_("Rotation 90°"));
   evas_object_show(item);
   _notify_trans_item_add(photo_data->panel_image, item);

   job = enlil_trans_job_add(photo_data->panel_image->history,
	 elm_photocam_file_get(photo_data->panel_image->photocam),
	 Enlil_TRANS_ROTATE_90, _end_trans_cb, photo);
   photo_data->panel_image->jobs_trans = eina_list_append(photo_data->panel_image->jobs_trans, job);

   photo_data->panel_image->save.save = EINA_TRUE;
}

void panel_image_rotation_R90(Enlil_Photo *photo)
{
   Evas_Object *item;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Enlil_Trans_Job *job;

   item = elm_label_add(photo_data->panel_image->photocam);
   elm_label_label_set(item, D_("Rotation -90°"));
   evas_object_show(item);
   _notify_trans_item_add(photo_data->panel_image, item);


   job = enlil_trans_job_add(photo_data->panel_image->history,
	 elm_photocam_file_get(photo_data->panel_image->photocam),
	 Enlil_TRANS_ROTATE_R90, _end_trans_cb, photo);
   photo_data->panel_image->jobs_trans = eina_list_append(photo_data->panel_image->jobs_trans, job);

   photo_data->panel_image->save.save = EINA_TRUE;
}

void panel_image_rotation_180(Enlil_Photo *photo)
{
   Evas_Object *item;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Enlil_Trans_Job *job;

   item = elm_label_add(photo_data->panel_image->photocam);
   elm_label_label_set(item, D_("Rotation 180°"));
   evas_object_show(item);
   _notify_trans_item_add(photo_data->panel_image, item);

   job = enlil_trans_job_add(photo_data->panel_image->history,
	 elm_photocam_file_get(photo_data->panel_image->photocam),
	 Enlil_TRANS_ROTATE_180, _end_trans_cb, photo);
   photo_data->panel_image->jobs_trans = eina_list_append(photo_data->panel_image->jobs_trans, job);

   photo_data->panel_image->save.save = EINA_TRUE;
}

void panel_image_flip_vertical(Enlil_Photo *photo)
{
   Evas_Object *item;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Enlil_Trans_Job *job;

   item = elm_label_add(photo_data->panel_image->photocam);
   elm_label_label_set(item, D_("Flip Vertical"));
   evas_object_show(item);
   _notify_trans_item_add(photo_data->panel_image, item);

   job = enlil_trans_job_add(photo_data->panel_image->history,
	 elm_photocam_file_get(photo_data->panel_image->photocam),
	 Enlil_TRANS_FLIP_VERTICAL, _end_trans_cb, photo);
   photo_data->panel_image->jobs_trans = eina_list_append(photo_data->panel_image->jobs_trans, job);

   photo_data->panel_image->save.save = EINA_TRUE;
}

void panel_image_flip_horizontal(Enlil_Photo *photo)
{
   Evas_Object *item;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Enlil_Trans_Job *job;

   item = elm_label_add(photo_data->panel_image->photocam);
   elm_label_label_set(item, D_("Flip Horizontal"));
   evas_object_show(item);
   _notify_trans_item_add(photo_data->panel_image, item);

   job = enlil_trans_job_add(photo_data->panel_image->history,
	 elm_photocam_file_get(photo_data->panel_image->photocam),
	 Enlil_TRANS_FLIP_HORIZONTAL, _end_trans_cb, photo);
   photo_data->panel_image->jobs_trans = eina_list_append(photo_data->panel_image->jobs_trans, job);

   photo_data->panel_image->save.save = EINA_TRUE;
}

void panel_image_blur(Enlil_Photo *photo)
{
   Evas_Object *item;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Enlil_Trans_Job *job;

   item = elm_label_add(photo_data->panel_image->photocam);
   elm_label_label_set(item, D_("Blur"));
   evas_object_show(item);
   _notify_trans_item_add(photo_data->panel_image, item);

   job = enlil_trans_job_add(photo_data->panel_image->history,
	 elm_photocam_file_get(photo_data->panel_image->photocam),
	 Enlil_TRANS_BLUR, _end_trans_cb, photo);
   photo_data->panel_image->jobs_trans = eina_list_append(photo_data->panel_image->jobs_trans, job);

   photo_data->panel_image->save.save = EINA_TRUE;
}

void panel_image_sharpen(Enlil_Photo *photo)
{
   Evas_Object *item;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Enlil_Trans_Job *job;

   item = elm_label_add(photo_data->panel_image->photocam);
   elm_label_label_set(item, D_("Sharpen"));
   evas_object_show(item);
   _notify_trans_item_add(photo_data->panel_image, item);

   job = enlil_trans_job_add(photo_data->panel_image->history,
	 elm_photocam_file_get(photo_data->panel_image->photocam),
	 Enlil_TRANS_SHARPEN, _end_trans_cb, photo);
   photo_data->panel_image->jobs_trans = eina_list_append(photo_data->panel_image->jobs_trans, job);

   photo_data->panel_image->save.save = EINA_TRUE;
}

void panel_image_sepia(Enlil_Photo *photo)
{
   Evas_Object *item;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Enlil_Trans_Job *job;

   item = elm_label_add(photo_data->panel_image->photocam);
   elm_label_label_set(item, D_("Sepia"));
   evas_object_show(item);
   _notify_trans_item_add(photo_data->panel_image, item);

   job = enlil_trans_job_add(photo_data->panel_image->history,
	 elm_photocam_file_get(photo_data->panel_image->photocam),
	 Enlil_TRANS_SEPIA, _end_trans_cb, photo);
   photo_data->panel_image->jobs_trans = eina_list_append(photo_data->panel_image->jobs_trans, job);

   photo_data->panel_image->save.save = EINA_TRUE;
}

void panel_image_grayscale(Enlil_Photo *photo)
{
   Evas_Object *item;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Enlil_Trans_Job *job;

   item = elm_label_add(photo_data->panel_image->photocam);
   elm_label_label_set(item, D_("Grayscale"));
   evas_object_show(item);
   _notify_trans_item_add(photo_data->panel_image, item);

   job = enlil_trans_job_add(photo_data->panel_image->history,
	 elm_photocam_file_get(photo_data->panel_image->photocam),
	 Enlil_TRANS_GRAYSCALE, _end_trans_cb, photo);
   photo_data->panel_image->jobs_trans = eina_list_append(photo_data->panel_image->jobs_trans, job);

   photo_data->panel_image->save.save = EINA_TRUE;
}

void panel_image_save_as(Enlil_Photo *photo)
{
   Evas_Object *inwin, *fs, *vbox;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Panel_Image *panel_image = photo_data->panel_image;

   //create inwin & file selector
   inwin = elm_win_inwin_add(photo_data->enlil_data->win->win);
   panel_image->inwin = inwin;
   evas_object_show(inwin);

   vbox = elm_box_add(photo_data->enlil_data->win->win);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(vbox);
   elm_win_inwin_content_set(inwin, vbox);

   fs = elm_fileselector_add(photo_data->enlil_data->win->win);
   elm_fileselector_is_save_set(fs, EINA_TRUE);
   elm_fileselector_expandable_set(fs, EINA_FALSE);
   elm_fileselector_path_set(fs,  enlil_photo_path_get(photo));
   evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbox, fs);
   evas_object_show(fs);

   evas_object_smart_callback_add(fs, "done", _bt_save_as_done_cb, panel_image);

   panel_image->save.save = EINA_FALSE;
}

void panel_image_save(Enlil_Photo *photo)
{
   char buf[PATH_MAX];
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Panel_Image *panel_image = photo_data->panel_image;

   const Enlil_Trans_History_Item *item = enlil_trans_history_current_get(panel_image->history);

   snprintf(buf, PATH_MAX, "%s/%s", enlil_photo_path_get(photo), enlil_photo_file_name_get(photo));

   enlil_photo_copy_exif_in_file(photo, enlil_trans_history_item_file_get(item));
   enlil_photo_save_iptc_in_custom_file(photo, enlil_trans_history_item_file_get(item));
   ecore_file_cp(enlil_trans_history_item_file_get(item), buf);

   panel_image->save.save = EINA_FALSE;
}

void _panel_select_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item)
{
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(data);
   current_photo = data;
   _update_undo_redo(photo_data->panel_image);

   elm_map_bubbles_close(enlil_data->map->map);
}

static void _bt_undo_cb(void *data, Evas_Object *obj, void *event_info)
{
   panel_image_undo(data);
}

static void _bt_redo_cb(void *data, Evas_Object *obj, void *event_info)
{
   panel_image_redo(data);
}

static void _bt_1_1_cb(void *data, Evas_Object *obj, void *event_info)
{
   panel_image_1_1(data);
}

static void _bt_fit_cb(void *data, Evas_Object *obj, void *event_info)
{
   panel_image_fit(data);
}

static void _bt_fill_cb(void *data, Evas_Object *obj, void *event_info)
{
   panel_image_fill(data);
}

static void _slider_photocam_zoom_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = (Panel_Image*) data;
   int zoom = 1;
   int i;
   int val = elm_slider_value_get(panel_image->sl);

   for(i=0; i<val - 1; i++)
     zoom *= 2;

   elm_photocam_zoom_mode_set(panel_image->photocam, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
   elm_photocam_zoom_set(panel_image->photocam, zoom);
}

static void _photocam_mouse_wheel_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev = (Evas_Event_Mouse_Wheel*) event_info;
   Panel_Image *panel_image = data;
   int zoom;
   double val;
   //unset the mouse wheel
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   zoom = elm_photocam_zoom_get(panel_image->photocam);
   if (ev->z>0 && zoom == 1) return;

   if (ev->z > 0)
     zoom /= 2;
   else
     zoom *= 2;

   val = 1;
   int _zoom = zoom;
   while(_zoom>1)
     {
	_zoom /= 2;
	val++;
     }

   if(val>10) return;

   elm_photocam_zoom_mode_set(panel_image->photocam, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
   if (zoom >= 1) elm_photocam_zoom_set(panel_image->photocam, zoom);

   elm_slider_value_set(panel_image->sl, val);
}

static void _photocam_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up*) event_info;
   Panel_Image *panel_image = data;

   if(ev->button != 3) return ;
   //unset the mouse up
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   elm_menu_move(panel_image->menu, ev->output.x, ev->output.y);
   evas_object_show(panel_image->menu);
}

static void _photocam_move_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = (Panel_Image*) data;
   int x,y,w,h;

   evas_object_geometry_get(panel_image->photocam,&x,&y,&w,&h);
   evas_object_resize(panel_image->rect,w,h);
   evas_object_move(panel_image->rect,x,y);

   evas_object_resize(panel_image->notify_trans,w,h);
   evas_object_move(panel_image->notify_trans,x,y);
}

static void _bt_rotate_180_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   panel_image_rotation_180(panel_image->photo);
}

static void _bt_rotate_90_cb(void *data, Evas_Object *obj, void *event_info)
{
Panel_Image *panel_image = data;
   panel_image_rotation_90(panel_image->photo);
}

static void _bt_rotate_R90_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   panel_image_rotation_R90(panel_image->photo);
}

static void _bt_flip_vertical_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   panel_image_flip_vertical(panel_image->photo);
}

static void _bt_flip_horizontal_cb(void *data, Evas_Object *obj, void *event_info)
{
Panel_Image *panel_image = data;
   panel_image_flip_horizontal(panel_image->photo);
}

static void _bt_blur_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   panel_image_blur(panel_image->photo);
}

static void _bt_sharpen_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   panel_image_sharpen(panel_image->photo);
}

static void _bt_grayscale_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   panel_image_grayscale(panel_image->photo);
}

static void _bt_sepia_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   panel_image_sepia(panel_image->photo);
}

static void _end_trans_cb(void *data, Enlil_Trans_Job *job, const char *file)
{
   Enlil_Photo *photo = data;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);

   elm_photocam_file_set(photo_data->panel_image->photocam, file);
   _notify_trans_item_first_del(photo_data->panel_image);
   photo_data->panel_image->jobs_trans = eina_list_remove(photo_data->panel_image->jobs_trans, job);

   _update_undo_redo(photo_data->panel_image);
}

static void _notify_trans_item_add(Panel_Image *panel_image, Evas_Object *item)
{
   if(!panel_image->notify_trans_items)
     evas_object_show(panel_image->notify_trans);

   panel_image->notify_trans_items = eina_list_append(panel_image->notify_trans_items, item);

   elm_box_pack_start(panel_image->notify_trans_bx, item);
}

static void _notify_trans_item_first_del(Panel_Image *panel_image)
{
   Evas_Object *item = eina_list_data_get(panel_image->notify_trans_items);
   panel_image->notify_trans_items = eina_list_remove(panel_image->notify_trans_items, item);

   elm_box_unpack(panel_image->notify_trans_bx, item);
   evas_object_del(item);

   if(!panel_image->notify_trans_items)
     evas_object_hide(panel_image->notify_trans);
}

static void _bt_notify_trans_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   Enlil_Trans_Job *job;

   if(!panel_image->jobs_trans)
     return ;

   job = eina_list_data_get(panel_image->jobs_trans);
   panel_image->jobs_trans = eina_list_remove(panel_image->jobs_trans, job);
   enlil_trans_job_del(job);

   _notify_trans_item_first_del(panel_image);
}

static void _update_undo_redo(Panel_Image *panel_image)
{
   const char *type = NULL;
   const Enlil_Trans_History_Item *current, *item;
   Elm_Menu_Item *mi_item;
   const Eina_List *h, *l;
   Evas_Object *icon;

   EINA_LIST_FREE(panel_image->undo.items_undo, mi_item)
      elm_menu_item_del(mi_item);

   EINA_LIST_FREE(panel_image->undo.items_redo, mi_item)
      elm_menu_item_del(mi_item);

   elm_menu_item_disabled_set(panel_image->undo.item_undo, 1);
   elm_menu_item_disabled_set(panel_image->undo.item_redo, 1);

   h = enlil_trans_history_get(panel_image->history);
   current = enlil_trans_history_current_get(panel_image->history);

   //jump the first item because this is the original file
   l = h;
   if(eina_list_data_get(h) == current) goto second_step;

   h = eina_list_next(h);

   EINA_LIST_FOREACH(h, l, item)
     {
	switch(enlil_trans_history_item_type_get(item))
	  {
	   case Enlil_TRANS_ROTATE_180:
	      type = D_("Rotate 180°");
	      break;
	   case Enlil_TRANS_ROTATE_90:
	      type = D_("Rotate 90°");
	      break;
	   case Enlil_TRANS_ROTATE_R90:
	      type = D_("Rotate -90°");
	      break;
	   case Enlil_TRANS_FLIP_VERTICAL:
	      type = D_("Flip Vertical");
	      break;
	   case Enlil_TRANS_FLIP_HORIZONTAL:
	      type = D_("Flip Horizontal");
	      break;
	   case Enlil_TRANS_BLUR:
	      type = D_("Blur");
	      break;
	   case Enlil_TRANS_SHARPEN:
	      type = D_("Sharpen");
	      break;
	   case Enlil_TRANS_GRAYSCALE:
	      type = D_("Grayscale");
	      break;
	   case Enlil_TRANS_SEPIA:
	      type = D_("Sepia");
	      break;
	  }
	icon = elm_icon_add(panel_image->undo.undo);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_icon_file_set(icon, THEME, "icons/undo");

	mi_item = elm_menu_item_add(panel_image->undo.undo, NULL, icon, type, _menu_history_cb, panel_image);
	panel_image->undo.items_undo = eina_list_append(panel_image->undo.items_undo, mi_item);

	elm_menu_item_disabled_set(panel_image->undo.item_undo, 0);
	if(item == current)
	  break;
     }

second_step:
   l = eina_list_next(l);
   EINA_LIST_FOREACH(l, l, item)
     {
	switch(enlil_trans_history_item_type_get(item))
	  {
	   case Enlil_TRANS_ROTATE_180:
	      type = D_("Rotate 180°");
	      break;
	   case Enlil_TRANS_ROTATE_90:
	      type = D_("Rotate 90°");
	      break;
	   case Enlil_TRANS_ROTATE_R90:
	      type = D_("Rotate -90°");
	      break;
	   case Enlil_TRANS_FLIP_VERTICAL:
	      type = D_("Flip Vertical");
	      break;
	   case Enlil_TRANS_FLIP_HORIZONTAL:
	      type = D_("Flip Horizontal");
	      break;
	   case Enlil_TRANS_BLUR:
	      type = D_("Blur");
	      break;
	   case Enlil_TRANS_SHARPEN:
	      type = D_("Sharpen");
	      break;
	   case Enlil_TRANS_GRAYSCALE:
	      type = D_("Grayscale");
	      break;
	   case Enlil_TRANS_SEPIA:
	      type = D_("Sepia");
	      break;
	  }

	icon = elm_icon_add(panel_image->undo.redo);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_icon_file_set(icon, THEME, "icons/redo");

	mi_item = elm_menu_item_add(panel_image->undo.redo, NULL, icon, type, _menu_history_cb, panel_image);
	panel_image->undo.items_redo = eina_list_append(panel_image->undo.items_redo, mi_item);

	elm_menu_item_disabled_set(panel_image->undo.item_redo, 0);
     }
}

void panel_image_undo(Enlil_Photo *photo)
{
   const char *file;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Panel_Image *panel_image = photo_data->panel_image;

   int count = eina_list_count(panel_image->undo.items_undo);
   if(count>=0)
     {
	file = enlil_trans_history_goto(panel_image->history, count-1);
	elm_photocam_file_set(panel_image->photocam, file);

	_update_undo_redo(panel_image);
     }

   panel_image->save.save = EINA_TRUE;
}

void panel_image_redo(Enlil_Photo *photo)
{
   const char *file;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   Panel_Image *panel_image = photo_data->panel_image;

   int count = eina_list_count(panel_image->undo.items_undo);
   if(panel_image->undo.items_redo)
     {
	file = enlil_trans_history_goto(panel_image->history, count+1);
	elm_photocam_file_set(panel_image->photocam, file);

	_update_undo_redo(panel_image);
     }

   panel_image->save.save = EINA_TRUE;
}

static void _menu_history_cb(void *data, Evas_Object *obj, void *event_info)
{
   const char *file;
   const Eina_List *l;
   Panel_Image *panel_image = data;
   const Elm_Menu_Item *item = event_info, *_item;
   int pos = 0;

   if(obj == panel_image->undo.undo)
     {
	EINA_LIST_FOREACH(panel_image->undo.items_undo, l, _item)
	  {
	     if(item == _item)
	       break;
	     else
	       pos++;
	  }
     }
   else
     {
	pos = eina_list_count(panel_image->undo.items_undo) + 1;
	EINA_LIST_FOREACH(panel_image->undo.items_redo, l, _item)
	  {
	     if(item == _item)
	       break;
	     else
	       pos++;
	  }
     }

   file = enlil_trans_history_goto(panel_image->history, pos);
   elm_photocam_file_set(panel_image->photocam, file);

   _update_undo_redo(panel_image);

   panel_image->save.save = EINA_TRUE;
}

static void _bt_save_as_done_cb(void *data, Evas_Object *obj, void *event_info)
{
   char buf[PATH_MAX];
   Panel_Image *panel_image = data;
   Enlil_Photo *photo = panel_image->photo;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
   const char *selected = event_info;

   if(panel_image->inwin)
     {
	evas_object_del(panel_image->inwin);
	panel_image->inwin = NULL;
     }

   if (selected)
     {
	const Enlil_Trans_History_Item *item = enlil_trans_history_current_get(panel_image->history);
	char *file = ecore_file_strip_ext(selected);
	snprintf(buf, PATH_MAX, "%s%s", file, strrchr(enlil_photo_file_name_get(photo),'.'));

	if(!panel_image->save.path && ecore_file_exists(buf))
	  {
	       panel_image->save.path = eina_stringshare_add(buf);
	       inwin_save_as_file_exists_new(NULL, _inwin_save_as_apply_cb, panel_image, buf);
	       return;
	  }
	ecore_file_cp(enlil_trans_history_item_file_get(item), buf);

	//copy exifs data
	enlil_photo_copy_exif_in_file(photo, buf);
	enlil_photo_save_iptc_in_custom_file(photo, buf);

	photo_data->enlil_data->auto_open = eina_list_append(
	      photo_data->enlil_data->auto_open, eina_stringshare_add(buf));
	FREE(file);
     }
   panel_image->save.save = EINA_FALSE;
}

static void _inwin_save_as_apply_cb(void *data)
{
   Panel_Image *panel_image = data;
   _bt_save_as_done_cb(panel_image->photo, NULL, (void *)panel_image->save.path);
   EINA_STRINGSHARE_DEL(panel_image->save.path);
}

static void _bt_save_as_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   panel_image_save_as(panel_image->photo);
}

static void _bt_save_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   panel_image_save(panel_image->photo);
}

static void _bt_close_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;

   if(panel_image->save_description_name)
     _save_description_name(panel_image);

   if(panel_image->save.save)
     {
	inwin_photo_save_new(NULL, _close_save_cb, _close_without_save_cb,
	      panel_image, panel_image->photo);
     }
   else
     panel_image_free(&panel_image);
}

static void _close_without_save_cb(void *data)
{
   Panel_Image *panel_image = data;
   panel_image_free(&panel_image);
}

static void _close_save_cb(void *data)
{
   Panel_Image *panel_image = data;
   panel_image_save(panel_image->photo);

   panel_image_free(&panel_image);
}

static void _entry_name_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   Enlil_Photo *photo = panel_image->photo;
   //Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);

   if(elm_entry_entry_get(panel_image->entry_name) != enlil_photo_name_get(photo))
     {
	panel_image->save_description_name = EINA_TRUE;

	if(panel_image->timer_description_name)
	  ecore_timer_del(panel_image->timer_description_name);
	panel_image->timer_description_name = ecore_timer_add(5, _save_description_name_timer, panel_image);
     }
}

static Eina_Bool _save_description_name_timer(void *data)
{
   Panel_Image *panel_image = data;
   _save_description_name(data);
   panel_image->timer_description_name = NULL;
   return EINA_FALSE;
}

static void _save_description_name(Panel_Image *panel_image)
{
   Enlil_Photo *photo = panel_image->photo;
   Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);

   if(!panel_image->save_description_name) return ;
   panel_image->save_description_name = EINA_FALSE;

   enlil_photo_name_set(photo, elm_entry_entry_get(panel_image->entry_name));
   enlil_photo_eet_save(photo);

   tabpanel_item_label_set(panel_image->tabpanel_item, enlil_photo_name_get(photo));

   Enlil_Photo *photo_prev = enlil_album_photo_prev_get(enlil_photo_album_get(photo), photo);
   if(!photo_prev)
     photos_list_object_child_move_after(photo_data->list_photo_item, NULL);
   else
     {
	Enlil_Photo_Data *photo_data_prev = enlil_photo_user_data_get(photo_prev);
	photos_list_object_child_move_after(photo_data->list_photo_item,
	      photo_data_prev->list_photo_item);
     }

   enlil_photo_description_set(photo, elm_entry_entry_get(panel_image->entry_description));
   enlil_photo_eet_save(photo);

   enlil_photo_save_iptc_in_file(photo);

   if(panel_image->timer_description_name)
     ecore_timer_del(panel_image->timer_description_name);
   panel_image->timer_description_name = NULL;
}

static void _entry_description_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;
   Enlil_Photo *photo = panel_image->photo;
   //Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);

   if(elm_entry_entry_get(panel_image->entry_description) != enlil_photo_description_get(photo))
     {
	panel_image->save_description_name = EINA_TRUE;

	if(panel_image->timer_description_name)
	  ecore_timer_del(panel_image->timer_description_name);
	panel_image->timer_description_name =
	   ecore_timer_add(5, _save_description_name_timer, panel_image);
     }
}

static void _panes_clicked_double(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;

   if(elm_panes_content_left_size_get(obj) > 0)
     {
	panel_image->panes_size = elm_panes_content_left_size_get(obj);
	elm_panes_content_left_size_set(obj, 0.0);
     }
   else
     elm_panes_content_left_size_set(obj, panel_image->panes_size);
}

static void _panes_h_clicked_double(void *data, Evas_Object *obj, void *event_info)
{
   Panel_Image *panel_image = data;

   if(elm_panes_content_left_size_get(obj) > 0)
     {
	panel_image->panes_h_size = elm_panes_content_left_size_get(obj);
	elm_panes_content_left_size_set(obj, 0.0);
     }
   else
     elm_panes_content_left_size_set(obj, panel_image->panes_h_size);
}

static Evas_Object *_slideshow_icon_get(const void *data, Evas_Object *obj)
{
	const char *s = NULL;
	Enlil_Photo *photo = (Enlil_Photo *) data;
	Enlil_Photo_Data *enlil_photo_data = enlil_photo_user_data_get(photo);

	Evas_Object *o = photo_object_add(obj);
	photo_object_theme_file_set(o, THEME, "photo_simple");

	if(enlil_photo_data->cant_create_thumb == 1)
		return o;

	s = enlil_thumb_photo_get(photo, Enlil_THUMB_FDO_NORMAL, thumb_done_cb, thumb_error_cb, NULL);

	evas_image_cache_flush (evas_object_evas_get(obj));

	if(s)
		photo_object_file_set(o, s , NULL);
	else
		photo_object_progressbar_set(o, EINA_TRUE);

	if(enlil_photo_type_get(photo) == ENLIL_PHOTO_TYPE_VIDEO)
		photo_object_camera_set(o, EINA_TRUE);

	photo_object_text_set(o, enlil_photo_name_get(photo));

	evas_object_show(o);
	return o;
}


static void _slideshow_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	Slideshow_Item *item = event_info;
	Panel_Image *panel_image = data;

	Enlil_Photo *photo = slideshow_object_item_data_get(item);

	_panel_image_photo_set(panel_image, photo);
}
