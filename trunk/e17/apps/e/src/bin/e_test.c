/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

static void _e_test_internal(E_Container *con);
static void _cb_change(void *data, E_Gadman_Client *gmc, E_Gadman_Change change);

void
e_test(void)
{
   Evas_List *managers, *l, *ll;
   
   managers = e_manager_list();
   for (l = managers; l; l = l->next)
     {
	E_Manager *man;
	
	man = l->data;
	for (ll = man->containers; ll; ll = ll->next)
	  {
	     E_Container *con;
	     
	     con = ll->data;
	     _e_test_internal(con);
	  }
     }
}

#if 0
/* local subsystem functions */
typedef struct _Dat Dat;
struct _Dat
{
   Evas_Object *table;
   Evas_List *items;
};

static void
_e_test_internal(E_Container *con)
{
   E_Gadman_Client *gmc;
   Dat *dat;
   Evas_Object *o;
   int i, j;
   
   dat = calloc(1, sizeof(Dat));
   dat->table = e_table_add(con->bg_evas);
   e_table_freeze(dat->table);
   e_table_homogenous_set(dat->table, 1);
   for (j = 0; j < 5; j++)
     {
	for (i = 0; i < 5; i++)
	  {
	     o = evas_object_rectangle_add(con->bg_evas);
	     dat->items = evas_list_append(dat->items, o);
	     evas_object_color_set(o, i * 50, j * 50, 100, 100);
	     e_table_pack(dat->table, o, i, j, 1, 1);
	     e_table_pack_options_set(o, 1, 1, 1, 1, 0.5, 0.5, 0, 0, -1, -1);
	     evas_object_show(o);
	  }
     }
   e_table_thaw(dat->table);
   evas_object_show(dat->table);
   
   gmc = e_gadman_client_new(con->gadman);
   e_gadman_client_domain_set(gmc, "TEST", 0);
   e_gadman_client_policy_set(gmc,
			      E_GADMAN_POLICY_ANYWHERE |
			      E_GADMAN_POLICY_HMOVE |
			      E_GADMAN_POLICY_VMOVE |
			      E_GADMAN_POLICY_HSIZE |
			      E_GADMAN_POLICY_VSIZE);
   e_gadman_client_min_size_set(gmc, 10, 10);
   e_gadman_client_auto_size_set(gmc, 128, 128);
   e_gadman_client_align_set(gmc, 0.5, 0.5);
   e_gadman_client_resize(gmc, 128, 128);
   e_gadman_client_change_func_set(gmc, _cb_change, dat);
   e_gadman_client_load(gmc);
}

static void
_cb_change(void *data, E_Gadman_Client *gmc, E_Gadman_Change change)
{
   Dat *dat;
   Evas_Coord x, y, w, h;
   
   dat = data;
   switch (change)
     {
      case E_GADMAN_CHANGE_MOVE_RESIZE:
	e_gadman_client_geometry_get(gmc, &x, &y, &w, &h);
	evas_object_move(dat->table, x, y);
	evas_object_resize(dat->table, w, h);
	break;
      default:
	break;
     }
}
#elif 0
typedef struct _Dat Dat;
struct _Dat
{
   Evas_Object *layout;
   Evas_List *items;
};

static void
_e_test_internal(E_Container *con)
{
   E_Gadman_Client *gmc;
   Dat *dat;
   Evas_Object *o;
   int i;
   
   dat = calloc(1, sizeof(Dat));
   dat->layout = e_layout_add(con->bg_evas);
   e_layout_freeze(dat->layout);
   e_layout_virtual_size_set(dat->layout, 800, 600);
   for (i = 0; i < 10; i++)
     {
	Evas_Coord x, y, w, h;
	
	o = evas_object_rectangle_add(con->bg_evas);
	dat->items = evas_list_append(dat->items, o);
	evas_object_color_set(o, i * 25, 255 - (i * 25), 100, 100);
	e_layout_pack(dat->layout, o);
	w = rand() % 800;
	h = rand() % 600;
	x = rand() % (800 - w);
	y = rand() % (600 - h);
	e_layout_child_move(o, x, y);
	e_layout_child_resize(o, w, h);
	evas_object_show(o);
     }
   e_layout_thaw(dat->layout);
   evas_object_show(dat->layout);
   
   gmc = e_gadman_client_new(con->gadman);
   e_gadman_client_domain_set(gmc, "TEST", 0);
   e_gadman_client_policy_set(gmc,
			      E_GADMAN_POLICY_ANYWHERE |
			      E_GADMAN_POLICY_HMOVE |
			      E_GADMAN_POLICY_VMOVE |
			      E_GADMAN_POLICY_HSIZE |
			      E_GADMAN_POLICY_VSIZE);
   e_gadman_client_min_size_set(gmc, 10, 10);
   e_gadman_client_auto_size_set(gmc, 128, 128);
   e_gadman_client_align_set(gmc, 0.5, 0.5);
   e_gadman_client_resize(gmc, 128, 128);
   e_gadman_client_change_func_set(gmc, _cb_change, dat);
   e_gadman_client_load(gmc);
}

static void
_cb_change(void *data, E_Gadman_Client *gmc, E_Gadman_Change change)
{
   Dat *dat;
   Evas_Coord x, y, w, h;
   
   dat = data;
   switch (change)
     {
      case E_GADMAN_CHANGE_MOVE_RESIZE:
	e_gadman_client_geometry_get(gmc, &x, &y, &w, &h);
	evas_object_move(dat->layout, x, y);
	evas_object_resize(dat->layout, w, h);
	break;
      default:
	break;
     }
}
#elif 0
static int
_e_test_timer(void *data)
{
   E_Menu *m;
   Evas_List *managers, *l;
   
   m = data;
   if (m)
     {
	e_menu_deactivate(m);
	e_object_del(E_OBJECT(m));
	ecore_timer_add(0.05, _e_test_timer, NULL);
	return 0;
     }
   managers = e_manager_list();
   for (l = managers; l; l = l->next)
     {
	E_Manager *man;
	
	man = l->data;
	m = e_int_menus_main_new();
	e_menu_activate_mouse(m,
			      e_container_zone_number_get(e_container_current_get(man), 0),
			      0, 0, 1, 1, E_MENU_POP_DIRECTION_DOWN, 0);
	ecore_timer_add(0.05, _e_test_timer, m);
	return 0;
     }
   return 0;
}

static void
_e_test_internal(E_Container *con)
{
   _e_test_timer(NULL);
}
#elif 0
static void
_e_test_resize(E_Win *win)
{
   Evas_Object *o;
   
   o = win->data;
   printf("RESIZE %i %i\n", win->w, win->h);
   evas_object_resize(o, win->w, win->h);
   evas_object_color_set(o, rand() & 0xff, rand() & 0xff, rand() & 0xff, 255);
}

static void
_e_test_delete(E_Win *win)
{
   printf("DEL!\n");
   e_object_del(E_OBJECT(win));
}

static void
_e_test_internal(E_Container *con)
{
   E_Win *win;
   Evas_Object *o;
   
   win = e_win_new(con);
   e_win_resize_callback_set(win, _e_test_resize);
   e_win_delete_callback_set(win, _e_test_delete);
   e_win_placed_set(win, 0);
   e_win_move_resize(win, 10, 80, 400, 200);
   e_win_name_class_set(win, "E", "_test_window");
   e_win_title_set(win, "A test window");
   e_win_raise(win);
   e_win_show(win);
   
   o = evas_object_rectangle_add(e_win_evas_get(win));
   evas_object_color_set(o, 255, 200, 100, 255);
   evas_object_resize(o, 400, 200);
   evas_object_show(o);
   
   win->data = o;
}
#elif 0
static int
_e_test_timer(void *data)
{
   E_Menu *m;
   static int y = 0;
   
   m = data;
   ecore_x_pointer_warp(m->evas_win, 20, y);
   y += 10;
   if (y > m->cur.h) y = 0;
   return 1;
}

static void
_e_test_internal(E_Container *con)
{
   E_Menu *m;
   Evas_List *managers, *l;
   
   managers = e_manager_list();
   for (l = managers; l; l = l->next)
     {
	E_Manager *man;
	
	man = l->data;
	m = e_int_menus_main_new();
	e_menu_activate_mouse(m,
			      e_container_zone_number_get(e_container_current_get(man), 0),
			      0, 0, 1, 1, E_MENU_POP_DIRECTION_DOWN, 0);
	ecore_timer_add(0.02, _e_test_timer, m);
     }
}
#elif 0
static void
_e_test_dialog_del(void *obj)
{
   E_Dialog *dia;
   
   dia = obj;
   printf("dialog delete hook!\n");
}

static void
_e_test_internal(E_Container *con)
{
   E_Dialog *dia;
   
   dia = e_dialog_new(con);
   e_object_del_attach_func_set(E_OBJECT(dia), _e_test_dialog_del);
   e_dialog_title_set(dia, "A Test Dialog");
   e_dialog_text_set(dia, "A Test Dialog<br>And another line<br><hilight>Hilighted Text</hilight>");
   e_dialog_icon_set(dia, "enlightenment/modules", 64);
   e_dialog_button_add(dia, "OK", NULL, NULL, NULL);
   e_dialog_button_add(dia, "Apply", "enlightenment/reset", NULL, NULL);
   e_dialog_button_add(dia, "Cancel", "enlightenment/exit", NULL, NULL);
   e_win_centered_set(dia->win, 1);
   e_dialog_show(dia);
}
#elif 0
static void
_e_test_click(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
#if 1
   double size;
   
   size = (double)(rand() % 1000) / 999;
   evas_object_resize(obj, size * 1024, size * 768);
#else
   e_scrollframe_child_region_show(data, 1024, 768, 100, 100);
#endif
}

static void
_e_test_internal(E_Container *con)
{
   E_Dialog *dia;
   Evas_Object *o, *o2, *o3;
   
   dia = e_dialog_new(con);
   e_dialog_title_set(dia, "A Test Dialog");
   
   o = e_icon_add(dia->win->evas);
   e_icon_file_set(o, "/home/raster/t.png");
   evas_object_resize(o, 1024, 768);
   evas_object_focus_set(o, 1);
   evas_object_show(o);
      
   o2 = e_scrollframe_add(dia->win->evas);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _e_test_click, o2);
//   e_scrollframe_custom_theme_set(o2, "base/theme/widgets", "widgets/scrollframe");
   evas_object_show(o2);
#if 0   
   o3 = e_pan_add(dia->win->evas);
   e_pan_child_set(o3, o);
   e_scrollframe_extern_pan_set(o2, o3, e_pan_set, e_pan_get, e_pan_max_get, e_pan_child_size_get);
#else
   e_scrollframe_child_set(o2, o);
#endif
   
   e_dialog_content_set(dia, o2, 500, 300);
   e_dialog_button_add(dia, "OK", NULL, NULL, NULL);
   e_win_centered_set(dia->win, 1);
   e_dialog_show(dia);
   
   evas_object_focus_set(o, 1);
}
#elif 0
static E_Dialog *
_e_test_dia(E_Container *con)
{
   E_Config_Dialog *dia;
   
   dia = e_int_config_focus(con);
   return dia;
}

static E_Container *tcon = NULL;

static int
_e_test_timer(void *data)
{
   E_Config_Dialog *dia;
	
   if (data == NULL)
     {
	dia = _e_test_dia(tcon);
	ecore_timer_add(0.2, _e_test_timer, dia);
     }
   else
     {
	dia = data;
	e_object_del(E_OBJECT(dia));
	ecore_timer_add(0.2, _e_test_timer, NULL);
     }
   return 0;
}

static void
_e_test_internal(E_Container *con)
{
   tcon = con;
   _e_test_timer(NULL);
}
#elif 0

struct _tmp
{
   Evas_Object *ilist, *scrollframe;
};

static struct _tmp tmp = {NULL, NULL};

static void
_e_test_sel(void *data)
{
   Evas_Coord x, y, w, h;
   
   printf("SEL:\n");
   e_ilist_selected_geometry_get(tmp.ilist, &x, &y, &w, &h);
   e_scrollframe_child_region_show(tmp.scrollframe, x, y, w, h);
}

static void
_e_test_internal(E_Container *con)
{
   E_Dialog *dia;
   Evas_Coord mw, mh, vw, vh;
   Evas_Object *o, *o2, *o3;
   
   dia = e_dialog_new(con);
   e_dialog_title_set(dia, "A Test Dialog");
   
   o = e_ilist_add(dia->win->evas);
   
   o3 = e_icon_add(dia->win->evas);
   e_icon_file_set(o3, "/home/raster/C/stuff/icons/star_office.png");
   e_ilist_append(o, o3, "Item 1", _e_test_sel, NULL);
   
   o3 = e_icon_add(dia->win->evas);
   e_icon_file_set(o3, "/home/raster/C/stuff/icons/palette.png");
   e_ilist_append(o, o3, "Item 2 (Some really long text goes here for testing)", _e_test_sel, NULL);
   
   o3 = e_icon_add(dia->win->evas);
   e_icon_file_set(o3, "/home/raster/C/stuff/icons/mozilla.png");
   e_ilist_append(o, o3, "Item 3 (Medium length)", _e_test_sel, NULL);
   
   o3 = e_icon_add(dia->win->evas);
   e_icon_file_set(o3, "/home/raster/C/stuff/icons/trash_open.png");
   e_ilist_append(o, o3, "Item POOP", _e_test_sel, NULL);

   o3 = e_icon_add(dia->win->evas);
   e_icon_file_set(o3, "/home/raster/C/stuff/icons/watch.png");
   e_ilist_append(o, o3, "Item BLING BLING", _e_test_sel, NULL);

   o3 = e_icon_add(dia->win->evas);
   e_icon_file_set(o3, "/home/raster/C/stuff/icons/quake3.png");
   e_ilist_append(o, o3, "Sukebelinth", _e_test_sel, NULL);

   o3 = e_icon_add(dia->win->evas);
   e_icon_file_set(o3, "/home/raster/C/stuff/icons/opera6.png");
   e_ilist_append(o, o3, "Panties", _e_test_sel, NULL);

   o3 = e_icon_add(dia->win->evas);
   e_icon_file_set(o3, "/home/raster/C/stuff/icons/drawer_open.png");
   e_ilist_append(o, o3, "Flimbert the cagey", _e_test_sel, NULL);

   o3 = e_icon_add(dia->win->evas);
   e_icon_file_set(o3, "/home/raster/C/stuff/icons/cd.png");
   e_ilist_append(o, o3, "Norbert", _e_test_sel, NULL);
   
   e_ilist_min_size_get(o, &mw, &mh);
   evas_object_resize(o, mw, mh);
   evas_object_focus_set(o, 1);
   evas_object_show(o);
      
   o2 = e_scrollframe_add(dia->win->evas);
   evas_object_resize(o2, mw, 150);
   evas_object_show(o2);
   e_scrollframe_child_set(o2, o);

   e_scrollframe_child_viewport_size_get(o2, &vw, &vh);
   e_dialog_content_set(dia, o2, mw + (mw - vw), 150);
   e_dialog_button_add(dia, "OK", NULL, NULL, NULL);
   e_win_centered_set(dia->win, 1);
   e_dialog_show(dia);
   
   tmp.ilist = o;
   tmp.scrollframe = o2;
   
   evas_object_focus_set(o, 1);
}
#else
static void
_e_test_internal(E_Container *con)
{    
}

static void
_cb_change(void *data, E_Gadman_Client *gmc, E_Gadman_Change change)
{
}
#endif
