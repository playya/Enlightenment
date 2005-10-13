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
static int my_val = 0;
static int my_val2 = 0;
static int my_val3 = 0;
static int my_val4 = 0;
static int my_val5 = 0;
static int my_val6 = 0;
static int my_rval = 1;
static int my_rval2 = 1;

static void
_e_test_internal(E_Container *con)
{
   E_Dialog *dia;
   
   dia = e_dialog_new(con);
   e_dialog_title_set(dia, "A Test Dialog");
     {
	Evas_Object *o, *ol, *hb;
	Evas_Coord mw, mh;
	E_Radio_Group *rg;
	Evas *e;
	
	e = e_win_evas_get(dia->win);
	
	ol = e_widget_list_add(e, 0, 0);
	
	hb = e_widget_list_add(e, 1, 1);
	
	o = e_widget_framelist_add(e, "My Checklist", 0);
	
	e_widget_framelist_object_append(o, e_widget_check_add(e, "Checkbox 1", &my_val));
	e_widget_framelist_object_append(o, e_widget_check_add(e, "Checkbox 2", &my_val2));
	e_widget_framelist_object_append(o, e_widget_check_add(e, "Checkbox 3", &my_val3));
	e_widget_framelist_object_append(o, e_widget_check_add(e, "Checkbox and some long text", &my_val4));
	e_widget_framelist_object_append(o, e_widget_check_add(e, "Checkbox 7", &my_val5));
	e_widget_framelist_object_append(o, e_widget_check_add(e, "Koneko", &my_val6));
	
	rg = e_widget_radio_group_new(&my_rval);
	e_widget_framelist_object_append(o, e_widget_radio_add(e, "Rad 0", 0, rg));
	e_widget_framelist_object_append(o, e_widget_radio_add(e, "Rad 1", 1, rg));
	e_widget_framelist_object_append(o, e_widget_radio_add(e, "Rad 2", 2, rg));
	e_widget_framelist_object_append(o, e_widget_radio_add(e, "Rad 3", 3, rg));
	e_widget_framelist_object_append(o, e_widget_radio_add(e, "Rad 4", 4, rg));

	e_widget_list_object_append(hb, o, 1, 1, 0.5);

	o = e_widget_framelist_add(e, "My Checklist on the right", 0);
	
	rg = e_widget_radio_group_new(&my_rval2);
	e_widget_framelist_object_append(o, e_widget_radio_add(e, "Rad 0", 0, rg));
	e_widget_framelist_object_append(o, e_widget_radio_add(e, "Rad 1", 1, rg));
	e_widget_framelist_object_append(o, e_widget_radio_add(e, "Rad 2", 2, rg));
	e_widget_framelist_object_append(o, e_widget_radio_add(e, "Rad 3", 3, rg));
	e_widget_framelist_object_append(o, e_widget_radio_add(e, "Rad 4", 4, rg));

	e_widget_list_object_append(hb, o, 0, 0, 0.0);
	
	o = e_widget_frametable_add(e, "My Frame Table", 0);
	
	e_widget_frametable_object_append(o, e_widget_check_add(e, "L1", &my_val),
					  0, 0, 1, 1, 
					  1, 1, 1, 1);
	e_widget_frametable_object_append(o, e_widget_check_add(e, "L2", &my_val),
					  1, 0, 1, 1, 
					  1, 1, 1, 1);
	e_widget_frametable_object_append(o, e_widget_check_add(e, "L3", &my_val),
					  2, 0, 1, 1, 
					  1, 1, 1, 1);
	
	e_widget_frametable_object_append(o, e_widget_check_add(e, "One", &my_val),
					  0, 1, 1, 1, 
					  1, 1, 1, 1);
	e_widget_frametable_object_append(o, e_widget_check_add(e, "This spans 2", &my_val),
					  1, 1, 2, 1, 
					  1, 1, 1, 1);

	e_widget_frametable_object_append(o, e_widget_check_add(e, "A Really Long Checkbox at the bottom", &my_val),
					  0, 2, 3, 1, 
					  1, 1, 1, 1);
	
	e_widget_list_object_append(hb, o, 0, 0, 0.5);
	
	e_widget_list_object_append(ol, hb, 1, 1, 0.5);
	
	o = e_widget_button_add(e, "Button", "enlightenment/exit", NULL, NULL, NULL);
	e_widget_list_object_append(ol, o, 0, 0, 1.0);
	
	e_widget_min_size_get(ol, &mw, &mh);
	e_dialog_content_set(dia, ol, mw, mh);
     }
   e_dialog_button_add(dia, "OK", NULL, NULL, NULL);
   e_dialog_button_add(dia, "Apply", "enlightenment/reset", NULL, NULL);
   e_dialog_button_add(dia, "Cancel", "enlightenment/exit", NULL, NULL);
   e_win_centered_set(dia->win, 1);
   e_dialog_show(dia);
}
#elif 0

typedef struct _B_CFData {
   int val;
} B_CFData;
typedef struct _A_CFData {
   int val2;
} A_CFData;

static void *_b_create_data(void *cfdata_other, E_Config_Dialog_CFData_Type type_other);
static void _b_free_data(B_CFData *cfdata);
static void _b_apply_data(B_CFData *cfdata);
static Evas_Object *_b_create_widgets(Evas *evas, B_CFData *cfdata);
static void *_a_create_data(void *cfdata_other, E_Config_Dialog_CFData_Type type_other);
static void _a_free_data(A_CFData *cfdata);
static void _a_apply_data(A_CFData *cfdata);
static Evas_Object *_a_create_widgets(Evas *evas, A_CFData *cfdata);

static void *_b_create_data(void *cfdata_other, E_Config_Dialog_CFData_Type type_other) {
   B_CFData *cfdata;
   cfdata = E_NEW(B_CFData, 1);
   if (cfdata_other) cfdata->val = ((A_CFData *)cfdata_other)->val2;
   return cfdata;
}
static void _b_free_data(B_CFData *cfdata) {
   printf("FREE BASIC\n");
   free(cfdata);
}
static void _b_apply_data(B_CFData *cfdata) {
   printf("APPLY BASIC %i\n", cfdata->val);
}
static Evas_Object *_b_create_widgets(Evas *evas, B_CFData *cfdata) {
   Evas_Object *o, *ob;
   o = e_widget_framelist_add(evas, "Basic Options", 0);
   ob = e_widget_check_add(evas, "Basic Checkbox", &(cfdata->val));
   e_widget_framelist_object_append(o, ob);
   return o;
}

static void *_a_create_data(void *cfdata_other, E_Config_Dialog_CFData_Type type_other) {
   A_CFData *cfdata;
   cfdata = E_NEW(A_CFData, 1);
   if (cfdata_other) cfdata->val2 = ((B_CFData *)cfdata_other)->val;
   return cfdata;
}
static void _a_free_data(A_CFData *cfdata) {
   printf("FREE ADVANCED\n");
   free(cfdata);
}
static void _a_apply_data(A_CFData *cfdata) {
   printf("APPLY ADVANCED %i\n", cfdata->val2);
}
static Evas_Object *_a_create_widgets(Evas *evas, A_CFData *cfdata) {
   Evas_Object *o, *ob;
   o = e_widget_framelist_add(evas, "Advanced Options", 0);
   ob = e_widget_check_add(evas, "Advanced Checkbox", &(cfdata->val2));
   e_widget_framelist_object_append(o, ob);
   return o;
}

static void
_e_test_internal(E_Container *con)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View a, b;
   
   b.create_cfdata  = _b_create_data;
   b.free_cfdata    = _b_free_data;
   b.apply_cfdata   = _b_apply_data;
   b.create_widgets = _b_create_widgets;
   a.create_cfdata  = _a_create_data;
   a.free_cfdata    = _a_free_data;
   a.apply_cfdata   = _a_apply_data;
   a.create_widgets = _a_create_widgets;
   cfd = e_config_dialog_new(con, "A test dialog", &b, &a);
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
