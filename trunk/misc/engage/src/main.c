#include "engage.h"
#include "Ecore_X.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif

static int      handle_idle(void *data);
bool            need_redraw = false;

static int
exit_cb(void *data, int type, void *event)
{
  ecore_main_loop_quit();
  return (0);
}

int
main(int argc, char **argv)
{
  ecore_init();
  ecore_x_init(NULL);
  ecore_config_init("engage");

  od_config_init();

  ecore_app_args_set(argc, (const char **) argv);
  ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_cb, NULL);
  ecore_evas_init();
  edje_init();
  edje_frametime_set(1.0 / 30.0);

  od_window_init();
  od_dock_init();

  od_icon_add_path(options.icon_path);
  ecore_config_evas_font_path_apply(evas);
  userconfig_load();

  od_icon_add_path(options.icon_path);

  od_dock_icons_update_begin();
  ecore_idle_enterer_add(handle_idle, NULL);
  ecore_main_loop_begin();

  edje_shutdown();
  ecore_evas_shutdown();
  ecore_config_save();
  ecore_config_exit();
  ecore_x_shutdown();
  ecore_shutdown();

  return 0;
}

int
handle_idle(void *data)
{
  if (need_redraw) {
    od_dock_redraw(NULL);
    evas_render(evas);
    need_redraw = false;
  }
  return 0;
}
