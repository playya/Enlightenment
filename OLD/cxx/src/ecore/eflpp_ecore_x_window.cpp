#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include "eflpp_ecore_x_window.h"

//===============================================================================================
// EcoreXWindow
//===============================================================================================

namespace efl {

EcoreXWindow::EcoreXWindow( Ecore_X_Window exwin )
{
    _exwin = exwin;
}

EcoreXWindow::~EcoreXWindow()
{
  
}

void EcoreXWindow::setNetWMWindowType( EcoreXWindowType type )
{
    ecore_x_netwm_window_type_set( _exwin, static_cast <Ecore_X_Window_Type> (type) );
}

//void EcoreXWindow::getNetWMWindowTypePrefetch()
//void EcoreXWindow::getNetWMWindowTypeFetch ()
//EAPI void                ecore_x_netwm_window_type_get_prefetch(Ecore_X_Window window);
//EAPI void                ecore_x_netwm_window_type_get_fetch(void);
    
bool EcoreXWindow::getNetWMWindowType( EcoreXWindowType &outType )
{
    Ecore_X_Window_Type eWinType;
    bool ret = ecore_x_netwm_window_type_get( _exwin, &eWinType );
    outType = static_cast <EcoreXWindowType> (eWinType);
  
    return ret;
}

bool EcoreXWindow::getDPMSEnabled ()
{
  return ecore_x_dpms_enabled_get ();
}

void EcoreXWindow::setDPMSEnabled (bool enabled)
{
  ecore_x_dpms_enabled_set (enabled);
}

} // end namespace efl
