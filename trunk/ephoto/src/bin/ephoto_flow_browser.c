#include "ephoto.h"

#ifdef HAVE_LIBEXIF
#include <libexif/exif-data.h>
#endif

#define ZOOM_MIN 0.1
#define ZOOM_MAX 10.0
#define ZOOM_STEP 0.2

typedef struct _Ephoto_Flow_Browser Ephoto_Flow_Browser;
typedef struct _Ephoto_Viewer Ephoto_Viewer;

struct _Ephoto_Flow_Browser
{
   Ephoto *ephoto;
   Evas_Object *layout;
   Evas_Object *edje;
   Evas_Object *orient_layout;
   Evas_Object *orient_edje;
   Evas_Object *viewer;
   Evas_Object *toolbar;
   struct {
      Elm_Toolbar_Item *zoom_in;
      Elm_Toolbar_Item *zoom_out;
      Elm_Toolbar_Item *zoom_1;
      Elm_Toolbar_Item *go_first;
      Elm_Toolbar_Item *go_prev;
      Elm_Toolbar_Item *go_next;
      Elm_Toolbar_Item *go_last;
      Elm_Toolbar_Item *rotate_counterclock;
      Elm_Toolbar_Item *rotate_clock;
      Elm_Toolbar_Item *flip_horiz;
      Elm_Toolbar_Item *flip_vert;
      Elm_Toolbar_Item *slideshow;
   } action;
   const char *path;
   Ephoto_Entry *entry;
   Ephoto_Orient orient;
   double zoom;
};

struct _Ephoto_Viewer
{
   Evas_Object *photocam;
   Evas_Object *scroller;
   Evas_Object *image;
};

static void _zoom_set(Ephoto_Flow_Browser *fb, double zoom);

static Eina_Bool
_path_is_jpeg(const char *path_stringshared)
{
   size_t len = eina_stringshare_strlen(path_stringshared);
   const char *ext;

   if (len < sizeof(".jpg")) return EINA_FALSE;
   ext = path_stringshared + len - (sizeof(".jpg") - 1);
   if (strcasecmp(ext, ".jpg") == 0) return EINA_TRUE;

   if (len < sizeof(".jpeg")) return EINA_FALSE;
   ext = path_stringshared + len - (sizeof(".jpeg") - 1);
   if (strcasecmp(ext, ".jpeg") == 0) return EINA_TRUE;

   return EINA_FALSE;
}

static void
_viewer_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Viewer *v = data;
   free(v);
}

static Evas_Object *
_viewer_add(Evas_Object *parent, const char *path)
{
   Ephoto_Viewer *v = calloc(1, sizeof(Ephoto_Viewer));
   Evas_Object *obj;
   int err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(v, NULL);
   if (_path_is_jpeg(path))
     {
        obj = v->photocam = elm_photocam_add(parent);
        EINA_SAFETY_ON_NULL_GOTO(obj, error);
        err = elm_photocam_file_set(obj, path);
        if (err != EVAS_LOAD_ERROR_NONE) goto load_error;
     }
   else
     {
        Evas_Coord w, h;
        obj = v->scroller = elm_scroller_add(parent);
        EINA_SAFETY_ON_NULL_GOTO(obj, error);
        v->image = evas_object_image_filled_add(evas_object_evas_get(parent));
        evas_object_image_file_set(v->image, path, NULL);
        err = evas_object_image_load_error_get(v->image);
        if (err != EVAS_LOAD_ERROR_NONE) goto load_error;
        evas_object_image_size_get(v->image, &w, &h);
        evas_object_size_hint_align_set(v->image, 0.5, 0.5);
        evas_object_size_hint_min_set(v->image, w, h);
        evas_object_size_hint_max_set(v->image, w, h);
        evas_object_resize(v->image, w, h);
        evas_object_show(v->image);
        elm_scroller_content_set(obj, v->image);
     }

   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_data_set(obj, "viewer", v);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _viewer_del, v);
   return obj;

 load_error:
   ERR("could not load image '%s': %s", path, evas_load_error_str(err));
   evas_object_del(obj);
 error:
   free(v);
   return NULL;
}

static void
_viewer_zoom_set(Evas_Object *obj, float zoom)
{
   Ephoto_Viewer *v = evas_object_data_get(obj, "viewer");
   EINA_SAFETY_ON_NULL_RETURN(v);

   if (v->photocam) elm_photocam_zoom_set(v->photocam, 1.0 / zoom);
   else
     {
        Evas_Coord w, h;
        evas_object_image_size_get(v->image, &w, &h);
        w *= zoom;
        h *= zoom;
        evas_object_size_hint_min_set(v->image, w, h);
        evas_object_size_hint_max_set(v->image, w, h);
     }
}

static void
_orient_apply(Ephoto_Flow_Browser *fb)
{
   const char *sig;
   switch (fb->orient)
     {
      case EPHOTO_ORIENT_0:
         sig = "state,rotate,0";
         break;
      case EPHOTO_ORIENT_90:
         sig = "state,rotate,90";
         break;
      case EPHOTO_ORIENT_180:
         sig = "state,rotate,180";
         break;
      case EPHOTO_ORIENT_270:
         sig = "state,rotate,270";
         break;
      case EPHOTO_ORIENT_FLIP_HORIZ:
         sig = "state,flip,horiz";
         break;
      case EPHOTO_ORIENT_FLIP_VERT:
         sig = "state,flip,vert";
         break;
      case EPHOTO_ORIENT_FLIP_HORIZ_90:
         sig = "state,flip,horiz,90";
         break;
      case EPHOTO_ORIENT_FLIP_VERT_90:
         sig = "state,flip,vert,90";
         break;
      default:
         return;
     }
   DBG("orient: %d, signal '%s'", fb->orient, sig);
   edje_object_signal_emit(fb->orient_edje, sig, "ephoto");
}

static void
_rotate_counterclock(Ephoto_Flow_Browser *fb)
{
   switch (fb->orient)
     {
      case EPHOTO_ORIENT_0:
         fb->orient = EPHOTO_ORIENT_270;
         break;
      case EPHOTO_ORIENT_90:
         fb->orient = EPHOTO_ORIENT_0;
         break;
      case EPHOTO_ORIENT_180:
         fb->orient = EPHOTO_ORIENT_90;
         break;
      case EPHOTO_ORIENT_270:
         fb->orient = EPHOTO_ORIENT_180;
         break;
      case EPHOTO_ORIENT_FLIP_HORIZ:
         fb->orient = EPHOTO_ORIENT_FLIP_HORIZ_90;
         break;
      case EPHOTO_ORIENT_FLIP_VERT:
         fb->orient = EPHOTO_ORIENT_FLIP_VERT_90;
         break;
      case EPHOTO_ORIENT_FLIP_HORIZ_90:
         fb->orient = EPHOTO_ORIENT_FLIP_VERT;
         break;
      case EPHOTO_ORIENT_FLIP_VERT_90:
         fb->orient = EPHOTO_ORIENT_FLIP_HORIZ;
         break;
     }
   _orient_apply(fb);
}

static void
_rotate_clock(Ephoto_Flow_Browser *fb)
{
   switch (fb->orient)
     {
      case EPHOTO_ORIENT_0:
         fb->orient = EPHOTO_ORIENT_90;
         break;
      case EPHOTO_ORIENT_90:
         fb->orient = EPHOTO_ORIENT_180;
         break;
      case EPHOTO_ORIENT_180:
         fb->orient = EPHOTO_ORIENT_270;
         break;
      case EPHOTO_ORIENT_270:
         fb->orient = EPHOTO_ORIENT_0;
         break;
      case EPHOTO_ORIENT_FLIP_HORIZ:
         fb->orient = EPHOTO_ORIENT_FLIP_VERT_90;
         break;
      case EPHOTO_ORIENT_FLIP_VERT:
         fb->orient = EPHOTO_ORIENT_FLIP_HORIZ_90;
         break;
      case EPHOTO_ORIENT_FLIP_HORIZ_90:
         fb->orient = EPHOTO_ORIENT_FLIP_HORIZ;
         break;
      case EPHOTO_ORIENT_FLIP_VERT_90:
         fb->orient = EPHOTO_ORIENT_FLIP_VERT;
         break;
     }
   _orient_apply(fb);
}

static void
_flip_horiz(Ephoto_Flow_Browser *fb)
{
   switch (fb->orient)
     {
      case EPHOTO_ORIENT_0:
         fb->orient = EPHOTO_ORIENT_FLIP_HORIZ;
         break;
      case EPHOTO_ORIENT_90:
         fb->orient = EPHOTO_ORIENT_FLIP_HORIZ_90;
         break;
      case EPHOTO_ORIENT_180:
         fb->orient = EPHOTO_ORIENT_FLIP_VERT;
         break;
      case EPHOTO_ORIENT_270:
         fb->orient = EPHOTO_ORIENT_FLIP_VERT_90;
         break;
      case EPHOTO_ORIENT_FLIP_HORIZ:
         fb->orient = EPHOTO_ORIENT_0;
         break;
      case EPHOTO_ORIENT_FLIP_VERT:
         fb->orient = EPHOTO_ORIENT_180;
         break;
      case EPHOTO_ORIENT_FLIP_HORIZ_90:
         fb->orient = EPHOTO_ORIENT_90;
         break;
      case EPHOTO_ORIENT_FLIP_VERT_90:
         fb->orient = EPHOTO_ORIENT_270;
         break;
     }
   _orient_apply(fb);
}

static void
_flip_vert(Ephoto_Flow_Browser *fb)
{
   switch (fb->orient)
     {
      case EPHOTO_ORIENT_0:
         fb->orient = EPHOTO_ORIENT_FLIP_VERT;
         break;
      case EPHOTO_ORIENT_90:
         fb->orient = EPHOTO_ORIENT_FLIP_VERT_90;
         break;
      case EPHOTO_ORIENT_180:
         fb->orient = EPHOTO_ORIENT_FLIP_HORIZ;
         break;
      case EPHOTO_ORIENT_270:
         fb->orient = EPHOTO_ORIENT_FLIP_HORIZ_90;
         break;
      case EPHOTO_ORIENT_FLIP_HORIZ:
         fb->orient = EPHOTO_ORIENT_180;
         break;
      case EPHOTO_ORIENT_FLIP_VERT:
         fb->orient = EPHOTO_ORIENT_0;
         break;
      case EPHOTO_ORIENT_FLIP_HORIZ_90:
         fb->orient = EPHOTO_ORIENT_270;
         break;
      case EPHOTO_ORIENT_FLIP_VERT_90:
         fb->orient = EPHOTO_ORIENT_90;
         break;
     }
   _orient_apply(fb);
}

static void
_mouse_wheel(void *data, Evas *e __UNUSED__, Evas_Object *o __UNUSED__, void *event_info)
{
   Ephoto_Flow_Browser *fb = data;
   Evas_Event_Mouse_Wheel *ev = event_info;
   if (!evas_key_modifier_is_set(ev->modifiers, "Control")) return;

   if (ev->z > 0) _zoom_set(fb, fb->zoom + ZOOM_STEP);
   else _zoom_set(fb, fb->zoom - ZOOM_STEP);
}

static Ephoto_Entry *
_first_entry_find(Ephoto_Flow_Browser *fb)
{
   const Eina_List *l;
   Ephoto_Entry *entry;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb->ephoto, NULL);

   EINA_LIST_FOREACH(fb->ephoto->entries, l, entry)
     if (!entry->is_dir) return entry;
   return NULL;
}

static Ephoto_Entry *
_last_entry_find(Ephoto_Flow_Browser *fb)
{
   const Eina_List *l;
   Ephoto_Entry *entry;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb->ephoto, NULL);

   EINA_LIST_REVERSE_FOREACH(fb->ephoto->entries, l, entry)
     if (!entry->is_dir) return entry;
   return NULL;
}

static void
_ephoto_flow_browser_toolbar_eval(Ephoto_Flow_Browser *fb)
{
   if (!fb->entry)
     {
        elm_toolbar_item_disabled_set(fb->action.go_first, EINA_TRUE);
        elm_toolbar_item_disabled_set(fb->action.go_prev, EINA_TRUE);
        elm_toolbar_item_disabled_set(fb->action.go_next, EINA_TRUE);
        elm_toolbar_item_disabled_set(fb->action.go_last, EINA_TRUE);
        elm_toolbar_item_disabled_set(fb->action.slideshow, EINA_TRUE);
     }
   else
     {
        Eina_Bool is_first = fb->entry == _first_entry_find(fb);
        Eina_Bool is_last = fb->entry == _last_entry_find(fb);

        elm_toolbar_item_disabled_set(fb->action.go_first, is_first);
        elm_toolbar_item_disabled_set(fb->action.go_prev, is_first);
        elm_toolbar_item_disabled_set(fb->action.go_next, is_last);
        elm_toolbar_item_disabled_set(fb->action.go_last, is_last);
        elm_toolbar_item_disabled_set(fb->action.slideshow, EINA_FALSE);
     }
}

Ephoto_Orient
ephoto_file_orient_get(const char *path)
{
#ifndef HAVE_LIBEXIF
   return EPHOTO_ORIENT_0;
#else
   Ephoto_Orient orient = EPHOTO_ORIENT_0;
   ExifData *exif;
   ExifEntry *entry;
   ExifByteOrder bo;

   if (!_path_is_jpeg(path)) return orient;

   exif = exif_data_new_from_file(path);
   if (!exif) goto end;
   bo = exif_data_get_byte_order(exif);
   entry = exif_data_get_entry(exif, EXIF_TAG_ORIENTATION);
   if (!entry) goto end_entry;

   orient = exif_get_short(entry->data, bo);
   DBG("orient=%d", orient);
   if ((orient < 1) || (orient > 8))
     {
        ERR("exif orient not supported: %d", orient);
        orient = EPHOTO_ORIENT_0;
     }

 end_entry:
   exif_data_free(exif);
 end:
   return orient;
#endif
}

static void
_ephoto_flow_browser_recalc(Ephoto_Flow_Browser *fb)
{
   if (fb->viewer)
     {
        evas_object_del(fb->viewer);
        fb->viewer = NULL;
     }

   if (fb->path)
     {
        const char *bname = ecore_file_file_get(fb->path);
        fb->viewer = _viewer_add(fb->orient_layout, fb->path);
        elm_layout_content_set
          (fb->orient_layout, "elm.swallow.content", fb->viewer);
        evas_object_show(fb->viewer);
        evas_object_event_callback_add
          (fb->viewer, EVAS_CALLBACK_MOUSE_WHEEL, _mouse_wheel, fb);
        edje_object_part_text_set(fb->edje, "elm.text.title", bname);
        ephoto_title_set(fb->ephoto, bname);
        fb->orient = ephoto_file_orient_get(fb->path);
        _orient_apply(fb);
     }

   _ephoto_flow_browser_toolbar_eval(fb);
}

static void
_zoom_set(Ephoto_Flow_Browser *fb, double zoom)
{
   if (zoom > ZOOM_MAX) zoom = ZOOM_MAX;
   else if (zoom < ZOOM_MIN) zoom = ZOOM_MIN;

   DBG("zoom %f", zoom);
   _viewer_zoom_set(fb->viewer, zoom);
   fb->zoom = zoom;

   elm_toolbar_item_disabled_set(fb->action.zoom_out, zoom <= ZOOM_MIN);
   elm_toolbar_item_disabled_set(fb->action.zoom_in, zoom >= ZOOM_MAX);
}

static void
_zoom_in(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.zoom_in);
   _zoom_set(fb, fb->zoom + ZOOM_STEP);
}

static void
_zoom_out(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.zoom_out);
   _zoom_set(fb, fb->zoom - ZOOM_STEP);
}

static void
_zoom_1(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.zoom_1);
   _zoom_set(fb, 1.0);
}

static void
_next_entry(Ephoto_Flow_Browser *fb)
{
   Elm_Gengrid_Item *it;
   Ephoto_Entry *entry;
   EINA_SAFETY_ON_NULL_RETURN(fb->entry);
   EINA_SAFETY_ON_NULL_RETURN(fb->entry->item);

   it = fb->entry->item;
   while ((it = elm_gengrid_item_next_get(it)))
     {
        entry = elm_gengrid_item_data_get(it);
        if (!entry->is_dir) break;
     }
   if (!it) return;
   DBG("next is '%s'", entry->path);
   ephoto_flow_browser_entry_set(fb->layout, entry);
}

static void
_prev_entry(Ephoto_Flow_Browser *fb)
{
   Elm_Gengrid_Item *it;
   Ephoto_Entry *entry;
   EINA_SAFETY_ON_NULL_RETURN(fb->entry);
   EINA_SAFETY_ON_NULL_RETURN(fb->entry->item);

   it = fb->entry->item;
   while ((it = elm_gengrid_item_prev_get(it)))
     {
        entry = elm_gengrid_item_data_get(it);
        if (!entry->is_dir) break;
     }
   if (!it) return;
   DBG("prev is '%s'", entry->path);
   ephoto_flow_browser_entry_set(fb->layout, entry);
}

static void
_first_entry(Ephoto_Flow_Browser *fb)
{
   Ephoto_Entry *entry = _first_entry_find(fb);
   if (!entry) return;
   DBG("first is '%s'", entry->path);
   ephoto_flow_browser_entry_set(fb->layout, entry);
}

static void
_last_entry(Ephoto_Flow_Browser *fb)
{
   Ephoto_Entry *entry = _last_entry_find(fb);
   if (!entry) return;
   DBG("last is '%s'", entry->path);
   ephoto_flow_browser_entry_set(fb->layout, entry);
}

static void
_go_first(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.go_first);
   _first_entry(fb);
}

static void
_go_prev(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.go_prev);
   _prev_entry(fb);
}

static void
_go_next(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.go_next);
   _next_entry(fb);
}

static void
_go_last(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.go_last);
   _last_entry(fb);
}

static void
_go_rotate_counterclock(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.rotate_counterclock);
   _rotate_counterclock(fb);
}

static void
_go_rotate_clock(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.rotate_clock);
   _rotate_clock(fb);
}

static void
_go_flip_horiz(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.flip_horiz);
   _flip_horiz(fb);
}

static void
_go_flip_vert(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.flip_vert);
   _flip_vert(fb);
}

static void
_slideshow(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   elm_toolbar_item_unselect(fb->action.slideshow);
   if (fb->entry)
     evas_object_smart_callback_call(fb->layout, "slideshow", fb->entry);
}

static void
_back(void *data, Evas_Object *o __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   evas_object_smart_callback_call(fb->layout, "back", fb->entry);
}

static void
_key_down(void *data, Evas *e __UNUSED__, Evas_Object *o __UNUSED__, void *event_info)
{
   Ephoto_Flow_Browser *fb = data;
   Evas_Event_Key_Down *ev = event_info;
   Eina_Bool ctrl = evas_key_modifier_is_set(ev->modifiers, "Control");
   Eina_Bool shift = evas_key_modifier_is_set(ev->modifiers, "Shift");
   const char *k = ev->keyname;

   if (ctrl)
     {
        if ((!strcmp(k, "plus")) || (!strcmp(k, "equal")))
          _zoom_set(fb, fb->zoom + ZOOM_STEP);
        else if (!strcmp(k, "minus"))
          _zoom_set(fb, fb->zoom - ZOOM_STEP);
        else if (!strcmp(k, "0"))
          _zoom_set(fb, 1.0);

        return;
     }

   if (!strcmp(k, "Escape"))
     evas_object_smart_callback_call(fb->layout, "back", fb->entry);
   else if (!strcmp(k, "Left"))
     _prev_entry(fb);
   else if (!strcmp(k, "Right"))
     _next_entry(fb);
   else if (!strcmp(k, "Home"))
     _first_entry(fb);
   else if (!strcmp(k, "End"))
     _last_entry(fb);
   else if (!strcmp(k, "bracketleft"))
     {
        if (!shift) _rotate_counterclock(fb);
        else        _flip_horiz(fb);
     }
   else if (!strcmp(k, "bracketright"))
     {
        if (!shift) _rotate_clock(fb);
        else        _flip_vert(fb);
     }
   else if (!strcmp(k, "F5"))
     {
        if (fb->entry)
          evas_object_smart_callback_call(fb->layout, "slideshow", fb->entry);
     }
}

static void
_entry_free(void *data, const Ephoto_Entry *entry __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   fb->entry = NULL;
}

static void
_layout_del(void *data, Evas *e __UNUSED__, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Ephoto_Flow_Browser *fb = data;
   if (fb->entry)
     ephoto_entry_free_listener_del(fb->entry, _entry_free, fb);
   eina_stringshare_del(fb->path);
   free(fb);
}

static Elm_Toolbar_Item *
_toolbar_item_add(Ephoto_Flow_Browser *fb, const char *icon, const char *label, int priority, Evas_Smart_Cb cb)
{
   Elm_Toolbar_Item *item = elm_toolbar_item_add(fb->toolbar, icon, label, cb, fb);
   elm_toolbar_item_priority_set(item, priority);
   return item;
}

static Elm_Toolbar_Item *
_toolbar_item_separator_add(Ephoto_Flow_Browser *fb)
{
   Elm_Toolbar_Item *it = elm_toolbar_item_add
     (fb->toolbar, NULL, NULL, NULL, NULL);
   elm_toolbar_item_separator_set(it, EINA_TRUE);
   return it;
}

Evas_Object *
ephoto_flow_browser_add(Ephoto *ephoto, Evas_Object *parent)
{
   Evas_Object *layout = elm_layout_add(parent);
   Ephoto_Flow_Browser *fb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(layout, NULL);

   fb = calloc(1, sizeof(Ephoto_Flow_Browser));
   EINA_SAFETY_ON_NULL_GOTO(fb, error);
   fb->ephoto = ephoto;
   fb->layout = layout;
   fb->edje = elm_layout_edje_get(layout);
   fb->zoom = 1.0;
   evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _layout_del, fb);
   evas_object_event_callback_add
     (layout, EVAS_CALLBACK_KEY_DOWN, _key_down, fb);
   evas_object_data_set(layout, "flow_browser", fb);

   edje_object_signal_callback_add(fb->edje, "elm,action,back", "", _back, fb);

   if (!elm_layout_theme_set
       (layout, "layout", "application", "toolbar-content-back"))
     {
        ERR("could not load style 'toolbar-content-back' from theme");
        goto error;
     }

   fb->toolbar = edje_object_part_external_object_get
     (fb->edje, "elm.external.toolbar");
   if (!fb->toolbar)
     {
        ERR("no toolbar in layout!");
        goto error;
     }
   elm_toolbar_homogenous_set(fb->toolbar, EINA_FALSE);
   elm_toolbar_mode_shrink_set(fb->toolbar, ELM_TOOLBAR_SHRINK_MENU);
   elm_toolbar_menu_parent_set(fb->toolbar, parent);

   fb->action.slideshow = _toolbar_item_add
     (fb, "media-playback-start", "Slideshow", 150, _slideshow);

   fb->action.zoom_in = _toolbar_item_add
     (fb, "zoom-in", "Zoom In", 100, _zoom_in);
   fb->action.zoom_out = _toolbar_item_add
     (fb, "zoom-out", "Zoom Out", 80, _zoom_out);
   fb->action.zoom_1 = _toolbar_item_add
     (fb, "zoom-original", "Zoom 1:1", 50, _zoom_1);

   _toolbar_item_separator_add(fb);

   fb->action.go_first = _toolbar_item_add(fb, "go-first", "First", 50, _go_first);
   fb->action.go_prev = _toolbar_item_add
     (fb, "go-previous", "Previous", 100, _go_prev);
   fb->action.go_next = _toolbar_item_add(fb, "go-next", "Next", 50, _go_next);
   fb->action.go_last = _toolbar_item_add(fb, "go-last", "Last", 50, _go_last);

   _toolbar_item_separator_add(fb);

   fb->action.rotate_counterclock = _toolbar_item_add
     (fb, "object-rotate-left", "Rotate Left", 50, _go_rotate_counterclock);
   fb->action.rotate_clock = _toolbar_item_add
     (fb, "object-rotate-right", "Rotate Right", 30, _go_rotate_clock);
   fb->action.flip_horiz = _toolbar_item_add
     (fb, "object-flip-horizontal", "Flip Horiz.", 30, _go_flip_horiz);
   fb->action.flip_vert = _toolbar_item_add
     (fb, "object-flip-vertical", "Flip Vert.", 30, _go_flip_vert);

   elm_toolbar_item_tooltip_text_set
     (fb->action.rotate_counterclock,
      "Rotate object to the left (counter-clockwise)");
   elm_toolbar_item_tooltip_text_set
     (fb->action.rotate_clock, "Rotate object to the right (clockwise)");

   elm_toolbar_item_tooltip_text_set
     (fb->action.flip_horiz, "Flip object horizontally");
   elm_toolbar_item_tooltip_text_set
     (fb->action.flip_vert, "Flip object vertically");

   fb->orient_layout = elm_layout_add(layout);
   if (!elm_layout_theme_set
       (fb->orient_layout, "layout", "ephoto", "orient"))
     {
        ERR("could not load style 'ephoto/orient' from theme");
        goto error;
     }
   fb->orient_edje = elm_layout_edje_get(fb->orient_layout);
   elm_layout_content_set(fb->layout, "elm.swallow.content", fb->orient_layout);
   elm_object_focus_custom_chain_append(fb->layout, fb->orient_layout, NULL);

   _ephoto_flow_browser_toolbar_eval(fb);

   return layout;

 error:
   evas_object_del(layout);
   return NULL;
}

void
ephoto_flow_browser_path_set(Evas_Object *obj, const char *path)
{
   Ephoto_Flow_Browser *fb = evas_object_data_get(obj, "flow_browser");
   EINA_SAFETY_ON_NULL_RETURN(fb);

   DBG("path '%s', was '%s'", path ? path : "", fb->path ? fb->path : "");
   if (!eina_stringshare_replace(&fb->path, path)) return;
   fb->entry = NULL;
   fb->zoom = 1.0;
   _ephoto_flow_browser_recalc(fb);
}

void
ephoto_flow_browser_entry_set(Evas_Object *obj, Ephoto_Entry *entry)
{
   Ephoto_Flow_Browser *fb = evas_object_data_get(obj, "flow_browser");
   EINA_SAFETY_ON_NULL_RETURN(fb);

   DBG("entry %p, was %p", entry, fb->entry);

   if (fb->entry)
     ephoto_entry_free_listener_del(fb->entry, _entry_free, fb);

   fb->entry = entry;

   if (entry)
     ephoto_entry_free_listener_add(entry, _entry_free, fb);


   if (!entry)
     {
        eina_stringshare_replace(&fb->path, NULL);
        _ephoto_flow_browser_toolbar_eval(fb);
     }
   else if (!eina_stringshare_replace(&fb->path, entry->path))
     _ephoto_flow_browser_toolbar_eval(fb);
   else
     {
        fb->zoom = 1.0;
        _ephoto_flow_browser_recalc(fb);
     }
}
