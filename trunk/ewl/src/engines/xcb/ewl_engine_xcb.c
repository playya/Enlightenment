/* vim: set sw=8 ts=8 sts=8 expandtab: */
#include <Ewl_Engine_Xcb.h>
#include "ewl_private.h"
#include "ewl_macros.h"
#include "ewl_debug.h"

static Ewl_Window *ee_current_pointer_grab_window = NULL;
static Ewl_Window *ee_current_key_grab_window = NULL;

/*
 * In general all of the X event handlers should find their matching window
 * with ewl_window_window_find, and not ewl_embed_canvas_window_find. If the
 * embed function is used, then we get duplicate events for apps that setup
 * their own handlers and embed EWL. The exception to this is selection events
 * such as copy/paste and DND. These events need to be handled for embedded
 * EWL, but have no equivalent in the Evas callback namespace.
 */
static int ewl_ev_x_window_expose(void *data, int type, void *_ev);
static int ewl_ev_x_window_configure(void *data, int type, void *_ev);
static int ewl_ev_x_window_delete(void *data, int type, void *_ev);
static int ewl_ev_x_key_down(void *data, int type, void *_ev);
static int ewl_ev_x_key_up(void *data, int type, void *_ev);
static int ewl_ev_x_mouse_down(void *data, int type, void *_ev);
static int ewl_ev_x_mouse_wheel(void *data, int type, void *_ev);
static int ewl_ev_x_mouse_up(void *data, int type, void *_ev);
static int ewl_ev_x_mouse_move(void *data, int type, void *_ev);
static int ewl_ev_x_mouse_out(void *data, int type, void *_ev);
static int ewl_ev_x_focus_in(void *data, int type, void *_ev);
static int ewl_ev_x_focus_out(void *data, int type, void *_ev);
static int ewl_ev_x_data_received(void *data, int type, void *_ev);
static int ewl_ev_x_data_request(void *data, int type, void *_ev);
static int ewl_ev_dnd_position(void *data, int type, void *_ev);
static int ewl_ev_dnd_enter(void *data, int type, void *_ev);
static int ewl_ev_dnd_leave(void *data, int type, void *_ev);
static int ewl_ev_dnd_drop(void *data, int type, void *_ev);

static Ecore_Event_Handler *ee_expose_handler = NULL;
static Ecore_Event_Handler *ee_configure_handler = NULL;
static Ecore_Event_Handler *ee_delete_request_handler = NULL;
static Ecore_Event_Handler *ee_key_down_handler = NULL;
static Ecore_Event_Handler *ee_key_up_handler = NULL;
static Ecore_Event_Handler *ee_dnd_position_handler = NULL;
static Ecore_Event_Handler *ee_dnd_enter_handler = NULL;
static Ecore_Event_Handler *ee_dnd_leave_handler = NULL;
static Ecore_Event_Handler *ee_dnd_drop_handler = NULL;
static Ecore_Event_Handler *ee_selection_notify_handler = NULL;
static Ecore_Event_Handler *ee_selection_request_handler = NULL;
static Ecore_Event_Handler *ee_mouse_down_handler = NULL;
static Ecore_Event_Handler *ee_mouse_up_handler = NULL;
static Ecore_Event_Handler *ee_mouse_move_handler = NULL;
static Ecore_Event_Handler *ee_mouse_wheel_handler = NULL;
static Ecore_Event_Handler *ee_mouse_out_handler = NULL;
static Ecore_Event_Handler *ee_focus_in_handler = NULL;
static Ecore_Event_Handler *ee_focus_out_handler = NULL;

static void ee_shutdown(Ewl_Engine *engine);
static int ee_init(Ewl_Engine *engine, int *argc, char ** argv);
static void ee_window_new(Ewl_Window *win);
static void ee_window_destroy(Ewl_Window *win);
static void ee_window_move(Ewl_Window *win);
static void ee_window_resize(Ewl_Window *win);
static void ee_window_min_max_size_set(Ewl_Window *win);
static void ee_window_show(Ewl_Window *win);
static void ee_window_hide(Ewl_Window *win);
static void ee_window_title_set(Ewl_Window *win);
static void ee_window_name_class_set(Ewl_Window *win);
static void ee_window_borderless_set(Ewl_Window *win);
static void ee_window_dialog_set(Ewl_Window *win);
static void ee_window_states_set(Ewl_Window *win);
static void ee_window_state_request(Ewl_Window *win, int states);
static void ee_window_state_handle(Ewl_Window *win, int states,
                                Ewl_Window_Flags ewl_flag,
                                Ecore_X_Window_State ecore_flag);
static void ee_window_states_set_helper(Ewl_Window *win);
static void ee_window_hints_set(Ewl_Window *win);
static void ee_window_transient_for(Ewl_Window *win);
static void ee_window_leader_set(Ewl_Window *win);
static void ee_window_raise(Ewl_Window *win);
static void ee_window_lower(Ewl_Window *win);
static int ee_keyboard_grab(Ewl_Window *win);
static void ee_keyboard_ungrab(Ewl_Window *win);
static int ee_pointer_grab(Ewl_Window *win);
static void ee_pointer_ungrab(Ewl_Window *win);
static void ee_window_selection_text_set(Ewl_Embed *emb, const char *txt);
static void ee_window_geometry_set(Ewl_Window *win, int *width, int *height);
static void ee_dnd_aware_set(Ewl_Embed *embed);
static void ee_desktop_size_get(Ewl_Embed *embed, int *w, int *h);
static void ee_dnd_drag_types_set(Ewl_Embed *embed, const char **types, unsigned int num);
static void ee_dnd_drag_begin(Ewl_Embed *embed);
static void ee_dnd_drag_drop(Ewl_Embed *embed);
static int ee_dnd_drag_data_send(Ewl_Embed *embed, void *handle, void *data, int len);

static int ee_pointer_data_new(Ewl_Embed *embed, int *data, int w, int h);
static void ee_pointer_free(Ewl_Embed *embed, int pointer);
static void ee_pointer_set(Ewl_Embed *embed, int pointer);
static int ee_pointer_get(Ewl_Embed *embed);

static void *window_funcs[EWL_ENGINE_WINDOW_MAX] =
        {
                ee_window_new,
                ee_window_destroy,
                ee_window_move,
                ee_window_resize,
                ee_window_min_max_size_set,
                ee_window_show,
                ee_window_hide,
                ee_window_title_set,
                ee_window_name_class_set,
                ee_window_borderless_set,
                ee_window_dialog_set,
                ee_window_states_set,
                ee_window_hints_set,
                ee_window_transient_for,
                ee_window_leader_set,
                ee_window_raise,
                ee_window_lower,
                ee_keyboard_grab,
                ee_keyboard_ungrab,
                ee_pointer_grab,
                ee_pointer_ungrab,
                ee_window_selection_text_set,
                ee_window_geometry_set,
                ee_dnd_aware_set,
                ee_desktop_size_get,
                ee_dnd_drag_types_set,
                ee_dnd_drag_begin,
                ee_dnd_drag_drop,
                ee_dnd_drag_data_send,
        };

static void *pointer_funcs[EWL_ENGINE_POINTER_MAX] =
        {
                ee_pointer_data_new,
                ee_pointer_free,
                ee_pointer_get,
                ee_pointer_set,
        };

Ecore_DList *
ewl_engine_dependancies(void)
{
        DENTER_FUNCTION(DLEVEL_STABLE);

        DRETURN_PTR(NULL, DLEVEL_STABLE);
}

Ewl_Engine *
ewl_engine_create(int *argc, char ** argv)
{
        Ewl_Engine_Xcb *engine;

        DENTER_FUNCTION(DLEVEL_STABLE);

        engine = NEW(Ewl_Engine_Xcb, 1);
        if (!engine)
                DRETURN_PTR(NULL, DLEVEL_STABLE);

        if (!ee_init(EWL_ENGINE(engine), argc, argv))
        {
                FREE(engine);
                DRETURN_PTR(NULL, DLEVEL_STABLE);
        }

        DRETURN_PTR(EWL_ENGINE(engine), DLEVEL_STABLE);
}

static int
ee_init(Ewl_Engine *engine, int *argc, char ** argv)
{
        Ewl_Engine_Info *info;
        char *display = NULL;
        int i;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(engine, FALSE);

        /* If the event handlers are already setup don't do it again */
        if (ee_expose_handler)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        if (argc && argv)
        {
                for (i = 1; i < *argc; i++)
                {
                        if (!strcmp(argv[i], "-display"))
                        {
                                if (++i < *argc)
                                        display = argv[i];
                        }
                }
        }

        if (!ecore_x_init(display))
        {
                fprintf(stderr, "Unable to initialize Ecore X.\n"
                                "Is your DISPLAY variable set correctly?\n\n");
                DRETURN_INT(FALSE, DLEVEL_STABLE);
        }

        ee_expose_handler = ecore_event_handler_add(
                                        ECORE_X_EVENT_WINDOW_DAMAGE,
                                        ewl_ev_x_window_expose, NULL);
        ee_configure_handler = ecore_event_handler_add(
                                        ECORE_X_EVENT_WINDOW_CONFIGURE,
                                        ewl_ev_x_window_configure, NULL);
        ee_delete_request_handler = ecore_event_handler_add(
                                        ECORE_X_EVENT_WINDOW_DELETE_REQUEST,
                                        ewl_ev_x_window_delete, NULL);

        /*
         * Register dispatching functions for keyboard events.
         */
        ee_key_down_handler = ecore_event_handler_add(
                                                ECORE_EVENT_KEY_DOWN,
                                                ewl_ev_x_key_down, NULL);
        ee_key_up_handler = ecore_event_handler_add(
                                                ECORE_EVENT_KEY_UP,
                                                ewl_ev_x_key_up, NULL);

        /*
         * Register dispatching functions for DND events
         */
        ee_dnd_position_handler = ecore_event_handler_add(
                                                ECORE_X_EVENT_XDND_POSITION,
                                                ewl_ev_dnd_position, NULL);
        ee_dnd_enter_handler = ecore_event_handler_add(
                                                ECORE_X_EVENT_XDND_ENTER,
                                                ewl_ev_dnd_enter, NULL);
        ee_dnd_leave_handler = ecore_event_handler_add(
                                                ECORE_X_EVENT_XDND_LEAVE,
                                                ewl_ev_dnd_leave, NULL);
        ee_dnd_drop_handler = ecore_event_handler_add(
                                                ECORE_X_EVENT_XDND_DROP,
                                                ewl_ev_dnd_drop, NULL);

        /*
         * Selection callbacks to allow for data transfers.
         */
        ee_selection_notify_handler = ecore_event_handler_add(
                                                ECORE_X_EVENT_SELECTION_NOTIFY,
                                                ewl_ev_x_data_received, NULL);

        /*
         * Selection callbacks to allow for pasting.
         */
        ee_selection_request_handler = ecore_event_handler_add(
                                                ECORE_X_EVENT_SELECTION_REQUEST,
                                                ewl_ev_x_data_request, NULL);

        /*
         * Finally, register dispatching functions for mouse events.
         */
        ee_mouse_down_handler = ecore_event_handler_add(
                                                ECORE_EVENT_MOUSE_BUTTON_DOWN,
                                                ewl_ev_x_mouse_down, NULL);
        ee_mouse_up_handler = ecore_event_handler_add(
                                                ECORE_EVENT_MOUSE_BUTTON_UP,
                                                ewl_ev_x_mouse_up, NULL);
        ee_mouse_move_handler = ecore_event_handler_add(
                                                ECORE_EVENT_MOUSE_MOVE,
                                                ewl_ev_x_mouse_move, NULL);
        ee_mouse_wheel_handler = ecore_event_handler_add(
                                                ECORE_EVENT_MOUSE_WHEEL,
                                                ewl_ev_x_mouse_wheel, NULL);
        ee_mouse_out_handler = ecore_event_handler_add(
                                                ECORE_X_EVENT_MOUSE_OUT,
                                                ewl_ev_x_mouse_out, NULL);
        ee_focus_in_handler = ecore_event_handler_add(
                                                ECORE_X_EVENT_WINDOW_FOCUS_IN,
                                                ewl_ev_x_focus_in, NULL);
        ee_focus_out_handler = ecore_event_handler_add(
                                                ECORE_X_EVENT_WINDOW_FOCUS_OUT,
                                                ewl_ev_x_focus_out, NULL);

        if (!ee_expose_handler || !ee_configure_handler
                        || !ee_delete_request_handler
                        || !ee_key_down_handler || !ee_key_up_handler
                        || !ee_dnd_position_handler || !ee_dnd_enter_handler
                        || !ee_dnd_leave_handler || !ee_dnd_drop_handler
                        || !ee_selection_notify_handler || !ee_selection_request_handler
                        || !ee_mouse_down_handler
                        || !ee_mouse_up_handler || !ee_mouse_move_handler
                        || !ee_mouse_wheel_handler || !ee_mouse_out_handler
                        || !ee_focus_in_handler || !ee_focus_out_handler)
        {
                ee_shutdown(EWL_ENGINE(engine));

                fprintf(stderr, "Unable to create Ecore X event handlers.\n");
                DRETURN_INT(FALSE, DLEVEL_STABLE);
        }

        info = NEW(Ewl_Engine_Info, 1);
        info->shutdown = ee_shutdown;
        info->hooks.window = window_funcs;
        info->hooks.pointer = pointer_funcs;

        engine->functions = info;

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static void
ee_shutdown(Ewl_Engine *engine)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(engine);

        if (ee_expose_handler)
                ecore_event_handler_del(ee_expose_handler);
        ee_expose_handler = NULL;

        if (ee_configure_handler)
                ecore_event_handler_del(ee_configure_handler);
        ee_configure_handler = NULL;

        if (ee_delete_request_handler)
                ecore_event_handler_del(ee_delete_request_handler);
        ee_delete_request_handler = NULL;

        if (ee_key_down_handler)
                ecore_event_handler_del(ee_key_down_handler);
        ee_key_down_handler = NULL;

        if (ee_key_up_handler)
                ecore_event_handler_del(ee_key_up_handler);
        ee_key_up_handler = NULL;

        if (ee_dnd_position_handler)
                ecore_event_handler_del(ee_dnd_position_handler);
        ee_dnd_position_handler = NULL;

        if (ee_dnd_enter_handler)
                ecore_event_handler_del(ee_dnd_enter_handler);
        ee_dnd_enter_handler = NULL;

        if (ee_dnd_leave_handler)
                ecore_event_handler_del(ee_dnd_leave_handler);
        ee_dnd_leave_handler = NULL;

        if (ee_dnd_drop_handler)
                ecore_event_handler_del(ee_dnd_drop_handler);
        ee_dnd_drop_handler = NULL;

        if (ee_selection_notify_handler)
                ecore_event_handler_del(ee_selection_notify_handler);
        ee_selection_notify_handler = NULL;

        if (ee_selection_request_handler)
                ecore_event_handler_del(ee_selection_request_handler);
        ee_selection_request_handler = NULL;

        if (ee_mouse_down_handler)
                ecore_event_handler_del(ee_mouse_down_handler);
        ee_mouse_down_handler = NULL;

        if (ee_mouse_up_handler)
                ecore_event_handler_del(ee_mouse_up_handler);
        ee_mouse_up_handler = NULL;

        if (ee_mouse_move_handler)
                ecore_event_handler_del(ee_mouse_move_handler);
        ee_mouse_move_handler = NULL;

        if (ee_mouse_wheel_handler)
                ecore_event_handler_del(ee_mouse_wheel_handler);
        ee_mouse_wheel_handler = NULL;

        if (ee_mouse_out_handler)
                ecore_event_handler_del(ee_mouse_out_handler);
        ee_mouse_out_handler = NULL;

        if (ee_focus_in_handler)
                ecore_event_handler_del(ee_focus_in_handler);
        ee_focus_in_handler = NULL;

        if (ee_focus_out_handler)
                ecore_event_handler_del(ee_focus_out_handler);
        ee_focus_out_handler = NULL;

        ecore_x_shutdown();

        IF_FREE(engine->functions);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_new(Ewl_Window *win)
{
        Ecore_X_Window xwin;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        if (win->flags & EWL_WINDOW_OVERRIDE)
                xwin = ecore_x_window_override_new(0,
                                EWL_EMBED(win)->x, EWL_EMBED(win)->y,
                                ewl_object_current_w_get(EWL_OBJECT(win)),
                                ewl_object_current_h_get(EWL_OBJECT(win)));
        else
                xwin = ecore_x_window_new(0,
                                EWL_EMBED(win)->x, EWL_EMBED(win)->y,
                                ewl_object_current_w_get(EWL_OBJECT(win)),
                                ewl_object_current_h_get(EWL_OBJECT(win)));

        ecore_x_icccm_protocol_get_prefetch(xwin);
        ecore_x_icccm_protocol_get_fetch();
        ecore_x_icccm_protocol_set(xwin, ECORE_X_WM_PROTOCOL_DELETE_REQUEST, 1);
        win->window = (void *)(long)xwin;

        if (win->flags & EWL_WINDOW_BORDERLESS)
                ee_window_borderless_set(win);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_destroy(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ee_window_hide(win);
        ecore_x_window_free((Ecore_X_Window)(EWL_EMBED(win)->canvas_window));
        ecore_x_window_free((Ecore_X_Window)(win->window));

        EWL_EMBED(win)->canvas_window = NULL;
        win->window = NULL;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_move(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ecore_x_window_move((Ecore_X_Window)win->window,
                                        EWL_EMBED(win)->x,
                                        EWL_EMBED(win)->y);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_resize(Ewl_Window *win)
{
        int width, height;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        width = ewl_object_current_w_get(EWL_OBJECT(win));
        height = ewl_object_current_h_get(EWL_OBJECT(win));

        ecore_x_window_resize((Ecore_X_Window)win->window, width, height);

        if (EWL_EMBED(win)->canvas_window != win->window)
                ecore_x_window_resize((Ecore_X_Window)EWL_EMBED(win)->canvas_window,
                                                width, height);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_min_max_size_set(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ecore_x_icccm_size_pos_hints_get_prefetch((Ecore_X_Window)win->window);
        ecore_x_icccm_size_pos_hints_get_fetch();
        ecore_x_icccm_size_pos_hints_set((Ecore_X_Window)win->window,
                                        0, ECORE_X_GRAVITY_NW,
                                        ewl_object_minimum_w_get(EWL_OBJECT(win)),
                                        ewl_object_minimum_h_get(EWL_OBJECT(win)),
                                        ewl_object_maximum_w_get(EWL_OBJECT(win)),
                                        ewl_object_maximum_h_get(EWL_OBJECT(win)),
                                        0, 0, /* base */
                                        0, 0, /* step */
                                        0, 0); /* aspect */

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_show(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ecore_x_window_show((Ecore_X_Window)win->window);
        ecore_x_window_show((Ecore_X_Window)EWL_EMBED(win)->canvas_window);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_hide(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ecore_x_window_hide((Ecore_X_Window)EWL_EMBED(win)->canvas_window);
        ecore_x_window_hide((Ecore_X_Window)EWL_WINDOW(win)->window);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_title_set(Ewl_Window *win)
{
        const char *title;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        title = win->title ? win->title : "";
        ecore_x_icccm_title_set((Ecore_X_Window)win->window, title);
        ecore_x_netwm_name_set((Ecore_X_Window)win->window, title);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_name_class_set(Ewl_Window *win)
{
        const char *name;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        name = win->name ? win->name : "";
        ecore_x_icccm_name_class_set((Ecore_X_Window)win->window, name,
                                (win->classname ? win->classname : name));

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_borderless_set(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ecore_x_mwm_borderless_set((Ecore_X_Window)win->window,
                                (!!(win->flags & EWL_WINDOW_BORDERLESS)));

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_dialog_set(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ecore_x_netwm_window_type_set((Ecore_X_Window)win->window,
                        ((!!(win->flags & EWL_WINDOW_DIALOG)) ?
                                        ECORE_X_WINDOW_TYPE_DIALOG :
                                        ECORE_X_WINDOW_TYPE_NORMAL));

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_states_set(Ewl_Window *win)
{
        int diff;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        diff = win->flags ^ win->old_flags;
        if (!diff) DRETURN(DLEVEL_STABLE);

        if (REALIZED(win))
                ee_window_state_request(win, diff);
        else
                ee_window_states_set_helper(win);

        win->old_flags = win->flags;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_state_handle(Ewl_Window *win, int states,
                                Ewl_Window_Flags ewl_flag,
                                Ecore_X_Window_State ecore_flag)
{
        int state = 0;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        if (states & ewl_flag)
        {
                state = (!!(win->flags & ewl_flag));
                ecore_x_netwm_state_request_send((Ecore_X_Window)win->window,
                        0, ecore_flag, ECORE_X_WINDOW_STATE_UNKNOWN, state);
        }

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_state_request(Ewl_Window *win, int states)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ee_window_state_handle(win, states, EWL_WINDOW_FULLSCREEN,
                                        ECORE_X_WINDOW_STATE_FULLSCREEN);
        ee_window_state_handle(win, states, EWL_WINDOW_SKIP_TASKBAR,
                                        ECORE_X_WINDOW_STATE_SKIP_TASKBAR);
        ee_window_state_handle(win, states, EWL_WINDOW_SKIP_PAGER,
                                        ECORE_X_WINDOW_STATE_SKIP_PAGER);
        ee_window_state_handle(win, states, EWL_WINDOW_MODAL,
                                        ECORE_X_WINDOW_STATE_MODAL);
        ee_window_state_handle(win, states, EWL_WINDOW_DEMANDS_ATTENTION,
                                        ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION);

        win->flags &= ~EWL_WINDOW_DEMANDS_ATTENTION;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_states_set_helper(Ewl_Window *win)
{
        Ecore_X_Window_State states[4];
        int count = 0;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        /* Note: We're ignoring the attention demand state in here as it is
         * only relevant if the window has been realized */

        if (!win->window)
                DRETURN(DLEVEL_STABLE);

        if (!!(win->flags & EWL_WINDOW_FULLSCREEN))
                states[count++] = ECORE_X_WINDOW_STATE_FULLSCREEN;

        if (!!(win->flags & EWL_WINDOW_SKIP_TASKBAR))
                states[count++] = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;

        if (!!(win->flags & EWL_WINDOW_SKIP_PAGER))
                states[count++] = ECORE_X_WINDOW_STATE_SKIP_PAGER;

        if (!!(win->flags & EWL_WINDOW_MODAL))
                states[count++] = ECORE_X_WINDOW_STATE_MODAL;

        ecore_x_netwm_window_state_set((Ecore_X_Window)win->window, states, count);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_hints_set(Ewl_Window *win)
{
        Ewl_Embed_Window *win_group;
        int urgent = FALSE;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        if (win->flags & EWL_WINDOW_LEADER)
                win_group = win->leader.ewl->window;
        else if (win->flags & EWL_WINDOW_LEADER_FOREIGN)
                win_group = win->leader.foreign;
        else
                win_group = NULL;

        if (win->flags & EWL_WINDOW_URGENT)
                urgent = TRUE;

        ecore_x_icccm_hints_set((Ecore_X_Window) win->window,
                                1, // accepts focus
                                0, // initial states
                                0, // icon pixmap
                                0, // icon mask
                                0, // icon window
                                (Ecore_X_Window) win_group, // window group
                                urgent); // is urgent

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_transient_for(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        if (win->flags & EWL_WINDOW_TRANSIENT)
                ecore_x_icccm_transient_for_set((Ecore_X_Window)win->window,
                                (Ecore_X_Window)win->transient.ewl->window);
        else if (win->flags & EWL_WINDOW_TRANSIENT_FOREIGN)
                ecore_x_icccm_transient_for_set((Ecore_X_Window)win->window,
                                        (Ecore_X_Window)win->transient.foreign);
        else
                ecore_x_icccm_transient_for_unset((Ecore_X_Window)win->window);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_leader_set(Ewl_Window *win)
{
        Ewl_Embed_Window *leader;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        if (win->flags & EWL_WINDOW_LEADER)
                leader = win->leader.ewl->window;
        else if (win->flags & EWL_WINDOW_LEADER_FOREIGN)
                leader = win->leader.foreign;
        else
                /* according to the icccm specs a client leader
                 * sets itself to the leader */
                leader = win->window;

        ecore_x_icccm_client_leader_set((Ecore_X_Window)win->window,
                                        (Ecore_X_Window)leader);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_raise(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ecore_x_window_raise((Ecore_X_Window)win->window);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_lower(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ecore_x_window_lower((Ecore_X_Window)win->window);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static int
ee_keyboard_grab(Ewl_Window *win)
{
        int ret = FALSE;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(win, FALSE);
        DCHECK_TYPE_RET(win, EWL_WINDOW_TYPE, FALSE);

        if ((!!(win->flags & EWL_WINDOW_GRAB_KEYBOARD)))
        {
                ret = ecore_x_keyboard_grab((Ecore_X_Window)win->window);
                ee_current_key_grab_window = win;
        }
        else if (ee_current_key_grab_window == win)
        {
                ecore_x_keyboard_ungrab();
                ee_current_key_grab_window = NULL;
        }

        DRETURN_INT(ret, DLEVEL_STABLE);
}

static void
ee_keyboard_ungrab(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ecore_x_keyboard_ungrab();
        ee_current_key_grab_window = NULL;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static int
ee_pointer_grab(Ewl_Window *win)
{
        int ret = FALSE;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(win, FALSE);
        DCHECK_TYPE_RET(win, EWL_WINDOW_TYPE, FALSE);

        if ((!!(win->flags & EWL_WINDOW_GRAB_POINTER)))
        {
                ret = ecore_x_pointer_grab((Ecore_X_Window)win->window);
                ee_current_pointer_grab_window = win;
        }
        else if (ee_current_pointer_grab_window == win)
        {
                ecore_x_pointer_ungrab();
                ee_current_pointer_grab_window = NULL;
        }

        DRETURN_INT(ret, DLEVEL_STABLE);
}

static void
ee_pointer_ungrab(Ewl_Window *win)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        ecore_x_pointer_ungrab();
        ee_current_pointer_grab_window = NULL;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_selection_text_set(Ewl_Embed *emb, const char *txt)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(emb);
        DCHECK_PARAM_PTR(txt);
        DCHECK_TYPE(emb, EWL_EMBED_TYPE);

        ecore_x_selection_primary_prefetch();
        ecore_x_selection_primary_fetch();
        if (txt)
                ecore_x_selection_primary_set(
                                (Ecore_X_Window)emb->canvas_window,
                                (unsigned char *)txt, strlen(txt) + 1);
        else
                ecore_x_selection_primary_clear();

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_window_geometry_set(Ewl_Window *win, int *width, int *height)
{
        DENTER_FUNCTION(DLEVEL_STABLE);

        ecore_x_drawable_geometry_get_prefetch((win ? (Ecore_X_Window)win->window : (Ecore_X_Window)0));
        ecore_x_drawable_geometry_get_fetch();
        ecore_x_window_geometry_get((win ? (Ecore_X_Window)win->window : (Ecore_X_Window)0),
                                                        NULL, NULL, width, height);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_dnd_aware_set(Ewl_Embed *embed)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(embed);
        DCHECK_TYPE(embed, EWL_EMBED_TYPE);

        ecore_x_dnd_aware_set((Ecore_X_Window)embed->canvas_window, TRUE);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_desktop_size_get(Ewl_Embed *embed, int *w, int *h)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(embed);
        DCHECK_TYPE(embed, EWL_EMBED_TYPE);

        ecore_x_drawable_geometry_get_prefetch((Ecore_X_Window)0);
        ecore_x_drawable_geometry_get_fetch();
        ecore_x_window_size_get(0, w, h);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_dnd_drag_types_set(Ewl_Embed *embed, const char **types, unsigned int num)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(embed);
        DCHECK_TYPE(embed, EWL_EMBED_TYPE);

        ecore_x_dnd_type_get_prefetch((Ecore_X_Window)embed->canvas_window);
        ecore_x_dnd_aware_set((Ecore_X_Window)embed->canvas_window,
                              (num > 0 ? 1 : 0));
        ecore_x_dnd_type_get_fetch();
        ecore_x_dnd_types_set((Ecore_X_Window)embed->canvas_window, types, num);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_dnd_drag_begin(Ewl_Embed *embed)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(embed);
        DCHECK_TYPE(embed, EWL_EMBED_TYPE);

        ecore_x_dnd_begin_prefetch((Ecore_X_Window)embed->canvas_window);
        ecore_x_dnd_begin_fetch();
        ecore_x_dnd_begin((Ecore_X_Window)embed->canvas_window, NULL, 0);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_dnd_drag_drop(Ewl_Embed *embed)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(embed);
        DCHECK_TYPE(embed, EWL_EMBED_TYPE);

        ecore_x_dnd_drop();

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static int
ee_dnd_drag_data_send(Ewl_Embed *embed, void *handle, void *data, int len)
{
        Ecore_X_Event_Selection_Request *request = handle;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(embed, FALSE);
        DCHECK_PARAM_PTR_RET(handle, FALSE);
        DCHECK_TYPE_RET(embed, EWL_EMBED_TYPE, FALSE);

        ecore_x_window_prop_property_set(request->requestor, request->property,
                                         request->target, 8, data, len);

        DRETURN_INT(ecore_x_selection_notify_send(request->requestor,
                                request->selection, request->target,
                                request->property, request->time), DLEVEL_STABLE);
}

static int
ee_pointer_data_new(Ewl_Embed *embed, int *data, int w, int h)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(embed, 0);
        DCHECK_TYPE_RET(embed, EWL_EMBED_TYPE, 0);

        DRETURN_INT(ecore_x_cursor_new((Ecore_X_Window)embed->canvas_window,
                                data, w, h, 0, 0), DLEVEL_STABLE);
}

static void
ee_pointer_free(Ewl_Embed *embed, int pointer)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(embed);
        DCHECK_TYPE(embed, EWL_EMBED_TYPE);

        ecore_x_cursor_free(pointer);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * FIXME: No function to poll the active pointer yet, and we probably want to
 * add size get/set hooks as well.
 */
static int
ee_pointer_get(Ewl_Embed *embed)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(embed, 0);
        DCHECK_TYPE_RET(embed, EWL_EMBED_TYPE, 0);

        DRETURN_INT(ecore_x_cursor_size_get(), DLEVEL_STABLE);
}

static void
ee_pointer_set(Ewl_Embed *embed, int pointer)
{
        Ecore_X_Cursor cur;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(embed);
        DCHECK_TYPE(embed, EWL_EMBED_TYPE);

        if (pointer >= EWL_MOUSE_CURSOR_MAX)
                cur = pointer;
        else
                cur = ecore_x_cursor_shape_get(pointer);
        ecore_x_window_cursor_set((Ecore_X_Window)embed->canvas_window, cur);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static int
ewl_ev_x_window_expose(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        /*
         * Widgets don't need to know about this usually, but we still need to
         * let them know in case a widget is using a non-evas based draw method
         */
        Ecore_X_Event_Window_Damage *ev;
        Ewl_Event_Window_Expose event;
        Ewl_Window *window;

        DENTER_FUNCTION(DLEVEL_STABLE);

        ev = e;

        window = ewl_window_window_find((void *)ev->win);
        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        event.x = ev->x;
        event.y = ev->y;
        event.w = ev->w;
        event.h = ev->h;

        ewl_engine_canvas_damage_add(EWL_EMBED(window), ev->x, ev->y, ev->w, ev->h);
        ewl_callback_call_with_event_data(EWL_WIDGET(window), EWL_CALLBACK_EXPOSE, &event);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_window_configure(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        /*
         * When a configure event occurs, we must update the windows geometry
         * based on the coordinates and dimensions given in the Ecore_Event.
         */
        Ecore_X_Event_Window_Configure *ev;
        Ewl_Window *window;
        Ewl_Embed *embed;
        int config = 0;

        DENTER_FUNCTION(DLEVEL_STABLE);

        ev = e;

        embed = ewl_embed_canvas_window_find((void *)ev->win);
        if (!embed)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        /*
         * Save coords and queue a configure event if the window is moved.
         */
        if ((ev->from_wm) && (ev->x != embed->x)) {
                embed->x = ev->x;
                config = 1;
        }

        if ((ev->from_wm) && (ev->y != embed->y)) {
                embed->y = ev->y;
                config = 1;
        }

        window = ewl_window_window_find((void *)ev->win);
        /*
         * we can finish when the embed is not a window
         */
        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        if (config)
                ewl_widget_configure(EWL_WIDGET(window));

        /*
         * Configure events really only need to occur on resize.
         */
        if ((ewl_object_current_w_get(EWL_OBJECT(window)) != ev->w)
                        || (ewl_object_current_h_get(EWL_OBJECT(window)) != ev->h)) {

                /*
                 * Right now this is commented out, as it was preventing windows
                 * from resizing when WE were the ones setting the size
                 * (mainly on initial show of the window). It would be nice to
                 * add this back in to limit the # of window resizes occuring.
                 * (As long as it doesn't break initial size.)
                 */
                if (ev->from_wm) window->flags |= EWL_WINDOW_USER_CONFIGURE;
                ewl_object_geometry_request(EWL_OBJECT(window), 0, 0, ev->w,
                                                                        ev->h);
        }

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_window_delete(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        /*
         * Retrieve the appropriate ewl_window using the x window id that is
         * held in the eevent, and call it's handlers for a window delete event.
         */
        Ecore_X_Event_Window_Destroy *ev;
        Ewl_Window *window;

        DENTER_FUNCTION(DLEVEL_STABLE);

        ev = e;

        window = ewl_window_window_find((void *)ev->win);
        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        ewl_callback_call(EWL_WIDGET(window), EWL_CALLBACK_DELETE_WINDOW);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_key_down(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ewl_Window *window;
        Ecore_Event_Key *ev;
        unsigned int key_modifiers;

        DENTER_FUNCTION(DLEVEL_STABLE);

        ev = e;
        window = ewl_window_window_find((void *)ev->window);

        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        key_modifiers = ewl_ev_modifiers_get();

        if (strstr(ev->keyname, "Shift_"))
                key_modifiers |= EWL_KEY_MODIFIER_SHIFT;
        else if (strstr(ev->keyname, "Control_"))
                key_modifiers |= EWL_KEY_MODIFIER_CTRL;
        else if (strstr(ev->keyname, "Alt_"))
                key_modifiers |= EWL_KEY_MODIFIER_ALT;
        else if (strstr(ev->keyname, "Mod_"))
                key_modifiers |= EWL_KEY_MODIFIER_MOD;
        else if (strstr(ev->keyname, "Super_"))
                key_modifiers |= EWL_KEY_MODIFIER_WIN;
        else if (strstr(ev->keyname, "Hyper_"))
                key_modifiers |= EWL_KEY_MODIFIER_WIN;
        else if (!ev->compose || iscntrl(*ev->compose))
                ewl_embed_key_down_feed(EWL_EMBED(window), ev->keyname,
                                                        key_modifiers);
        else
                ewl_embed_key_down_feed(EWL_EMBED(window), ev->compose,
                                                        key_modifiers);

        ewl_ev_modifiers_set(key_modifiers);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_key_up(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ewl_Window *window;
        Ecore_Event_Key *ev;
        unsigned int key_modifiers;

        DENTER_FUNCTION(DLEVEL_STABLE);

        ev = e;

        window = ewl_window_window_find((void *)ev->window);
        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        key_modifiers = ewl_ev_modifiers_get();

        if (strstr(ev->keyname, "Shift_"))
                key_modifiers &= ~EWL_KEY_MODIFIER_SHIFT;
        else if (strstr(ev->keyname, "Control_"))
                key_modifiers &= ~EWL_KEY_MODIFIER_CTRL;
        else if (strstr(ev->keyname, "Alt_"))
                key_modifiers &= ~EWL_KEY_MODIFIER_ALT;
        else if (strstr(ev->keyname, "Mod_"))
                key_modifiers &= ~EWL_KEY_MODIFIER_MOD;
        else if (strstr(ev->keyname, "Super_"))
                key_modifiers &= ~EWL_KEY_MODIFIER_WIN;
        else if (strstr(ev->keyname, "Hyper_"))
                key_modifiers &= ~EWL_KEY_MODIFIER_WIN;
        else if (!ev->compose || iscntrl(*ev->compose))
                ewl_embed_key_up_feed(EWL_EMBED(window), ev->keyname,
                                                        key_modifiers);
        else
                ewl_embed_key_up_feed(EWL_EMBED(window), ev->compose,
                                                        key_modifiers);

        ewl_ev_modifiers_set(key_modifiers);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_mouse_down(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        int clicks = 1;
        Ewl_Window *window;
        Ecore_Event_Mouse_Button *ev;
        unsigned int key_modifiers;

        DENTER_FUNCTION(DLEVEL_STABLE);

        ev = e;

        window = ewl_window_window_find((void *)ev->window);
        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        if (ev->double_click)
                clicks = 2;
        if (ev->triple_click)
                clicks = 3;

        key_modifiers = ewl_ev_modifiers_get();
        ewl_embed_mouse_down_feed(EWL_EMBED(window), ev->buttons, clicks,
                                                ev->x, ev->y, key_modifiers);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_mouse_up(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        int clicks = 1;
        Ewl_Window *window;
        Ecore_Event_Mouse_Button *ev;
        unsigned int key_modifiers;

        DENTER_FUNCTION(DLEVEL_STABLE);

        ev = e;

        window = ewl_window_window_find((void *)ev->window);
        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        if (ev->double_click)
                clicks = 2;
        if (ev->triple_click)
                clicks = 3;

        key_modifiers = ewl_ev_modifiers_get();
        ewl_embed_mouse_up_feed(EWL_EMBED(window), ev->buttons, clicks, ev->x,
                                                ev->y, key_modifiers);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_mouse_move(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ewl_Window *window;
        Ecore_Event_Mouse_Move *ev;
        unsigned int key_modifiers;

        DENTER_FUNCTION(DLEVEL_STABLE);

        ev = e;

        window = ewl_window_window_find((void *)ev->window);
        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        key_modifiers = ewl_ev_modifiers_get();
        ewl_embed_mouse_move_feed(EWL_EMBED(window), ev->x, ev->y,
                                                        key_modifiers);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ewl_Window *window;
        Ecore_X_Event_Mouse_Out *ev = e;
        unsigned int key_modifiers;

        DENTER_FUNCTION(DLEVEL_STABLE);

        window = ewl_window_window_find((void *)ev->win);
        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        key_modifiers = ewl_ev_modifiers_get();
        ewl_embed_mouse_out_feed(EWL_EMBED(window), ev->x, ev->y,
                                                key_modifiers);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_mouse_wheel(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ewl_Window *window;
        Ecore_Event_Mouse_Wheel *ev = e;
        unsigned int key_modifiers;

        DENTER_FUNCTION(DLEVEL_STABLE);

        window = ewl_window_window_find((void *)ev->window);
        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        key_modifiers = ewl_ev_modifiers_get();
        ewl_embed_mouse_wheel_feed(EWL_EMBED(window), ev->x, ev->y,
                                        ev->z, ev->direction, key_modifiers);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}


static int
ewl_ev_x_focus_in(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ewl_Window *window;
        Ecore_X_Event_Window_Focus_In *ev = e;

        DENTER_FUNCTION(DLEVEL_STABLE);

        window = ewl_window_window_find((void *)ev->win);
        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        ewl_callback_call(EWL_WIDGET(window), EWL_CALLBACK_FOCUS_IN);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_focus_out(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ewl_Window *window;
        Ecore_X_Event_Window_Focus_Out *ev = e;

        DENTER_FUNCTION(DLEVEL_STABLE);

        window = ewl_window_window_find((void *)ev->win);
        if (!window)
                DRETURN_INT(TRUE, DLEVEL_STABLE);

        ewl_callback_call(EWL_WIDGET(window), EWL_CALLBACK_FOCUS_OUT);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_data_received(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ecore_X_Event_Selection_Notify *ev;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(e, FALSE);

        ev = e;

        /* Handle everything *except* XDND selection */
        if (ev->selection != ECORE_X_SELECTION_XDND)
                printf("Paste event received\n");

        else
        {
                Ewl_Embed *embed;
                Ecore_X_Selection_Data *data = ev->data;

                embed = ewl_embed_canvas_window_find((void *)ev->win);
                if (embed)
                {
                        if (data->content == ECORE_X_SELECTION_CONTENT_FILES)
                        {
                                Ecore_X_Selection_Data_Files *files = ev->data;
                                ewl_embed_dnd_data_received_feed(embed,
                                                        ev->target,
                                                        files->files,
                                                        files->num_files,
                                                        data->format);
                        }
                        else if (data->content == ECORE_X_SELECTION_CONTENT_TEXT)
                        {
                                Ecore_X_Selection_Data_Text *text = ev->data;
                                ewl_embed_dnd_data_received_feed(embed,
                                                        ev->target, text->text,
                                                        data->length,
                                                        data->format);
                        }
                        else
                        {
                                ewl_embed_dnd_data_received_feed(embed,
                                                        ev->target, data->data,
                                                        data->length,
                                                        data->format);
                        }
                }

                ecore_x_dnd_send_finished();
        }

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_x_data_request(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ecore_X_Event_Selection_Request *ev;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(e, FALSE);

        ev = e;

        /* Handle everything *except* XDND selection */
        if (ev->selection != ECORE_X_ATOM_SELECTION_XDND)
        {
                char *rec, *dnd;

                ecore_x_atom_name_get_prefetch(ev->selection);
                ecore_x_atom_name_get_prefetch(ECORE_X_ATOM_SELECTION_XDND);
                ecore_x_atom_name_get_fetch();
                rec = ecore_x_atom_name_get(ev->selection);
                ecore_x_atom_name_get_fetch();
                dnd = ecore_x_atom_name_get(ECORE_X_ATOM_SELECTION_XDND);
                printf("Data request event received: %s not %s\n", rec, dnd);
                free(rec);
                free(dnd);
        }

        else if (ev->selection == ECORE_X_ATOM_SELECTION_XDND)
        {
                Ewl_Embed *embed;
                char *atom;

                ecore_x_atom_name_get_prefetch(ev->target);
                embed = ewl_embed_canvas_window_find((void *)ev->owner);
                ecore_x_atom_name_get_fetch();
                atom = ecore_x_atom_name_get(ev->target);
                ewl_embed_dnd_data_request_feed(embed, ev, atom);
                free(atom);
        }

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_dnd_position(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ewl_Window *window;
        Ecore_X_Event_Xdnd_Position *ev;
        int x, y, wx, wy;
        int px, py, pw, ph;
        Ecore_X_Rectangle rect;
        int will_accept = 0;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(e, FALSE);

        ev = e;

        window = ewl_window_window_find((void *)ev->win);
        if (window) {
                Ewl_Embed *embed;

                ewl_embed_window_position_get(EWL_EMBED(window), &wx, &wy);
                x = ev->position.x - wx;
                y = ev->position.y - wy;

                /*
                 * Look for the child here
                 */
                embed = ewl_embed_canvas_window_find((void *)ev->win);
                if (embed) {
                        /* First see if we need to send an 'enter'
                         * to the widget */
                        if (ewl_embed_dnd_position_feed(embed, x, y, &px,
                                                        &py, &pw, &ph))
                                will_accept = 1;

                        if (embed->last.drop_widget) {
                                rect.x = px;
                                rect.y = py;
                                rect.width = pw;
                                rect.height = ph;
                        } else {
                                rect.x = 0;
                                rect.y = 0;
                                rect.width = 0;
                                rect.height = 0;
                        }

                        /* Don't send status for windows we don't own */
                        ecore_x_dnd_send_status(will_accept, 0, rect, ECORE_X_DND_ACTION_PRIVATE);
                }
        }

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_dnd_enter(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ewl_Embed *embed;
        Ecore_X_Event_Xdnd_Enter *ev;
        int i = 0;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(e, FALSE);

        ev = e;

        embed = ewl_embed_canvas_window_find((void *)ev->win);
        if (embed) {
                embed->dnd_types.num_types = ev->num_types;
                embed->dnd_types.types = malloc(sizeof(char*) * ev->num_types);

                for (i = 0; i < ev->num_types; i++)
                        embed->dnd_types.types[i] = strdup(ev->types[i]);
        }
        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_dnd_leave(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ewl_Embed *embed;
        Ecore_X_Event_Xdnd_Leave *ev;
        int i;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(e, FALSE);

        ev = e;

        embed = ewl_embed_canvas_window_find((void *)ev->win);
        if (embed) {
                if (embed->dnd_types.num_types > 0) {
                        for (i = 0; i < embed->dnd_types.num_types; i++)
                                FREE(embed->dnd_types.types[i]);

                        FREE(embed->dnd_types.types);
                        embed->dnd_types.types = NULL;
                        embed->dnd_types.num_types = 0;

                }
        }
        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static int
ewl_ev_dnd_drop(void *data __UNUSED__, int type __UNUSED__, void *e)
{
        Ewl_Embed *embed;
        Ecore_X_Event_Xdnd_Drop *ev;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(e, FALSE);

        ev = e;

        embed = ewl_embed_canvas_window_find((void *)ev->win);
        if (embed) {
                int x, y, wx, wy;
                const char *type;

                ewl_embed_window_position_get(embed, &wx, &wy);

                x = ev->position.x - wx;
                y = ev->position.y - wy;

                type = ewl_embed_dnd_drop_feed(embed, x, y);
                if (type)
                        ecore_x_selection_xdnd_request(ev->win, (char *)type);
        }

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}


