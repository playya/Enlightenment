#ifndef ECORE_EVAS_WINDOW_H
#define ECORE_EVAS_WINDOW_H

/* EFLxx */
#include <eflxx/eflpp_common.h>
#include "EcoreApplication.h"
#include "EcoreEvasWindow.h"

/* EFL */
#include <Ecore_Evas.h>

//===============================================================================================
// EcoreEvasWindow
//===============================================================================================

namespace efl {

/* forward declarations */
class EvasCanvas;
class EcoreXWindow;

// TODO: Test for supported backends?
class EcoreEvasWindow : public Trackable
{
public:
  enum Event { Resize, Move, Show, Hide, DeleteRequest, Destroy,
               FocusIn, FocusOut, MouseIn, MouseOut, PreRender, PostRender
             };

  enum EngineType
  {
    SoftwareX11 = ECORE_EVAS_ENGINE_SOFTWARE_XLIB,
    SoftwareFB = ECORE_EVAS_ENGINE_SOFTWARE_FB,
    GLX11 = ECORE_EVAS_ENGINE_OPENGL_X11,
    SoftwareBuffer = ECORE_EVAS_ENGINE_SOFTWARE_BUFFER,
    XRenderX11 = ECORE_EVAS_ENGINE_XRENDER_X11,
    DirectFB = ECORE_EVAS_ENGINE_DIRECTFB,
    SoftwareX11_16 = ECORE_EVAS_ENGINE_SOFTWARE_16_X11,
    SDL = ECORE_EVAS_ENGINE_SOFTWARE_SDL
  };

public:
  virtual ~EcoreEvasWindow();

  EvasCanvas &getCanvas() const;

  /* Events */
  void setEventEnabled( Event, bool );
  virtual void resizeEvent();
  virtual void moveEvent();
  virtual void showEvent();
  virtual void hideEvent();
  virtual void deleteRequestEvent();
  virtual void destroyEvent();
  virtual void focusInEvent();
  virtual void focusOutEvent();
  virtual void mouseInEvent();
  virtual void mouseOutEvent();
  virtual void preRenderEvent();
  virtual void postRenderEvent();

  /*!
   * \return A boolean specifying whether the window should be closed upon a delete request or not.
   *         The default implementation returns true.
   */
  virtual bool canClose() const;

  static bool isEngineTypeSupported (EngineType et);

  /* Window Management */
  void move( const Point& );
  void resize( const Size& );
  void setGeometry( const Rect& );
  Rect geometry() const;

  void setRotation( int );
  int rotation() const;
  void setShaped( bool );
  bool isShaped() const;

  void setAlpha( bool alpha );
  bool hasAlpha () const;

  void show();
  void hide();
  void raise();
  void lower();

  void setTitle( const std::string &title );
  const std::string getTitle() const;

  // void        ecore_evas_name_class_set(Ecore_Evas *ee, const char *n, const char *c);
  // void        ecore_evas_name_class_get(Ecore_Evas *ee, const char **n, const char **c);

  void setMinimumSize( const Size& );
  Size getMinimumSize() const;
  void setMaximumSize( const Size& );
  Size getMaximumSize() const;
  void setBaseSize( const Size& );
  Size getBaseSize() const;
  void setStepSize( const Size& );
  Size getStepSize() const;

  void setCursor( const char* file, int layer, int hot_x, int hot_y );
  // void        ecore_evas_cursor_get(Ecore_Evas *ee, char **file, int *layer, int *hot_x, int *hot_y);

  void setLayer( int layer );
  int getLayer() const;

  void setFocus( int on );
  int hasFocus() const;

  void setIconified( int on );
  int isIconified() const;

  void setBorderless( int on );
  int isBorderless() const;

  void setOverride( int on );
  int isOverride() const;

  void setMaximized( int on );
  int isMaximized() const;

  void setFullscreen( int on );
  int isFullscreen() const;

  void setAvoidDamage( Ecore_Evas_Avoid_Damage_Type value );
  Ecore_Evas_Avoid_Damage_Type AvoidDamage() const;

  void setWithdrawn( int on );
  int isWithdrawn() const;

  void setSticky( int on );
  int isSticky() const;

protected:
  EvasCanvas *_canvas;
  Ecore_Evas *_ee;

  EcoreEvasWindow();
  static EcoreEvasWindow* objectLink( Ecore_Evas* );

private:
  bool operator=( const EcoreEvasWindow& );
  bool operator==( const EcoreEvasWindow& );
};

class EcoreEvasWindowSoftwareX11 : public EcoreEvasWindow
{
public:
  EcoreEvasWindowSoftwareX11( const Size &size, const char* display = 0 );
  virtual ~EcoreEvasWindowSoftwareX11();

  /*!
   * \brief Gets a pointer to the X window. The returned pointer needs to be
   *        deleted if you don't need it any longer.
   *
   * \return The EcoreXWindow.
   */
  EcoreXWindow *getXWindow();

private:
  EcoreEvasWindowSoftwareX11();
  bool operator=( const EcoreEvasWindowSoftwareX11& );
  bool operator==( const EcoreEvasWindowSoftwareX11& );
};

class EcoreEvasWindowGLX11 : public EcoreEvasWindow
{
public:
  EcoreEvasWindowGLX11( const Size &size, const char* display = 0 );
  virtual ~EcoreEvasWindowGLX11();

  /*!
   * \brief Gets a pointer to the X window. The returned pointer needs to be
   *        deleted if you don't need it any longer.
   *
   * \return The EcoreXWindow.
   */
  EcoreXWindow *getXWindow();

private:
  EcoreEvasWindowGLX11();
  bool operator=( const EcoreEvasWindowGLX11& );
  bool operator==( const EcoreEvasWindowGLX11& );
};

class EcoreEvasWindowXRenderX11 : public EcoreEvasWindow
{
public:
  EcoreEvasWindowXRenderX11( const Size &size, const char* display = 0 );
  virtual ~EcoreEvasWindowXRenderX11();

  /*!
   * \brief Gets a pointer to the X window. The returned pointer needs to be
   *        deleted if you don't need it any longer.
   *
   * \return The EcoreXWindow.
   */
  EcoreXWindow *getXWindow();

private:
  EcoreEvasWindowXRenderX11();
  bool operator=( const EcoreEvasWindowXRenderX11& );
  bool operator==( const EcoreEvasWindowXRenderX11& );
};

class EcoreEvasWindowFB : public EcoreEvasWindow
{
public:
  EcoreEvasWindowFB( const Size &size, const char* display = 0, int rotation = 0 );
  virtual ~EcoreEvasWindowFB();

  EcoreXWindow *getXWindow();

private:
  EcoreEvasWindowFB();
  bool operator=( const EcoreEvasWindowFB& );
  bool operator==( const EcoreEvasWindowFB& );
};

} // end namespace efl

#endif // ECORE_EVAS_WINDOW_H
