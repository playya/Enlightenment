#ifndef ECORETIMER_H
#define ECORETIMER_H

/* EFLxx */
#include <eflxx/Common.h>

/* EFL */
#include <Ecore.h>

namespace Ecorexx {

class Timer
{
  typedef sigc::signal <void,Timer*> Signal;
  typedef sigc::signal <void,Timer*> Loop;
  typedef sigc::slot1  <void,Timer*> Slot;

public:
  Timer( double seconds, bool singleshot = false );
  virtual ~Timer();

  //virtual void tick();

  static Timer* singleShot( double seconds, const Timer::Slot& ); // TODO: CountedPtr
  
  void del ();
  
  void setInterval (double interval);
  
  double getInterval ();
  
  void freeze ();
  
  void thaw ();
  
  void delay (double add);
  
  double getPending ();

  static double getPrecision ();
    
  static void setPrecision (double precision);

  
public: /* signals */
  Timer::Signal timeout;
  Timer::Loop loop;
  
private:
  Ecore_Timer* _et;
  bool _ss;

  static Eina_Bool __dispatcher( void* data );
};

} // end namespace Ecorexx

#endif // ECORETIMER_H
