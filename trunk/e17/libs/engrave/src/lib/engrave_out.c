#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <Engrave.h>
#include "engrave_macros.h"

static void _engrave_output_part(Engrave_Part *part, void *data);
static void _engrave_output_program(Engrave_Program *program, void *data);
static void _engrave_output_state(Engrave_Part_State *state, Engrave_Part *part, void *data);
static void _engrave_output_image(Engrave_Image *image, void *data);
static void _engrave_output_font(Engrave_Font *font, void *data);
static void _engrave_output_data(Engrave_Data *data, void *udata);
static void _engrave_output_group(Engrave_Group *group, void *data);
static void _engrave_program_output_after(char *after, void *data);
static void _engrave_program_output_target(char *target, void *data);
static void _engrave_part_state_output_tween(Engrave_Image *ei, void *data);

static int level = 0;

static char *_image_type_string[ENGRAVE_IMAGE_TYPE_NUM] = {
        "RAW",
        "COMP",
        "LOSSY"
        };

static char *_part_type_string[ENGRAVE_PART_TYPE_NUM] = {
        "IMAGE",
        "TEXT",
        "RECT",
        "SWALLOW"
        };

static char *_text_effect_string[ENGRAVE_TEXT_EFFECT_NUM] = {
        "NONE",
        "PLAIN",
        "OUTLINE",
        "SOFT_OUTLINE",
        "SHADOW",
        "OUTLINE_SHADOW",
        "SOFT_SHADOW",
        "OUTLINE_SOFT_SHADOW"
        };

static char *_action_string[ENGRAVE_ACTION_NUM] = {
        "STATE_SET",
        "ACTION_STOP",
        "SIGNAL_EMIT",
        "DRAG_VAL_SET",
        "DRAG_VAL_STEP",
        "DRAG_VAL_PAGE",
        "SCRIPT"
        };

static char *_transition_string[ENGRAVE_TRANSITION_NUM] = {
        "LINEAR",
        "SINUSOIDAL",
        "ACCELERATE",
        "DECELERATE"
        };

static char *_aspect_preference_string[ENGRAVE_ASPECT_PREFERENCE_NUM] = {
        "NONE",
        "VERTICAL",
        "HORIZONTAL",
        "BOTH"
        };

static char *
engrave_output_mk_tabs(void)
{
    char *buf = (char *)calloc(level + 1, sizeof(char));
    int i = 0;
    for(i = 0; i < level; i++)
        buf[i] = '\t';
    return buf;
}

static void
engrave_out_start(FILE *out, char *type)
{
    char *buf = engrave_output_mk_tabs();
    fprintf(out, "%s%s {\n", buf, type);
    FREE(buf);
    level ++;
}

static void
engrave_out_end(FILE *out)
{
    char *buf = NULL;
    level--;
    buf = engrave_output_mk_tabs();
    fprintf(out, "%s}\n", buf);
    FREE(buf);
}

static void
engrave_out_data(FILE *out, char *name, char *fmt, ...)
{
    va_list ap;
    char *fmt_new = (char *)calloc(strlen(fmt) + strlen(name) + level + 5, sizeof(char));
    char *buf = engrave_output_mk_tabs();

    sprintf(fmt_new, "%s%s: %s;\n", buf, name, fmt);
    va_start(ap, fmt);
    vfprintf(out, fmt_new, ap);
    va_end(ap);
    FREE(buf);
}

/** 
 * engrave_eet_output -- Create an EET file from the in-memory data.
 * @param engrave_file: The Engrave_File to use to create the EET file.
 * @param path: The filename to save the EET file too.
 *
 * @return Returns 1 on success 0 otherwise.
 */
int
engrave_eet_output(Engrave_File *engrave_file, char *path)
{
  static char tmpn[1024];
  int len = 0, fd = 0, ret = 0;
  char *cmd = NULL;

  strcpy(tmpn, "/tmp/engrave_cc.edc-tmp-XXXXXX");
  fd = mkstemp(tmpn);
  if (fd < 0) {
    fprintf(stderr, "Unable to create tmp file: %s\n", strerror(errno));
    return 0;
  }
  close(fd);

  engrave_edc_output(engrave_file, tmpn);
  /* FIXME images and fonts ??? */

  len = strlen(tmpn) + strlen(path) + 13;
  cmd = (char *)calloc(len, sizeof(char));
  snprintf(cmd, len, "edje_cc -v %s %s", tmpn, path);
  ret = system(cmd);

  if (ret < 0) {
    fprintf(stderr, "Unable to execute edje_cc on tmp file: %s\n", 
                                                    strerror(errno));
    return 0;                                                    
  }

  unlink(tmpn);
  return 1;
}

/** 
 * engrave_edc_output -- Create an EDC file from the in-memory data.
 * @param engrave_file: The Engrave_File to use to create the EET file.
 * @param path: The filename to save the EDC file too.
 *
 * @return Returns 1 on success 0 otherwise.
 */
int
engrave_edc_output(Engrave_File *engrave_file, char *path)
{
  FILE *out = NULL;
  
  if (!engrave_file) return 0;

  out = fopen(path, "w");
  if (!out)
  {
    printf("can't open %s for writing\n", path);
    return 0;
  }

  /* fonts */
  engrave_out_start(out, "fonts");
  engrave_file_font_foreach(engrave_file, _engrave_output_font, out);
  engrave_out_end(out);

  /* images */
  engrave_out_start(out, "images");
  engrave_file_image_foreach(engrave_file, _engrave_output_image, out);
  engrave_out_end(out);

  /* data */
  engrave_out_start(out, "data");
  engrave_file_data_foreach(engrave_file, _engrave_output_data, out);
  engrave_out_end(out);

  /* collections */
  engrave_out_start(out, "collections");
  engrave_file_group_foreach(engrave_file, _engrave_output_group, out);
  engrave_out_end(out);

  fclose(out);
  return 1;
}

static void
_engrave_output_group(Engrave_Group *group, void *data)
{
  FILE *out = data;
  char *tmp = NULL;
  int w, h;

  engrave_out_start(out, "group");

  tmp = engrave_group_name_get(group);
  engrave_out_data(out, "name", "\"%s\"", tmp);
  IF_FREE(tmp);

  engrave_group_min_size_get(group, &w, &h);
  if (w != 0 || h != 0)
    engrave_out_data(out, "min", "%d %d", w, h);

  engrave_group_max_size_get(group, &w, &h);
  if(w >= 0 || h >= 0)
    engrave_out_data(out, "max", "%d %d", w, h);
    
  /* data */
  if (engrave_group_data_count(group) > 0)
  {
    engrave_out_start(out, "data");
    engrave_group_data_foreach(group, _engrave_output_data, out);
    engrave_out_end(out);
  }

  /* script */
  tmp = engrave_group_script_get(group);
  if (tmp) {
    engrave_out_start(out, "script");
    /* FIXME scripts are wierd... */
    fprintf(out, "%s", tmp);
    engrave_out_end(out);
    FREE(tmp);
  }

  /* parts */
  engrave_out_start(out, "parts");
  engrave_group_parts_foreach(group, _engrave_output_part, out);
  engrave_out_end(out);

  /* programs */
  engrave_out_start(out, "programs");
  engrave_group_programs_foreach(group, _engrave_output_program, out);
  engrave_out_end(out);

  engrave_out_end(out);   /* group */
}

static void
_engrave_output_part(Engrave_Part *part, void *data)
{
  FILE *out = data;
  char *tmp;
  int x, step_x, count_x;
  int y, step_y, count_y;

  engrave_out_start(out, "part");

  tmp = engrave_part_name_get(part);
  engrave_out_data(out, "name", "\"%s\"", tmp);
  IF_FREE(tmp);

  engrave_out_data(out, "type", "%s", 
        _part_type_string[engrave_part_type_get(part)]);

  if (!engrave_part_mouse_events_get(part))
    engrave_out_data(out, "mouse_events", "%d",
                    engrave_part_mouse_events_get(part));

  if (engrave_part_repeat_events_get(part))
    engrave_out_data(out, "repeat_events", "%d",
                  engrave_part_repeat_events_get(part));

  if (engrave_part_effect_get(part) != ENGRAVE_TEXT_EFFECT_NONE)
    engrave_out_data(out, "effect", "%s", 
                  _text_effect_string[engrave_part_effect_get(part)]);

  tmp = engrave_part_clip_to_get(part);
  if (tmp)
  {
    engrave_out_data(out, "clip_to", "\"%s\"", tmp);
    FREE(tmp);
  }

  engrave_part_dragable_x_get(part, &x, &step_x, &count_x);
  engrave_part_dragable_y_get(part, &y, &step_y, &count_y);
  if (x || y)
  {
    engrave_out_start(out, "dragable");
    engrave_out_data(out, "x", "%d %d %d", x, step_x, count_x);
    engrave_out_data(out, "y", "%d %d %d", y, step_y, count_y);

    tmp = engrave_part_dragable_confine_get(part);
    if (tmp)
    {
      engrave_out_data(out, "confine", "\"%s\"", tmp);
      FREE(tmp);
    }
    engrave_out_end(out);
  }
  engrave_part_state_foreach(part, _engrave_output_state, out);
  engrave_out_end(out);
}

static void
_engrave_output_program(Engrave_Program *program, void *data)
{
  FILE *out = data;
  char *tmp;
  Engrave_Action action;
  double value, value2;
  char state[125], state2[125];

  engrave_out_start(out, "program");

  tmp = engrave_program_name_get(program);
  engrave_out_data(out, "name", "\"%s\"", tmp);
  IF_FREE(tmp);

  tmp = engrave_program_signal_get(program);
  engrave_out_data(out, "signal", "\"%s\"", tmp);
  IF_FREE(tmp);

  tmp = engrave_program_source_get(program);
  engrave_out_data(out, "source", "\"%s\"", tmp);
  IF_FREE(tmp);

  engrave_program_action_get(program, &action, state, 
                              state2, &value, &value2);
  switch (action) {
    case ENGRAVE_ACTION_STOP:
      engrave_out_data(out, "action", "%s", _action_string[action]);
      break;

    case ENGRAVE_ACTION_DRAG_VAL_SET:
    case ENGRAVE_ACTION_DRAG_VAL_STEP:
    case ENGRAVE_ACTION_DRAG_VAL_PAGE:
      engrave_out_data(out, "action", "%s %.2f %.2f",
            _action_string[action], value, value2);
      break;

    case ENGRAVE_ACTION_STATE_SET:
      {
        Engrave_Transition transition;
        double duration;

        engrave_out_data(out, "action", "%s \"%s\" %.2f",
              _action_string[action], state ? state : "", value);

        engrave_program_transition_get(program, &transition, &duration);
        engrave_out_data(out, "transition", "%s %.2f",
              _transition_string[transition], duration);
      }
      break;

    case ENGRAVE_ACTION_SIGNAL_EMIT:
      engrave_out_data(out, "action", "%s \"%s\" \"%s\"",
            _action_string[action], state, state2);
      break;

    case ENGRAVE_ACTION_SCRIPT:
      {
        char *script = engrave_program_script_get(program);
        if (script)
        {
          engrave_out_start(out, "script");
          /* FIXME scripts are wierd ... */
          fprintf(out, "%s", script);
          engrave_out_end(out);
          FREE(script);
        }
      }
      break;

    case ENGRAVE_ACTION_NUM:
      break;

    default:
      fprintf(stderr, "Unknown program action: %d\n", action);
      break;
  }

  engrave_program_in_get(program, &value, &value2);
  if (value || value2)
    engrave_out_data(out, "in", "%.2f %.2f", value, value2 );

  engrave_program_target_foreach(program, _engrave_program_output_target, out);
  engrave_program_after_foreach(program, _engrave_program_output_after, out);

  engrave_out_end(out);
}

/* Note: We only output if the value is different from the default to save
 * space 
 */
static void
_engrave_output_state(Engrave_Part_State *state, Engrave_Part *part, void *data)
{
  FILE *out = data;
  char *tmp, *tmp2;
  double x, y;
  int w, h, ix, iy;
  int r, g, b, a, l, t;
  Engrave_Aspect_Preference aspect_pref;
  Engrave_Image *im;

  engrave_out_start(out, "description");

  tmp = engrave_part_state_name_get(state, &x);
  engrave_out_data(out, "state", "\"%s\" %.2f", tmp, x);
  IF_FREE(tmp);

  engrave_out_data(out, "visible", "%d",
                          engrave_part_state_visible_get(state));

  engrave_part_state_align_get(state, &x, &y);
  if (x != .5 || y != .5)
    engrave_out_data(out, "align", "%.2f %.2f", x, y);

  engrave_part_state_step_get(state, &x, &y);
  if (x || y)
    engrave_out_data(out, "step", "%.2f %.2f", x, y);

  engrave_part_state_min_get(state, &w, &h);
  if (w > 0 || h > 0)
    engrave_out_data(out, "min", "%d %d", w, h);

  engrave_part_state_max_get(state, &w, &h);
  if (w >= 0 || h >= 0)
    engrave_out_data(out, "max", "%d %d", w, h);

  engrave_part_state_aspect_get(state, &x, &y);
  if (x || y)
    engrave_out_data(out, "aspect", "%.2f %.2f", x, y);

  aspect_pref = engrave_part_state_aspect_preference_get(state);
  if (aspect_pref)
    engrave_out_data(out, "aspect_preference", "%s",
            _aspect_preference_string[aspect_pref]);
  
  /* rel 1 */ 
  engrave_out_start(out, "rel1");

  engrave_part_state_rel1_relative_get(state, &x, &y);
  engrave_out_data(out, "relative", "%.2f %.2f", x, y);
  
  engrave_part_state_rel1_offset_get(state, &ix, &iy);
  engrave_out_data(out, "offset", "%d %d", ix, iy);

  tmp = engrave_part_state_rel1_to_x_get(state);
  tmp2 = engrave_part_state_rel1_to_y_get(state);
  if (tmp || tmp2)
  {
    if (!strcmp(tmp, tmp2))
      engrave_out_data(out, "to", "\"%s\"", tmp);
    else
    {
      engrave_out_data(out, "to_x", "\"%s\"", tmp);
      engrave_out_data(out, "to_y", "\"%s\"", tmp2);
    }
  }
  IF_FREE(tmp);
  IF_FREE(tmp2);

  engrave_out_end(out);

  /* rel 2 */ 
  engrave_out_start(out, "rel2");

  engrave_part_state_rel2_relative_get(state, &x, &y);
  engrave_out_data(out, "relative", "%.2f %.2f", x, y);

  engrave_part_state_rel2_offset_get(state, &ix, &iy);
  engrave_out_data(out, "offset", "%d %d", ix, iy);

  tmp = engrave_part_state_rel2_to_x_get(state);
  tmp2 = engrave_part_state_rel2_to_y_get(state);
  if (tmp || tmp2)
  {
    if (!strcmp(tmp, tmp2))
      engrave_out_data(out, "to", "\"%s\"", tmp);
    else
    {
      engrave_out_data(out, "to_x", "\"%s\"", tmp);
      engrave_out_data(out, "to_y", "\"%s\"", tmp2);
    }
  }
  IF_FREE(tmp);
  IF_FREE(tmp2);

  engrave_out_end(out);

  tmp = engrave_part_state_color_class_get(state);
  if (tmp) {
    engrave_out_data(out, "color_class", "\"%s\"", tmp); 
    FREE(tmp);
  }

  engrave_part_state_color_get(state, &r, &g, &b, &a);
  if (r != 255 || g != 255 || b != 255 || a != 255)
    engrave_out_data(out, "color", "%d %d %d %d", r, g, b, a);

  engrave_part_state_color2_get(state, &r, &g, &b, &a);
  if (r != 0 || g != 0 || b != 0 || a != 255)
    engrave_out_data(out, "color2", "%d %d %d %d", r, g, b, a);

  engrave_part_state_color3_get(state, &r, &g, &b, &a);
  if (r != 0 || g != 0 || b != 0 || a != 128)
    engrave_out_data(out, "color3", "%d %d %d %d", r, g, b, a);

  im = engrave_part_state_image_normal_get(state);
  if ((engrave_part_type_get(part) == ENGRAVE_PART_TYPE_IMAGE) && im)
  {
    engrave_out_start(out, "image");

    tmp = engrave_image_name_get(im);
    engrave_out_data(out, "normal", "\"%s\"", tmp);
    IF_FREE(tmp);

    engrave_part_state_tween_foreach(state,
                        _engrave_part_state_output_tween, out);

    engrave_part_state_image_border_get(state, &l, &r, &t, &b);
    if (l || r || t || b)
      engrave_out_data(out, "border", "%d %d %d %d", l, r, t, b);
    engrave_out_end(out);
  }
  else if (engrave_part_type_get(part) == ENGRAVE_PART_TYPE_TEXT)
  {
    engrave_out_start(out, "text");

    tmp = engrave_part_state_text_text_get(state);
    if (tmp) {
      engrave_out_data(out, "text", "\"%s\"", tmp);
      FREE(tmp);
    }
   
    tmp = engrave_part_state_text_text_class_get(state);
    if (tmp) {
      engrave_out_data(out, "text_class", "\"%s\"", tmp);
      FREE(tmp);
    }
  
    tmp = engrave_part_state_text_font_get(state);
    if (tmp) {
      engrave_out_data(out, "font", "\"%s\"", tmp);
      FREE(tmp);
    }
   
    h = engrave_part_state_text_size_get(state);
    if (h)
      engrave_out_data(out, "size", "%d", h);

    engrave_part_state_text_fit_get(state, &ix, &iy); 
    if (x || y)
      engrave_out_data(out, "fit", "%d %d", ix, iy);
 
    engrave_part_state_text_min_get(state, &ix, &iy);
    if (x || y)
      engrave_out_data(out, "min", "%d %d", ix, iy);

    engrave_part_state_text_align_get(state, &x, &y);
    if (x || y)
      engrave_out_data(out, "align", "%.2f %.2f", x, y);

    engrave_out_end(out);
  }
  engrave_out_end(out);
}

static void
_engrave_output_image(Engrave_Image *image, void *data)
{
  char *name;
  FILE *out;

  out = data;
  name = engrave_image_name_get(image);

  if (engrave_image_type_get(image) == ENGRAVE_IMAGE_TYPE_LOSSY)
    engrave_out_data(out, "image", "\"%s\" %s %.2f", name,
              _image_type_string[engrave_image_type_get(image)],
              engrave_image_compression_value_get(image));
  else
    engrave_out_data(out, "image", "\"%s\" %s", name,
              _image_type_string[engrave_image_type_get(image)]);

  IF_FREE(name);
}

static void
_engrave_output_font(Engrave_Font *font, void *data)
{
  char *name, *path;
  FILE *out;

  out = data;
  name = engrave_font_name_get(font);
  path = engrave_font_path_get(font);

  engrave_out_data(out, "font", "\"%s\" \"%s\"", path, name);
  IF_FREE(name);
  IF_FREE(path);
}

static void
_engrave_output_data(Engrave_Data *data, void *udata)
{
  char *key, *value;
  FILE *out;

  out = udata;
  key = engrave_data_key_get(data);
  value = engrave_data_value_get(data);

  if (value)
    engrave_out_data(out, "item", "\"%s\" \"%s\"", key, value);
  else
    engrave_out_data(out, "item", "\"%s\" %d",
                            key, engrave_data_int_value_get(data));

  IF_FREE(key);
  IF_FREE(value);
}

static void
_engrave_program_output_target(char *target, void *data)
{
    FILE *out = data;
    engrave_out_data(out, "target", "\"%s\"", target);
}

static void
_engrave_program_output_after(char *after, void *data)
{
  FILE *out = data;
  engrave_out_data(out, "after", "\"%s\"", after);
}

static void
_engrave_part_state_output_tween(Engrave_Image *ei, void *data)
{
  FILE *out = data;
  char *name = engrave_image_name_get(ei);

  engrave_out_data(out, "tween", "\"%s\"", name);
  IF_FREE(name);
}


