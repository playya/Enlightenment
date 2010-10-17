/**
 * @addtogroup Magnetslider Magnetslider
 *
 * A magnet slider is a switcher for 3 labels with customizable
 * magnet properties. When the position is set with magnet, the knob
 * will be moved to it if it's nearest the magnetized position.
 *
 * Signals emmitted:
 * "selected" - when user selects a position (the label is passed as
 * event info)".
 * "pos_changed" - when a button reaches to the special position like
 * "left", "right" and "center".
 */

#include <Elementary.h>
#include <math.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *ms;     // magnetslider
   Evas_Object *icon;      // an icon for a button or a bar
   Elm_Magnetslider_Pos magnet_position, enabled_position;
   const char *text_left, *text_right, *text_center;
   Ecore_Animator *icon_animator;
   double final_position;
   Eina_Bool mouse_down : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _icon_down_cb(void *data, Evas *e,
                          Evas_Object *obj, void *event_info);
static void _icon_move_cb(void *data, Evas *e,
                          Evas_Object *obj, void *event_info);
static void _icon_up_cb(void *data, Evas *e,
                        Evas_Object *obj, void *event_info);
static Eina_Bool _icon_animation(void *data);

#define SIG_CHANGED "pos_changed"
#define SIG_SELECTED "selected"

static const Evas_Smart_Cb_Description _signals[] =
{
   {SIG_CHANGED, ""},
   {SIG_SELECTED, ""},
   {NULL, NULL}
};


static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->text_left) eina_stringshare_del(wd->text_left);
   if (wd->text_right) eina_stringshare_del(wd->text_right);
   if (wd->text_center) eina_stringshare_del(wd->text_center);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!edje_object_part_swallow_get(wd->ms, "elm.swallow.icon"))
     edje_object_part_unswallow(wd->ms, wd->icon);

   _elm_theme_object_set(obj, wd->ms, "magnetslider",
                         "base", elm_widget_style_get(obj));
   _elm_theme_object_set(obj, wd->icon, "magnetslider",
                         "icon", elm_widget_style_get(obj));
   edje_object_part_swallow(wd->ms, "elm.swallow.icon", wd->icon);
   edje_object_part_text_set(wd->ms, "elm.text.left", wd->text_left);
   edje_object_part_text_set(wd->ms, "elm.text.right", wd->text_right);
   edje_object_part_text_set(wd->ms, "elm.text.center", wd->text_center);
   edje_object_message_signal_process(wd->ms);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   if (!wd) return;
   elm_coords_finger_size_adjust(4, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->ms, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_icon_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get((Evas_Object *) data);
   if (!wd) return;
   wd->mouse_down = EINA_TRUE;
}

static void
_icon_move_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *as = (Evas_Object *) data;
   Widget_Data *wd = elm_widget_data_get(as);
   double pos = 0.0;
   if (!wd) return;

   if (!wd->mouse_down) return;
   edje_object_part_drag_value_get(wd->ms, "elm.swallow.icon", &pos, NULL);
   if (pos == 0.0)
     evas_object_smart_callback_call(as, SIG_CHANGED, "left");
   else if (pos == 1.0)
     evas_object_smart_callback_call(as, SIG_CHANGED, "right");
   else if (pos >= 0.45 && pos <= 0.55)
     evas_object_smart_callback_call(as, SIG_CHANGED, "center");
}

static void
_icon_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   double position = 0.0;
   if (!wd) return;

   wd->mouse_down = EINA_FALSE;

   edje_object_part_drag_value_get(wd->ms, "elm.swallow.icon",
                                   &position, NULL);

   if (position == 0.0 && (wd->enabled_position & ELM_MAGNETSLIDER_LEFT))
     {
        wd->final_position = 0;
        evas_object_smart_callback_call(data, SIG_SELECTED,
                                        (void *)wd->text_left);
        return;
     }
   if (position >= 0.45 && position <= 0.55 &&
       (wd->enabled_position & ELM_MAGNETSLIDER_CENTER))
     {
        wd->final_position = 0.5;
        evas_object_smart_callback_call(data, SIG_SELECTED,
                                        (void *)wd->text_center);
        return;
     }
   if (position == 1.0 && (wd->enabled_position & ELM_MAGNETSLIDER_RIGHT))
     {
        wd->final_position = 1;
        evas_object_smart_callback_call(data, SIG_SELECTED,
                                        (void *)wd->text_right);
        return;
     }

   if (!wd->magnet_position)
     {
        wd->final_position = 0;
        goto as_anim;
     }

   if (position < 0.3)
     {
        if (wd->magnet_position & ELM_MAGNETSLIDER_LEFT)
          wd->final_position = 0;
        else if (wd->magnet_position & ELM_MAGNETSLIDER_CENTER)
          wd->final_position = 0.5;
        else if (wd->magnet_position & ELM_MAGNETSLIDER_RIGHT)
          wd->final_position = 1;
     }
   else if ((position >= 0.3) && (position <= 0.7))
     {
        if (wd->magnet_position & ELM_MAGNETSLIDER_CENTER)
          wd->final_position = 0.5;
        else if (position < 0.5)
          {
             if (wd->magnet_position & ELM_MAGNETSLIDER_LEFT)
               wd->final_position = 0;
             else
               wd->final_position = 1;
          }
        else
          {
             if (wd->magnet_position & ELM_MAGNETSLIDER_RIGHT)
               wd->final_position = 1;
             else
               wd->final_position = 0;
          }
     }
   else
     {
        if (wd->magnet_position & ELM_MAGNETSLIDER_RIGHT)
          wd->final_position = 1;
        else if (wd->magnet_position & ELM_MAGNETSLIDER_CENTER)
          wd->final_position = 0.5;
        else
          wd->final_position = 0;
     }
as_anim:
   wd->icon_animator = ecore_animator_add(_icon_animation, data);
}

static Eina_Bool
_icon_animation(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   double cur_position = 0.0, new_position = 0.0;
   double move_amount = 0.05;
   Eina_Bool flag_finish_animation = EINA_FALSE;
   if (!wd) return EINA_FALSE;

   edje_object_part_drag_value_get(wd->ms,
                                   "elm.swallow.icon", &cur_position, NULL);
   if ((wd->final_position == 0.0) ||
       (wd->final_position == 0.5 && cur_position >= wd->final_position))
     {
        new_position = cur_position - move_amount;
        if (new_position <= wd->final_position)
          {
             new_position = wd->final_position;
             flag_finish_animation = EINA_TRUE;
          }
     }
   else if ((wd->final_position == 1.0) ||
            (wd->final_position == 0.5 && cur_position < wd->final_position))
     {
        new_position = cur_position + move_amount;
        if (new_position >= wd->final_position)
          {
             new_position = wd->final_position;
             flag_finish_animation = EINA_TRUE;
          }
     }
   edje_object_part_drag_value_set(wd->ms,
                                   "elm.swallow.icon", new_position, 0.5);
   if (flag_finish_animation)
     {
        if ((wd->final_position == 0) &&
            (wd->enabled_position & ELM_MAGNETSLIDER_LEFT))
          evas_object_smart_callback_call(data, SIG_SELECTED,
                                          (void *)wd->text_left);
        else if ((wd->final_position == 0.5) &&
                 (wd->enabled_position & ELM_MAGNETSLIDER_CENTER))
          evas_object_smart_callback_call(data, SIG_SELECTED,
                                          (void *)wd->text_center);
        else if ((wd->final_position == 1) &&
                 (wd->enabled_position & ELM_MAGNETSLIDER_RIGHT))
          evas_object_smart_callback_call(data, SIG_SELECTED,
                                          (void *)wd->text_right);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/**
 * Add a new magnetslider to the parent.
 *
 * @param parent The parent object
 * @return The new magnetslider object or NULL if it cannot be created
 *
 * @ingroup Magnetslider
 */
EAPI Evas_Object *
elm_magnetslider_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Widget_Data *wd;
   Evas *e;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "magnetslider");
   elm_widget_type_set(obj, "magnetslider");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->mouse_down = EINA_FALSE;
   wd->enabled_position = ELM_MAGNETSLIDER_ALL;

   wd->ms = edje_object_add(e);
   _elm_theme_object_set(obj, wd->ms, "magnetslider", "base", "default");
   elm_widget_resize_object_set(obj, wd->ms);

   wd->icon = edje_object_add(e);
   elm_widget_sub_object_add(obj, wd->icon);
   _elm_theme_object_set(obj, wd->icon, "magnetslider", "icon", "default");
   edje_object_part_swallow(wd->ms, "elm.swallow.icon", wd->icon);

   evas_object_event_callback_add(wd->icon, EVAS_CALLBACK_MOUSE_DOWN,
                                  _icon_down_cb, obj);
   evas_object_event_callback_add(wd->icon, EVAS_CALLBACK_MOUSE_MOVE,
                                  _icon_move_cb, obj);
   evas_object_event_callback_add(wd->icon, EVAS_CALLBACK_MOUSE_UP,
                                  _icon_up_cb, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   _sizing_eval(obj);
   return obj;
}

/**
 * Set magnetslider indicator position.
 *
 * @param obj The magnetslider object.
 * @param pos The position of the indicator.
 *
 * @ingroup Magnetslider
 */
EAPI void
elm_magnetslider_indicator_pos_set(Evas_Object *obj, Elm_Magnetslider_Pos pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   double position = 0.0;
   if (!wd) return;
   if (pos == ELM_MAGNETSLIDER_CENTER) position = 0.5;
   else if (pos == ELM_MAGNETSLIDER_RIGHT) position = 1.0;
   edje_object_part_drag_value_set(wd->ms, "elm.swallow.icon", position, 0.5);
}

/**
 * Get magnetslider indicator position.
 *
 * @param obj The magnetslider object.
 * @return The position of the indicator.
 *
 * @ingroup Magnetslider
 */
EAPI Elm_Magnetslider_Pos
elm_magnetslider_indicator_pos_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_MAGNETSLIDER_NONE;
   Widget_Data *wd = elm_widget_data_get(obj);
   double position;
   if (!wd) return ELM_MAGNETSLIDER_NONE;

   edje_object_part_drag_value_get(wd->ms, "elm.swallow.icon", &position, NULL);
   if (position < 0.3)
     return ELM_MAGNETSLIDER_LEFT;
   else if (position < 0.7)
     return ELM_MAGNETSLIDER_CENTER;
   else
     return ELM_MAGNETSLIDER_RIGHT;
}

/**
 * Set magnetslider magnet position.
 *
 * @param obj The magnetslider object.
 * @param pos Bit mask indicating the magnet positions.
 * Example: use (ELM_MAGNETSLIDER_LEFT | ELM_MAGNETSLIDER_RIGHT)
 * to put magnet property on both positions
 *
 * @ingroup Magnetslider
 */
EAPI void
elm_magnetslider_magnet_pos_set(Evas_Object *obj, Elm_Magnetslider_Pos pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->magnet_position = pos;
}

/**
 * Get magnetslider magnet position.
 *
 * @param obj The magnetslider object.
 * @return The positions with magnet property.
 *
 * @ingroup Magnetslider
 */
EAPI Elm_Magnetslider_Pos
elm_magnetslider_magnet_pos_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_MAGNETSLIDER_NONE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_MAGNETSLIDER_NONE;
   return wd->magnet_position;
}

/**
 * Set magnetslider enabled position.
 *
 * All the positions are enabled by default.
 *
 * @param obj The magnetslider object.
 * @param pos Bit mask indicating the enabled positions.
 * Example: use (ELM_MAGNETSLIDER_LEFT | ELM_MAGNETSLIDER_RIGHT)
 * to enable both positions, so the user can select it.
 *
 * @ingroup Magnetslider
 */
EAPI void
elm_magnetslider_enabled_pos_set(Evas_Object *obj, Elm_Magnetslider_Pos pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->enabled_position = pos;
}

/**
 * Get magnetslider enabled position.
 *
 * All the positions are enabled by default.
 *
 * @param obj The magnetslider object.
 * @return The enabled positions.
 *
 * @ingroup Magnetslider
 */
EAPI Elm_Magnetslider_Pos
elm_magnetslider_enabled_pos_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_MAGNETSLIDER_NONE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_MAGNETSLIDER_NONE;
   return wd->enabled_position;
}

/**
 * Set magnetslider labels.
 *
 * @param obj The magnetslider object
 * @param left_label The label which is going to be set.
 * @param center_label The label which is going to be set.
 * @param right_label The label which is going to be set.
 *
 * @ingroup Magnetslider
 */
EAPI void
elm_magnetslider_labels_set(Evas_Object *obj, const char *left_label, const char *center_label, const char *right_label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   eina_stringshare_replace(&wd->text_left, left_label);
   edje_object_part_text_set(wd->ms, "elm.text.left", left_label);
   eina_stringshare_replace(&wd->text_center, center_label);
   edje_object_part_text_set(wd->ms, "elm.text.center", center_label);
   eina_stringshare_replace(&wd->text_right, right_label);
   edje_object_part_text_set(wd->ms, "elm.text.right", right_label);
}

/**
 * Get magnetslider labels.
 *
 * @param obj The magnetslider object
 * @param left_label A char** to place the left_label of @p obj into
 * @param center_label A char** to place the center_label of @p obj into
 * @param right_label A char** to place the right_label of @p obj into
 *
 * @ingroup Magnetslider
 */
EAPI void
elm_magnetslider_labels_get(const Evas_Object *obj, const char **left_label, const char **center_label, const char **right_label)
{
   if (left_label) *left_label= NULL;
   if (center_label) *center_label= NULL;
   if (right_label) *right_label= NULL;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (left_label) *left_label = wd->text_left;
   if (center_label) *center_label = wd->text_center;
   if (right_label) *right_label = wd->text_right;
}

/**
 * Get magnetslider selected label.
 *
 * @param obj The magnetslider object
 * @return The selected label
 *
 * @ingroup Magnetslider
 */
EAPI const char *
elm_magnetslider_selected_label_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   if ((wd->final_position == 0.0) &&
       (wd->enabled_position & ELM_MAGNETSLIDER_LEFT))
     return wd->text_left;

   if ((wd->final_position == 0.5) &&
       (wd->enabled_position & ELM_MAGNETSLIDER_CENTER))
     return wd->text_center;

   if ((wd->final_position == 1.0) &&
       (wd->enabled_position & ELM_MAGNETSLIDER_RIGHT))
     return wd->text_right;

   return NULL;
}
