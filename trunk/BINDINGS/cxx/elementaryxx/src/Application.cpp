#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include "../include/elementaryxx/Application.h"

/* EFL */
#include <Elementary.h>

using namespace std;

namespace Elmxx {

Application::Application (int argc, char **argv)
{
  elm_init (argc, argv);
}

Application::~Application ()
{
  elm_shutdown ();
}

void Application::run ()
{
  elm_run ();
}

void Application::exit ()
{
  elm_exit ();
}

double Application::getScale ()
{
  return elm_scale_get ();
}

void Application::setScale (double scale)
{
  elm_scale_set (scale);
}
  
} // end namespace Elmxx
