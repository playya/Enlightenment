// vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2

#include "main.h"

static Elm_Genlist_Item_Class itc_album;
static Elm_Genlist_Item_Class itc_col;
static Elm_Genlist_Item_Class itc_col_album;
static Elm_Genlist_Item_Class itc_tag;

static char *_gl_label_get(const void *data, Evas_Object *obj, const char *part);
static void _gl_sel(void *data, Evas_Object *obj, void *event_info);
static void _right_click_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

static char *_gl_col_label_get(const void *data, Evas_Object *obj, const char *part);
static void _gl_col_sel(void *data, Evas_Object *obj, void *event_info);
static char *_gl_col_album_label_get(const void *data, Evas_Object *obj, const char *part);
static void _gl_col_album_sel(void *data, Evas_Object *obj, void *event_info);
static void _col_right_click_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _gl_col_exp(void *data, Evas_Object *obj, void *event_info);
static void _gl_col_con(void *data, Evas_Object *obj, void *event_info);
static void _gl_col_exp_req(void *data, Evas_Object *obj, void *event_info);
static void _gl_col_con_req(void *data, Evas_Object *obj, void *event_info);

static char *_gl_tag_label_get(const void *data, Evas_Object *obj, const char *part);
static void _gl_tag_sel(void *data, Evas_Object *obj, void *event_info);
static void _tag_right_click_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _tabpanel_album_select_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item);
static void _tabpanel_collection_select_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item);
static void _tabpanel_tag_select_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item);


List_Left *list_left_new(Evas_Object *win)
{
    Evas_Object *gl, *panels, *tabs, *bx;
    Tabpanel_Item *tp_item;
    List_Left *list_left = calloc(1,sizeof(List_Left));

    bx = elm_box_add(win);
    list_left->bx = bx;
    evas_object_size_hint_weight_set(bx, 1.0, 1.0);
    evas_object_size_hint_align_set(bx, -1.0, -1.0);
    evas_object_show(bx);

    //
    list_left->tb_liste_map = tabpanel_add(win);

    tabs = tabpanel_tabs_obj_get(list_left->tb_liste_map);
    evas_object_size_hint_weight_set(tabs, -1.0, -1.0);
    evas_object_size_hint_align_set(tabs, -1.0, 0.0);
    elm_box_pack_end(bx, tabs);
    evas_object_show(tabs);

    list_left->panels_map = tabpanel_panels_obj_get(list_left->tb_liste_map);
    evas_object_size_hint_weight_set(list_left->panels_map, 1.0, 1.0);
    evas_object_size_hint_align_set(list_left->panels_map, -1.0, -1.0);
    evas_object_show(list_left->panels_map);
    //

    //
    list_left->tabpanel = tabpanel_add(win);

    tabs = tabpanel_tabs_obj_get(list_left->tabpanel);
    evas_object_size_hint_weight_set(tabs, -1.0, -1.0);
    evas_object_size_hint_align_set(tabs, -1.0, 0.0);
    elm_box_pack_end(bx, tabs);
    evas_object_show(tabs);

    panels = tabpanel_panels_obj_get(list_left->tabpanel);
    evas_object_size_hint_weight_set(panels, 1.0, 1.0);
    evas_object_size_hint_align_set(panels, -1.0, -1.0);
    evas_object_show(panels);
    elm_box_pack_end(bx, panels);
    //


    gl = elm_genlist_add(win);
    list_left->gl_albums = gl;
    elm_genlist_horizontal_mode_set(gl, ELM_LIST_SCROLL);
    evas_object_size_hint_weight_set(gl, 1.0, 1.0);
    evas_object_size_hint_align_set(gl, -1.0, -1.0);
    evas_object_show(gl);

    itc_album.item_style     = "default";
    itc_album.func.label_get = _gl_label_get;
    itc_album.func.icon_get  = NULL;
    itc_album.func.state_get = NULL;
    itc_album.func.del       = NULL;

    tp_item = tabpanel_item_add(list_left->tabpanel, D_("Albums"), gl, _tabpanel_album_select_cb, list_left);

    gl = elm_genlist_add(win);
    list_left->gl_collections = gl;
    elm_genlist_horizontal_mode_set(gl, ELM_LIST_SCROLL);
    evas_object_size_hint_weight_set(gl, 1.0, 1.0);
    evas_object_size_hint_align_set(gl, -1.0, -1.0);
    evas_object_show(gl);

    itc_col.item_style     = "default";
    itc_col.func.label_get = _gl_col_label_get;
    itc_col.func.icon_get  = NULL;
    itc_col.func.state_get = NULL;
    itc_col.func.del       = NULL;

    itc_col_album.item_style     = "default";
    itc_col_album.func.label_get = _gl_col_album_label_get;
    itc_col_album.func.icon_get  = NULL;
    itc_col_album.func.state_get = NULL;
    itc_col_album.func.del       = NULL;

    evas_object_smart_callback_add(gl, "expanded", _gl_col_exp, gl);
    evas_object_smart_callback_add(gl, "contracted", _gl_col_con, gl);
    evas_object_smart_callback_add(gl, "expand,request", _gl_col_exp_req, gl);
    evas_object_smart_callback_add(gl, "contract,request", _gl_col_con_req, gl);

    tabpanel_item_add(list_left->tabpanel, D_("Collections"), gl, _tabpanel_collection_select_cb, list_left);

    gl = elm_genlist_add(win);
    list_left->gl_tags = gl;
    elm_genlist_horizontal_mode_set(gl, ELM_LIST_SCROLL);
    evas_object_size_hint_weight_set(gl, 1.0, 1.0);
    evas_object_size_hint_align_set(gl, -1.0, -1.0);
    evas_object_show(gl);

    itc_tag.item_style     = "default";
    itc_tag.func.label_get = _gl_tag_label_get;
    itc_tag.func.icon_get  = NULL;
    itc_tag.func.state_get = NULL;
    itc_tag.func.del       = NULL;

    tabpanel_item_add(list_left->tabpanel, D_("Tags"), gl, _tabpanel_tag_select_cb, list_left);

    tabpanel_item_select(tp_item);

    return list_left;
}

void list_left_data_set(List_Left *list_left, Enlil_Data *enlil_data)
{
    Eina_List *l;
    Enlil_Album *album;
    Enlil_Root *root = enlil_data->root;

    list_left->enlil_data = enlil_data;
    enlil_data->list_left = list_left;

    elm_genlist_clear(list_left->gl_albums);
    elm_genlist_clear(list_left->gl_collections);

    EINA_LIST_FOREACH(enlil_root_albums_get(root), l, album)
        list_left_add(list_left, album);
}

void list_left_add(List_Left *list_left, Enlil_Album *album)
{
    Enlil_Album_Data *enlil_album_data = enlil_album_user_data_get(album);

    enlil_album_data->list_album_item = elm_genlist_item_append(list_left->gl_albums, &itc_album,
				    album, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel,
				    album);
}

void list_left_append_relative(List_Left *list_left, Enlil_Album *album, Elm_Genlist_Item *relative)
{
    Enlil_Album_Data *enlil_album_data = enlil_album_user_data_get(album);

    if(!relative)
      enlil_album_data->list_album_item = elm_genlist_item_prepend(list_left->gl_albums, &itc_album,
	    album, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel,
	    album);
    else
      enlil_album_data->list_album_item = elm_genlist_item_insert_after(list_left->gl_albums, &itc_album,
	    album, relative, ELM_GENLIST_ITEM_NONE, _gl_sel,
	    album);
}


void list_left_col_add(List_Left *list_left, Enlil_Collection *col)
{
    Enlil_Collection_Data *col_data = enlil_collection_user_data_get(col);

    col_data->list_col_item = elm_genlist_item_append(list_left->gl_collections, &itc_col,
				    col, NULL, ELM_GENLIST_ITEM_SUBITEMS, _gl_col_sel,
				    col);
}

void list_left_col_album_add(List_Left *list_left, Enlil_Collection *col, Enlil_Album *album)
{
   ASSERT_RETURN_VOID(list_left != NULL);
   ASSERT_RETURN_VOID(col!= NULL);
   ASSERT_RETURN_VOID(album!= NULL);

   Enlil_Collection_Data *col_data = enlil_collection_user_data_get(col);

   if(elm_genlist_item_expanded_get(col_data->list_col_item))
     {
	elm_genlist_item_append(list_left->gl_collections, &itc_col_album,
	      album, col_data->list_col_item, ELM_GENLIST_ITEM_NONE, _gl_col_album_sel,
	      album);
     }
}

void list_left_tag_add(List_Left *list_left, Enlil_Tag *tag)
{
    Enlil_Tag_Data *tag_data = enlil_tag_user_data_get(tag);

    tag_data->list_tag_item = elm_genlist_item_append(list_left->gl_tags, &itc_tag,
				    tag, NULL, ELM_GENLIST_ITEM_NONE, _gl_tag_sel,
				    tag);
}

void list_left_col_album_remove(List_Left *list_left, Enlil_Collection *col, Enlil_Album *album)
{
   Eina_List *l;
   Enlil_Album *_album;
   ASSERT_RETURN_VOID(list_left != NULL);
   ASSERT_RETURN_VOID(col!= NULL);
   ASSERT_RETURN_VOID(album!= NULL);

   Enlil_Collection_Data *col_data = enlil_collection_user_data_get(col);

   if(elm_genlist_item_expanded_get(col_data->list_col_item))
     {
	photos_list_object_freeze(col_data->enlil_data->list_photo->o_list, 1);
	photos_list_object_hide_all(col_data->enlil_data->list_photo->o_list);

	EINA_LIST_FOREACH(enlil_collection_albums_get(col), l, _album)
	  {
	     Enlil_Album_Data *album_data = enlil_album_user_data_get(_album);
	     photos_list_object_header_show(album_data->list_photo_item);
	  }

	elm_genlist_item_subitems_clear(col_data->list_col_item);
	elm_genlist_item_expanded_set(col_data->list_col_item, 0);
	elm_genlist_item_expanded_set(col_data->list_col_item, 1);
	photos_list_object_freeze(col_data->enlil_data->list_photo->o_list, 0);
	photos_list_object_top_goto(col_data->enlil_data->list_photo->o_list);
     }
}

void list_left_remove(List_Left *list_left, Enlil_Album *album)
{
    Enlil_Album_Data *enlil_album_data = enlil_album_user_data_get(album);

    elm_genlist_item_del(enlil_album_data->list_album_item);
}

void list_left_update(List_Left *list_left, Enlil_Album *album)
{
    Enlil_Album_Data *enlil_album_data = enlil_album_user_data_get(album);

    elm_genlist_item_update(enlil_album_data->list_album_item);
}

static void _tabpanel_album_select_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item)
{
   List_Left *list_left = data;

   if(list_left->enlil_data)
     photos_list_object_show_all(list_left->enlil_data->list_photo->o_list);
}

static void _tabpanel_tag_select_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item)
{
   List_Left *list_left = data;

   Elm_Genlist_Item *gl_item = elm_genlist_selected_item_get(list_left->gl_tags);
   if(gl_item)
     elm_genlist_item_selected_set(gl_item,0);

   if(list_left->enlil_data)
     photos_list_object_show_all(list_left->enlil_data->list_photo->o_list);
}

static void _tabpanel_collection_select_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item)
{
   List_Left *list_left = data;
   Elm_Genlist_Item *gl_item;

   gl_item = elm_genlist_first_item_get(list_left->gl_collections);
   for(;gl_item;gl_item=elm_genlist_item_next_get(gl_item))
     elm_genlist_item_expanded_set(gl_item, 0);

   gl_item = elm_genlist_selected_item_get(list_left->gl_collections);
   if(gl_item)
     elm_genlist_item_selected_set(gl_item,0);

   if(list_left->enlil_data)
     photos_list_object_show_all(list_left->enlil_data->list_photo->o_list);
}

static char *_gl_label_get(const void *data, Evas_Object *obj, const char *part)
{
   Enlil_Album *album = (Enlil_Album *)data;
   Enlil_Album_Data *album_data = enlil_album_user_data_get(album);

   const Evas_Object *o = elm_genlist_item_object_get(album_data->list_album_item);
   evas_object_event_callback_add((Evas_Object*)o, EVAS_CALLBACK_MOUSE_UP, _right_click_cb, album);

   return strdup(enlil_album_name_get(album));
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
   Enlil_Album *album = (Enlil_Album *)data;
   Eina_List *l;
   Eina_List *markers = NULL;
   Enlil_Photo *photo;
   Enlil_Album_Data *enlil_album_data = (Enlil_Album_Data*) enlil_album_user_data_get(album);

   if(!enlil_data->list_left->is_map)
     {
	photos_list_object_header_bring_in(enlil_album_data->list_photo_item);
     }
   else
     {
	photos_list_object_header_goto(enlil_album_data->list_photo_item);
	EINA_LIST_FOREACH(enlil_album_photos_get(album), l, photo)
	  {
	     Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
	     if(photo_data->marker)
	       markers = eina_list_append(markers, photo_data->marker);
	  }
	if(markers)
	  {
	     elm_map_markers_list_show(markers);
	     eina_list_free(markers);
	     elm_slider_value_set(enlil_data->map->sl, elm_map_zoom_get(enlil_data->map->map));
	  }
     }
}

static void _gl_col_album_sel(void *data, Evas_Object *obj, void *event_info)
{
   Eina_List *l;
   Enlil_Album *album;
   Enlil_Photo *photo;
   Eina_List *markers = NULL;

   const Enlil_Collection *col = elm_genlist_item_data_get(elm_genlist_item_parent_get(event_info));
   Enlil_Album_Data *enlil_album_data = enlil_album_user_data_get(data);
   Enlil_Collection_Data *col_data = enlil_collection_user_data_get(col);

   //Hide all the albums in the albums list except the albums which are in the collection
   photos_list_object_freeze(col_data->enlil_data->list_photo->o_list, 1);
   photos_list_object_hide_all(col_data->enlil_data->list_photo->o_list);

   EINA_LIST_FOREACH(enlil_collection_albums_get(col), l, album)
     {
	 Enlil_Album_Data *album_data = enlil_album_user_data_get(album);
	 photos_list_object_header_show(album_data->list_photo_item);
     }
   photos_list_object_freeze(col_data->enlil_data->list_photo->o_list, 0);

   if(!enlil_data->list_left->is_map)
     {
	photos_list_object_header_bring_in(enlil_album_data->list_photo_item);
     }
   else
     {
	photos_list_object_header_goto(enlil_album_data->list_photo_item);
	EINA_LIST_FOREACH(enlil_album_photos_get(album), l, photo)
	  {
	     Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
	     if(photo_data->marker)
	       markers = eina_list_append(markers, photo_data->marker);
	  }
	if(markers)
	  {
	     elm_map_markers_list_show(markers);
	     eina_list_free(markers);
	     elm_slider_value_set(enlil_data->map->sl, elm_map_zoom_get(enlil_data->map->map));
	  }
     }
}

static char *_gl_col_label_get(const void *data, Evas_Object *obj, const char *part)
{
   const Enlil_Collection *col = data;
   Enlil_Collection_Data *col_data = enlil_collection_user_data_get(col);

   const Evas_Object *o = elm_genlist_item_object_get(col_data->list_col_item);
   evas_object_event_callback_add((Evas_Object*)o, EVAS_CALLBACK_MOUSE_UP, _col_right_click_cb, col);

   return strdup(enlil_collection_name_get(col));
}

static char *_gl_col_album_label_get(const void *data, Evas_Object *obj, const char *part)
{
   const Enlil_Album *album = data;

   return strdup(enlil_album_name_get(album));
}

static void _gl_col_sel(void *data, Evas_Object *obj, void *event_info)
{
   Eina_List *l;
   Enlil_Album *album;

   Enlil_Collection *col = data;
   Enlil_Collection_Data *col_data = enlil_collection_user_data_get(col);

   //Hide all the albums in the albums list except the albums which are in the collection
   photos_list_object_freeze(col_data->enlil_data->list_photo->o_list, 1);
   photos_list_object_hide_all(col_data->enlil_data->list_photo->o_list);

   EINA_LIST_FOREACH(enlil_collection_albums_get(col), l, album)
     {
	 Enlil_Album_Data *album_data = enlil_album_user_data_get(album);
	 photos_list_object_header_show(album_data->list_photo_item);
     }
   photos_list_object_freeze(col_data->enlil_data->list_photo->o_list, 0);
   photos_list_object_top_goto(col_data->enlil_data->list_photo->o_list);
}

static void _gl_col_exp_req(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 1);
}

static void _gl_col_con_req(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 0);
}

static void _gl_col_exp(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   Eina_List *l;
   Enlil_Album *album;

   const Enlil_Collection *col = elm_genlist_item_data_get(it);
   Enlil_Collection_Data *col_data = enlil_collection_user_data_get(col);

   EINA_LIST_FOREACH(enlil_collection_albums_get(col), l, album)
     {
	elm_genlist_item_append(col_data->enlil_data->list_left->gl_collections, &itc_col_album,
	      album, it, ELM_GENLIST_ITEM_NONE, _gl_col_album_sel,
	      album);
     }
}

static void _gl_col_con(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_subitems_clear(it);
}

static char *_gl_tag_label_get(const void *data, Evas_Object *obj, const char *part)
{
   const Enlil_Tag *tag = data;
   Enlil_Tag_Data *tag_data = enlil_tag_user_data_get(tag);

   const Evas_Object *o = elm_genlist_item_object_get(tag_data->list_tag_item);
   evas_object_event_callback_add((Evas_Object*)o, EVAS_CALLBACK_MOUSE_UP, _tag_right_click_cb, tag);

   return strdup(enlil_tag_name_get(tag));
}

static void _right_click_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Enlil_Album *album = data;
   Enlil_Album_Data *album_data = enlil_album_user_data_get(album);
   Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up*) event_info;

   if(ev->button != 3) return ;

   elm_genlist_item_selected_set(album_data->list_album_item, 1);
   Album_Menu *album_menu = album_menu_new(album_data->enlil_data->win->win, album);
   elm_menu_move(album_menu->menu, ev->output.x, ev->output.y);
}

static void _col_right_click_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Enlil_Collection *col = data;
   Enlil_Collection_Data *col_data = enlil_collection_user_data_get(col);
   Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up*) event_info;

   if(ev->button != 3) return ;

   elm_genlist_item_selected_set(col_data->list_col_item, 1);
   Collection_Menu *col_menu = collection_menu_new(col_data->enlil_data->win->win, col);
   elm_menu_move(col_menu->menu, ev->output.x, ev->output.y);
}

static void _tag_right_click_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Enlil_Tag *tag = data;
   Enlil_Tag_Data *tag_data = enlil_tag_user_data_get(tag);
   Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up*) event_info;

   if(ev->button != 3) return ;

   elm_genlist_item_selected_set(tag_data->list_tag_item, 1);
   Tag_Menu *tag_menu = tag_menu_new(tag_data->enlil_data->win->win, tag);
   elm_menu_move(tag_menu->menu, ev->output.x, ev->output.y);
}

static void _gl_tag_sel(void *data, Evas_Object *obj, void *event_info)
{
   Eina_List *l;
   Enlil_Photo *photo;

   Enlil_Tag *tag = data;
   Enlil_Tag_Data *tag_data = enlil_tag_user_data_get(tag);

   //Hide all the albums in the albums list except the albums which are in the collection
   photos_list_object_freeze(tag_data->enlil_data->list_photo->o_list, 1);
   photos_list_object_hide_all(tag_data->enlil_data->list_photo->o_list);

   EINA_LIST_FOREACH(enlil_tag_photos_get(tag), l, photo)
     {
	 Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
	 photos_list_object_item_show(photo_data->list_photo_item);
     }
   photos_list_object_freeze(tag_data->enlil_data->list_photo->o_list, 0);
   photos_list_object_top_goto(tag_data->enlil_data->list_photo->o_list);
}

