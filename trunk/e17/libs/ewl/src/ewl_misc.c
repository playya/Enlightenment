
#include <Ewl.h>
#include "getopt.h"

void ewl_idle_render(void *data);

extern Ewd_List *ewl_window_list;

char *xdisplay = NULL;

static void ewl_init_parse_options(int argc, char **argv);
static void ewl_parse_option_array(int argc, char **argv);

void ewl_reread_config(int val, void *data);


void
ewl_init(int argc, char **argv)
{
	char *xdisplay = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_init_parse_options(argc, argv);

	if (!ecore_display_init(xdisplay))
	  {
		  fprintf(stderr, "ERRR: Cannot connect to X display!\n");
		  exit(-1);
	  }

	ecore_event_filter_init();
	ecore_event_signal_init();
	ecore_event_x_init();

	if (!ewl_config_init())
	  {
		  DERROR("Couldn not init config data. Exiting....");
		  exit(-1);
	  }

	if (!ewl_ev_init())
	  {
		  DERROR("Could not init event data. Exiting....");
		  exit(-1);
	  }

	if (!ewl_fx_init())
	  {
		  DERROR("Could not init fx data. Exiting....");
		  exit(-1);
	  }

	if (!ewl_theme_init())
	  {
		  DERROR("Could not init theme data. Exiting....");
		  exit(-1);
	  }

	ewl_window_list = ewd_list_new();
	ecore_event_filter_idle_handler_add(ewl_idle_render, NULL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_main(void)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_reread_config(0, NULL);

	ewl_idle_render(NULL);
	ecore_event_loop();

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_idle_render(void *data)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);

	if (!ewl_window_list)
	  {
		  DERROR("EWL has not been initialized. Exiting....\n");
		  exit(-1);
	  }

	if (ewd_list_is_empty(ewl_window_list))
		DRETURN(DLEVEL_STABLE);

	ewd_list_goto_first(ewl_window_list);

	while ((w = EWL_WIDGET(ewd_list_next(ewl_window_list))) != NULL)
	  {

		  /*
		   * If we have any unrealized windows at this point, we want to
		   * realize and configure them to layout the children correct.
		   */
		  if (!REALIZED(w))
		    {
			    ewl_widget_realize(w);
			    ewl_widget_configure(w);
		    }

		  if (w->evas)
			  evas_render(w->evas);
	  }

	DRETURN(DLEVEL_STABLE);
	data = NULL;
}

void
ewl_main_quit(void)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ecore_event_loop_quit();

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_init_parse_options(int argc, char **argv)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_parse_option_array(argc, argv);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_parse_option_array(int argc, char **argv)
{
	char stropts[] =
		"a:A:b:BcC:dD:e:f:Fg:hH:iIklL:mM:nNo:O:pPqQrR:sS:tT:uUvVwW:xXy:zZ1:2:3:4:56:78:90:";

	static struct option lopts[] = {
		{"ewl_display", 1, 0, '$'},
		{0, 0, 0, 0}
	};
	int optch = 0, cmdx = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);

	while ((optch =
		getopt_long_only(argc, argv, stropts, lopts, &cmdx)) != EOF)
	  {
		  switch (optch)
		    {
		    case 0:
			    break;
		    case '$':
			    xdisplay = optarg;
			    break;
		    default:
			    break;
		    }
	  }

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_reread_config(int val, void *data)
{
	ewl_config_reread_and_apply();

	ecore_add_event_timer("reread_config_timer", 0.5,
			      ewl_reread_config, 0, NULL);
}
