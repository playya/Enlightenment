#include "Engrave.h"

static Engrave_File *engrave_file = 0;
extern FILE *yyin;

Engrave_File *
engrave_parse(char *file)
{
  engrave_file = engrave_file_new();

  yyin = fopen(file, "r");
  yyparse();
  fclose(yyin);

  return (engrave_file);
}

void
engrave_parse_font(char *file, char *name)
{
  Engrave_Font *font;
  font = engrave_font_new(file, name);
  engrave_file_font_add(engrave_file, font);
}

void
engrave_parse_image(char *name, Engrave_Image_Type type, double value)
{
  Engrave_Image *image;
  image = engrave_image_new(name, type, value);
  engrave_file_image_add(engrave_file, image);
}

void
engrave_parse_data(char *key, char *value)
{
  Engrave_Data *data;
  data = engrave_data_new(key, value);
  engrave_file_data_add(engrave_file, data);
}

void
engrave_parse_group()
{
  Engrave_Group *group;
  group = engrave_group_new();
  engrave_file_group_add(engrave_file, group);
}

void
engrave_parse_group_data(char *key, char *value)
{
  Engrave_Group *group;
  Engrave_Data *data;
 
  /* XXX why is this put inboth file and group data? */
  data = engrave_data_new(key, value);
  engrave_file_data_add(engrave_file, data);

  group = engrave_file_group_last_get(engrave_file);
  engrave_group_data_add(group, data);
}

void
engrave_parse_group_script(char *script)
{
  Engrave_Group *group;
  group = engrave_file_group_last_get(engrave_file);
  engrave_group_script_set(group, script); 
}

void
engrave_parse_group_name(char *name)
{
  Engrave_Group *group;
  group = engrave_file_group_last_get(engrave_file);
  engrave_group_name_set(group, name);
}

void
engrave_parse_group_min(int w, int h)
{
  Engrave_Group *group;
  group = engrave_file_group_last_get(engrave_file);
  engrave_group_min_size_set(group, w, h);
}

void
engrave_parse_group_max(int w, int h)
{
  Engrave_Group *group;
  group = engrave_file_group_last_get(engrave_file);
  engrave_group_max_size_set(group, w, h);
}

void
engrave_parse_part()
{
  Engrave_Group *group;
  Engrave_Part *part;

  part = engrave_part_new(ENGRAVE_PART_TYPE_IMAGE);
  engrave_part_mouse_events_set(part, 1);
  engrave_part_repeat_events_set(part, 0);
 
  group = engrave_file_group_last_get(engrave_file);
  engrave_group_part_add(group, part);
}

void
engrave_parse_part_name(char *name)
{
  Engrave_Group *group;
  Engrave_Part *part;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  engrave_part_name_set(part, name);
}

void
engrave_parse_part_type(Engrave_Part_Type type)
{
  Engrave_Group *group;
  Engrave_Part *part;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  engrave_part_type_set(part, type);
}

void
engrave_parse_part_effect(Engrave_Text_Effect effect)
{
  Engrave_Group *group;
  Engrave_Part *part;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  engrave_part_effect_set(part, effect);
}

void
engrave_parse_part_mouse_events(int mouse_events)
{
  Engrave_Group *group;
  Engrave_Part *part;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  engrave_part_mouse_events_set(part, mouse_events);
}

void
engrave_parse_part_repeat_events(int repeat_events)
{
  Engrave_Group *group;
  Engrave_Part *part;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  engrave_part_repeat_events_set(part, repeat_events);
}

void
engrave_parse_part_clip_to(char *clip_to)
{
  Engrave_Group *group;
  Engrave_Part *part;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  engrave_part_clip_to_set(part, clip_to);
}

void
engrave_parse_part_dragable_x(int x, int step, int count)
{
  Engrave_Group *group;
  Engrave_Part *part;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  engrave_part_dragable_x_set(part, x, step, count);
}

void
engrave_parse_part_dragable_y(int y, int step, int count)
{
  Engrave_Group *group;
  Engrave_Part *part;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  engrave_part_dragable_y_set(part, y, step, count);
}

void
engrave_parse_part_dragable_confine(char *confine)
{
  Engrave_Group *group;
  Engrave_Part *part;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  engrave_part_dragable_confine_set(part, confine);
}

void
engrave_parse_state()
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  state = engrave_part_state_new();

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  engrave_part_state_add(part, state);
}

void
engrave_parse_state_name(char *name, double value)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_name_set(state, name, value);
}

void
engrave_parse_state_visible(int visible)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_visible_set(state, visible);
}

void
engrave_parse_state_inherit(char *name, double val)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *to;
  Engrave_Part_State *from;
  char *state_name;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);

  to = engrave_part_state_last_get(part);
  state_name = engrave_part_state_name_get(to, NULL);

  /* must have a name set before we can be inherited into */
  if (!state_name) {
    char *part_name = engrave_part_name_get(part);
    fprintf(stderr, "part %s: inherit may only be used after state!\n",
                                                            part_name);
    free(part_name);
    return;
  }

  /* can't inherit into the default part */
  if ((strlen(state_name) == 7) && (!strncmp(state_name, "default", 7))) {
    char *part_name = engrave_part_name_get(part);
    fprintf(stderr, "part %s: "
              "inherit may not be used in the default description!\n",
              part_name);
    free(part_name);
    return;
  }

  from = engrave_part_state_by_name_value_find(part, name, val);
  if (from)
    engrave_part_state_copy(from, to);
  else
    fprintf(stderr, "Unable to locate description %s %f\n", name, val);
}

void
engrave_parse_state_align(double x, double y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_align_set(state, x, y);
}

void
engrave_parse_state_step(double x, double y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_step_set(state, x, y);
}

void
engrave_parse_state_min(double w, double h)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_min_size_set(state, w, h);
}

void
engrave_parse_state_max(double w, double h)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_min_size_set(state, w, h);
}

void
engrave_parse_state_aspect(double w, double h)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_aspect_set(state, w, h);
}

void
engrave_parse_state_aspect_preference(Engrave_Aspect_Preference prefer)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_aspect_preference_set(state, prefer);
}

void
engrave_parse_state_rel1_relative(double x, double y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_rel1_relative_set(state, x, y);
}

void
engrave_parse_state_rel1_offset(int x, int y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_rel1_offset_set(state, x, y);
}

void
engrave_parse_state_rel1_to_x(char *to)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_rel1_to_x_set(state, to);
}

void
engrave_parse_state_rel1_to_y(char *to)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_rel1_to_y_set(state, to);
}

void
engrave_parse_state_rel1_to(char *to)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_rel1_to_set(state, to);
}

void
engrave_parse_state_rel2_relative(double x, double y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_rel2_relative_set(state, x, y);
}

void
engrave_parse_state_rel2_offset(int x, int y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_rel2_offset_set(state, x, y);
}

void
engrave_parse_state_rel2_to_x(char *to)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_rel2_to_x_set(state, to);
}

void
engrave_parse_state_rel2_to_y(char *to)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_rel2_to_y_set(state, to);
}

void
engrave_parse_state_rel2_to(char *to)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_rel2_to_set(state, to);
}

void
engrave_parse_state_image_normal(char *name)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;
  Engrave_Image *im;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);

  im = engrave_file_image_by_name_find(engrave_file, name);
  if (im)
    engrave_part_state_image_normal_set(state, im);
  else
    printf("Error: image \"%s\" does not exist\n", name);
}

void
engrave_parse_state_image_tween(char *name)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;
  Engrave_Image *im;
 
  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);

  im = engrave_file_image_by_name_find(engrave_file, name);
  if (im)
    engrave_part_state_image_tween_add(state, im);
  else
    printf("Error: image \"%s\" does not exist\n", name);
}

void
engrave_parse_image_border(int l, int r, int t, int b)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_image_border_set(state, l, r, t, b);
}

void
engrave_parse_state_color_class(char *color_class)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_color_class_set(state, color_class);
}

void
engrave_parse_state_color(int r, int g, int b, int a)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_color_set(state, r, g, b, a);
}

void
engrave_parse_state_color2(int r, int g, int b, int a)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_color2_set(state, r, g, b, a);
}

void
engrave_parse_state_color3(int r, int g, int b, int a)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_color3_set(state, r, g, b, a);
}

void
engrave_parse_state_fill_smooth(int smooth)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_fill_smooth_set(state, smooth);
}

void
engrave_parse_state_fill_origin_relative(double x, double y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_fill_origin_relative_set(state, x, y);
}

void
engrave_parse_state_fill_size_relative(double x, double y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_fill_size_relative_set(state, x, y);
}

void
engrave_parse_state_fill_origin_offset(int x, int y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_fill_origin_offset_set(state, x, y);
}

void
engrave_parse_state_fill_size_offset(int x, int y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_fill_size_offset_set(state, x, y);
}

void
engrave_parse_state_text_text(char *text)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_text_text_set(state, text);
}

void
engrave_parse_state_text_text_class(char *text_class)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_text_text_class_set(state, text_class);
}

void
engrave_parse_state_text_font(char *font)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_text_font_set(state, font);
}

void
engrave_parse_state_text_size(int size)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_text_size_set(state, size);
}

void
engrave_parse_state_text_fit(int x, int y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_text_fit_set(state, x, y);
}

void
engrave_parse_state_text_min(int x, int y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_text_min_set(state, x, y);
}

void
engrave_parse_state_text_align(double x, double y)
{
  Engrave_Group *group;
  Engrave_Part *part;
  Engrave_Part_State *state;

  group = engrave_file_group_last_get(engrave_file);
  part = engrave_group_part_last_get(group);
  state = engrave_part_state_last_get(part);
  engrave_part_state_text_align_set(state, x, y);
}

void
engrave_parse_program()
{
  Engrave_Group *group;
  Engrave_Program *program;

  program = engrave_program_new();
  group = engrave_file_group_last_get(engrave_file);

  group->programs = evas_list_append(group->programs, program);
}

void
engrave_parse_program_script(char *script)
{
  Engrave_Group *group;
  Engrave_Program *program;

  group = engrave_file_group_last_get(engrave_file);
  program = engrave_group_program_last_get(group);
  engrave_program_script_set(program, script);
}

void
engrave_parse_program_name(char *name)
{
  Engrave_Group *group;
  Engrave_Program *program;

  group = engrave_file_group_last_get(engrave_file);
  program = engrave_group_program_last_get(group);
  engrave_program_name_set(program, name);
}

void
engrave_parse_program_signal(char *signal)
{
  Engrave_Group *group;
  Engrave_Program *program;

  group = engrave_file_group_last_get(engrave_file);
  program = engrave_group_program_last_get(group);
  engrave_program_signal_set(program, signal);
}

void
engrave_parse_program_source(char *source)
{
  Engrave_Group *group;
  Engrave_Program *program;

  group = engrave_file_group_last_get(engrave_file);
  program = engrave_group_program_last_get(group);
  engrave_program_source_set(program, source);
}

void
engrave_parse_program_target(char *target)
{
  Engrave_Group *group;
  Engrave_Program *program;

  group = engrave_file_group_last_get(engrave_file);
  program = engrave_group_program_last_get(group);
  engrave_program_target_add(program, target);
}

void
engrave_parse_program_after(char *after)
{
  Engrave_Group *group;
  Engrave_Program *program;

  group = engrave_file_group_last_get(engrave_file);
  program = engrave_group_program_last_get(group);
  engrave_program_after_add(program, after);
}

void
engrave_parse_program_in(double from, double range)
{
  Engrave_Group *group;
  Engrave_Program *program;

  group = engrave_file_group_last_get(engrave_file);
  program = engrave_group_program_last_get(group);
  engrave_program_in_set(program, from, range);
}

/* handle different action types */
void
engrave_parse_program_action(Engrave_Action action, char *state, 
                                char *state2, double value, double value2)
{
  Engrave_Group *group;
  Engrave_Program *program;

  group = engrave_file_group_last_get(engrave_file);
  program = engrave_group_program_last_get(group);
  engrave_program_action_set(program, action, state, state2, value, value2);
}

void
engrave_parse_program_transition(Engrave_Transition transition, double duration)
{
  Engrave_Group *group;
  Engrave_Program *program;

  group = engrave_file_group_last_get(engrave_file);
  program = engrave_group_program_last_get(group);
  engrave_program_transition_set(program, transition, duration);
}

