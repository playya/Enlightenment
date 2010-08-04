#include "ewl_base.h"
#include "ewl_scrollbar.h"
#include "ewl_scrollport.h"
#include "ewl_scrollport_kinetic.h"
#include "ewl_range.h"
#include "ewl_macros.h"
#include "ewl_private.h"
#include "ewl_debug.h"
#include <math.h>

#define HIST_NUM 20

/* Normal scrolling functions */
static void ewl_scrollport_kinetic_cb_mouse_down_normal(Ewl_Widget *w, void *ev, void *data);
static void ewl_scrollport_kinetic_cb_mouse_up_normal(Ewl_Widget *w, void *ev, void *data);
static void ewl_scrollport_kinetic_cb_mouse_move_normal(Ewl_Widget *w, void *ev, void *data);
static Eina_Bool ewl_scrollport_kinetic_cb_scroll_timer_normal(void *data);

typedef struct Ewl_Scrollport_Kinetic_Info_Normal Ewl_Scrollport_Kinetic_Info_Normal;

/**
 * @brief Stores information on a normal scrollport
 */
struct Ewl_Scrollport_Kinetic_Info_Normal
{
        int x;
        int y;
        int xc;
        int yc;
        double vel_x;
        double vel_y;
};

typedef struct Ewl_Scrollport_Kinetic_Info_Embedded Ewl_Scrollport_Kinetic_Info_Embedded;

/**
 * @brief Stores information on an embedded scrollport
 */
struct Ewl_Scrollport_Kinetic_Info_Embedded
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
static void ewl_scrollport_kinetic_cb_mouse_down_embedded(Ewl_Widget *w,
                                                void *ev, void *data);
static void ewl_scrollport_kinetic_cb_mouse_up_embedded(Ewl_Widget *w,
                                                void *ev, void *data);
static void ewl_scrollport_kinetic_cb_mouse_move_embedded(Ewl_Widget *w,
                                                void *ev, void *data);
static Eina_Bool ewl_scrollport_kinetic_cb_scroll_timer_embedded(void *data);
static void ewl_scrollport_kinetic_scroll(Ewl_Scrollport *s, double x,
                                                double y, int *tx, int *ty);
static void ewl_scrollport_kinetic_cb_destroy(Ewl_Widget *w, void *ev, void *data);


/**
 * @param s: The scrollport to setup
 * @param type: To use kinetic scrolling or not
 * @return Returns no value
 * @brief Sets up default values and callbacks for kinetic scrolling
 */
void
ewl_scrollport_kinetic_scrolling_set(Ewl_Scrollport *s, Ewl_Kinetic_Scroll type)
{
        Ewl_Widget *va;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPORT_TYPE);

        /* If set to current value we have nothing to do */
        if ((s->type) && (type == s->type))
                DRETURN(DLEVEL_STABLE);

        va = EWL_WIDGET(ewl_scrollport_visible_area_get(EWL_SCROLLPORT(s)));
        /* Remove all present callbacks and free the kinfo */
        if ((s->type == EWL_KINETIC_SCROLL_NORMAL) && (s->kinfo))
        {
                ewl_callback_del(va, EWL_CALLBACK_MOUSE_DOWN,
                                ewl_scrollport_kinetic_cb_mouse_down_normal);
                ewl_callback_del(va, EWL_CALLBACK_MOUSE_UP,
                                ewl_scrollport_kinetic_cb_mouse_up_normal);
                ewl_callback_del(va, EWL_CALLBACK_MOUSE_MOVE,
                                ewl_scrollport_kinetic_cb_mouse_move_normal);
        }
        else if ((s->type == EWL_KINETIC_SCROLL_EMBEDDED) && (s->kinfo))
        {
                ewl_callback_del(va, EWL_CALLBACK_MOUSE_DOWN,
                                ewl_scrollport_kinetic_cb_mouse_down_embedded);
                ewl_callback_del(va, EWL_CALLBACK_MOUSE_UP,
                                ewl_scrollport_kinetic_cb_mouse_up_embedded);
                ewl_callback_del(va, EWL_CALLBACK_MOUSE_MOVE,
                                ewl_scrollport_kinetic_cb_mouse_move_embedded);
        }

        if (s->kinfo)
                IF_FREE(s->kinfo->extra);
        else
        {
                s->kinfo = NEW(Ewl_Scrollport_Kinetic_Info, 1);
                s->kinfo->fps = 15;
                s->kinfo->vmax = 50.0;
                s->kinfo->vmin = 0.0;
                s->kinfo->dampen = 0.95;
                ewl_callback_append(EWL_WIDGET(s), EWL_CALLBACK_DESTROY,
                                ewl_scrollport_kinetic_cb_destroy, NULL);
        }

        if (type == EWL_KINETIC_SCROLL_NORMAL)
        {
                ewl_callback_append(va, EWL_CALLBACK_MOUSE_DOWN,
                                ewl_scrollport_kinetic_cb_mouse_down_normal, s);
                ewl_callback_append(va, EWL_CALLBACK_MOUSE_UP,
                                ewl_scrollport_kinetic_cb_mouse_up_normal, s);
                ewl_callback_append(va, EWL_CALLBACK_MOUSE_MOVE,
                                ewl_scrollport_kinetic_cb_mouse_move_normal, s);
                s->kinfo->extra = NEW(Ewl_Scrollport_Kinetic_Info_Normal, 1);
        }
        else if (type == EWL_KINETIC_SCROLL_EMBEDDED)
        {
                ewl_callback_append(va, EWL_CALLBACK_MOUSE_DOWN,
                                ewl_scrollport_kinetic_cb_mouse_down_embedded,
                                s);
                ewl_callback_append(va, EWL_CALLBACK_MOUSE_UP,
                                ewl_scrollport_kinetic_cb_mouse_up_embedded, s);
                ewl_callback_append(va, EWL_CALLBACK_MOUSE_MOVE,
                                ewl_scrollport_kinetic_cb_mouse_move_embedded,
                                s);

                s->kinfo->extra = NEW(Ewl_Scrollport_Kinetic_Info_Embedded, 1);
        }

        s->type = type;
        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: The scrollport to use
 * @return Returns the type of scrolling used
 * @brief Gets the type of kinetic scrolling used
 */
Ewl_Kinetic_Scroll
ewl_scrollport_kinetic_scrolling_get(Ewl_Scrollport *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, EWL_KINETIC_SCROLL_NONE);
        DCHECK_TYPE_RET(s, EWL_SCROLLPORT_TYPE, EWL_KINETIC_SCROLL_NONE);

        DRETURN_INT(s->type, DLEVEL_STABLE);
}

/**
 * @param s: The scrollport to work with
 * @param v: The maximum velocity
 * @return Returns no value
 * @brief Sets the maximum velocity for kinetic scrolling
 */
void
ewl_scrollport_kinetic_max_velocity_set(Ewl_Scrollport *s, double v)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPORT_TYPE);

        s->kinfo->vmax = v;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: The scrollport to work with
 * @return Returns the maximum velocity
 * @brief Gets the maximum velocity for kinetic scrolling
 */
double
ewl_scrollport_kinetic_max_velocity_get(Ewl_Scrollport *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, -1);
        DCHECK_TYPE_RET(s, EWL_SCROLLPORT_TYPE, -1);

        DRETURN_INT(s->kinfo->vmax, DLEVEL_STABLE);
}

/**
 * @param s: The scrollport to work with
 * @param v: The minimum velocity
 * @return Returns no value
 * @brief Sets the minimum velocity for kinetic scrolling
 */
void
ewl_scrollport_kinetic_min_velocity_set(Ewl_Scrollport *s, double v)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPORT_TYPE);

        s->kinfo->vmin = v;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: The scrollport to work with
 * @return Returns the minimum velocity
 * @brief Gets the minimum velocity for kinetic scrolling
 */
double
ewl_scrollport_kinetic_min_velocity_get(Ewl_Scrollport *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, -1);
        DCHECK_TYPE_RET(s, EWL_SCROLLPORT_TYPE, -1);

        DRETURN_INT(s->kinfo->vmin, DLEVEL_STABLE);
}

/**
 * @param s: The scrollport to work with
 * @param d: The multiplier to reduce velocity
 * @return Returns no value
 * @brief Sets the multiplier to reduce the velocity of kinetic scrolling
 */
void
ewl_scrollport_kinetic_dampen_set(Ewl_Scrollport *s, double d)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPORT_TYPE);

        s->kinfo->dampen = d;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: The scrollport to work with
 * @return Returns the minimum velocity
 * @brief Gets the minimum velocity for kinetic scrolling
 */
double
ewl_scrollport_kinetic_dampen_get(Ewl_Scrollport *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, -1);
        DCHECK_TYPE_RET(s, EWL_SCROLLPORT_TYPE, -1);

        DRETURN_INT(s->kinfo->dampen, DLEVEL_STABLE);
}

/**
 * @param s: The scrollport to work with
 * @param fps: The desired frames per second
 * @return Returns no value
 * @brief Sets the number of times per second to recalculate velocity and update the tree
 */
void
ewl_scrollport_kinetic_fps_set(Ewl_Scrollport *s, int fps)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPORT_TYPE);

        s->kinfo->fps = fps;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: The scrollport to work with
 * @return Returns the current frames per second
 * brief Gets the times per second the timer used for scrolling will be called
 */
int
ewl_scrollport_kinetic_fps_get(Ewl_Scrollport *s)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(s, -1);
        DCHECK_TYPE_RET(s, EWL_SCROLLPORT_TYPE, -1);

        DRETURN_INT(s->kinfo->fps, DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @ev_data: The Ewl_Event_Mouse_Down data
 * @param data: The scrollport
 * @return Returns no value
 * @brief The mouse down setting up kinetic scrolling
 */
static void
ewl_scrollport_kinetic_cb_mouse_down_normal(Ewl_Widget *w, void *ev, void *data)
{
        Ewl_Scrollport *s;
        Ewl_Event_Mouse *md;
        Ewl_Scrollport_Kinetic_Info_Normal *info;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(ev);
        DCHECK_PARAM_PTR(data);
        DCHECK_TYPE(w, EWL_WIDGET_TYPE);

        s = EWL_SCROLLPORT(data);
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
 * @param data: The scrollport
 * @return Returns no value
 * @brief The mouse down function for kinetic scrolling
 */
static void
ewl_scrollport_kinetic_cb_mouse_down_embedded(Ewl_Widget *w, void *ev,
                void *data)
{
        Ewl_Scrollport *s;
        Ewl_Event_Mouse *md;
        Ewl_Scrollport_Kinetic_Info_Embedded *info;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(ev);
        DCHECK_PARAM_PTR(data);
        DCHECK_TYPE(w, EWL_WIDGET_TYPE);

        s = EWL_SCROLLPORT(data);
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
 * @param data: The scrollport
 * @return Returns no value
 * @brief The mouse move callback for kinetic scrolling
 */
static void
ewl_scrollport_kinetic_cb_mouse_move_normal(Ewl_Widget *w, void *ev, void *data)
{
        Ewl_Scrollport *s;
        Ewl_Event_Mouse *mm;
        Ewl_Scrollport_Kinetic_Info_Normal *info;
        int cx, cy;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(ev);
        DCHECK_PARAM_PTR(data);

        s = EWL_SCROLLPORT(data);
        mm = EWL_EVENT_MOUSE(ev);
        info = s->kinfo->extra;

        if (!s->kinfo->clicked)
                DRETURN(DLEVEL_STABLE);

        if (!s->kinfo->active)
        {
                ecore_timer_add(1.0/s->kinfo->fps,
                                ewl_scrollport_kinetic_cb_scroll_timer_normal,
                                s);
                s->kinfo->active = !!TRUE;
        }

        info->xc = mm->x;
        info->yc = mm->y;
        cx = (info->xc - info->x);
        cy = (info->yc - info->y);

        /* v = (change in position / (width or height of scroll *
         *        (range of velocities) + min))
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
 * @param_data: The scrollport
 * @return Returns no value
 * @brief The mouse move callback for kinetic scrolling
 */
static void
ewl_scrollport_kinetic_cb_mouse_move_embedded(Ewl_Widget *w __UNUSED__,
                void *ev, void *data)
{
        Ewl_Scrollport *s;
        Ewl_Event_Mouse *mm;
        Ewl_Scrollport_Kinetic_Info_Embedded *info;
        int x = 0, y = 0;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(ev);
        DCHECK_PARAM_PTR(data);

        s = EWL_SCROLLPORT(data);
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

        ewl_scrollport_kinetic_scroll(s, x, y, NULL, NULL);
        info->xs = mm->x;
        info->ys = mm->y;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @ev_data: The Ewl_Event_Mouse_Up data
 * @param data: The scrollport
 * @return Returns no value
 * @brief The mouse up callback for kinetic scrolling
 */
static void
ewl_scrollport_kinetic_cb_mouse_up_normal(Ewl_Widget *w __UNUSED__,
                        void *ev __UNUSED__, void *data)
{
        Ewl_Scrollport *s;
        
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(data);

        s = EWL_SCROLLPORT(data);
        s->kinfo->clicked = !!FALSE;

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @ev_data: The Ewl_Event_Mouse_Up data
 * @data: The scrollport
 * @return Returns no value
 * @brief The mouse up callback for kinetic scrolling
 */
static void
ewl_scrollport_kinetic_cb_mouse_up_embedded(Ewl_Widget *w __UNUSED__, 
                                        void *ev __UNUSED__, void *data)
{
        Ewl_Scrollport *s;
        Ewl_Event_Mouse *mm;
        Ewl_Scrollport_Kinetic_Info_Embedded *info;
        int ax, ay, dx, dy, i;
        double at, dt, t;
        int rx = 1, ry = 1;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(data);

        s = EWL_SCROLLPORT(data);
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
        ecore_timer_add(1.0 / s->kinfo->fps,
                        ewl_scrollport_kinetic_cb_scroll_timer_embedded, s);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param data: The scrollport to work with
 * @return Returns 1 if the timer is to continue, 0 otherwise
 * @brief Performs some calculations then calls the scroll function
 */
static Eina_Bool
ewl_scrollport_kinetic_cb_scroll_timer_normal(void *data)
{
        Ewl_Scrollport *s;
        Ewl_Scrollport_Kinetic_Info_Normal *info;
        double h, w;
        int tx = 0, ty = 0;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(data, FALSE);

        s = EWL_SCROLLPORT(data);
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

        /* Actually scroll the port */
        ewl_scrollport_kinetic_scroll(s, info->vel_x, info->vel_y, &tx, &ty);

        /* If at the end of a scrollbar, set x/y to current */
        if (!tx)
                info->x = info->xc;
        if (!ty)
                info->y = info->yc;

        DRETURN_INT(1, DLEVEL_STABLE);
}

/**
 * @internal
 * @param data: The scrollport to work with
 * @return Returns 1 if the timer is to continue, 0 otherwise
 * @brief Performs some calculations then calls the scroll functions
 */
static Eina_Bool
ewl_scrollport_kinetic_cb_scroll_timer_embedded(void *data)
{
        Ewl_Scrollport *s;
        Ewl_Scrollport_Kinetic_Info_Embedded *info;
        double h, w, t;
        int tx = 0, ty = 0;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(data, FALSE);

        s = EWL_SCROLLPORT(data);
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
        
        ewl_scrollport_kinetic_scroll(s, w, h, &tx, &ty);

        if (!tx && !ty)
                DRETURN_INT(FALSE, DLEVEL_STABLE);

        info->vel_x *= s->kinfo->dampen;
        info->vel_y *= s->kinfo->dampen;

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @internal
 * @param s: The scrollport to work with
 * @param x: The horizontal velocity
 * @param y: The vertical velocity
 * @param tx: Pointer to integer tested
 * @param ty: Pointer to integer tested
 * @return Returns no value
 * @brief Scrolls the scrollport based on the given parameters
 */
static void
ewl_scrollport_kinetic_scroll(Ewl_Scrollport *s, double x, double y,
                                                int *tx, int *ty)
{
        double w, h;
        Ewl_Scrollbar *ry, *rx;
        Ewl_Scrollport *sp;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(s);
        DCHECK_TYPE(s, EWL_SCROLLPORT_TYPE);

        sp = EWL_SCROLLPORT(s);
        ry = EWL_SCROLLBAR(ewl_scrollport_vscrollbar_get(sp));
        rx = EWL_SCROLLBAR(ewl_scrollport_hscrollbar_get(sp));

        if (!((ewl_scrollport_vscrollbar_value_get(sp) == 1.0) &&
                                (y > 0)) &&
                        !((ewl_scrollport_vscrollbar_value_get(sp) == 0.0) &&
                                (y < 0)))
        {
                h = ewl_scrollport_vscrollbar_value_get(sp) +
                        (y / (double)(s->area_h));

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

                ewl_scrollport_vscrollbar_value_set(sp, h);
        }

        if (!((ewl_scrollport_hscrollbar_value_get(sp) == 1.0) &&
                                (x > 0)) &&
                        !((ewl_scrollport_hscrollbar_value_get(sp) == 0.0) &&
                                (x < 0)))
        {
                w = ewl_scrollport_hscrollbar_value_get(sp) +
                        (x / (double)(s->area_w));

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

                ewl_scrollport_hscrollbar_value_set(sp, w);
        }

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to use
 * @parma ev: The event data
 * @param data: User data
 * @return Returns no value
 * @brief Frees data from the scrollport
 */
static void
ewl_scrollport_kinetic_cb_destroy(Ewl_Widget *w, void *ev __UNUSED__,
                                void *data __UNUSED__)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(w);
        DCHECK_TYPE(w, EWL_SCROLLPORT_TYPE);

        if (EWL_SCROLLPORT(w)->kinfo)
                FREE(EWL_SCROLLPORT(w)->kinfo->extra);
        FREE(EWL_SCROLLPORT(w)->kinfo);


        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

