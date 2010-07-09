#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include "../include/elementaryxx/Label.h"

using namespace std;

namespace Elmxx {

Label::Label (Evasxx::Object &parent)
{
  o = elm_label_add (parent.obj ());
  
  elmInit ();
}

Label::~Label () {}

Label *Label::factory (Evasxx::Object &parent)
{
  return new Label (parent);
}

void Label::setLabel (const std::string &label)
{
  elm_label_label_set (o, label.c_str ());
}

const std::string Label::getLabel () const
{
  return elm_label_label_get (o);
}

void Label::setLineWrap (bool wrap)
{
  elm_label_line_wrap_set (o, wrap);
}

bool Label::getLineWrap () const
{
  return elm_label_line_wrap_get (o);
}

void Label::setWrapWidth (Evas_Coord w)
{
  elm_label_wrap_width_set (o, w);
}

Evas_Coord Label::getWrapWidth () const
{
  return elm_label_wrap_width_get (o);
}

} // end namespace Elmxx
