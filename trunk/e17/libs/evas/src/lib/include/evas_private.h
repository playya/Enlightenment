#ifndef EVAS_PRIVATE_H
#define EVAS_PRIVATE_H
#endif

#define _GNU_SOURCE

/* complain when peole pass in wrong object types etc. */
#define MAGIC_DEBUG

#define RENDER_METHOD_SOFTWARE_X11       0x00000001
#define RENDER_METHOD_DIRECTFB           0x00000002
#define RENDER_METHOD_FB                 0x00000003
#define RENDER_METHOD_BUFFER             0x00000004
#define RENDER_METHOD_SOFTWARE_WIN32_GDI 0x00000005
#define RENDER_METHOD_SOFTWARE_QTOPIA    0x00000006
#define RENDER_METHOD_GL_X11             0x00000007

#define RENDER_METHOD_INVALID            0x00000000

typedef enum _Evas_Callback_Type
{
   EVAS_CALLBACK_MOUSE_IN,
   EVAS_CALLBACK_MOUSE_OUT,
   EVAS_CALLBACK_MOUSE_DOWN,
   EVAS_CALLBACK_MOUSE_UP,
   EVAS_CALLBACK_MOUSE_MOVE,
   EVAS_CALLBACK_MOUSE_WHEEL,	 
   EVAS_CALLBACK_FREE,
   EVAS_CALLBACK_KEY_DOWN,
   EVAS_CALLBACK_KEY_UP,
   EVAS_CALLBACK_FOCUS_IN,
   EVAS_CALLBACK_FOCUS_OUT,
   EVAS_CALLBACK_SHOW,
   EVAS_CALLBACK_HIDE,
   EVAS_CALLBACK_MOVE,
   EVAS_CALLBACK_RESIZE,
   EVAS_CALLBACK_RESTACK
} Evas_Callback_Type;

typedef struct _Evas_Rectangle              Evas_Rectangle;

typedef struct _Evas                        Evas;
typedef struct _Evas_Layer                  Evas_Layer;
typedef struct _Evas_Object                 Evas_Object;
typedef struct _Evas_Data_Node              Evas_Data_Node;
typedef struct _Evas_Func_Node              Evas_Func_Node;
typedef struct _Evas_Func                   Evas_Func;
typedef struct _Evas_Object_Func            Evas_Object_Func;
typedef struct _Evas_Modifier               Evas_Modifier;
typedef struct _Evas_Lock                   Evas_Lock;
typedef unsigned long long                  Evas_Modifier_Mask;
typedef struct _Evas_Smart                  Evas_Smart;
typedef void                                Evas_Performance;
typedef struct _Evas_Intercept_Func         Evas_Intercept_Func;
typedef struct _Evas_Intercept_Func_Basic   Evas_Intercept_Func_Basic;
typedef struct _Evas_Intercept_Func_SizePos Evas_Intercept_Func_SizePos;
typedef struct _Evas_Intercept_Func_Obj     Evas_Intercept_Func_Obj;
typedef struct _Evas_Intercept_Func_Int     Evas_Intercept_Func_Int;
typedef struct _Evas_Key_Grab               Evas_Key_Grab;
typedef struct _Evas_Callbacks              Evas_Callbacks;
typedef struct _Evas_Smart_Class            Evas_Smart_Class;
#if 1 /* able to change co-ordinate systems to remove all fp ops */
typedef double                              Evas_Coord;
typedef double                              Evas_Font_Size;
typedef double                              Evas_Angle;
#ifndef EVAS_COMMON_H
typedef int                                 Evas_Bool;
#endif
#else
typedef int                                 Evas_Coord;
typedef int                                 Evas_Font_Size;
typedef int                                 Evas_Angle;
#ifndef EVAS_COMMON_H
typedef char                                Evas_Bool;
#endif
#endif

#define MAGIC_EVAS          0x70777770
#define MAGIC_OBJ           0x71777770
#define MAGIC_OBJ_RECTANGLE 0x71777771
#define MAGIC_OBJ_LINE      0x71777772
#define MAGIC_OBJ_GRADIENT  0x71777773
#define MAGIC_OBJ_POLYGON   0x71777774
#define MAGIC_OBJ_IMAGE     0x71777775
#define MAGIC_OBJ_TEXT      0x71777776
#define MAGIC_OBJ_SMART     0x71777777
#define MAGIC_SMART         0x72777770

#ifdef MAGIC_DEBUG
#define MAGIC_CHECK_FAILED(o, t, m) \
{evas_debug_error(); \
 if (!o) evas_debug_input_null(); \
 else if (((t *)o)->magic == 0) evas_debug_magic_null(); \
 else evas_debug_magic_wrong((m), ((t *)o)->magic); \
}
#else
#define MAGIC_CHECK_FAILED(o, t, m)
#endif
#define MAGIC_CHECK(o, t, m) \
{if ((!o) || (!(((t *)o)->magic == (m)))) { \
MAGIC_CHECK_FAILED(o, t, m)
#define MAGIC_CHECK_END() \
}}

#define NEW_RECT(_r, _x, _y, _w, _h) \
{(_r) = malloc(sizeof(Evas_Rectangle)); \
if (_r) \
{ \
   (_r)->x = (_x); (_r)->y = (_y); \
   (_r)->w = (_w); (_r)->h = (_h); \
}}

#define MERR_NONE() _evas_alloc_error = EVAS_ALLOC_ERROR_NONE
#define MERR_FATAL() _evas_alloc_error = EVAS_ALLOC_ERROR_FATAL
#define MERR_BAD() _evas_alloc_error = EVAS_ALLOC_ERROR_RECOVERED

#define EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o)                              \
   if ((o)->cur.file)                                                       \
     {                                                                      \
         free((o)->cur.file);                                               \
	 if ((o)->prev.file == (o)->cur.file)                               \
	       (o)->prev.file = NULL;                                       \
	 (o)->cur.file = NULL;                                              \
     }                                                                      \
   if ((o)->cur.key)                                                        \
     {                                                                      \
         free((o)->cur.key);                                                \
	 if ((o)->prev.key == (o)->cur.key)                                 \
	       (o)->prev.key = NULL;                                        \
	 (o)->cur.key = NULL;                                               \
     }                                                                      \
   if ((o)->prev.file)                                                      \
     {                                                                      \
         free((o)->prev.file);                                              \
	 (o)->prev.file = NULL;                                             \
     }                                                                      \
   if ((o)->prev.key)                                                       \
     {                                                                      \
         free((o)->prev.key);                                               \
	 (o)->prev.key = NULL;                                              \
     }

struct _Evas_Rectangle
{
   int x, y, w, h;
};

struct _Evas_Intercept_Func_Basic
{
   void (*func) (void *data, Evas_Object *obj);
   void *data;
};

struct _Evas_Intercept_Func_SizePos
{
   void (*func) (void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y);
   void *data;
};

struct _Evas_Intercept_Func_Obj
{
   void (*func) (void *data, Evas_Object *obj, Evas_Object *obj2);
   void *data;
};

struct _Evas_Intercept_Func_Int
{
   void (*func) (void *data, Evas_Object *obj, int n);
   void *data;
};

struct _Evas_Key_Grab
{
   char               *keyname;
   Evas_Modifier_Mask  modifiers;
   Evas_Modifier_Mask  not_modifiers;
   Evas_Object        *object;
   char                exclusive : 1;
   char                just_added : 1;
   char                delete_me : 1;
};

struct _Evas_Intercept_Func
{
   Evas_Intercept_Func_Basic   show;
   Evas_Intercept_Func_Basic   hide;
   Evas_Intercept_Func_SizePos move;
   Evas_Intercept_Func_SizePos resize;
   Evas_Intercept_Func_Basic   raise;
   Evas_Intercept_Func_Basic   lower;
   Evas_Intercept_Func_Obj     stack_above;
   Evas_Intercept_Func_Obj     stack_below;
   Evas_Intercept_Func_Int     layer_set;
}; 

struct _Evas_Smart
{
   DATA32            magic;

   int               usage;
   
   char              delete_me : 1;
   char              class_allocated : 1;
   
   Evas_Smart_Class *smart_class;
};

struct _Evas_Smart_Class /** a smart object class */
{
   const char *name; /** the string name of the class */
   
   void  (*add)         (Evas_Object *o);
   void  (*del)         (Evas_Object *o);
   void  (*layer_set)   (Evas_Object *o, int l);
   void  (*raise)       (Evas_Object *o);
   void  (*lower)       (Evas_Object *o);
   void  (*stack_above) (Evas_Object *o, Evas_Object *above);
   void  (*stack_below) (Evas_Object *o, Evas_Object *below);
   void  (*move)        (Evas_Object *o, Evas_Coord x, Evas_Coord y);
   void  (*resize)      (Evas_Object *o, Evas_Coord w, Evas_Coord h);
   void  (*show)        (Evas_Object *o);
   void  (*hide)        (Evas_Object *o);
   void  (*color_set)   (Evas_Object *o, int r, int g, int b, int a);
   void  (*clip_set)    (Evas_Object *o, Evas_Object *clip);
   void  (*clip_unset)  (Evas_Object *o);
   
   const void *data;
};

struct _Evas_Modifier
{
   struct {
      int       count;
      char    **list;
   } mod;
   Evas_Modifier_Mask mask; /* ok we have a max of 64 modifiers */
};

struct _Evas_Lock
{
   struct {
      int       count;
      char    **list;
   } lock;
   Evas_Modifier_Mask mask; /* we have a max of 64 locks */
};

struct _Evas_Callbacks
{
   char              deletions_waiting : 1;
   int               walking_list;
   Evas_Object_List *down;
   Evas_Object_List *up;
   Evas_Object_List *move;
   Evas_Object_List *in;
   Evas_Object_List *out;
   Evas_Object_List *wheel;
   Evas_Object_List *key_down;
   Evas_Object_List *key_up;
   Evas_Object_List *free;
   Evas_Object_List *obj_focus_in;
   Evas_Object_List *obj_focus_out;
   Evas_Object_List *obj_show;
   Evas_Object_List *obj_hide;
   Evas_Object_List *obj_move;
   Evas_Object_List *obj_resize;
   Evas_Object_List *obj_restack;
};

struct _Evas
{
   Evas_Object_List  _list_data;
   
   DATA32            magic;
   
   struct {
      char           inside : 1;
      char           mouse_grabbed : 1;
      DATA32         button;
      int            x, y;

      Evas_Coord         canvas_x, canvas_y;
      
      struct {
	 Evas_List *in;
      } object;
      
   } pointer;
   
   struct  {
      Evas_Coord         x, y, w, h;
      char           changed : 1;
   } viewport;
   
   struct {
      int            w, h;
      DATA32         render_method;
      char           changed : 1;
   } output;
   
   int               output_validity;
   
   Evas_List        *damages;
   Evas_List        *obscures;
   
   Evas_Layer       *layers;
   
   Evas_Hash        *name_hash;
   
   char              changed : 1;
   char              walking_layers : 1;
   
   int               events_frozen;
   
   struct {
      Evas_Func *func;
      struct {
	 void *output;
	 
	 void *context;
      } data;
      
      void *info;
      int   info_magic;
   } engine;

   int            delete_grabs;
   int            walking_grabs;
   Evas_List     *grabs;
   
   Evas_List     *font_path;
   
   Evas_Object   *focused;
   Evas_Modifier  modifiers;
   Evas_Lock      locks;
};

struct _Evas_Layer
{
   Evas_Object_List  _list_data;
   
   int               layer;
   Evas_Object      *objects;
   
   Evas             *evas;

   void             *engine_data;
   char              delete_me : 1;
};

struct _Evas_Object
{
   Evas_Object_List  _list_data;
   
   DATA32            magic;
   
   const char       *type;   
   Evas_Layer       *layer;   
   
   struct {
      struct {
	 struct {
	    int            x, y, w, h;
	    int            validity;
	 } geometry;
	 struct {
	    int            x, y, w, h;
	    unsigned char  r, g, b, a;
	    char           visible : 1;
	 } clip;
      } cache;
      struct {
	 Evas_Coord         x, y, w, h;
      } geometry;
      struct {
	 unsigned char  r, g, b, a;
      } color;      
      char              visible : 1;      
      int               layer;      
      Evas_Object      *clipper;      
   } cur, prev;
   
   char                       *name;
   
   Evas_Intercept_Func *interceptors;
   
   struct {
      Evas_List *elements;
   } data;
   
   Evas_List *grabs;

   Evas_Callbacks *callbacks;
   
   struct {
      Evas_List   *clipees;
      Evas_List   *changes;
   } clip;

   Evas_Object_Func *func;

   void             *object_data;
   
   struct {
      int            walking_list;
      Evas_Smart    *smart;
      void          *data;
      Evas_Object   *parent;
      Evas_List     *contained;
      Evas_List     *callbacks;
      char           deletions_waiting : 1;
   } smart;
   
   short                       store : 1;
   short                       pass_events : 1;
   short                       repeat_events : 1;
   short                       restack : 1;
   short                       changed : 1;
   short                       mouse_in : 1;
   short                       mouse_grabbed : 1;
   short                       pre_render_done : 1;
   short                       intercepted : 1;
   short                       focused : 1;

   unsigned char               delete_me;   
};

struct _Evas_Func_Node
{
   Evas_Object_List  _list_data;   
   char delete_me : 1;
   void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info);
   void *data;
};

struct _Evas_Data_Node
{
   char *key;
   void *data;
};

struct _Evas_Object_Func
{
   void (*free) (Evas_Object *obj);
   void (*render) (Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
   void (*render_pre) (Evas_Object *obj);
   void (*render_post) (Evas_Object *obj);
   
   void (*store) (Evas_Object *obj);
   void (*unstore) (Evas_Object *obj);
   
   int  (*is_visible) (Evas_Object *obj);
   int  (*was_visible) (Evas_Object *obj);
   
   int  (*is_opaque) (Evas_Object *obj);
   int  (*was_opaque) (Evas_Object *obj);
   
   int  (*is_inside) (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
   int  (*was_inside) (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
   
   void (*coords_recalc) (Evas_Object *obj);
};

struct _Evas_Func
{
   void *(*info)                           (Evas *e);
   void (*info_free)                       (Evas *e, void *info);
   void (*setup)                           (Evas *e, void *info);
   
   void (*output_free)                     (void *data);
   void (*output_resize)                   (void *data, int w, int h);
   void (*output_tile_size_set)            (void *data, int w, int h);
   void (*output_redraws_rect_add)         (void *data, int x, int y, int w, int h);
   void (*output_redraws_rect_del)         (void *data, int x, int y, int w, int h);
   void (*output_redraws_clear)            (void *data);
   void *(*output_redraws_next_update_get) (void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
   void (*output_redraws_next_update_push) (void *data, void *surface, int x, int y, int w, int h);
   void (*output_flush)                    (void *data);
   
   void *(*context_new)                    (void *data);
   void (*context_free)                    (void *data, void *context);
   void (*context_clip_set)                (void *data, void *context, int x, int y, int w, int h);
   void (*context_clip_clip)               (void *data, void *context, int x, int y, int w, int h);
   void (*context_clip_unset)              (void *data, void *context);
   int  (*context_clip_get)                (void *data, void *context, int *x, int *y, int *w, int *h);
   void (*context_color_set)               (void *data, void *context, int r, int g, int b, int a);
   int  (*context_color_get)               (void *data, void *context, int *r, int *g, int *b, int *a);
   void (*context_multiplier_set)          (void *data, void *context, int r, int g, int b, int a);
   void (*context_multiplier_unset)        (void *data, void *context);
   int  (*context_multiplier_get)          (void *data, void *context, int *r, int *g, int *b, int *a);
   void (*context_cutout_add)              (void *data, void *context, int x, int y, int w, int h);
   void (*context_cutout_clear)            (void *data, void *context);
   
   void (*rectangle_draw)                  (void *data, void *context, void *surface, int x, int y, int w, int h);

   void (*line_draw)                       (void *data, void *context, void *surface, int x1, int y1, int x2, int y2);
   
   void *(*polygon_point_add)              (void *data, void *context, void *polygon, int x, int y);
   void *(*polygon_points_clear)           (void *data, void *context, void *polygon);
   void (*polygon_draw)                    (void *data, void *context, void *surface, void *polygon);
   
   void *(*gradient_color_add)             (void *data, void *context, void *gradient, int r, int g, int b, int a, int distance);
   void *(*gradient_colors_clear)          (void *data, void *context, void *gradient);

   void (*gradient_draw)                   (void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle);
   
   void *(*image_load)                     (void *data, char *file, char *key, int *error);
   void *(*image_new_from_data)            (void *data, int w, int h, DATA32 *image_data);
   void *(*image_new_from_copied_data)     (void *data, int w, int h, DATA32 *image_data);
   void (*image_free)                      (void *data, void *image);
   void (*image_size_get)                  (void *data, void *image, int *w, int *h);
   void *(*image_size_set)                 (void *data, void *image, int w, int h);
   void *(*image_dirty_region)             (void *data, void *image, int x, int y, int w, int h);
   void *(*image_data_get)                 (void *data, void *image, int to_write, DATA32 **image_data);
   void *(*image_data_put)                 (void *data, void *image, DATA32 *image_data);
   void *(*image_alpha_set)                (void *data, void *image, int has_alpha);
   int  (*image_alpha_get)                 (void *data, void *image);
   void (*image_draw)                      (void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth);
   char *(*image_comment_get)              (void *data, void *image, char *key);
   char *(*image_format_get)               (void *data, void *image);
   
   void (*image_cache_flush)               (void *data);
   void (*image_cache_set)                 (void *data, int bytes);
   int  (*image_cache_get)                 (void *data);
   
   void *(*font_load)                      (void *data, char *name, int size);
   void *(*font_memory_load)               (void *data, char *name, int size, const void *fdata, int fdata_size);
   void (*font_free)                       (void *data, void *font);
   int  (*font_ascent_get)                 (void *data, void *font);
   int  (*font_descent_get)                (void *data, void *font);
   int  (*font_max_ascent_get)             (void *data, void *font);
   int  (*font_max_descent_get)            (void *data, void *font);
   void (*font_string_size_get)            (void *data, void *font, char *text, int *w, int *h);
   int  (*font_inset_get)                  (void *data, void *font, char *text);
   int  (*font_h_advance_get)              (void *data, void *font, char *text);
   int  (*font_v_advance_get)              (void *data, void *font, char *text);
   int  (*font_char_coords_get)            (void *data, void *font, char *text, int pos, int *cx, int *cy, int *cw, int *ch);
   int  (*font_char_at_coords_get)         (void *data, void *font, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch);
   void (*font_draw)                       (void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, char *text);
     
   void (*font_cache_flush)                (void *data);
   void (*font_cache_set)                  (void *data, int bytes);
   int  (*font_cache_get)                  (void *data);
   
   /* Engine functions will over time expand from here */
};

#ifdef __cplusplus
extern "C" {
#endif
   
Evas_Object *evas_object_new(void);
void evas_object_free(Evas_Object *obj, int clean_layer);
void evas_object_inject(Evas_Object *obj, Evas *e);
void evas_object_release(Evas_Object *obj, int clean_layer);
void evas_object_change(Evas_Object *obj);
Evas_List *evas_object_render_pre_visible_change(Evas_List *updates, Evas_Object *obj, int is_v, int was_v);
Evas_List *evas_object_render_pre_clipper_change(Evas_List *updates, Evas_Object *obj);
Evas_List *evas_object_render_pre_prev_cur_add(Evas_List *updates, Evas_Object *obj);
void evas_object_render_pre_effect_updates(Evas_List *updates, Evas_Object *obj, int is_v, int was_v);
Evas_List * evas_rects_return_difference_rects(int x, int y, int w, int h, int xx, int yy, int ww, int hh);
void evas_object_clip_recalc(Evas_Object *obj);
Evas_Layer *evas_layer_new(Evas *e);
void evas_layer_pre_free(Evas_Layer *lay);
void evas_layer_free(Evas_Layer *lay);
Evas_Layer *evas_layer_find(Evas *e, int layer_num);
void evas_layer_add(Evas_Layer *lay);
void evas_layer_del(Evas_Layer *lay);
void evas_object_coords_recalc(Evas_Object *obj);
int evas_object_is_active(Evas_Object *obj);
int evas_object_is_in_output_rect(Evas_Object *obj, int x, int y, int w, int h);
int evas_object_was_in_output_rect(Evas_Object *obj, int x, int y, int w, int h);
int evas_object_is_visible(Evas_Object *obj);
int evas_object_was_visible(Evas_Object *obj);
int evas_object_is_opaque(Evas_Object *obj);
int evas_object_was_opaque(Evas_Object *obj);
void evas_object_recalc_clippees(Evas_Object *obj);
int evas_object_clippers_is_visible(Evas_Object *obj);
int evas_object_clippers_was_visible(Evas_Object *obj);
void evas_object_event_callback_call(Evas_Object *obj, Evas_Callback_Type type, void *event_info);
Evas_List *evas_event_objects_event_list(Evas *e, Evas_Object *stop, int x, int y);
int evas_file_path_is_full_path(char *path);
char *evas_file_path_join(char *path, char *end);
int evas_file_path_exists(char *path);
int evas_file_path_is_file(char *path);
int evas_file_path_is_dir(char *path);
Evas_List *evas_file_path_list(char *path, char *match, int match_case);
DATA64 evas_file_modified_time(const char *file);
char *evas_file_path_resolve(const char *file);
int evas_mem_free(int mem_required);
int evas_mem_degrade(int mem_required);
void evas_debug_error(void);
void evas_debug_input_null(void);
void evas_debug_magic_null(void);
void evas_debug_magic_wrong(DATA32 expected, DATA32 supplied);
void evas_debug_generic(const char *str);
char *evas_debug_magic_string_get(DATA32 magic);
void evas_object_smart_use(Evas_Smart *s);
void evas_object_smart_unuse(Evas_Smart *s);   
void evas_object_smart_del(Evas_Object *obj);
void evas_object_smart_cleanup(Evas_Object *obj);
void *evas_mem_calloc(int size);
void evas_object_event_callback_cleanup(Evas_Object *obj);       
void evas_object_inform_call_show(Evas_Object *obj);
void evas_object_inform_call_hide(Evas_Object *obj);
void evas_object_inform_call_move(Evas_Object *obj);
void evas_object_inform_call_resize(Evas_Object *obj);
void evas_object_inform_call_restack(Evas_Object *obj);
void evas_object_intercept_cleanup(Evas_Object *obj);
int evas_object_intercept_call_show(Evas_Object *obj);
int evas_object_intercept_call_hide(Evas_Object *obj);
int evas_object_intercept_call_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
int evas_object_intercept_call_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
int evas_object_intercept_call_raise(Evas_Object *obj);
int evas_object_intercept_call_lower(Evas_Object *obj);
int evas_object_intercept_call_stack_above(Evas_Object *obj, Evas_Object *above);
int evas_object_intercept_call_stack_below(Evas_Object *obj, Evas_Object *below);
int evas_object_intercept_call_layer_set(Evas_Object *obj, int l);
void evas_object_grabs_cleanup(Evas_Object *obj);
void evas_key_grab_free(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers);
   
extern int _evas_alloc_error;

typedef struct _Evas_Imaging_Image Evas_Imaging_Image;
typedef struct _Evas_Imaging_Font Evas_Imaging_Font;

struct _Evas_Imaging_Image
{
   RGBA_Image *image;
};

struct _Evas_Imaging_font
{
   RGBA_Font *font;
};
   
#ifdef __cplusplus
}
#endif
