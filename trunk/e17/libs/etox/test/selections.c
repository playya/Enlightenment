#include <../etox-config.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Etox.h>

#define IM PACKAGE_DATA_DIR"/images/"

void _test_sel1(Evas_Object *etox);


int win_w = 500, win_h = 200;
Ecore_Evas *ee;
Evas *evas;
Evas_Object *bg;
Evas_Object *etox;
char msg[] =
	    "The Etox Test utility consists in a series "
	    "of test suites designed to exercise all of "
	    "the etox functions.\n"
	    "Informational messages will be displayed here, "
	    "the test text will be presented in the colored "
	    "rectangle below.\n"
	    "To start a test suite, select it from the "
	    "navigation panel on the left.";

void
window_resize(Ecore_Evas *ee)
{
	evas_output_size_get(evas, &win_w, &win_h);
	evas_object_resize(bg, win_w, win_h);
	evas_object_image_fill_set(bg, 0, 0, win_w, win_h);
	evas_object_resize(etox, win_w, win_h);

	return;
	ee = NULL;
}

int sig_exit(void *data, int type, void * ev)
{
        printf("sig exit!!\n");
	ecore_main_loop_quit();
	return 1;
	data = NULL;
	type = 0;
	ev = NULL;
}

int main(int argc, const char **argv)
{
	/*
	Etox_Selection *selected1;
	Etox_Selection *selected2;
	*/
	Etox_Obstacle *obstacle;

	ecore_init();
	ecore_app_args_set(argc, argv);

	if (!ecore_evas_init())
		return -1;

	ee= ecore_evas_software_x11_new(NULL, 0, 0, 0, win_w, win_h);
	if (!ee)
		return 1;

	ecore_evas_title_set(ee, "Etox Selection Test");
	ecore_evas_show(ee);

	evas = ecore_evas_get(ee);

	bg = evas_object_image_add(evas);
	if (!bg)
		return 1;

	evas_object_image_file_set(bg, IM "bg.png", NULL);
	evas_object_move(bg, 0, 0);
	evas_object_resize(bg, win_w, win_h);
	evas_object_image_fill_set(bg, 0, 0, win_w, win_h);
	evas_object_show(bg);

	/* Create message etox */
	etox = etox_new_all(evas, 0, 0, win_w, win_h, 255,
			ETOX_ALIGN_LEFT | ETOX_ALIGN_BOTTOM);
	etox_context_set_font(etox, "Vera", 14);
	etox_context_set_color(etox, 173, 193, 79, 255);
	etox_context_set_style(etox, "shadow");
	etox_context_set_soft_wrap(etox, 1);
	etox_set_text(etox, msg);
	etox_set_alpha(etox, 255);
	etox_set_layer(etox, 1000);
	etox_show(etox);

	/*
	selected1 = etox_select_index(etox, 9, 60);
	if (selected1) {
		etox_selection_set_font(selected1, "morpheus", 20);
		etox_selection_set_style(selected1, "outline");
		etox_selection_set_color(selected1, 255, 0, 0, 255);
	}

	selected2 = etox_select_index(etox, 0, 20);
	if (selected2) {
		etox_selection_set_color(selected2, 0, 0, 255, 255);
	}

	etox_selection_free(selected1);

	selected1 = etox_select_index(etox, 59, 200);
	if (selected1) {
		etox_selection_set_style(selected1, "outline");
	}

	etox_selection_free(selected2);

	selected2 = etox_select_index(etox, 200, 1000);
	if (selected2) {
		etox_selection_set_color(selected2, 0, 0, 255, 255);
	}
	*/

	//etox_append_text(etox, "Fear the boring test text!");
        
        _test_sel1(etox); 

	//obstacle = etox_obstacle_add(etox, 20, 20, 80, 20);

	ecore_evas_callback_resize_set(ee, window_resize);
	ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, sig_exit, NULL);
        ecore_evas_callback_delete_request_set(ee, sig_exit);
        ecore_evas_callback_destroy_set(ee, sig_exit);

	ecore_main_loop_begin();

	ecore_evas_shutdown();
	ecore_shutdown();

	return 0;
}

void
_test_sel1(Evas_Object *etox)
{
  Etox_Selection *sel;
  Etox_Context *old, *cont;


  printf("style: %s\n", etox_context_get_style(etox));
  sel = etox_select_index(etox, 10, 120);

  old = etox_context_save(etox);

  etox_context_set_color(etox, 220, 0, 0, 255);
  etox_context_set_font(etox, "Vera", 12);
  cont = etox_context_save(etox);
  etox_selection_apply_context(sel, cont);

  etox_selection_free_by_etox(etox);
  
  sel = etox_select_index(etox, 50, 200);
  etox_context_set_color(etox, 200, 102, 10, 255);
  etox_context_free(cont);
  cont = etox_context_save(etox);
  etox_selection_apply_context(sel, cont);

  etox_context_load(etox, old);

  printf("style: %s\n", etox_context_get_style(etox));
  etox_append_text(etox, " Blah. Blah. Blum de dum.");

  etox_context_free(cont);
  etox_context_free(old);
}
