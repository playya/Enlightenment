#ifndef ELM_FRAME_H
#define ELM_FRAME_H

/* STL */
#include <string>

/* EFL */
#include <Elementary.h>

/* ELFxx */
#include "ElmWidget.h"

namespace efl {

class ElmFrame : public ElmWidget
{
public:
  virtual ~ElmFrame ();
  
  static ElmFrame *factory (EvasObject &parent);

  void setLabel (const std::string &label);

  void setContent (const EvasObject &content);

 /*! 
  * available styles:
  * default
  * pad_small
  * pad_medium
  * pad_large
  * pad_huge
  * outdent_top
  * outdent_bottom
  */
  void setStyle (const std::string &style);
  
private:
  ElmFrame (); // forbid standard constructor
  ElmFrame (const ElmFrame&); // forbid copy constructor
  ElmFrame (EvasObject &parent); // private construction -> use factory ()
};

} // end namespace efl

#endif // ELM_FRAME_H
