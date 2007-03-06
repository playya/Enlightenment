#include "Empower.h"
#include <Ecore_X.h>

void key_down_cb(Ewl_Widget *w, void *event, void *data)
{
	Ewl_Event_Key_Down *ev;
	
	ev = event;
	
	if(!ev->base.modifiers)
	{
		if(strcmp(ev->base.keyname, "Escape") == 0)
		{
			ewl_widget_destroy(EWL_WIDGET(win));
			ewl_main_quit();
		}
	}
}

void destroy_cb(Ewl_Widget *w, void *event, void *data)
{
	ewl_widget_destroy(EWL_WIDGET(win));
	ewl_main_quit();
}

void reveal_cb(Ewl_Widget *w, void *event, void *data)
{
	ewl_window_raise(EWL_WINDOW(win));
}

void pipe_to_sudo_cb(Ewl_Widget *w, void *event, void *data)
{	
	FILE *sudo_pipe;
	
	const char *pass = ewl_password_text_get(EWL_PASSWORD(data));

	ewl_widget_destroy(win);
	ewl_main_quit();

	if(pass)
	{
		snprintf(password, 1024, "%s", pass);
		
		pid_t pid = fork();
		
		if(pid == 0)
		{	
			sudo_pipe = popen(buf, "w");
			fprintf(sudo_pipe, "%s\n", password);
			pclose(sudo_pipe);
		}
	}
}
