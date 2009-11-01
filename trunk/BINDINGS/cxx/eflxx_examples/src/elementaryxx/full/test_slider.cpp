#include "test.h"

void test_slider (void *data, Evas_Object *obj, void *event_info)
{
  ElmIcon *ic = NULL;
  ElmSlider *sl = NULL;

  ElmWindow *win = ElmWindow::factory ("slider", ELM_WIN_BASIC);
  win->setTitle ("Slider");
  win->setAutoDel (true);

  ElmBackground *bg = ElmBackground::factory (*win);
  win->addObjectResize (*bg);
  bg->setWeightHintSize (EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  bg->show ();

  ElmBox *bx = ElmBox::factory (*win);
  win->addObjectResize (*bx);
  bx->setWeightHintSize (EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  bx->show ();

  ic = ElmIcon::factory (*win);
  ic->setFile (searchPixmapFile ("elementaryxx/logo_small.png"));
  ic->setAspectHintSize (EVAS_ASPECT_CONTROL_VERTICAL, Size (1, 1));
  ic->show ();
  
  sl = ElmSlider::factory (*win);
  sl->setLabel ("Label");
  sl->setIcon (*ic);
  sl->setUnitFormat ("%1.1f units");
  sl->setSpanSize (120);
  sl->setAlignHintSize (EVAS_HINT_FILL, 0.5);
  sl->setWeightHintSize (EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  bx->packEnd (*sl);
  sl->show ();

  sl = ElmSlider::factory (*win);
  sl->setLabel ("Label 2");
  sl->setSpanSize (80);
  //sl->setAlignHintSize (EVAS_HINT_FILL, 0.5);
  //sl->setWeightHintSize (EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  sl->setIndicatorFormat ("%3.0f");
  sl->setMinMax (50, 150);
  sl->setValue (80);
  sl->setInverted (true);
  sl->setAlignHintSize (0.5, 0.5);
  sl->setWeightHintSize (0.0, 0.0);
  bx->packEnd (*sl);
  sl->show ();

  sl = ElmSlider::factory (*win);
  sl->setLabel ("Label 3");
  sl->setUnitFormat ("units");
  sl->setSpanSize (40);
  sl->setAlignHintSize (EVAS_HINT_FILL, 0.5);
  sl->setWeightHintSize (EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  sl->setIndicatorFormat ("%3.0f");
  sl->setMinMax (50, 150);
  sl->setValue (80);
  sl->setInverted (true);
  sl->setScale (2.0);
  bx->packEnd (*sl);
  sl->show ();

  ic = ElmIcon::factory (*win);
  ic->setFile (searchPixmapFile ("elementaryxx/logo_small.png"));
  ic->setAspectHintSize (EVAS_ASPECT_CONTROL_HORIZONTAL, Size (1, 1));
  ic->show ();

  sl = ElmSlider::factory (*win);
  sl->setLabel ("Label 4");
  sl->setIcon (*ic);
  sl->setUnitFormat ("units");
  sl->setSpanSize (60);
  sl->setAlignHintSize (0.5L, EVAS_HINT_FILL);
  sl->setWeightHintSize (0.0, EVAS_HINT_EXPAND);
  sl->setIndicatorFormat ("%1.1f");
  sl->setValue (0.2);
  sl->setScale (1.0);
  sl->setHorizontal (false);
  bx->packEnd (*sl);
  sl->show ();

  win->show ();
}
