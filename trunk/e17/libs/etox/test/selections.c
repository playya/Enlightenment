#include <../etox-config.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Etox.h>

#define IM PACKAGE_DATA_DIR"/images/"

Ecore_Evas *ee;
Evas *evas;
Evas_Object *bg;
Etox *etox;
char msg[] =
	    "The Etox Test utility consists in a series\n"
	    "of test suites designed to exercise all of\n"
	    "the etox functions.\n"
	    "Informational messages will be displayed here,\n"
	    "the test text will be presented in the colored\n"
	    "rectangle below.\n"
	    "To start a test suite, select it from the\n"
	    "navigation panel on the left.\n";

int main(int argc, const char **argv)
{
	Etox_Selection *selected1;
	Etox_Selection *selected2;

	ecore_init();
	ecore_app_args_set(argc, argv);

	if (!ecore_evas_init())
		return -1;

	ee= ecore_evas_software_x11_new(NULL, 0, 0, 0, 500, 200);
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
	evas_object_resize(bg, 500, 200);
	evas_object_image_fill_set(bg, 0, 0, 500, 200);
	evas_object_show(bg);

	/* Create message etox */
	etox = etox_new_all(evas, 10, 10, 480, 180, 255, ETOX_ALIGN_LEFT);
	etox_context_set_align(etox, ETOX_ALIGN_LEFT);
	etox_context_set_font(etox, "sinon", 14);
	etox_context_set_color(etox, 0, 255, 0, 255);
	etox_context_set_style(etox, "shadow");
	/* etox_context_set_soft_wrap(etox, 1); */
	etox_set_text(etox, msg);
	etox_set_alpha(etox, 255);
	etox_set_layer(etox, 1000);
	etox_show(etox);

	selected1 = etox_select_index(etox, 9, 60);
	printf("Selected %p\n", selected1);
	if (selected1) {
		printf("Selected from %p to %p\n", selected1->start.bit,
				selected1->end.bit);
		etox_selection_set_font(selected1, "morpheus", 20);
		etox_selection_set_style(selected1, "outline");
		etox_selection_set_color(selected1, 255, 0, 0, 255);
	}

	selected2 = etox_select_index(etox, 0, 20);
	printf("Selected %p\n", selected2);
	if (selected2) {
		printf("Selected from %p to %p\n", selected2->start.bit,
				selected2->end.bit);
		etox_selection_set_color(selected2, 0, 0, 255, 255);
	}

	etox_selection_free(selected1);

	selected1 = etox_select_index(etox, 59, 200);
	printf("Selected %p\n", selected1);
	if (selected1) {
		printf("Selected from %p to %p\n", selected1->start.bit,
				selected1->end.bit);
		etox_selection_set_style(selected1, "outline");
	}

	etox_selection_free(selected2);

	selected2 = etox_select_index(etox, 200, 1000);
	printf("Selected %p\n", selected2);
	if (selected2) {
		printf("Selected from %p to %p\n", selected2->start.bit,
				selected2->end.bit);
		etox_selection_set_color(selected2, 0, 0, 255, 255);
	}

	ecore_main_loop_begin();

	ecore_evas_shutdown();
	ecore_shutdown();

	return 0;
}
