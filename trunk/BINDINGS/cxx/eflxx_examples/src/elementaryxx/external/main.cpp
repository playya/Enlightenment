#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <evasxx/Evasxx.h>
#include <elementaryxx/Elementaryxx.h>
#include <edjexx/Edjexx.h>
#include "../../common/searchFile.h"

#include <memory>
#include <cassert>

using namespace std;
using namespace Eflxx;
using namespace Elmxx;

static void
my_win_del(Evasxx::Object &obj, void *event_info)
{
  /* called when my_win_main is requested to be deleted */
  Application::exit(); /* exit the program's main loop that runs in elm_run() */
}

static void testFunc (Evasxx::Object &obj, void *event_info)
{
  cout << "sub-object-del" << endl;
}

int main (int argc, char **argv)
{
  Application elmApp (argc, argv);

  Window *elmWin = Window::factory ("window1", ELM_WIN_BASIC);
  elmWin->getEventSignal ("delete-request")->connect (sigc::ptr_fun (&my_win_del));
  
  Background *bg = Background::factory (*elmWin);

  Evas *e = evas_object_evas_get(bg->obj ());

  // TODO: CountedPtr
  Evasxx::Canvas *ec = Evasxx::Canvas::wrap (e);

  // TODO: CountedPtr
  Edjexx::Object *edje = new Edjexx::Object (*ec, searchEdjeFile ("elementaryxx-test_external.edj"), "main");

  elmWin->resize (Eflxx::Size (620, 500));
  edje->setLayer (0);
  edje->show ();

  Eflxx::CountedPtr <Evasxx::Object> ext_eo (edje->getPart ("Button01")->getExternalObject ());
  Elmxx::Button *button = static_cast <Elmxx::Button*> (&(*ext_eo));
  button->setLabel ("This is a changed button");

  Eflxx::CountedPtr <Evasxx::Object> ext_eo2 (edje->getPart ("List01")->getExternalObject ());
  Elmxx::List *list = static_cast <Elmxx::List*> (&(*ext_eo2));
  assert (list->append ("1. Line", NULL, NULL));
  assert (list->append ("2. Line", NULL, NULL));
  assert (list->append ("3. Line", NULL, NULL));
  assert (list->append ("4. Line", NULL, NULL));
  list->go ();

  Eflxx::CountedPtr <Evasxx::Object> ext_eo3 (edje->getPart ("Progressbar01")->getExternalObject ());
  Elmxx::Progressbar *progressbar = static_cast <Elmxx::Progressbar*> (&(*ext_eo3));
  progressbar->setLabel ("This is the status");
  progressbar->setValue (0.5);

  Eflxx::CountedPtr <Edjexx::Part> part (edje->getPart ("Slider01"));

  Edje_External_Param param;
  param.type = EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
  param.name = "value";
  param.d = 5;

  Edje_External_Param param2;
  param2.type = EDJE_EXTERNAL_PARAM_TYPE_STRING;
  param2.name = "label";
  param2.s = "Changed Slider Value";

  part->setParam (&param);
  part->setParam (&param2);

  
  bg->setWeightHintSize (1.0, 1.0);
  elmWin->addObjectResize (*bg);

  /* set size hints. a minimum size for the bg. this should propagate back
   * to the window thus limiting its size based off the bg as the bg is one
   * of the window's resize objects. */
  bg->setMinHintSize (Size (160, 160));
  /* and set a maximum size. not needed very often. normally used together
   * with evas_object_size_hint_min_set() at the same size to make a
   * window not resizable */
  bg->setMaxHintSize (Size (640, 640));
    
  elmWin->setTitle ("Elementaryxx Simple Example");
  
  elmWin->setAutoDel (true);

  elmWin->show ();
  bg->show ();

  elmApp.run ();
}
