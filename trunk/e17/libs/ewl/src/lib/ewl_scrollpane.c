/* vim: set sw=8 ts=8 sts=8 expandtab: */
#include "ewl_base.h"
#include "ewl_overlay.h"
#include "ewl_scrollpane.h"
#include "ewl_box.h"
#include "ewl_scrollbar.h"
#include "ewl_range.h"
#include "ewl_macros.h"
#include "ewl_private.h"
#include "ewl_debug.h"
#include <math.h>

#define HIST_NUM 20

/* Normal scrolling functions */
static void ewl_scrollpane_cb_mouse_down_normal(Ewl_Widget *w, void *ev, void *data);
static void ewl_scrollpane_cb_mouse_up_normal(Ewl_Widget *w, void *ev, void *data);
static void ewl_scrollpane_cb_mouse_move_normal(Ewl_Widget *w, void *ev, void *data);
static int ewl_scrollpane_cb_scroll_timer_normal(void *data);

typedef struct Ewl_Scrollpane_Scroll_Info_Normal Ewl_Scrollpane_Scroll_Info_Normal;

/**
 * @brief Stores information on a normal scrollpane
 */
struct Ewl_Scrollpane_Scroll_Info_Normal
{
        int x;
        int y;
        int xc;
        int yc;
        double vel_x;
        double vel_y;
};

typedef struct Ewl_Scrollpane_Scroll_Info_Embedded Ewl_Scrollpane_Scroll_Info_Embedded;

/**
 * @brief Stores information on an embedded scrollpane
 */
struct Ewl_Scrollpane_Scroll_Info_Embedded
{
        int xs;
        int ys;
        double vel_x;
        double vel_y;
        double at;

        struct
        {
        	int x;
        	int y;
        	double time;
        } back[HIST_NUM];
};

/* Iphonish (embedded) scrolling functions */
static void ewl_scrollpane_cb_mouse_down_embedded(Ewl_Widget *w, void *ev, void *data);
static void ewl_scrollpane_cb_mouse_up_embedded(Ewl_Widget *w, void *ev, void *data);
static void ewl_scrollpane_cb_mouse_move_embedded(Ewl_Widget *w, void *ev, void *data);
static int ewl_scrollpane_cb_scroll_timer_embedded(void *data);
static void ewl_scrollpane_cb_scroll(Ewl_Scrollpane *s, double x, double y,
        						int *tx, int *ty);
static void ewl_scrollpane_cb_destroy(Ewl_Widget *w, void *ev, void *data);

/**
 * @return Returns a new scrollpane on success, NULL on failure.
 * @brief Create a new scrollpane
 */
Ewl_Widget *
ewl_scrollpane_new(void)
{
        Ewl_Scrollpane *s;

        DENTER_FUNCTION(DLEVEL_STABLE);

        s = NEW(Ewl_Scrollpane, 1);
        if (!s)
        	DRETURN_PTR(NULL, DLEVEL_STABLE);

        if (!ewl_scrollpane_init(s)) {
        	ewl_widget_destroy(EWL_WIDGET(s));
        	s = NULL;
        }

        DRETURN_PTR(EWL_WIDGET(s), DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to initialize
 * @return Returns no value.
 * @brief Initialize the fields of a scrollpane
 *
 * Sets up default callbacks and field values for the scrollpane @a s.
 */
int
ewl_scrollpane_init(Ewl_Scrollpane *s)
{
        Ewl_Widget *w;
        const char *kst;
        Ewl_Kinetic_Scroll type;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, FALSE);

        w = EWL_WIDGET(s);

        if (!ewl_container_init(EWL_CONTAINER(s)))
        	DRETURN_INT(FALSE, DLEVEL_STABLE);

        ewl_widget_appearance_set(w, EWL_SCROLLPANE_TYPE);
        ewl_widget_inherit(w, EWL_SCROLLPANE_TYPE);
        ewl_widget_focusable_set(EWL_WIDGET(s), TRUE);
        ewl_object_fill_policy_set(EWL_OBJECT(s), EWL_FLAG_FILL_ALL);

        ewl_container_callback_notify(EWL_CONTAINER(s), EWL_CALLBACK_FOCUS_IN);
        ewl_container_callback_notify(EWL_CONTAINER(s), EWL_CALLBACK_FOCUS_OUT);

        /* Remove the default focus out callback and replace with our own */
        ewl_callback_del(w, EWL_CALLBACK_FOCUS_OUT, ewl_widget_cb_focus_out);
        ewl_callback_append(w, EWL_CALLBACK_FOCUS_OUT,
        			ewl_container_cb_container_focus_out, NULL);


        s->hflag = EWL_SCROLLPANE_FLAG_AUTO_VISIBLE;
        s->vflag = EWL_SCROLLPANE_FLAG_AUTO_VISIBLE;

        s->overlay = ewl_overlay_new();
        ewl_object_fill_policy_set(EWL_OBJECT(s->overlay), EWL_FLAG_FILL_ALL);
        ewl_container_child_append(EWL_CONTAINER(s), s->overlay);
        ewl_widget_internal_set(s->overlay, TRUE);
        ewl_widget_show(s->overlay);

        /*
         * Create the container to hold the contents and it's configure
         * callback to position it's child.
         */
        s->box = ewl_vbox_new();
        ewl_object_fill_policy_set(EWL_OBJECT(s->box), EWL_FLAG_FILL_FILL);
        ewl_container_child_append(EWL_CONTAINER(s->overlay), s->box);
        ewl_widget_internal_set(s->box, TRUE);
        ewl_widget_show(s->box);

        /*
         * Create the scrollbars for the scrollpane.
         */
        s->hscrollbar = ewl_hscrollbar_new();
        ewl_container_child_append(EWL_CONTAINER(s), s->hscrollbar);
        ewl_widget_internal_set(s->hscrollbar, TRUE);
        ewl_widget_show(s->hscrollbar);

        s->vscrollbar = ewl_vscrollbar_new();
        ewl_widget_internal_set(s->vscrollbar, TRUE);
        ewl_container_child_append(EWL_CONTAINER(s), s->vscrollbar);
        ewl_widget_show(s->vscrollbar);

        /* after we added our internal widgets we can redirect the 
         * scrollpane to the content box */
        ewl_container_redirect_set(EWL_CONTAINER(s), EWL_CONTAINER(s->box));

        /*
         * Append necessary callbacks for the scrollpane.
         */
        ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
        		ewl_scrollpane_cb_configure, NULL);
        ewl_callback_append(w, EWL_CALLBACK_FOCUS_IN,
        		ewl_scrollpane_cb_focus_jump, NULL);

        /*
         * We need to know whent he scrollbars have value changes in order to
         * know when to scroll.
         */
        ewl_callback_append(s->hscrollbar, EWL_CALLBACK_VALUE_CHANGED,
        				ewl_scrollpane_cb_hscroll, s);
        ewl_callback_append(s->vscrollbar, EWL_CALLBACK_VALUE_CHANGED,
        				ewl_scrollpane_cb_vscroll, s);
        ewl_callback_append(w, EWL_CALLBACK_MOUSE_WHEEL,
        			ewl_scrollpane_cb_wheel_scroll, NULL);

        /*
         * Setup kinetic scrolling info here
         */
        s->kinfo = NULL;

        kst = ewl_theme_data_str_get(w, "/scrollpane/kscroll_type");

        if (kst && !strcmp(kst, "embedded"))
        	type = EWL_KINETIC_SCROLL_EMBEDDED;
        else if (kst && !strcmp(kst, "normal"))
        	type = EWL_KINETIC_SCROLL_NORMAL;
        else
        	type = EWL_KINETIC_SCROLL_NONE;
        	
        ewl_scrollpane_kinetic_scrolling_set(s, type);
        ewl_callback_append(w, EWL_CALLBACK_DESTROY,
        			ewl_scrollpane_cb_destroy, NULL);

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param s: The scrollpane to setup
 * @param type: To use kinetic scrolling or not
 * @return Returns no value
 * @brief Sets up default values and callbacks for kinetic scrolling
 */
void
ewl_scrollpane_kinetic_scrolling_set(Ewl_Scrollpane *s, Ewl_Kinetic_Scroll type)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPANE_TYPE);

        /* If set to current value we have nothing to do */
        if ((s->type) && (type == s->type))
        	DRETURN(DLEVEL_STABLE);

        /* Remove all present callbacks and free the kinfo */
        if ((s->type == EWL_KINETIC_SCROLL_NORMAL) && (s->kinfo))
        {
        	ewl_callback_del(s->overlay, EWL_CALLBACK_MOUSE_DOWN,
        			ewl_scrollpane_cb_mouse_down_normal);
        	ewl_callback_del(s->overlay, EWL_CALLBACK_MOUSE_UP,
        			ewl_scrollpane_cb_mouse_up_normal);
        	ewl_callback_del(s->overlay, EWL_CALLBACK_MOUSE_MOVE,
        			ewl_scrollpane_cb_mouse_move_normal);
        }

        else if ((s->type == EWL_KINETIC_SCROLL_EMBEDDED) && (s->kinfo))
        {
        	ewl_callback_del(s->overlay, EWL_CALLBACK_MOUSE_DOWN,
        			ewl_scrollpane_cb_mouse_down_embedded);
        	ewl_callback_del(s->overlay, EWL_CALLBACK_MOUSE_UP,
        			ewl_scrollpane_cb_mouse_up_embedded);
        	ewl_callback_del(s->overlay, EWL_CALLBACK_MOUSE_MOVE,
        			ewl_scrollpane_cb_mouse_move_embedded);
        }
        if (s->kinfo)
        {
        	IF_FREE(s->kinfo->extra)
        }
        else
        {
        	s->kinfo = NEW(Ewl_Scrollpane_Scroll_Info_Base, 1);
        	s->kinfo->fps = 15;
        	s->kinfo->vmax = 50.0;
        	s->kinfo->vmin = 0.0;
        	s->kinfo->dampen = 0.95;
        }

        if (type == EWL_KINETIC_SCROLL_NORMAL)
        {
        	ewl_callback_append(s->overlay, EWL_CALLBACK_MOUSE_DOWN,
        			ewl_scrollpane_cb_mouse_down_normal, s);
        	ewl_callback_append(s->overlay, EWL_CALLBACK_MOUSE_UP,
        			ewl_scrollpane_cb_mouse_up_normal, s);
        	ewl_callback_append(s->overlay, EWL_CALLBACK_MOUSE_MOVE,
        			ewl_scrollpane_cb_mouse_move_normal, s);
        	s->kinfo->extra = NEW(Ewl_Scrollpane_Scroll_Info_Normal, 1);
        }

        else if (type == EWL_KINETIC_SCROLL_EMBEDDED)
        {
        	ewl_callback_append(s->overlay, EWL_CALLBACK_MOUSE_DOWN,
        			ewl_scrollpane_cb_mouse_down_embedded, s);
        	ewl_callback_append(s->overlay, EWL_CALLBACK_MOUSE_UP,
        			ewl_scrollpane_cb_mouse_up_embedded, s);
        	ewl_callback_append(s->overlay, EWL_CALLBACK_MOUSE_MOVE,
        			ewl_scrollpane_cb_mouse_move_embedded, s);

        	s->kinfo->extra = NEW(Ewl_Scrollpane_Scroll_Info_Embedded, 1);
        }

        s->type = type;
        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: The scrollpane to use
 * @return Returns the type of scrolling used
 * @brief Gets the type of kinetic scrolling used
 */
Ewl_Kinetic_Scroll
ewl_scrollpane_kinetic_scrolling_get(Ewl_Scrollpane *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, EWL_KINETIC_SCROLL_NONE);
        DCHECK_TYPE_RET(s, EWL_SCROLLPANE_TYPE, EWL_KINETIC_SCROLL_NONE);

        DRETURN_INT(s->type, DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane that contains the scrollbar to change
 * @param f: the flags to set on the horizontal scrollbar in @a s
 * @return Returns no value.
 * @brief Set flags for horizontal scrollbar
 *
 * The scrollbar flags for the horizontal scrollbar are set to @a f.
 */
void
ewl_scrollpane_hscrollbar_flag_set(Ewl_Scrollpane *s, Ewl_Scrollpane_Flags f)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPANE_TYPE);

        s->hflag = f;

        if (f & EWL_SCROLLPANE_FLAG_ALWAYS_HIDDEN) {
        	unsigned int fill;
        	fill = ewl_object_fill_policy_get(EWL_OBJECT(s->box));
        	ewl_object_fill_policy_set(EWL_OBJECT(s->box),
        			fill | EWL_FLAG_FILL_HSHRINK);
        }

        ewl_widget_configure(EWL_WIDGET(s));

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane that contains the scrollbar to change
 * @param f: the flags to set on the vertical scrollbar in @a s
 * @return Returns no value.
 * @brief Set flags for vertical scrollbar
 *
 * The scrollbar flags for the vertical scrollbar are set to @a f.
 */
void
ewl_scrollpane_vscrollbar_flag_set(Ewl_Scrollpane *s, Ewl_Scrollpane_Flags f)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPANE_TYPE);

        s->vflag = f;

        if (f & EWL_SCROLLPANE_FLAG_ALWAYS_HIDDEN) {
        	unsigned int fill;
        	fill = ewl_object_fill_policy_get(EWL_OBJECT(s->box));
        	ewl_object_fill_policy_set(EWL_OBJECT(s->box),
        			fill | EWL_FLAG_FILL_VSHRINK);
        }

        ewl_widget_configure(EWL_WIDGET(s));

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane that contains the scrollbar to retrieve
 * @return Returns the flags of the horizontal scrollbar, 0 on failure.
 * @brief Get flags for horizontal scrollbar
 */
Ewl_Scrollpane_Flags
ewl_scrollpane_hscrollbar_flag_get(Ewl_Scrollpane *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, 0);
        DCHECK_TYPE_RET(s, EWL_SCROLLPANE_TYPE, 0);

        DRETURN_INT(s->hflag, DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane that contains the scrollbar to retrieve
 * @return Returns the flags of the vertical scrollbar on success, 0 on failure.
 * @brief Get flags for vertical scrollbar
 */
Ewl_Scrollpane_Flags
ewl_scrollpane_vscrollbar_flag_get(Ewl_Scrollpane *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, 0);
        DCHECK_TYPE_RET(s, EWL_SCROLLPANE_TYPE, 0);

        DRETURN_INT(s->vflag, DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to retrieve it's horizontal scrollbar value
 * @return Returns the value of the horizontal scrollbar in @a s on success.
 * @brief Retrieves the value of the horizontal scrollbar in @a s.
 */
double
ewl_scrollpane_hscrollbar_value_get(Ewl_Scrollpane *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, 0.0);
        DCHECK_TYPE_RET(s, EWL_SCROLLPANE_TYPE, 0.0);

        DRETURN_FLOAT(ewl_scrollbar_value_get(EWL_SCROLLBAR(s->hscrollbar)),
        							DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to retrieve it's vertical scrollbar value
 * @return Returns the value of the vertical scrollbar in @a s on success.
 * @brief Retrieves the value of the vertical scrollbar in @a s.
 */
double
ewl_scrollpane_vscrollbar_value_get(Ewl_Scrollpane *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, 0.0);
        DCHECK_TYPE_RET(s, EWL_SCROLLPANE_TYPE, 0.0);

        DRETURN_FLOAT(ewl_scrollbar_value_get(EWL_SCROLLBAR(s->vscrollbar)),
        							DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to set the horizontal scrollbar value
 * @param val: the value to set the scrollbar too
 * @return Returns nothing
 * @brief Set the value of the horizontal scrollbar in @a s to @a val
 */
void
ewl_scrollpane_hscrollbar_value_set(Ewl_Scrollpane *s, double val)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPANE_TYPE);

        ewl_scrollbar_value_set(EWL_SCROLLBAR(s->hscrollbar), val);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to set the vertical scrollbar value
 * @param val: the value to set the scrollbar too
 * @return Returns nothing
 * @brief Set the value of the vertical scrollbar in @a s to @a val
 */
void
ewl_scrollpane_vscrollbar_value_set(Ewl_Scrollpane *s, double val)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPANE_TYPE);

        ewl_scrollbar_value_set(EWL_SCROLLBAR(s->vscrollbar), val);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to retrieve its vertical scrollbar stepping
 * @return Returns the value of the stepping of the vertical scrollbar
 *        	in @a s on success.
 * @brief Retrives the value of the stepping of the vertical scrollbar in @a s.
 */
double
ewl_scrollpane_hscrollbar_step_get(Ewl_Scrollpane *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, 0.0);
        DCHECK_TYPE_RET(s, EWL_SCROLLPANE_TYPE, 0.0);

        DRETURN_FLOAT(ewl_scrollbar_step_get(EWL_SCROLLBAR(s->hscrollbar)),
        							DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to retrieve its vertical scrollbar stepping
 * @return Returns the value of the stepping of the vertical scrollbar
 *        	in @a s on success.
 * @brief Retrives the value of the stepping of the vertical scrollbar in @a s.
 */
double
ewl_scrollpane_vscrollbar_step_get(Ewl_Scrollpane *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, 0.0);
        DCHECK_TYPE_RET(s, EWL_SCROLLPANE_TYPE, 0.0);

        DRETURN_FLOAT(ewl_scrollbar_step_get(EWL_SCROLLBAR(s->vscrollbar)),
        							DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @param ev_data: UNUSED
 * @param user_data: UNUSED
 * @return Returns no value
 * @brief Move the contents of the scrollbar into place
 */
void
ewl_scrollpane_cb_configure(Ewl_Widget *w, void *ev_data __UNUSED__,
        				void *user_data __UNUSED__)
{
        Ewl_Scrollpane *s;
        int vs_width = 0, hs_height = 0;
        int b_width, b_height;
        int content_w, content_h;
        unsigned int old_fill, box_fill = EWL_FLAG_FILL_FILL;
        double hstep = 1.0, vstep = 1.0;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(w);
        DCHECK_TYPE(w, EWL_WIDGET_TYPE);

        s = EWL_SCROLLPANE(w);

        /*
         * Get the space needed by the scrolbars.
         */
        vs_width = ewl_object_preferred_w_get(EWL_OBJECT(s->vscrollbar));
        hs_height = ewl_object_preferred_h_get(EWL_OBJECT(s->hscrollbar));

        /*
         * Determine the space used by the contents.
         */
        content_w = CURRENT_W(w);
        content_h = CURRENT_H(w);

        /*
         * FIXME: This is exposing box internals, should probably just make a
         * dumb box for the scrollpane.
         * Force the box to recalculate preferred size to work around children
         * with shrink fill policies.
         */
        ewl_container_largest_prefer(EWL_CONTAINER(s->box),
        				EWL_ORIENTATION_HORIZONTAL);
        ewl_container_sum_prefer(EWL_CONTAINER(s->box),
        				EWL_ORIENTATION_VERTICAL);

        /*
         * Get the preferred size of contents to scroll correctly.
         */
        b_width = ewl_object_preferred_w_get(EWL_OBJECT(s->box));
        b_height = ewl_object_preferred_h_get(EWL_OBJECT(s->box));

        /*
         * Calculate initial steps.
         */
        if (content_w < b_width)
        	hstep = (double)content_w / (double)b_width;
        if (content_h < b_height)
        	vstep = (double)content_h / (double)b_height;

        /*
         * Determine visibility of scrollbars based on the flags.
         */
        if (s->hflag == EWL_SCROLLPANE_FLAG_NONE ||
        		(hstep < 1.0 &&
        		 s->hflag == EWL_SCROLLPANE_FLAG_AUTO_VISIBLE))
        	ewl_widget_show(s->hscrollbar);
        else {
        	box_fill |= EWL_FLAG_FILL_HSHRINK;
        	ewl_widget_hide(s->hscrollbar);
        }

        if (s->vflag == EWL_SCROLLPANE_FLAG_NONE ||
        		(vstep < 1.0 &&
        		 s->vflag == EWL_SCROLLPANE_FLAG_AUTO_VISIBLE))
        	ewl_widget_show(s->vscrollbar);
        else {
        	box_fill |= EWL_FLAG_FILL_VSHRINK;
        	ewl_widget_hide(s->vscrollbar);
        }

        /*
         * Adjust the step and width dependant on scrollbar visibility.
         */
        if (VISIBLE(s->hscrollbar)) {
        	content_h -= hs_height;
        	if (content_h < b_height)
        		vstep = (double)content_h / (double)b_height;
        }

        if (VISIBLE(s->vscrollbar)) {
        	content_w -= vs_width;
        	if (content_w < b_width)
        		hstep = (double)content_w / (double)b_width;
        }

        /*
         * Ensure the step is not negative.
         */
        if (hstep == 1.0)
        	b_width = content_w;

        if (vstep == 1.0)
        	b_height = content_h;

        /*
         * Calcuate the offset for the box position
         */
        b_width = (int)(ewl_scrollbar_value_get(EWL_SCROLLBAR(s->hscrollbar)) *
        		(double)(b_width - content_w));
        b_height = (int)(ewl_scrollbar_value_get(EWL_SCROLLBAR(s->vscrollbar)) *
        		(double)(b_height - content_h));

        /*
         * Assign the step values to the scrollbars to adjust scale.
         */
        ewl_scrollbar_step_set(EWL_SCROLLBAR(s->hscrollbar), hstep);
        ewl_scrollbar_step_set(EWL_SCROLLBAR(s->vscrollbar), vstep);

        /*
         * Set the fill policy on the box based on scrollbars visible.
         */
        old_fill = ewl_object_fill_policy_get(EWL_OBJECT(s->box));
        ewl_object_fill_policy_set(EWL_OBJECT(s->box), box_fill);

        /*
         * Position the horizontal scrollbar.
         */
        ewl_object_geometry_request(EWL_OBJECT(s->hscrollbar),
        				CURRENT_X(w), CURRENT_Y(w) + content_h,
        				content_w, hs_height);

        /*
         * Position the vertical scrollbar.
         */
        ewl_object_geometry_request(EWL_OBJECT(s->vscrollbar),
        				CURRENT_X(w) + content_w, CURRENT_Y(w),
        				vs_width, content_h);

        /*
         * Now move the box into position. For the scrollpane to work we move
         * the box relative to the scroll value.
         */
        ewl_object_geometry_request(EWL_OBJECT(s->overlay),
        				CURRENT_X(w), CURRENT_Y(w),
        				content_w, content_h);
        ewl_object_geometry_request(EWL_OBJECT(s->box),
        				CURRENT_X(w) - b_width,
        				CURRENT_Y(w) - b_height,
        				content_w + b_width,
        				content_h + b_height);

        /*
         * Reset the default fill policy on the box to get updated sizes..
         */
        ewl_object_fill_policy_set(EWL_OBJECT(s->box), old_fill);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @param ev_data: UNUSED
 * @param user_data: UNUSED
 * @return Returns no value
 * @brief The focus jump callback
 */
void
ewl_scrollpane_cb_focus_jump(Ewl_Widget *w, void *ev_data __UNUSED__,
        				void *user_data __UNUSED__)
{
        int endcoord = 0;
        double value;
        int fx, fy, fw, fh;
        Ewl_Embed *emb;
        Ewl_Widget *focus;
        Ewl_Widget *bar = NULL;
        Ewl_Scrollpane *s = EWL_SCROLLPANE(w);

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(w);
        DCHECK_TYPE(w, EWL_SCROLLPANE_TYPE);

        emb = ewl_embed_widget_find(w);
        if (!emb)
        	DRETURN(DLEVEL_STABLE);

        /*
         * Get the focused widget and stop if its an internal one.
         */
        focus = ewl_embed_focused_widget_get(emb);
        if (!focus || !ewl_widget_parent_of(s->box, focus) ||
        		ewl_widget_onscreen_is(focus))
        	DRETURN(DLEVEL_STABLE);

        ewl_object_current_geometry_get(EWL_OBJECT(focus), &fx, &fy, &fw, &fh);

        /*
         * Adjust horizontally to show the focused widget
         */
        if (fx < CURRENT_X(s->overlay)) {
        	bar = s->hscrollbar;
        	endcoord = fx;
        }
        else if (fx + fw > CURRENT_X(s->overlay) + CURRENT_W(s->overlay)) {
        	bar = s->hscrollbar;
        	endcoord = fx + fw;
        }

        if (bar) {
        	value = (double)endcoord /
        		(double)(ewl_object_current_x_get(EWL_OBJECT(s->box)) +
        			 ewl_object_preferred_w_get(EWL_OBJECT(s->box)));
        	ewl_scrollbar_value_set(EWL_SCROLLBAR(bar), value);
        }

        /*
         * Adjust vertically to show the focused widget
         */
        if (fy < CURRENT_Y(s->overlay)) {
        	bar = s->vscrollbar;
        	endcoord = fy;
        }
        else if (fy + fh > CURRENT_Y(s->overlay) + CURRENT_H(s->overlay)) {
        	bar = s->vscrollbar;
        	endcoord = fy + fh;
        }

        /*
         * Adjust the value of the scrollbar to jump to the position
         */
        if (bar) {
        	value = (double)endcoord /
        		(double)(ewl_object_current_y_get(EWL_OBJECT(s->box)) +
        			 ewl_object_preferred_h_get(EWL_OBJECT(s->box)));
        	ewl_scrollbar_value_set(EWL_SCROLLBAR(bar), value);
        }

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: UNUSED
 * @param ev_data: UNUSED
 * @param user_data: The scrollbar
 * @return Returns no value
 * @brief When a horizontal scrollbar is clicked we need to move the
 * contents of the scrollpane horizontally.
 */
void
ewl_scrollpane_cb_hscroll(Ewl_Widget *w __UNUSED__,
        	void *ev_data __UNUSED__, void *user_data)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(user_data);

        ewl_callback_call(user_data, EWL_CALLBACK_VALUE_CHANGED);
        ewl_widget_configure(user_data);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 */
/**
 * @internal
 * @param w: UNUSED
 * @param ev_data: UNUSED
 * @param user_data: The scrollbar
 * @return Returns no value
 * @brief When a vertical scrollbar is clicked we need to move the
 * contents of the scrollpane vertically.
 */
void
ewl_scrollpane_cb_vscroll(Ewl_Widget *w __UNUSED__, void *ev_data __UNUSED__,
        					void *user_data)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(user_data);

        ewl_callback_call(user_data, EWL_CALLBACK_VALUE_CHANGED);
        ewl_widget_configure(user_data);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param cb: The widget to work with
 * @param ev_data: The Ewl_Event_Mouse_Wheel data
 * @param user_data: UNUSED
 * @return Returns no value
 * @brief The wheel scroll callback
 */
void
ewl_scrollpane_cb_wheel_scroll(Ewl_Widget *cb, void *ev_data,
        			void *user_data __UNUSED__)
{
        Ewl_Scrollpane *s;
        Ewl_Event_Mouse_Wheel *ev;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(cb);
        DCHECK_TYPE(cb, EWL_WIDGET_TYPE);

        s = EWL_SCROLLPANE(cb);
        ev = ev_data;
        ewl_scrollpane_vscrollbar_value_set(s,
        		ewl_scrollpane_vscrollbar_value_get(s) +
        		ev->z * ewl_scrollpane_vscrollbar_step_get(s));

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @ev_data: The Ewl_Event_Mouse_Down data
 * @param data: The scrollpane
 * @return Returns no value
 * @brief The mouse down setting up kinetic scrolling
 */
static void
ewl_scrollpane_cb_mouse_down_normal(Ewl_Widget *w, void *ev, void *data)
{
        Ewl_Scrollpane *s;
        Ewl_Event_Mouse *md;
        Ewl_Scrollpane_Scroll_Info_Normal *info;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(ev);
        DCHECK_PARAM_PTR(data);
        DCHECK_TYPE(w, EWL_WIDGET_TYPE);

        s = EWL_SCROLLPANE(data);
        md = EWL_EVENT_MOUSE(ev);
        info = s->kinfo->extra;
        info->vel_x = 0.0;
        info->vel_y = 0.0;
        info->x = md->x;
        info->y = md->y;
        s->kinfo->clicked = !!TRUE;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @ev_data: The Ewl_Event_Mouse_Down data
 * @param data: The scrollpane
 * @return Returns no value
 * @brief The mouse down function for kinetic scrolling
 */
static void
ewl_scrollpane_cb_mouse_down_embedded(Ewl_Widget *w, void *ev, void *data)
{
        Ewl_Scrollpane *s;
        Ewl_Event_Mouse *md;
        Ewl_Scrollpane_Scroll_Info_Embedded *info;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(ev);
        DCHECK_PARAM_PTR(data);
        DCHECK_TYPE(w, EWL_WIDGET_TYPE);

        s = EWL_SCROLLPANE(data);
        md = EWL_EVENT_MOUSE(ev);
        info = s->kinfo->extra;
        s->kinfo->clicked = !!TRUE;
        s->kinfo->active = !!FALSE;

        memset(&(info->back[0]), 0, sizeof(info->back[0]) * HIST_NUM);
        info->back[0].x = md->x;
        info->back[0].y = md->y;
        info->back[0].time = ecore_time_get();
        info->xs = md->x;
        info->ys = md->y;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @ev_data: The Ewl_Event_Mouse_Move data
 * @param data: The scrollpane
 * @return Returns no value
 * @brief The mouse move callback for kinetic scrolling
 */
static void
ewl_scrollpane_cb_mouse_move_normal(Ewl_Widget *w, void *ev, void *data)
{
        Ewl_Scrollpane *s;
        Ewl_Event_Mouse *mm;
        Ewl_Scrollpane_Scroll_Info_Normal *info;
        int cx, cy;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(ev);
        DCHECK_PARAM_PTR(data);

        s = EWL_SCROLLPANE(data);
        mm = EWL_EVENT_MOUSE(ev);
        info = s->kinfo->extra;

        if (!s->kinfo->clicked)
        	DRETURN(DLEVEL_STABLE);

        if (!s->kinfo->active)
        {
        	ecore_timer_add(1.0/s->kinfo->fps,
        				ewl_scrollpane_cb_scroll_timer_normal, s);
        	s->kinfo->active = !!TRUE;
        }

        info->xc = mm->x;
        info->yc = mm->y;
        cx = (info->xc - info->x);
        cy = (info->yc - info->y);

        /* v = (change in position / (width or height of scroll *
         *	(range of velocities) + min))
         */
        info->vel_x = ((cx / 
        	(double)ewl_object_current_w_get(EWL_OBJECT(w))) *
        	(s->kinfo->vmax - s->kinfo->vmin)) + s->kinfo->vmin;

        info->vel_y = ((cy /
        	(double)ewl_object_current_h_get(EWL_OBJECT(w))) *
        	(s->kinfo->vmax - s->kinfo->vmin)) + s->kinfo->vmin;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @ev_data: The Ewl_Event_Mouse_Move data
 * @param_data: The scrollpane
 * @return Returns no value
 * @brief The mouse move callback for kinetic scrolling
 */
static void
ewl_scrollpane_cb_mouse_move_embedded(Ewl_Widget *w, void *ev, void *data)
{
        Ewl_Scrollpane *s;
        Ewl_Event_Mouse *mm;
        Ewl_Scrollpane_Scroll_Info_Embedded *info;
        int x = 0, y = 0;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(ev);
        DCHECK_PARAM_PTR(data);

        s = EWL_SCROLLPANE(data);
        mm = EWL_EVENT_MOUSE(ev);
        info = s->kinfo->extra;

        if (!s->kinfo->clicked)
        	DRETURN(DLEVEL_STABLE);

        memmove(&(info->back[1]), &(info->back[0]), sizeof(info->back[0]) * (HIST_NUM - 1));
        info->back[0].x = mm->x;
        info->back[0].y = mm->y;
        info->back[0].time = ecore_time_get();

        /* Move accordingly here */
        x = info->xs - mm->x;
        y = info->ys - mm->y;

        ewl_scrollpane_cb_scroll(s, x, y, NULL, NULL);
        info->xs = mm->x;
        info->ys = mm->y;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @ev_data: The Ewl_Event_Mouse_Up data
 * @param data: The scrollpane
 * @return Returns no value
 * @brief The mouse up callback for kinetic scrolling
 */
static void
ewl_scrollpane_cb_mouse_up_normal(Ewl_Widget *w, void *ev, void *data)
{
        Ewl_Scrollpane *s;
        
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(data);

        s = EWL_SCROLLPANE(data);
        s->kinfo->clicked = !!FALSE;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @ev_data: The Ewl_Event_Mouse_Up data
 * @data: The scrollpane
 * @return Returns no value
 * @brief The mouse up callback for kinetic scrolling
 */
static void
ewl_scrollpane_cb_mouse_up_embedded(Ewl_Widget *w, void *ev, void *data)
{
        Ewl_Scrollpane *s;
        Ewl_Event_Mouse *mm;
        Ewl_Scrollpane_Scroll_Info_Embedded *info;
        int ax, ay, dx, dy, i;
        double at, dt, t;
        int rx = 1, ry = 1;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(data);

        s = EWL_SCROLLPANE(data);
        mm = EWL_EVENT_MOUSE(ev);
        s->kinfo->clicked = !!FALSE;
        s->kinfo->active = !!TRUE;
        info = s->kinfo->extra;

        t = ecore_time_get();
        ax = mm->x;
        ay = mm->y;
        at = 0.0;

        for (i = 0; i < HIST_NUM; i++)
        {
        	dt = t - info->back[i].time;
        	if (dt > 0.2) break;
        	at = at + dt;
        	ax = ax + info->back[i].x;
        	ay = ay + info->back[i].y;
        }

        ax = (ax / (i + 1));
        ay = (ay / (i + 1));
        at = (at / (i + 1));
        at = at * 4.0;
        dx = mm->x - ax;
        dy = mm->y - ay;

        info->vel_x = (double)dx / at;
        info->vel_y = (double)dy / at;

        if (info->vel_y < 0)
        	ry = -1;
        if (info->vel_x < 0)
        	rx = -1;

        /* This should do something better */
        info->vel_x = sqrt(info->vel_x * rx);
        info->vel_y = sqrt(info->vel_y * ry);

        /* Set to minimum velocity if below */
        if (abs(info->vel_x) < s->kinfo->vmin)
        	info->vel_x = s->kinfo->vmin;
        else if (abs(info->vel_x) > s->kinfo->vmax)
        	info->vel_x = s->kinfo->vmax;

        /* Check upper velocity */
        if (abs(info->vel_y) < s->kinfo->vmin)
        	info->vel_y = s->kinfo->vmin;
        else if (abs(info->vel_y) > s->kinfo->vmax)
        	info->vel_y = s->kinfo->vmax;

        /* Return to proper direction */
        info->vel_x = info->vel_x * rx;
        info->vel_y = info->vel_y * ry;

        info->at = at;
        ecore_timer_add(1/s->kinfo->fps, ewl_scrollpane_cb_scroll_timer_embedded, s);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param data: The scrollpane to work with
 * @return Returns 1 if the timer is to continue, 0 otherwise
 * @brief Performs some calculations then calls the scroll function
 */
static int
ewl_scrollpane_cb_scroll_timer_normal(void *data)
{
        Ewl_Scrollpane *s;
        Ewl_Scrollpane_Scroll_Info_Normal *info;
        double h, w;
        int tx = 0, ty = 0;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(data, FALSE);

        s = EWL_SCROLLPANE(data);
        info = s->kinfo->extra;

        /* If the mouse is down, accelerate and check velocity */
        if (!s->kinfo->clicked)
        {
        	info->vel_x *= s->kinfo->dampen;
        	info->vel_y *= s->kinfo->dampen;

        	h = info->vel_y * ((info->vel_y < 0) ? -1 : 1);
        	w = info->vel_x * ((info->vel_x < 0) ? -1 : 1);

        	if ((w < 0.5) && (h < 0.5))
        	{
        		s->kinfo->active = !!FALSE;
        		DRETURN_INT(0, DLEVEL_STABLE);
        	}
        }

        /* Actually scroll the pane */
        ewl_scrollpane_cb_scroll(s, info->vel_x, info->vel_y, &tx, &ty);

        /* If at the end of a scrollbar, set x/y to current */
        if (!tx)
        	info->x = info->xc;
        if (!ty)
        	info->y = info->yc;

        DRETURN_INT(1, DLEVEL_STABLE);
}

/**
 * @internal
 * @param data: The scrollpane to work with
 * @return Returns 1 if the timer is to continue, 0 otherwise
 * @brief Performs some calculations then calls the scroll functions
 */
static int
ewl_scrollpane_cb_scroll_timer_embedded(void *data)
{
        Ewl_Scrollpane *s;
        Ewl_Scrollpane_Scroll_Info_Embedded *info;
        double h, w, t;
        int tx = 0, ty = 0;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(data, FALSE);

        s = EWL_SCROLLPANE(data);
        info = s->kinfo->extra;

        if ((s->kinfo->clicked) || (!s->kinfo->active))
        	DRETURN_INT(FALSE, DLEVEL_STABLE);

        h = info->vel_y * ((info->vel_y < 0) ? -1 : 1);
        w = info->vel_x * ((info->vel_x < 0) ? -1 : 1);

        if ((w < 0.5) && (h < 0.5))
        {
        	s->kinfo->active = !!FALSE;
        	DRETURN_INT(FALSE, DLEVEL_STABLE);
        }

        t = 1.0 / (info->at * s->kinfo->fps);
        h = info->vel_y * -t;
        w = info->vel_x * -t;
        
        ewl_scrollpane_cb_scroll(s, w, h, &tx, &ty);

        if (!tx && !ty)
        	DRETURN_INT(FALSE, DLEVEL_STABLE);

        info->vel_x *= s->kinfo->dampen;
        info->vel_y *= s->kinfo->dampen;

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @internal
 * @param s: The scrollpane to work with
 * @param x: The horizontal velocity
 * @param y: The vertical velocity
 * @param tx: Pointer to integer tested
 * @param ty: Pointer to integer tested
 * @return Returns no value
 * @brief Scrolls the scrollpane based on the given parameters
 */
static void
ewl_scrollpane_cb_scroll(Ewl_Scrollpane *s, double x, double y,
        					int *tx, int *ty)
{
        double w, h;
        Ewl_Scrollbar *ry, *rx;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPANE_TYPE);

        ry = EWL_SCROLLBAR(s->vscrollbar);
        rx = EWL_SCROLLBAR(s->hscrollbar);

        if (!((ewl_scrollpane_vscrollbar_value_get(s) == 1.0) &&
        			(y > 0)) &&
        		!((ewl_scrollpane_vscrollbar_value_get(s) == 0.0) &&
        			(y < 0)))
        {
        	h = ewl_scrollpane_vscrollbar_value_get(s) +
        		(y / (double)ewl_object_preferred_h_get(EWL_OBJECT(s->box)));

        	/* If h is greater than possible setting, set to remainder */
        	if (h > ewl_range_maximum_value_get(EWL_RANGE(ry->seeker)))
        	{
        		h = ewl_range_maximum_value_get(EWL_RANGE(ry->seeker));
        		if (ty) *ty = FALSE;
        	}
        	else if (h < ewl_range_minimum_value_get(EWL_RANGE(ry->seeker)))
        	{
        		h = ewl_range_minimum_value_get(EWL_RANGE(ry->seeker));
        		if (ty) *ty = FALSE;
        	}
        	else
        		if (ty) *ty = TRUE;

        	ewl_scrollpane_vscrollbar_value_set(s, h);
        }

        if (!((ewl_scrollpane_hscrollbar_value_get(s) == 1.0) &&
        			(x > 0)) &&
        		!((ewl_scrollpane_hscrollbar_value_get(s) == 0.0) &&
        			(x < 0)))
        {
        	w = ewl_scrollpane_hscrollbar_value_get(s) +
        		(x / (double)ewl_object_preferred_w_get(EWL_OBJECT(s->box)));

        	/* And again for the w */
        	if (w > ewl_range_maximum_value_get(EWL_RANGE(rx->seeker)))
        	{
        		w = ewl_range_maximum_value_get(EWL_RANGE(rx->seeker));
        		if (tx) *tx = FALSE;
        	}
        	else if (w < ewl_range_minimum_value_get(EWL_RANGE(rx->seeker)))
        	{
        		w = ewl_range_minimum_value_get(EWL_RANGE(rx->seeker));
        		if (tx) *tx = FALSE;
        	}
        	else
        		if (tx) *tx = TRUE;

        	ewl_scrollpane_hscrollbar_value_set(s, w);
        }

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: The scrollpane to work with
 * @param v: The maximum velocity
 * @return Returns no value
 * @brief Sets the maximum velocity for kinetic scrolling
 */
void
ewl_scrollpane_kinetic_max_velocity_set(Ewl_Scrollpane *s, double v)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPANE_TYPE);

        s->kinfo->vmax = v;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: The scrollpane to work with
 * @return Returns the maximum velocity
 * @brief Gets the maximum velocity for kinetic scrolling
 */
double
ewl_scrollpane_kinetic_max_velocity_get(Ewl_Scrollpane *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, -1);
        DCHECK_TYPE_RET(s, EWL_SCROLLPANE_TYPE, -1);

        DRETURN_INT(s->kinfo->vmax, DLEVEL_STABLE);
}

/**
 * @param s: The scrollpane to work with
 * @param v: The minimum velocity
 * @return Returns no value
 * @brief Sets the minimum velocity for kinetic scrolling
 */
void
ewl_scrollpane_kinetic_min_velocity_set(Ewl_Scrollpane *s, double v)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPANE_TYPE);

        s->kinfo->vmin = v;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: The scrollpane to work with
 * @return Returns the minimum velocity
 * @brief Gets the minimum velocity for kinetic scrolling
 */
double
ewl_scrollpane_kinetic_min_velocity_get(Ewl_Scrollpane *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, -1);
        DCHECK_TYPE_RET(s, EWL_SCROLLPANE_TYPE, -1);

        DRETURN_INT(s->kinfo->vmin, DLEVEL_STABLE);
}

/**
 * @param s: The scrollpane to work with
 * @param d: The multiplier to reduce velocity
 * @return Returns no value
 * @brief Sets the multiplier to reduce the velocity of kinetic scrolling
 */
void
ewl_scrollpane_kinetic_dampen_set(Ewl_Scrollpane *s, double d)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPANE_TYPE);

        s->kinfo->dampen = d;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: The scrollpane to work with
 * @return Returns the minimum velocity
 * @brief Gets the minimum velocity for kinetic scrolling
 */
double
ewl_scrollpane_kinetic_dampen_get(Ewl_Scrollpane *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, -1);
        DCHECK_TYPE_RET(s, EWL_SCROLLPANE_TYPE, -1);

        DRETURN_INT(s->kinfo->dampen, DLEVEL_STABLE);
}

/**
 * @param s: The scrollpane to work with
 * @param fps: The desired frames per second
 * @return Returns no value
 * @brief Sets the number of times per second to recalculate velocity and update the tree
 */
void
ewl_scrollpane_kinetic_fps_set(Ewl_Scrollpane *s, int fps)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPANE_TYPE);

        s->kinfo->fps = fps;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: The scrollpane to work with
 * @return Returns the current frames per second
 * brief Gets the times per second the timer used for scrolling will be called
 */
int
ewl_scrollpane_kinetic_fps_get(Ewl_Scrollpane *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, -1);
        DCHECK_TYPE_RET(s, EWL_SCROLLPANE_TYPE, -1);

        DRETURN_INT(s->kinfo->fps, DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to use
 * @parma ev: The event data
 * @param data: User data
 * @return Returns no value
 * @brief Frees data from the scrollpane
 */
void
ewl_scrollpane_cb_destroy(Ewl_Widget *w, void *ev, void *data)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(w);
        DCHECK_TYPE(w, EWL_SCROLLPANE_TYPE);

        if (EWL_SCROLLPANE(w)->kinfo)
        	FREE(EWL_SCROLLPANE(w)->kinfo->extra);
        FREE(EWL_SCROLLPANE(w)->kinfo);


        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

