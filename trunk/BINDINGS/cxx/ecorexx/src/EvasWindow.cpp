#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* EFLxx */
#include <evasxx/Canvas.h>
#include "../include/ecorexx/EvasWindow.h"
#include "../include/ecorexx/XWindow.h"

#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fb.h>

//===============================================================================================
// EvasWindow
//===============================================================================================

namespace Ecorexx {

#define ECORE_EVAS_CALLBACK( name, func )  \
static void name( Ecore_Evas* ee )         \
{                                          \
  eApp->getMainWindow()->func();              \
}

ECORE_EVAS_CALLBACK( callback_resize_dispatcher, resizeEvent )
ECORE_EVAS_CALLBACK( callback_move_dispatcher, moveEvent )
ECORE_EVAS_CALLBACK( callback_show_dispatcher, showEvent )
ECORE_EVAS_CALLBACK( callback_hide_dispatcher, hideEvent )
ECORE_EVAS_CALLBACK( callback_delete_request_dispatcher, deleteRequestEvent )
ECORE_EVAS_CALLBACK( callback_destroy_dispatcher, destroyEvent )
ECORE_EVAS_CALLBACK( callback_focus_in_dispatcher, focusInEvent )
ECORE_EVAS_CALLBACK( callback_focus_out_dispatcher, focusOutEvent )
ECORE_EVAS_CALLBACK( callback_mouse_in_dispatcher, mouseInEvent )
ECORE_EVAS_CALLBACK( callback_mouse_out_dispatcher, mouseOutEvent )
ECORE_EVAS_CALLBACK( callback_pre_render_dispatcher, preRenderEvent )
ECORE_EVAS_CALLBACK( callback_post_render_dispatcher, postRenderEvent )

EvasWindow::EvasWindow()
    :Trackable( "EvasWindow" )
{
  // initialize the Evas backend
  ecore_evas_init ();
}


EvasWindow::~EvasWindow()
{
  Dout( dc::notice, "EvasWindow::~EvasWindow" );

  delete _canvas;

  // shutdown the Evas backend (if internal count is zero)
  ecore_evas_shutdown ();
}

Evasxx::Canvas &EvasWindow::getCanvas() const 
{
  // be sure that we never dereference a NULL pointer. If we have a NULL 
  // pointer here than something is really broken!
  assert (_canvas);
  
  return *_canvas;
};

void EvasWindow::setEventEnabled( Event e, bool on )
{
  switch ( e )
  {
  case Resize:
    ecore_evas_callback_resize_set( _ee, on? &callback_resize_dispatcher:0 );
    break;
  case Move:
    ecore_evas_callback_move_set( _ee, on? &callback_move_dispatcher:0 );
    break;
  case Show:
    ecore_evas_callback_show_set( _ee, on? &callback_show_dispatcher:0 );
    break;
  case Hide:
    ecore_evas_callback_hide_set( _ee, on? &callback_hide_dispatcher:0 );
    break;
  case DeleteRequest:
    ecore_evas_callback_delete_request_set( _ee, on? &callback_delete_request_dispatcher:0 );
    break;
  case Destroy:
    ecore_evas_callback_destroy_set( _ee, on? &callback_destroy_dispatcher:0 );
    break;
  case FocusIn:
    ecore_evas_callback_focus_in_set( _ee, on? &callback_focus_in_dispatcher:0 );
    break;
  case FocusOut:
    ecore_evas_callback_focus_out_set( _ee, on? &callback_focus_out_dispatcher:0 );
    break;
  case MouseIn:
    ecore_evas_callback_mouse_in_set( _ee, on? &callback_mouse_in_dispatcher:0 );
    break;
  case MouseOut:
    ecore_evas_callback_mouse_out_set( _ee, on? &callback_mouse_out_dispatcher:0 );
    break;
  case PreRender:
    ecore_evas_callback_pre_render_set( _ee, on? &callback_pre_render_dispatcher:0 );
    break;
  case PostRender:
    ecore_evas_callback_post_render_set( _ee, on? &callback_post_render_dispatcher:0 );
    break;
  default:
    assert( 0 );
  }
}

void EvasWindow::resizeEvent()
{
  int x, y, w, h;
  ecore_evas_geometry_get( _ee, &x, &y, &w, &h );
  Dout( dc::notice, "EvasWindow::resizeEvent( " << x << "," << y << "*" << w << "," << h << " )" );
  Dout( dc::notice, " - evas viewport size = " << _canvas->viewport() );
  Dout( dc::notice, " - evas output   size = " << _canvas->size() );
  //FIXME: Resize manually if not fullscreen
}

void EvasWindow::moveEvent()
{
  Dout( dc::notice, "EvasWindow::moveEvent()" );
}

void EvasWindow::showEvent()
{
  Dout( dc::notice, "EvasWindow::showEvent()" );
}

void EvasWindow::hideEvent()
{
  Dout( dc::notice, "EvasWindow::hideEvent()" );
}


void EvasWindow::deleteRequestEvent()
{
  Dout( dc::notice, "EvasWindow::deleteRequestEvent()" );
  if ( canClose() ) eApp->quit();
}

void EvasWindow::destroyEvent()
{
  Dout( dc::notice, "EvasWindow::destroyEvent()" );
}


void EvasWindow::focusInEvent()
{
  Dout( dc::notice, "EvasWindow::focusInEvent()" );
}

void EvasWindow::focusOutEvent()
{
  Dout( dc::notice, "EvasWindow::focusOutEvent()" );
}

void EvasWindow::mouseInEvent()
{
  Dout( dc::notice, "EvasWindow::mouseInEvent()" );
}

void EvasWindow::mouseOutEvent()
{
  Dout( dc::notice, "EvasWindow::mouseOutEvent()" );
}

void EvasWindow::preRenderEvent()
{
  Dout( dc::notice, "EvasWindow::preRenderEvent()" );
}

void EvasWindow::postRenderEvent()
{
  Dout( dc::notice, "EvasWindow::postRenderEvent()" );
}

bool EvasWindow::isEngineTypeSupported (EngineType et)
{
  return ecore_evas_engine_type_supported_get (static_cast <Ecore_Evas_Engine_Type> (et));
}

bool EvasWindow::canClose() const
{
  return true;
}

void EvasWindow::move( const Eflxx::Point& point )
{
  ecore_evas_move( _ee, point.x(), point.y() );
}

void EvasWindow::resize( const Eflxx::Size& size )
{
  ecore_evas_resize( _ee, size.width(), size.height() );
}

void EvasWindow::setGeometry( const Eflxx::Rect& rect )
{
  ecore_evas_move_resize( _ee, rect.x(), rect.y(), rect.width(), rect.height() );
}

Eflxx::Rect EvasWindow::geometry() const
{
  int x, y, width, height;
  ecore_evas_geometry_get( _ee, &x, &y, &width, &height );
  return Eflxx::Rect( x, y, width, height );
}

void EvasWindow::setRotation( int rot )
{
  ecore_evas_rotation_set( _ee, rot );
}

int EvasWindow::rotation() const
{
  return ecore_evas_rotation_get( _ee );
}

void EvasWindow::setShaped( bool b )
{
  ecore_evas_shaped_set( _ee, b );
}

bool EvasWindow::isShaped() const
{
  return ecore_evas_shaped_get( _ee );
}

void EvasWindow::setAlpha( bool alpha )
{
  ecore_evas_alpha_set( _ee, alpha );
}

bool EvasWindow::hasAlpha () const
{
  return ecore_evas_alpha_get( _ee );
}

void EvasWindow::show()
{
  ecore_evas_show( _ee );
}
void EvasWindow::hide()
{
  ecore_evas_hide( _ee );
}
void EvasWindow::raise()
{
  ecore_evas_raise( _ee );
}

void EvasWindow::lower()
{
  ecore_evas_lower( _ee );
}

void EvasWindow::setTitle( const std::string &title )
{
  ecore_evas_title_set( _ee, title.c_str () );
}

const std::string EvasWindow::getTitle() const
{
  return ecore_evas_title_get( _ee );
}

void EvasWindow::setMinimumSize( const Eflxx::Size& size )
{
  ecore_evas_size_min_set( _ee, size.width(), size.height() );
}

Eflxx::Size EvasWindow::getMinimumSize() const
{
  int x;
  int y;
  ecore_evas_size_min_get( _ee, &x, &y );
  return Eflxx::Size( x, y );
}

void EvasWindow::setMaximumSize( const Eflxx::Size& size )
{
  ecore_evas_size_max_set( _ee, size.width(), size.height() );
}

Eflxx::Size EvasWindow::getMaximumSize() const
{
  int x;
  int y;
  ecore_evas_size_max_get( _ee, &x, &y );
  return Eflxx::Size( x, y );
}

void EvasWindow::setBaseSize( const Eflxx::Size& size )
{
  ecore_evas_size_base_set( _ee, size.width(), size.height() );
}

Eflxx::Size EvasWindow::getBaseSize() const
{
  int x;
  int y;
  ecore_evas_size_base_get( _ee, &x, &y );
  return Eflxx::Size( x, y );
}

void EvasWindow::setStepSize( const Eflxx::Size& size )
{
  ecore_evas_size_step_set( _ee, size.width(), size.height() );
}

Eflxx::Size EvasWindow::getStepSize() const
{
  int x;
  int y;
  ecore_evas_size_step_get( _ee, &x, &y );
  return Eflxx::Size( x, y );
}

void EvasWindow::setCursor( const char* file, int layer, int hot_x, int hot_y )
{
  ecore_evas_cursor_set( _ee, file, layer, hot_x, hot_y );
}

void EvasWindow::setLayer( int layer )
{
  ecore_evas_layer_set( _ee, layer );
}

int EvasWindow::getLayer() const
{
  return ecore_evas_layer_get( _ee );
}

void EvasWindow::setFocus( int on )
{
  ecore_evas_focus_set( _ee, on );
}

int EvasWindow::hasFocus() const
{
  return ecore_evas_focus_get( _ee );
}

void EvasWindow::setIconified( int on )
{
  ecore_evas_iconified_set( _ee, on );
}

int EvasWindow::isIconified() const
{
  return ecore_evas_iconified_get( _ee );
}

void EvasWindow::setBorderless( int on )
{
  ecore_evas_borderless_set( _ee, on );
}

int EvasWindow::isBorderless() const
{
  return ecore_evas_borderless_get( _ee );
}

void EvasWindow::setOverride( int on )
{
  ecore_evas_override_set( _ee, on );
}

int EvasWindow::isOverride() const
{
  return ecore_evas_override_get( _ee );
}

void EvasWindow::setMaximized( int on )
{
  ecore_evas_maximized_set( _ee, on );
}
int EvasWindow::isMaximized() const
{
  return ecore_evas_maximized_get( _ee );
}

void EvasWindow::setFullscreen( int on )
{
  ecore_evas_fullscreen_set( _ee, on );
}

int EvasWindow::isFullscreen() const
{
  return ecore_evas_fullscreen_get( _ee );
}

void EvasWindow::setAvoidDamage( Ecore_Evas_Avoid_Damage_Type value )
{
  ecore_evas_avoid_damage_set( _ee, value );
}

Ecore_Evas_Avoid_Damage_Type EvasWindow::AvoidDamage() const
{
  return ecore_evas_avoid_damage_get( _ee );
}

void EvasWindow::setWithdrawn( int on )
{
  ecore_evas_withdrawn_set( _ee, on );
}

int EvasWindow::isWithdrawn() const
{
  return ecore_evas_withdrawn_get( _ee );
}

void EvasWindow::setSticky( int on )
{
  ecore_evas_sticky_set( _ee, on );
}

int EvasWindow::isSticky() const
{
  return ecore_evas_sticky_get( _ee );
}

/* PRIVATE */
EvasWindow* EvasWindow::objectLink( Ecore_Evas* ee )
{
  return static_cast<EvasWindow*>( ecore_evas_data_get( ee, "obj_c++" ) );
}

EvasWindowSoftwareX11::EvasWindowSoftwareX11( const Eflxx::Size &size, const char* display )
    :EvasWindow()
{
  Dout( dc::notice, "EvasWindow::EvasWindowSoftwareX11" );
  Application::application()->setMainWindow( this );

  /*if ( ::getenv( "EFL_DISPLAY" ) ) display = ::getenv( "EFL_DISPLAY" );
  if ( ::getenv( "EFL_WIDTH" ) ) size.width (atoi( ::getenv( "EFL_WIDTH" ) ));
  if ( ::getenv( "EFL_HEIGHT" ) ) size.height (atoi( ::getenv( "EFL_HEIGHT" ) ));*/

  Dout( dc::notice, "- detected display string '" << ( display ? display:"<null>" ) << "' - starting X11 engine" );
  //FIXME: Should we care about positioning? 0, 0 for now
  _ee = ecore_evas_software_x11_new( const_cast<char*>( display ), 0, 0, 0, size.width (), size.height () );

  ecore_evas_title_set( _ee, eApp->getName().c_str() );
  ecore_evas_borderless_set( _ee, 0 );

  _canvas = new Evasxx::Canvas( ecore_evas_get( _ee ) );

  /* Set up magic object back link */
  ecore_evas_data_set( _ee, "obj_c++", this );

  /* Set up default callbacks */
  setEventEnabled( Resize, true );
  setEventEnabled( DeleteRequest, true );
}

EvasWindowSoftwareX11::~EvasWindowSoftwareX11()
{
}

XWindow *EvasWindowSoftwareX11::getXWindow()
{
  Ecore_X_Window exw = ecore_evas_software_x11_window_get( _ee );
  XWindow *exwin = new XWindow( exw );

  return exwin;
}

EvasWindowGLX11::EvasWindowGLX11( const Eflxx::Size &size, const char* display )
    :EvasWindow()
{
  Dout( dc::notice, "EvasWindow::EvasWindowGL" );
  Application::application()->setMainWindow( this );

  /*if ( ::getenv( "EFL_DISPLAY" ) ) display = ::getenv( "EFL_DISPLAY" );
  if ( ::getenv( "EFL_WIDTH" ) ) size.width (atoi( ::getenv( "EFL_WIDTH" ) ));
  if ( ::getenv( "EFL_HEIGHT" ) ) size.height (atoi( ::getenv( "EFL_HEIGHT" ) ));*/

  Dout( dc::notice, "- detected display string '" << ( display ? display:"<null>" ) << "' - starting X11 engine" );
  //FIXME: Should we care about positioning? 0, 0 for now
  _ee = ecore_evas_gl_x11_new( const_cast<char*>( display ), 0, 0, 0, size.width (), size.height () );

  ecore_evas_title_set( _ee, eApp->getName().c_str() );
  ecore_evas_borderless_set( _ee, 0 );

  _canvas = new Evasxx::Canvas( ecore_evas_get( _ee ) );

  /* Set up magic object back link */
  ecore_evas_data_set( _ee, "obj_c++", this );

  /* Set up default callbacks */
  setEventEnabled( Resize, true );
  setEventEnabled( DeleteRequest, true );
}

EvasWindowGLX11::~EvasWindowGLX11()
{

}

XWindow *EvasWindowGLX11::getXWindow()
{
  Ecore_X_Window exw = ecore_evas_gl_x11_window_get( _ee );
  XWindow *exwin = new XWindow( exw );

  return exwin;
}

EvasWindowXRenderX11::EvasWindowXRenderX11( const Eflxx::Size &size, const char* display )
    :EvasWindow()
{
  Dout( dc::notice, "EvasWindow::EvasWindowGL" );
  Application::application()->setMainWindow( this );

  /*if ( ::getenv( "EFL_DISPLAY" ) ) display = ::getenv( "EFL_DISPLAY" );
  if ( ::getenv( "EFL_WIDTH" ) ) size.width (atoi( ::getenv( "EFL_WIDTH" ) ));
  if ( ::getenv( "EFL_HEIGHT" ) ) size.height (atoi( ::getenv( "EFL_HEIGHT" ) ));*/

  Dout( dc::notice, "- detected display string '" << ( display ? display:"<null>" ) << "' - starting X11 engine" );
  //FIXME: Should we care about positioning? 0, 0 for now
  _ee = ecore_evas_xrender_x11_new( const_cast<char*>( display ), 0, 0, 0, size.width (), size.height () );

  ecore_evas_title_set( _ee, eApp->getName().c_str() );
  ecore_evas_borderless_set( _ee, 0 );

  _canvas = new Evasxx::Canvas( ecore_evas_get( _ee ) );

  /* Set up magic object back link */
  ecore_evas_data_set( _ee, "obj_c++", this );

  /* Set up default callbacks */
  setEventEnabled( Resize, true );
  setEventEnabled( DeleteRequest, true );
}

EvasWindowXRenderX11::~EvasWindowXRenderX11()
{
}

XWindow *EvasWindowXRenderX11::getXWindow()
{
  Ecore_X_Window exw = ecore_evas_xrender_x11_window_get( _ee );
  XWindow *exwin = new XWindow( exw );

  return exwin;
}

// TODO: Possible without #ifdef stuff?
EvasWindowFB::EvasWindowFB( const Eflxx::Size &size, const char* display, int rotation )
    :EvasWindow()
{
  Dout( dc::notice, "EvasWindow::EvasWindowFB" );
  Application::application()->setMainWindow( this );

  /*if ( ::getenv( "EFL_DISPLAY" ) ) display = ::getenv( "EFL_DISPLAY" );
  if ( ::getenv( "EFL_WIDTH" ) ) size.width (atoi( ::getenv( "EFL_WIDTH" ) ));
  if ( ::getenv( "EFL_HEIGHT" ) ) size.height (atoi( ::getenv( "EFL_HEIGHT" ) ));
  if ( ::getenv( "EFL_ROTATION" ) ) rotation = atoi( ::getenv( "EFL_ROTATION" ) );*/

  if ( display /*&& ::strstr( display, "/dev/fb" )*/ )
  {
#ifdef ENABLE_EFLPP_FB
    int fb_dev_fd = ::open( display, O_RDONLY );
    if ( fb_dev_fd < 0 )
    {
      fprintf(stderr,"Can't open display '%s': %s\n", display, strerror(errno));
      exit( 1 );
    }

    struct fb_var_screeninfo fb_vinfo;
    struct fb_fix_screeninfo fb_finfo;

    // read VScreen info from fb
    if ( ioctl( fb_dev_fd, FBIOGET_VSCREENINFO, &fb_vinfo ) )
    {
      fprintf(stderr,"Can't get VSCREENINFO: %s\n", strerror(errno));
      exit(1);
    }

    // readFScreen info from fb
    if ( ioctl( fb_dev_fd, FBIOGET_FSCREENINFO, &fb_finfo ) )
    {
      fprintf(stderr,"Can't get FSCREENINFO: %s\n", strerror(errno));
      exit(1);
    }

    Dout( dc::notice, "- using display '" << display << "' [" << fb_finfo.id << "] - " << fb_vinfo.xres << "x" << fb_vinfo.yres << "@" << rotation );
    width = ( rotation % 180 ) ? fb_vinfo.yres : fb_vinfo.xres;
    height = ( rotation % 180 ) ? fb_vinfo.xres : fb_vinfo.yres;
    Dout( dc::notice, "- using size (after rotating) " << size.width () << "x" << size.height () );
#ifdef ECORE_FB_NO_ROTATION_BUG
    //FIXME EFL BUG: initial rotation is not taken into account for evas calculation
    _ee = ecore_evas_fb_new( const_cast<char*>( display ), rotation, 50, 50 );
#else
    _ee = ecore_evas_fb_new( const_cast<char*>( display ), 0, 50, 50 ); // start with rotation 0 to workaround bug
#endif
    ecore_evas_fullscreen_set( _ee, 1 ); // fullscreen is default to get auto resize on changing rotation
    ecore_evas_rotation_set( _ee, rotation ); // force resize
#else
    printf("FB engine not enabled\n");
#endif
  }

  ecore_evas_title_set( _ee, eApp->getName().c_str() );
  ecore_evas_borderless_set( _ee, 0 );

  _canvas = new Evasxx::Canvas( ecore_evas_get( _ee ) );

  /* Set up magic object back link */
  ecore_evas_data_set( _ee, "obj_c++", this );

  /* Set up default callbacks */
  setEventEnabled( Resize, true );
  setEventEnabled( DeleteRequest, true );
}

EvasWindowFB::~EvasWindowFB()
{
}

} // end namespace Ecorexx
