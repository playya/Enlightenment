#ifndef ENGRAVE_PART_STATE_H
#define ENGRAVe_PART_STATE_H

typedef struct _Engrave_Part_State Engrave_Part_State;
struct _Engrave_Part_State
{
  char *name;
  double value;

  unsigned char visible;

  struct
  {
    double x, y;
  } align, step;

  struct
  {
    double w, h;
  } min, max;

  struct
  {
    double w, h;
    Engrave_Aspect_Preference prefer;
  } aspect;

  struct
  {
    struct
    {
      double x, y;
    } relative;
    
    struct
    {
      int x, y;
    } offset;
    
    char *to_x;
    char *to_y;
  } rel1, rel2;

  struct
  {
    Engrave_Image *normal;
    Evas_List *tween;
  } image;

  struct
  {
    int l, r, t, b;
  } border;

  char *color_class;

  struct
  {
    int r, g, b, a;
  } color, color2, color3;

  struct
  {
    int           smooth; 

    struct
    {
      double x, y;
    } pos_rel, rel;
    struct
    {
      int x, y;
    } pos_abs, abs;
  } fill;

  struct
  {
    char          *text; 
    char          *text_class; 
    char          *font; 

    int            size; 

    struct {
      int x, y;
    } fit, min;

    struct {
      double      x, y; 
    } align;
  } text;
};

Engrave_Part_State * engrave_part_state_new(void);
void engrave_part_state_name_set(Engrave_Part_State *eps, char *name, 
                                                          double value);
void engrave_part_state_visible_set(Engrave_Part_State *eps, int visible);
void engrave_part_state_align_set(Engrave_Part_State *eps, int x, int y);
void engrave_part_state_step_set(Engrave_Part_State *eps, int x, int y);
void engrave_part_state_min_size_set(Engrave_Part_State *eps, int w, int h);
void engrave_part_state_max_size_set(Engrave_Part_State *eps, int w, int h);
void engrave_part_state_aspect_set(Engrave_Part_State *eps, int w, int h);
void engrave_part_state_aspect_preference_set(Engrave_Part_State *eps,
                                            Engrave_Aspect_Preference prefer);
void engrave_part_state_rel1_relative_set(Engrave_Part_State *eps, 
                                                        int x, int y);
void engrave_part_state_rel1_offset_set(Engrave_Part_State *eps, int x, int y);
void engrave_part_state_rel1_to_set(Engrave_Part_State *eps, char *to);
void engrave_part_state_rel1_to_x_set(Engrave_Part_State *eps, char *to);
void engrave_part_state_rel1_to_y_set(Engrave_Part_State *eps, char *to);

void engrave_part_state_rel2_relative_set(Engrave_Part_State *eps, 
                                                        int x, int y);
void engrave_part_state_rel2_offset_set(Engrave_Part_State *eps, int x, int y);
void engrave_part_state_rel2_to_set(Engrave_Part_State *eps, char *to);
void engrave_part_state_rel2_to_x_set(Engrave_Part_State *eps, char *to);
void engrave_part_state_rel2_to_y_set(Engrave_Part_State *eps, char *to);

void engrave_part_state_image_normal_set(Engrave_Part_State *eps, 
                                                    Engrave_Image *im);
void engrave_part_state_image_tween_add(Engrave_Part_State *eps,
                                                    Engrave_Image *im);

void engrave_part_state_border_set(Engrave_Part_State *eps, 
                                        int l, int r, int t, int b);
void engrave_part_state_color_class_set(Engrave_Part_State *eps,
                                                        char *class);
void engrave_part_state_color_set(Engrave_Part_State *eps, 
                                        int r, int g, int b, int a);
void engrave_part_state_color2_set(Engrave_Part_State *eps, 
                                        int r, int g, int b, int a);
void engrave_part_state_color3_set(Engrave_Part_State *eps, 
                                        int r, int g, int b, int a);
void engrave_part_state_fill_smooth_set(Engrave_Part_State *eps,
                                                        int smooth);
void engrave_part_state_fill_origin_relative_set(Engrave_Part_State *state,
                                                double x, double y);
void engrave_part_state_fill_size_relative_set(Engrave_Part_State *eps,
                                                double x, double y);
void engrave_part_state_fill_origin_offset_set(Engrave_Part_State *eps,
                                                      int x, int y);
void engrave_part_state_fill_size_offset_set(Engrave_Part_State *eps,
                                                int x, int y);

void engrave_part_state_text_text_set(Engrave_Part_State *eps, char *text);
void engrave_part_state_text_text_class_set(Engrave_Part_State *eps, 
                                                            char *text);
void engrave_part_state_text_font_set(Engrave_Part_State *eps, char *font);
void engrave_part_state_text_size_set(Engrave_Part_State *eps, int size);
void engrave_part_state_text_fit_set(Engrave_Part_State *eps, int x, int y);
void engrave_part_state_text_min_set(Engrave_Part_State *eps, int x, int y);
void engrave_part_state_text_align_set(Engrave_Part_State *eps, 
                                                        double x, double y);

#endif

