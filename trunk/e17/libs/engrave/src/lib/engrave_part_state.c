#include <Engrave.h>
#include "engrave_macros.h"

/**
 * engrave_part_state_new - create a new Engrave_Part_State object.
 *
 * @return Returns a pointer to a newly allocated Engrave_Part_State object
 * on success or NULL on failure
 */
Engrave_Part_State *
engrave_part_state_new(void)
{
  Engrave_Part_State *state;

  state = NEW(Engrave_Part_State, 1);
  if (!state) return NULL;

  /* defaults */
  state->visible = 1;
  state->align.x = 0.5;
  state->align.y = 0.5;
  state->min.w = 0;
  state->min.h = 0;
  state->max.w = -1;
  state->max.h = -1;
  state->rel1.relative.x = 0.0;
  state->rel1.relative.y = 0.0;
  state->rel1.offset.x = 0;
  state->rel1.offset.y = 0;
  state->rel2.relative.x = 1.0;
  state->rel2.relative.y = 1.0;
  state->rel2.offset.x = -1;
  state->rel2.offset.y = -1;
  state->fill.smooth = 1;
  state->fill.pos_rel.x = 0.0;
  state->fill.pos_abs.x = 0;
  state->fill.rel.x = 1.0;
  state->fill.abs.x = 0;
  state->fill.pos_rel.y = 0.0;
  state->fill.pos_abs.y = 0;
  state->fill.rel.y = 1.0;
  state->fill.abs.y = 0;
  state->color_class = NULL;
  state->color.r = 255;
  state->color.g = 255;
  state->color.b = 255;
  state->color.a = 255;
  state->color2.r = 0;
  state->color2.g = 0;
  state->color2.b = 0;
  state->color2.a = 255;
  state->color3.r = 0;
  state->color3.g = 0;
  state->color3.b = 0;
  state->color3.a = 128;
  state->text.align.x = 0.5;
  state->text.align.y = 0.5;

  return state;
}

/**
 * engrave_part_state_free - free the state
 * @param eps: The Engrave_Part_State to free
 * 
 * @return Returns no value.
 */
void
engrave_part_state_free(Engrave_Part_State *eps)
{
  Evas_List *l;
  if (!eps) return;

  IF_FREE(eps->name);
  IF_FREE(eps->rel1.to_x);
  IF_FREE(eps->rel1.to_y);
  IF_FREE(eps->rel2.to_x);
  IF_FREE(eps->rel2.to_y);

  /* don't free the image here cuz its freed in the _file code */
  eps->image.normal = NULL;
  eps->image.tween = evas_list_free(eps->image.tween);

  IF_FREE(eps->color_class);
  IF_FREE(eps->text.text);
  IF_FREE(eps->text.text_class);
  IF_FREE(eps->text.font);
}

/**
 * engrave_part_state_name_set - Set the name of the state
 * @param eps: The Engrave_Part_State to set the value too.
 * @param name: The name to set to the state.
 * @param value: The value to set to the state.
 *
 * @return Returns no value 
 */
void
engrave_part_state_name_set(Engrave_Part_State *eps, const char *name, 
                                                          double value)
{
  if (!eps) return;
  IF_FREE(eps->name);
  eps->name = (name ? strdup(name) : NULL);
  eps->value = value;
}

/**
 * engrave_part_state_visible_set - Set the visiblity of the state
 * @param eps: The Engrave_Part_State to set the value too.
 * @param visible: The visibility value to set to the state
 *
 * @return Returns no value 
 */
void
engrave_part_state_visible_set(Engrave_Part_State *eps, int visible)
{
  if (!eps) return;
  eps->visible = visible;
}

/**
 * engrave_part_state_align_set - Set the alignment of the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x alignment value to set
 * @param y: The y alignment value to set
 *
 * @return Returns no value 
 */
void
engrave_part_state_align_set(Engrave_Part_State *eps, int x, int y)
{
  if (!eps) return;
  eps->align.x = x;
  eps->align.y = y;
}

/**
 * engrave_part_state_step_set - Set the step value of the state
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x step value to set
 * @param y: The y step value to set
 *
 * @return Returns no value 
 */
void
engrave_part_state_step_set(Engrave_Part_State *eps, int x, int y)
{
  if (!eps) return;
  eps->step.x = x;
  eps->step.y = y;
}

/**
 * engrave_part_state_min_size_set - Set the min size of the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param w: The minimum width to set
 * @param h: The minimum height to set
 *
 * @return Returns no value 
 */
void
engrave_part_state_min_size_set(Engrave_Part_State *eps, int w, int h)
{
  if (!eps) return;
  eps->min.w = w;
  eps->min.h = h;
}

/**
 * engrave_part_state_max_size_set - Set the max size of the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param w: The maximum width of the state.
 * @param h: The maximum height of the state.
 *
 * @return Returns no value 
 */
void
engrave_part_state_max_size_set(Engrave_Part_State *eps, int w, int h)
{
  if (!eps) return;
  eps->max.w = w;
  eps->max.h = h;
}

/**
 * engrave_part_state_aspect_set - Set the width and height of the aspect ratio
 * @param eps: The Engrave_Part_State to set the value too.
 * @param w: The width value to set.
 * @param h: The height value to set.
 *
 * @return Returns no value 
 */
void
engrave_part_state_aspect_set(Engrave_Part_State *eps, int w, int h)
{
  if (!eps) return;
  eps->aspect.w = w;
  eps->aspect.h = h;
}

/**
 * engrave_part_state_aspect_preference_set - Set the aspect preferenece.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param prefer: The Engrave_Aspect_Preference value to set on the state.
 *
 * @return Returns no value 
 */
void
engrave_part_state_aspect_preference_set(Engrave_Part_State *eps,
                                    Engrave_Aspect_Preference prefer)
{
  if (!eps) return;
  eps->aspect.prefer = prefer;
}

/**
 * engrave_part_state_rel1_relative_set - Set the relative value of rel1.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x value to set.
 * @param y: The y value to set.
 *
 * @return Returns no value 
 */
void
engrave_part_state_rel1_relative_set(Engrave_Part_State *eps, int x, int y)
{
  if (!eps) return;
  eps->rel1.relative.x = x;
  eps->rel1.relative.y = y;
}

/**
 * engrave_part_state_rel2_relative_set - Set the rel2 relative value
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x value to set.
 * @param y: The y value to set.
 *
 * @return Returns no value 
 */
void
engrave_part_state_rel2_relative_set(Engrave_Part_State *eps, int x, int y)
{
  if (!eps) return;
  eps->rel2.relative.x = x;
  eps->rel2.relative.y = y;
}

/**
 * engrave_part_state_rel1_offset_set - Set the rel1 offset value.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x value to set.
 * @param y: The y value to set.
 *
 * @return Returns no value 
 */
void
engrave_part_state_rel1_offset_set(Engrave_Part_State *eps, int x, int y)
{
  if (!eps) return;
  eps->rel1.offset.x = x;
  eps->rel1.offset.y = y;
}

/**
 * engrave_part_state_rel2_offset_set - Set the rel2 offset value.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x value to set.
 * @param y: The y value to set.
 *
 * @return Returns no value 
 */
void
engrave_part_state_rel2_offset_set(Engrave_Part_State *eps, int x, int y)
{
  if (!eps) return;
  eps->rel2.offset.x = x;
  eps->rel2.offset.y = y;
}

/**
 * engrave_part_state_rel1_to_x_set - Set what rel1 is relative too in the x dir.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param to: The name of the part we are relative too.
 *
 * @return Returns no value 
 */
void
engrave_part_state_rel1_to_x_set(Engrave_Part_State *eps, const char *to)
{
  if (!eps) return;
  IF_FREE(eps->rel1.to_x);
  eps->rel1.to_x = (to ? strdup(to) : NULL);
}

/**
 * engrave_part_state_rel2_to_x_set - Set what rel2 is relative too in the x dir.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param to: The name of the part we are relative too.
 *
 * @return Returns no value 
 */
void
engrave_part_state_rel2_to_x_set(Engrave_Part_State *eps, const char *to)
{
  if (!eps) return;
  IF_FREE(eps->rel2.to_x);
  eps->rel2.to_x = (to ? strdup(to) : NULL);
}

/**
 * engrave_part_state_rel1_to_y_set - Set what rel1 is relative too in the y dir.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param to: The name of the part we are relative too.
 *
 * @return Returns no value 
 */
void
engrave_part_state_rel1_to_y_set(Engrave_Part_State *eps, const char *to)
{
  if (!eps) return;
  IF_FREE(eps->rel1.to_y);
  eps->rel1.to_y = (to ? strdup(to) : NULL);
}

/**
 * engrave_part_state_rel2_to_y_set - Set what rel2 is relative too in the y dir. 
 * @param eps: The Engrave_Part_State to set the value too.
 * @param to: The name of the part we are relative too.
 *
 * @return Returns no value 
 */
void
engrave_part_state_rel2_to_y_set(Engrave_Part_State *eps, const char *to)
{
  if (!eps) return;
  IF_FREE(eps->rel2.to_y);
  eps->rel2.to_y = (to ? strdup(to) : NULL);
}

/**
 * engrave_part_state_rel1_to_set - Set the rel1 to value for both x and y dirs.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param to: The name of the part we are relative too.
 *
 * @return Returns no value 
 */
void
engrave_part_state_rel1_to_set(Engrave_Part_State *eps, const char *to)
{
  engrave_part_state_rel1_to_x_set(eps, to);
  engrave_part_state_rel1_to_y_set(eps, to);
}

/**
 * engrave_part_state_rel2_to_set - Set the rel2 to value for both x and y dirs.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param to: The name of the part we are relative too.
 *
 * @return Returns no value 
 */
void
engrave_part_state_rel2_to_set(Engrave_Part_State *eps, const char *to)
{
  engrave_part_state_rel2_to_x_set(eps, to);
  engrave_part_state_rel2_to_y_set(eps, to);
}

/**
 * engrave_part_state_image_normal_set - Set the image we are associated with.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param im: The Engrave_Image attached to this state.
 *
 * @return Returns no value 
 */
void
engrave_part_state_image_normal_set(Engrave_Part_State *eps, 
                                      Engrave_Image *im)
{
  if (!eps) return;
  eps->image.normal = im;
}

/**
 * engrave_part_state_image_tween_add - Add a tween to the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param im: The Engrave_Image to add to the tween list.
 *
 * @return Returns no value 
 */
void
engrave_part_state_image_tween_add(Engrave_Part_State *eps,
                                      Engrave_Image *im)
{
  if (!eps || !im) return;
  eps->image.tween = evas_list_append(eps->image.tween, im);
}

/**
 * engrave_part_state_image_border_set - Set the border on the image.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param l: The left border value.
 * @param r: The right border value.
 * @param t: The top border value.
 * @param b: The bottom border value.
 *
 * @return Returns no value k
 */
void
engrave_part_state_image_border_set(Engrave_Part_State *eps, 
                                int l, int r, int t, int b)
{
  if (!eps) return;
  eps->image.border.l = l;
  eps->image.border.r = r;
  eps->image.border.t = t;
  eps->image.border.b = b;
}

/**
 * engrave_part_state_color_class_set - Set the colour class on the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param class: The colour class to associate with the state.
 *
 * @return Returns no value 
 */
void
engrave_part_state_color_class_set(Engrave_Part_State *eps, const char *class)
{
  if (!eps) return;
  IF_FREE(eps->color_class);
  eps->color_class = (class ? strdup(class) : NULL);
}

/**
 * engrave_part_state_color_set - Set the colour of the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param r: The red value.
 * @param g: The green value.
 * @param b: the blue value.
 * @param a: The alpha value.
 *
 * @return Returns no value 
 */
void
engrave_part_state_color_set(Engrave_Part_State *eps, 
                                int r, int g, int b, int a)
{
  if (!eps) return;
  eps->color.r = r;
  eps->color.g = g;
  eps->color.b = b;
  eps->color.a = a;
}

/**
 * engrave_part_state_color2_set - Set the colour2 of the state
 * @param eps: The Engrave_Part_State to set the value too.
 * @param r: The red value.
 * @param g: The green value.
 * @param b: the blue value.
 * @param a: The alpha value.
 *
 * @return Returns no value 
 */
void
engrave_part_state_color2_set(Engrave_Part_State *eps, 
                                int r, int g, int b, int a)
{
  if (!eps) return;
  eps->color2.r = r;
  eps->color2.g = g;
  eps->color2.b = b;
  eps->color2.a = a;
}

/**
 * engrave_part_state_color3_set - Set the colour 3 of the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param r: The red value.
 * @param g: The green value.
 * @param b: the blue value.
 * @param a: The alpha value.
 *
 * @return Returns no value 
 */
void
engrave_part_state_color3_set(Engrave_Part_State *eps, 
                                int r, int g, int b, int a)
{
  if (!eps) return;
  eps->color3.r = r;
  eps->color3.g = g;
  eps->color3.b = b;
  eps->color3.a = a;
}

/**
 * engrave_part_state_fill_smooth_set - Set the fill smooth value of the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param smooth: The smooth value to set.
 *
 * @return Returns no value 
 */
void
engrave_part_state_fill_smooth_set(Engrave_Part_State *eps, int smooth)
{
  if (!eps) return;
  eps->fill.smooth = smooth;
}

/**
 * engrave_part_state_fill_origin_relative_set - Set the relative value of the fill.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x value to set
 * @param y: The y value to set
 *
 * @return Returns no value 
 */
void
engrave_part_state_fill_origin_relative_set(Engrave_Part_State *eps,
                                                double x, double y)
{
  if (!eps) return;
  eps->fill.pos_rel.x = x;
  eps->fill.pos_rel.y = y;
}

/**
 * engrave_part_state_fill_size_relative_set - Set the fills relative size on the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x value.
 * @param y: The y value.
 *
 * @return Returns no value 
 */
void
engrave_part_state_fill_size_relative_set(Engrave_Part_State *eps,
                                                double x, double y)
{
  if (!eps) return;
  eps->fill.rel.x = x;
  eps->fill.rel.y = y;
}

/**
 * engrave_part_state_fill_origin_offset_set - Set the offset of the fill position
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x value.
 * @param y: The y value.
 *
 * @return Returns no value 
 */
void
engrave_part_state_fill_origin_offset_set(Engrave_Part_State *eps,
                                                      int x, int y)
{
  if (!eps) return;
  eps->fill.pos_abs.x = x;
  eps->fill.pos_abs.y = y;
}

/**
 * engrave_part_state_fill_size_offset_set - Set the fill size offset.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x value.
 * @param y: The y value.
 *
 * @return Returns no value 
 */
void
engrave_part_state_fill_size_offset_set(Engrave_Part_State *eps,
                                                int x, int y)
{
  if (!eps) return;
  eps->fill.abs.x = x;
  eps->fill.abs.y = y;
}

/**
 * engrave_part_state_text_text_set - Set the text string of the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param text: The text string to set.
 *
 * @return Returns no value 
 */
void
engrave_part_state_text_text_set(Engrave_Part_State *eps, const char *text)
{
  if (!eps) return;
  IF_FREE(eps->text.text);
  eps->text.text = (text ? strdup(text) : NULL); 
}

/**
 * engrave_part_state_text_text_class_set - Set the text class of the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param text_class: The text class to set to the state.
 *
 * @return Returns no value 
 */
void
engrave_part_state_text_text_class_set(Engrave_Part_State *eps,
                                                const char *text_class)
{
  if (!eps) return;
  IF_FREE(eps->text.text_class);
  eps->text.text_class = (text_class ? strdup(text_class) : NULL); 
}

/**
 * engrave_part_state_text_font_set - Set the font for the text
 * @param eps: The Engrave_Part_State to set the value too.
 * @param font: The font to apply to the text.
 *
 * @return Returns no value 
 */
void
engrave_part_state_text_font_set(Engrave_Part_State *eps,
                                                const char *font)
{
  if (!eps) return;
  IF_FREE(eps->text.font);
  eps->text.font = (font ? strdup(font) : NULL); 
}

/**
 * engrave_part_state_text_size_set - Set the text size.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param size: The font size to set.
 *
 * @return Returns no value 
 */
void
engrave_part_state_text_size_set(Engrave_Part_State *eps, int size)
{
  if (!eps) return;
  eps->text.size = size;
}

/**
 * engrave_part_state_text_fit_set - Set the fit parameter of the text
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x value to set.
 * @param y: The y value to set.
 *
 * @return Returns no value 
 */
void
engrave_part_state_text_fit_set(Engrave_Part_State *eps, int x, int y)
{
  if (!eps) return;
  eps->text.fit.x = x;
  eps->text.fit.y = y;
}

/**
 * engrave_part_state_text_min_set - Set the text min value.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x value to set.
 * @param y: The y value to set.
 *
 * @return Returns no value 
 */
void
engrave_part_state_text_min_set(Engrave_Part_State *eps, int x, int y)
{
  if (!eps) return;
  eps->text.min.x = x;
  eps->text.min.y = y;
}

/**
 * engrave_part_state_text_align_set - Set the text alignment of the state.
 * @param eps: The Engrave_Part_State to set the value too.
 * @param x: The x alignment setting.
 * @param y: the y alignment setting.
 *
 * @return Returns no value 
 */
void
engrave_part_state_text_align_set(Engrave_Part_State *eps, double x, double y)
{
  if (!eps) return;
  eps->text.align.x = x;
  eps->text.align.y = y;
}

/**
 * engrave_part_state_copy - Copy the values from @a from to @a to
 * @param from: The Engrave_Part_State from which to retrieve the values
 * @param to: The Engrave_Part_State to place the values into
 *
 * @return Returns no value.
 */
void
engrave_part_state_copy(Engrave_Part_State *from, Engrave_Part_State *to)
{
  Evas_List *l;
  char *to_name;
  double to_val;

  if (!from || !to) return;
 
  /* back these up */
  to_name = to->name;
  to_val = to->value;

  /* free all of the possible strings */
  IF_FREE(to->rel1.to_x);
  IF_FREE(to->rel1.to_y);
  IF_FREE(to->rel2.to_x);
  IF_FREE(to->rel2.to_y);
  IF_FREE(to->color_class);
  IF_FREE(to->text.text);
  IF_FREE(to->text.text_class);
  IF_FREE(to->text.font);

  /* just dup the memory and restore the 2 unchanging entries */
  to = memcpy(to, from, sizeof(Engrave_Part_State));
  to->name = to_name;
  to->value = to_val;

  /* re-dup these so we don't screw with the originals */
  if (to->rel1.to_x) to->rel1.to_x = strdup(to->rel1.to_x);
  if (to->rel1.to_y) to->rel1.to_y = strdup(to->rel1.to_y);
  if (to->rel2.to_x) to->rel2.to_x = strdup(to->rel2.to_x);
  if (to->rel2.to_y) to->rel2.to_y = strdup(to->rel2.to_y);
  if (to->color_class) to->color_class = strdup(to->color_class);
  if (to->text.text) to->text.text = strdup(to->text.text);
  if (to->text.text_class) to->text.text_class = strdup(to->text.text_class);
  if (to->text.font) to->text.font = strdup(to->text.font);

  /* fix up the image normal pointer */
  if (to->image.normal) {
    Engrave_Image *ei;
    
    ei = engrave_image_dup(to->image.normal);
    if (ei)
      to->image.normal = ei;
    else
      fprintf(stderr, "Insufficient memory to dup image\n");
  }

  /* copy the tween list correctly */
  to->image.tween = NULL;
  for (l = from->image.tween; l; l = l->next) {
    Engrave_Image *ei;
    
    ei = engrave_image_dup((Engrave_Image *)l->data);
    if (ei)
      to->image.tween = evas_list_append(to->image.tween, ei);
    else
      fprintf(stderr, "Insufficient memory to dup image\n");
  }
}

/** 
 * engrave_part_state_name_get - get the state name/value
 * @param eps: The Engave_Part_State to get the values from
 * @param val: Where to put the value setting
 *
 * @return Returns the state name, or NULL on failure.
 */
const char *
engrave_part_state_name_get(Engrave_Part_State *eps, double *val)
{
  if (val) *val = (eps ? eps->value : 0);
  return (eps ? eps->name : NULL); 
}

/**
 * engrave_part_state_visible_get - get the visiblity flag
 * @param eps: The Engrave_Part_State to get the visiblity from
 *
 * @return Returns the visiblity of the state
 */
int
engrave_part_state_visible_get(Engrave_Part_State *eps)
{
  if (!eps) return 1;
  return eps->visible;
}

/**
 * engrave_part_state_align_get - get the alignment of the state
 * @param eps: The Engrave_Part_State to get the info from
 * @param x: Where to store the x value or 0.5 if not set
 * @param y: Where to store the y value or 0.5 if not set
 *
 * @return Returns no value.
 */
void
engrave_part_state_align_get(Engrave_Part_State *eps, double *x, double *y)
{
  if (x) *x = (eps ? eps->align.x : 0.5);
  if (y) *y = (eps ? eps->align.y : 0.5);
}

/**
 * engrave_part_state_step_get - get the stepping info for the state
 * @param eps: The Engrave_Part_State to get the info from
 * @param x: Where to store the x value
 * @param y: Where to store the y value
 *
 * @return Returns no value.
 */
void
engrave_part_state_step_get(Engrave_Part_State *eps, double *x, double *y)
{
  if (x) *x = (eps ? eps->step.x : 0);
  if (y) *y = (eps ? eps->step.y : 0);
}

/**
 * engrave_part_state_min_get - get the minimum size of the state
 * @param eps: The Engrave_Part_State to get the min from
 * @param w: Where to store the w value
 * @param h: Where to store the h value
 *
 * @return Returns no value.
 */
void
engrave_part_state_min_get(Engrave_Part_State *eps, int *w, int *h)
{
  if (w) *w = (eps ? eps->min.w : 0);
  if (h) *h = (eps ? eps->min.h : 0);
}

/**
 * engrave_part_state_max_get - get the maximum size of the state
 * @param eps: The Engrave_Part_State to get the max from
 * @param w: Where to store the w value
 * @param h: Where to store the h value
 *
 * @return Returns no value.
 */
void
engrave_part_state_max_get(Engrave_Part_State *eps, int *w, int *h)
{
  if (w) *w = (eps ? eps->max.w : -1);
  if (h) *h = (eps ? eps->max.h : -1);
}

/**
 * engrave_part_state_aspect_get - get the aspect ratio for the state
 * @param eps: The Engrave_Part_State to get the aspect info from
 * @param h: Where to store the w value
 * @param w: Where to store the h value
 * 
 * @return Returns no value
 */
void
engrave_part_state_aspect_get(Engrave_Part_State *eps, double *w, double *h)
{
  if (w) *w = (eps ? eps->aspect.w : 0);
  if (h) *h = (eps ? eps->aspect.h : 0);
}

/**
 * engrave_part_state_aspect_preference_get - get the aspect preference
 * @param eps: The Engrave_Part_State to get the aspect preference from
 *
 * @return Returns the Engrave_Aspect_Preference for the state
 */
Engrave_Aspect_Preference
engrave_part_state_aspect_preference_get(Engrave_Part_State *eps)
{
  if (!eps) return ENGRAVE_ASPECT_PREFERENCE_NONE;
  return eps->aspect.prefer;
}

/** 
 * engrave_part_state_rel1_relative_get - get the rel1 relative values
 * @param eps: The Engrave_Part_State to get the values from
 * @param x: Where to store the x value
 * @param y: Where to store the y value
 * 
 * @return Returns no value.
 */
void
engrave_part_state_rel1_relative_get(Engrave_Part_State *eps, 
                                          double *x, double *y)
{
  if (x) *x = (eps ? eps->rel1.relative.x : 0);
  if (y) *y = (eps ? eps->rel1.relative.y : 0);
}

/**
 * engrave_part_state_rel1_offset_get - get the rel1 offset values
 * @param eps: The Engrave_Part_State to get the values from
 * @param x: Where to store the x value
 * @param y: Where to store the y value
 *
 * @return Returns no value.
 */
void
engrave_part_state_rel1_offset_get(Engrave_Part_State *eps, int *x, int*y)
{
  if (x) *x = (eps ? eps->rel1.offset.x : 0);
  if (y) *y = (eps ? eps->rel1.offset.y : 0);
}

/**
 * engrave_part_state_rel1_to_x_get - get the to_x value for rel1
 * @param eps: The Engrave_Part_State to get the value from
 * 
 * @return Returns the to_x value or NULL on failure.
 */
const char *
engrave_part_state_rel1_to_x_get(Engrave_Part_State *eps)
{
  return (eps ? eps->rel1.to_x : NULL);
}

/**
 * engrave_part_state_rel1_to_y_get - get the to_y value for rel1
 * @param eps: The Engrave_Part_State to get the value from
 * 
 * @return Returns the to_y value or NULL on failure.
 */
const char *
engrave_part_state_rel1_to_y_get(Engrave_Part_State *eps)
{
  return (eps ? eps->rel1.to_y : NULL);
}

/** 
 * engrave_part_state_rel2_relative_get - get the rel2 relative values
 * @param eps: The Engrave_Part_State to get the values from
 * @param x: Where to store the x value
 * @param y: Where to store the y value
 * 
 * @return Returns no value.
 */
void
engrave_part_state_rel2_relative_get(Engrave_Part_State *eps, 
                                          double *x, double *y)
{
  if (x) *x = (eps ? eps->rel2.relative.x : 1.0);
  if (y) *y = (eps ? eps->rel2.relative.y : 1.0);
}

/**
 * engrave_part_state_rel2_offset_get - get the rel2 offset values
 * @param eps: The Engrave_Part_State to get the values from
 * @param x: Where to store the x value
 * @param y: Where to store the y value
 *
 * @return Returns no value.
 */
void
engrave_part_state_rel2_offset_get(Engrave_Part_State *eps, int *x, int*y)
{
  if (x) *x = (eps ? eps->rel2.offset.x : -1);
  if (y) *y = (eps ? eps->rel2.offset.y : -1);
}

/**
 * engrave_part_state_rel2_to_x_get - get the to_x value for rel2
 * @param eps: The Engrave_Part_State to get the value from
 * 
 * @return Returns the to_x value or NULL on failure.
 */
const char *
engrave_part_state_rel2_to_x_get(Engrave_Part_State *eps)
{
  return (eps ? eps->rel2.to_x : NULL);
}

/**
 * engrave_part_state_rel2_to_y_get - get the to_y value for rel2
 * @param eps: The Engrave_Part_State to get the value from
 * 
 * @return Returns the to_y value or NULL on failure.
 */
const char *
engrave_part_state_rel2_to_y_get(Engrave_Part_State *eps)
{
  return (eps ? eps->rel2.to_y : NULL);
}

/**
 * engrave_part_state_color_class_get - get the color class for the state
 * @param eps: The Engrave_Part_State to get the colour class from
 * 
 * @return Returns the colour class on success or NULL on failure.
 */
const char *
engrave_part_state_color_class_get(Engrave_Part_State *eps)
{
  return (eps ? eps->color_class : NULL);
}

/**
 * engrave_part_state_color_get - get the color value from the state
 * @param eps: The Engrave_Part_State to get the values from
 * @param r: Where to store the r value
 * @param g: Where to store the g value
 * @param b: Where to store the b value
 * @param a: Where to store the a value
 *
 * @return Returns no value.
 */
void engrave_part_state_color_get(Engrave_Part_State *eps, 
                                  int *r, int *g, int *b, int *a)
{
  if (r) *r = (eps ? eps->color.r : 255);
  if (g) *g = (eps ? eps->color.g : 255);
  if (b) *b = (eps ? eps->color.b : 255);
  if (a) *a = (eps ? eps->color.a : 255);
}

/**
 * engrave_part_state_color2_get - get the color2 value from the state
 * @param eps: The Engrave_Part_State to get the values from
 * @param r: Where to store the r value
 * @param g: Where to store the g value
 * @param b: Where to store the b value
 * @param a: Where to store the a value
 *
 * @return Returns no value.
 */
void engrave_part_state_color2_get(Engrave_Part_State *eps, 
                                  int *r, int *g, int *b, int *a)
{
  if (r) *r = (eps ? eps->color2.r : 0);
  if (g) *g = (eps ? eps->color2.g : 0);
  if (b) *b = (eps ? eps->color2.b : 0);
  if (a) *a = (eps ? eps->color2.a : 255);
}

/**
 * engrave_part_state_color3_get - get the color3 value from the state
 * @param eps: The Engrave_Part_State to get the values from
 * @param r: Where to store the r value
 * @param g: Where to store the g value
 * @param b: Where to store the b value
 * @param a: Where to store the a value
 *
 * @return Returns no value.
 */
void engrave_part_state_color3_get(Engrave_Part_State *eps, 
                                  int *r, int *g, int *b, int *a)
{
  if (r) *r = (eps ? eps->color3.r : 0);
  if (g) *g = (eps ? eps->color3.g : 0);
  if (b) *b = (eps ? eps->color3.b : 0);
  if (a) *a = (eps ? eps->color3.a : 128);
}

/**
 * engrave_part_state_image_normal_get - get the Engrave_Image for the normal setting
 * @param eps: The Engrave_Part_State to get the image from
 *
 * @return Returns the Engrave_Image for the normal setting or NULL on
 * failure.
 */
Engrave_Image *
engrave_part_state_image_normal_get(Engrave_Part_State *eps)
{
  return (eps ? eps->image.normal : NULL);
}

/**
 * engrave_part_state_image_border_get - get the border settings
 * @param eps: The Engrave_Part_State to get the settings from
 * @param l: Where to store the left value
 * @param r: Where to store the right value
 * @param t: Where to store the top value
 * @param b: Where to store the bottom value
 *
 * @return Returns no value.
 */
void
engrave_part_state_image_border_get(Engrave_Part_State *eps,
                                  int *l, int *r, int *t, int *b)
{
  if (l) *l = (eps ? eps->image.border.l : 0);
  if (r) *r = (eps ? eps->image.border.r : 0);
  if (t) *t = (eps ? eps->image.border.t : 0);
  if (b) *b = (eps ? eps->image.border.b : 0);
}

/**
 * engrave_part_state_text_text_get - get the text value
 * @param eps: The Engrave_Part_State to get the text from
 *
 * @return Returns the text value on success or NULL on failure.
 */
const char *
engrave_part_state_text_text_get(Engrave_Part_State *eps)
{
  return (eps ? eps->text.text : NULL);
}

/**
 * engrave_part_state_text_text_class_get - get the text class
 * @param eps: The Engrave_Part_State to get the value from
 *
 * @return Returns the text class or NULL on failure.
 */
const char *
engrave_part_state_text_text_class_get(Engrave_Part_State *eps)
{
  return (eps ? eps->text.text_class : NULL);
}

/**
 * engrave_part_state_text_font_get - get the text font
 * @param eps: The Engrave_Part_State to get the font from
 *
 * @return Returns the font on success or NULL on failure.
 */
const char *
engrave_part_state_text_font_get(Engrave_Part_State *eps)
{
  return (eps ? eps->text.font : NULL);
}

/**
 * engrave_part_state_text_size_get - get the font size
 * @param eps: The Engrave_Part_State to get the font size from
 *
 * @return Returns the font size.
 */
int
engrave_part_state_text_size_get(Engrave_Part_State *eps)
{
  return (eps ? eps->text.size : 0);
}

/**
 * engrave_part_state_text_fit_get - get the text fit settings
 * @param eps: The Engrave_Part_State to get the values from
 * @param x: Where to store the x value
 * @param y: Where to store the y value
 * 
 * @return Returns no value.
 */
void
engrave_part_state_text_fit_get(Engrave_Part_State *eps, int *x, int *y)
{
  if (x) *x = (eps ? eps->text.fit.x : 0);
  if (y) *y = (eps ? eps->text.fit.y : 0);
}

/**
 * engrave_part_state_text_min_get - get the text min size
 * @param eps: The Engrave_Part_State to get the values from
 * @param x: Where to store the x value
 * @param y: Where to store the y value
 * 
 * @return Returns no value.
 */
void
engrave_part_state_text_min_get(Engrave_Part_State *eps, int *x, int *y)
{
  if (x) *x = (eps ? eps->text.min.x : 0);
  if (y) *y = (eps ? eps->text.min.y : 0);
}

/**
 * engrave_part_state_text_align_get - get the text alignment settings
 * @param eps: The Engrave_Part_State to get the align from
 * @param x: Where to store the x value
 * @param y: Where to store the y value
 *
 * @return Returns no value.
 */
void
engrave_part_state_text_align_get(Engrave_Part_State *eps,
                                      double *x, double *y)
{
  if (x) *x = (eps ? eps->text.align.x : 0.5);
  if (y) *y = (eps ? eps->text.align.y : 0.5);
}

/**
 * engrave_part_state_tweens_count - get the number of tweens in the state
 * @param eps: The Engrave_Part_State to check for tweens
 *
 * @return Returns number of tweens, 0 otherwise
 */
int
engrave_part_state_tweens_count(Engrave_Part_State *eps)
{
  if (!eps) return 0;
  return evas_list_count(eps->image.tween);
}

/**
 * engrave_part_state_tween_foreach - Iterate over the tween list
 * @param eps: The Engrave_Part_State to get the tweens from
 * @param func: The function to call for each tween
 * @param data: User data to pass to the function
 * 
 * @return Returns no value.
 */
void
engrave_part_state_tween_foreach(Engrave_Part_State *eps,
                void (*func)(Engrave_Image *, void *), void *data)
{
  Evas_List *l;

  for (l = eps->image.tween; l; l = l->next) {
    Engrave_Image *ei = l->data;
    if (ei) func(ei, data);
  }
}


