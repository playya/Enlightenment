#include <stdlib.h>
#include <stdio.h>

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#include "esmart_dvi.h"


static void app_resize(Ecore_Evas *ee);
static int app_signal_exit(void *data, int ev_type, void *ev);
static void app_delete_request(Ecore_Evas *ee);


int
main (int argc, char *argv[])
{
  Ecore_Evas *ee;
  Evas *evas;
  Evas_Object *o;
  char *filename;
  int page_number;

  if (argc < 3)
    {
      printf ("\nUsage: %s filename page_number\n\n", argv[0]);
      exit (-1);
    }

  printf ("[DVI] version : %s\n", edvi_version_get ());
  if (!edvi_init (300, "cx", 4,
                  1.0, 1.0,
                  0, 0, 0, 0, 255, 255, 255)) {
    return -1;
  }

  filename = argv[1];
  sscanf (argv[2], "%d", &page_number);

  if (!evas_init()) {
    edvi_shutdown ();
    return -1;
  }
  if (!ecore_init()) {
    evas_shutdown ();
    edvi_shutdown ();
    return -1;
  }
  if (!ecore_evas_init()) {
    ecore_shutdown ();
    evas_shutdown ();
    edvi_shutdown ();
    return -1;
  }

  ee = ecore_evas_software_x11_new(NULL, 0,  0, 0, 600, 850);
  ecore_event_handler_add (ECORE_EVENT_SIGNAL_EXIT, app_signal_exit, NULL);
  ecore_evas_callback_delete_request_set(ee, app_delete_request);
  ecore_evas_title_set(ee, "Esmart Dvi Test");
  ecore_evas_name_class_set(ee, "esmart_dvi_test", "test_esmart_dvi");
  ecore_evas_callback_resize_set(ee, app_resize);
  ecore_evas_show(ee);
  
  evas = ecore_evas_get(ee);

  o = esmart_dvi_add (evas);
  if (!esmart_dvi_init (o)) {
    ecore_evas_shutdown ();
    ecore_shutdown ();
    evas_shutdown ();
    return -1;
  }
  esmart_dvi_file_set (o, filename);
  esmart_dvi_page_set (o, page_number);
  esmart_dvi_scale_set (o, 0.5, 0.5);
  evas_object_move (o, 0, 0);
  evas_object_show (o);

  ecore_main_loop_begin ();
   
  ecore_evas_shutdown ();
  ecore_shutdown ();
  evas_shutdown ();
  edvi_shutdown ();

  return 0;
}

static void
app_resize(Ecore_Evas *ee)
{
   Evas_Coord w, h;
   Evas *evas;
   
   evas = ecore_evas_get(ee);
   evas_output_viewport_get(evas, NULL, NULL, &w, &h);
/*    bg_resize(w, h); */
}

static int
app_signal_exit(void *data, int ev_type, void *ev)
{
   ecore_main_loop_quit();
   edvi_shutdown ();
   return 1;
}

static void
app_delete_request(Ecore_Evas *ee)
{
   ecore_main_loop_quit();
   edvi_shutdown ();
}
