#ifndef ECOREXX_APPLICATION_H
#define ECOREXX_APPLICATION_H

/* EFLxx */
#include "EvasWindow.h"

/* EFL */
#include <Evas.h>
#include <Ecore_Config.h>
#include <Ecore_Data.h>
#include <Ecore_Ipc.h>
#include <Ecore_Txt.h>
#include <Ecore_Con.h>
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Ecore_Job.h>

/* STD */
#include <string>
#include <list>

#define eApp Application::application()

/**
 * C++ Wrapper for the Enlightenment Core Library (Ecore)
 *
 * @author Michael 'Mickey' Lauer <mickey@Vanille.de>
 */

namespace Ecorexx {

/* Forward declarations */
class EvasWindow;
class Application;
class Callback;
class Config;
class Timer;

class Callback
{
public:
  Callback( Application* object ) : o(object) {};
  ~Callback() {};
private:
  Application* o;
};

//===============================================================================================
// Application
//===============================================================================================

class Application : public Eflxx::Trackable
{
public:
  Application( int argc, const char** argv, const std::string& name );
  virtual ~Application();

  static Application* application();
  
  std::string getName() const;

  /* Main Window */
  void setMainWindow( EvasWindow* );
  EvasWindow* getMainWindow() const;

  /* Main Loop */
  static void exec();
  static void processOneEvent();
  static void quit();

  /* Timer */
  void startTimer( double seconds );
  virtual void timerEvent( Timer* ) {};

  /* Font */
  /*EvasFont font();
  EvasFont fixedFont();
  void setFont( const EvasFont& );
  void setFixedFont( const EvasFont& );*/

  /* Configuration */
  Config* getConfig();

private:
  const std::string _binary;
  std::string _name;
  static Application* _instance;
  EvasWindow* _mainWindow;
  //EvasFont* _font;
  //EvasFont* _fixedFont;
  Config* _config;

private:
  Application();
  bool operator=( const Application& );
  bool operator==( const Application& );
};

} // end namespace Ecorexx

#endif // ECOREXX_APPLICATION_H
