#ifndef ELM_SLIDER_H
#define ELM_SLIDER_H

/* STL */
#include <string>

/* EFL */
#include <Elementary.h>

/* ELFxx */
#include "ElmWidget.h"

namespace efl {
  
/*!
 * smart callbacks called:
 * "changed" - when the slider value changes
 * "delay,changed" - when the slider value changed, but a small time after a 
 *                   change (use this if you only want to respond to a change 
 *                   once the slider is held still for a short while).
 */
class ElmSlider : public ElmWidget
{
public:
  static ElmSlider *factory (EvasObject &parent);

  void setLabel (const std::string &label);
  
  void setIcon (const EvasObject &icon);
  
  void setSpanSize (Evas_Coord size);
  
  void setUnitFormat (const std::string &format);
  
  void setIndicatorFormat (const std::string &indicator);
  
  void setHorizontal (bool horizontal);
  
  void setMinMax (double min, double max);
  
  void setValue (double val);
  
  double getValue () const;
  
  void setInverted (bool inverted);

private:
  ElmSlider (); // forbid standard constructor
  ElmSlider (const ElmSlider&); // forbid copy constructor
  ElmSlider (EvasObject &parent); // private construction -> use factory ()
  ~ElmSlider (); // forbid direct delete -> use ElmWidget::destroy()
};

} // end namespace efl

#endif // ELM_SLIDER_H
