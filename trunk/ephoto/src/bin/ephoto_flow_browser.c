#include "ephoto.h"

#ifdef HAVE_LIBEXIF
  #include <libexif/exif-data.h>
#endif


/*Callbacks*/
static void _ephoto_go_back(void *data, Evas_Object *obj, void *event_info);
static void _ephoto_go_first(void *data, Evas_Object *obj, void *event_info);
static void _ephoto_go_last(void *data, Evas_Object *obj, void *event_info);
static void _ephoto_go_next(void *data, Evas_Object *obj, void *event_info);
static void _ephoto_go_previous(void *data, Evas_Object *obj, void *event_info);
static void _ephoto_go_slideshow(void *data, Evas_Object *obj, void *event_info);
static void _ephoto_go_rotate_counterclockwise(void *data, Evas_Object *obj, void *event_info);
static void _ephoto_go_rotate_clockwise(void *data, Evas_Object *obj, void *event_info);
static void _ephoto_go_editor(void *data, Evas_Object *obj, void *event_info);
static void _ephoto_key_pressed(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _ephoto_mouse_wheel(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _ephoto_flow_browser_show_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _ephoto_flow_browser_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _ephoto_zoom_in(void *data, Evas_Object *obj, void *event_info);
static void _ephoto_zoom_out(void *data, Evas_Object *obj, void *event_info);
static void _ephoto_zoom_regular_size(void *data, Evas_Object *obj, void *event_info);

typedef struct _Ephoto_Flow_Browser Ephoto_Flow_Browser;

struct _Ephoto_Flow_Browser
{
        Evas_Object *flow_browser;
        Eina_List *iter;
        Evas_Object *image;
        Evas_Object *image2;
        Evas_Object *toolbar;
        const char *cur_image;

        Ephoto_Orient orient;
};

static const char *toolbar_items[] = {
	"First",
	"Previous",
	"Next",
	"Last",
	"Slideshow",
	"Rotate CW",
	"Rotate CCW",
	"Zoom In",
	"Zoom Out"
};

static void
_ephoto_set_title(const char *file)
{
	char *buffer;
	int length;

	length = strlen(file) + strlen("Ephoto - ") + 1;
	buffer = alloca(length);
	snprintf(buffer, length, "Ephoto - %s", file);
	elm_win_title_set(em->win, buffer);
}

static void
_photocal_loaded_cb(void *data, Evas_Object *obj, void *event)
{
  Ephoto_Flow_Browser *ef = data;
  printf("Load\n");
  elm_photocam_paused_set(ef->image, EINA_FALSE);
}

static void
_ephoto_go_update(Ephoto_Flow_Browser *ef)
{
	const char *file_type;
        int success = 0;

	elm_layout_content_unset(ef->flow_browser, "ephoto.flow.swallow");

	evas_object_hide(ef->image);
        evas_object_hide(ef->image2);

	file_type = efreet_mime_type_get(ef->cur_image);
	if (file_type && !strcmp(file_type, "image/jpeg"))
	{
		success = elm_photocam_file_set(ef->image, ef->cur_image) == EVAS_LOAD_ERROR_NONE;
		elm_photocam_zoom_mode_set(ef->image, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);     
		evas_object_smart_callback_add(ef->image, "loaded", _photocal_loaded_cb, ef);
	    elm_photocam_paused_set(ef->image, EINA_TRUE);
		elm_layout_content_set(ef->flow_browser, "ephoto.flow.swallow", ef->image);
		
		evas_object_show(ef->image);
		evas_object_hide(ef->image2);

#ifdef HAVE_LIBEXIF
                int orientation = 0;

                ExifData  *exif = exif_data_new_from_file(ef->cur_image);
                ExifEntry *entry = NULL;
                ExifByteOrder bo;
               
                if (exif)
                {
                        entry = exif_data_get_entry(exif, EXIF_TAG_ORIENTATION);
                        if (entry)
                        {
                                bo = exif_data_get_byte_order(exif);
                                orientation = exif_get_short(entry->data, bo);
                        }
                        exif_data_free(exif);
                }

                if (orientation > 1 && orientation < 9)
                {
                        Evas_Object *o = elm_layout_edje_get(ef->flow_browser);

                        switch (orientation)
                        {
                                case 2:		/* Horizontal flip */
                                        break;
                                case 3:		/* Rotate 180 clockwise */
                                        ef->orient = EPHOTO_ORIENT_180;
                                        edje_object_signal_emit(o, "ef,state,rotate,180", "ef");
                                        break;
                                case 4:		/* Vertical flip */
                                        break;
                                case 5:		/* Transpose */
                                        break;
                                case 6:		/* Rotate 90 clockwise */
                                        ef->orient = EPHOTO_ORIENT_90;
                                        edje_object_signal_emit(o, "ef,state,rotate,90", "ef");
                                        break;
                                case 7:		/* Transverse */
                                        break;
                                case 8:		/* Rotate 90 counter-clockwise */
                                        ef->orient = EPHOTO_ORIENT_270;
                                        edje_object_signal_emit(o, "ef,state,rotate,270", "ef");
                                        break;
                        }
                }
#endif
	} else {
  	        success = elm_image_file_set(ef->image2, ef->cur_image, NULL);
		elm_layout_content_set(ef->flow_browser, "ephoto.flow.swallow", ef->image2);
		evas_object_show(ef->image2);
		evas_object_hide(ef->image);
	}

	if (success)
                _ephoto_set_title(ef->cur_image);

        elm_toolbar_item_unselect_all(ef->toolbar);
}

/*Create the flow browser*/
Evas_Object *
ephoto_create_flow_browser(Evas_Object *parent)
{
	Evas_Object *o;
	Ephoto_Flow_Browser *ef;

	ef = calloc(1, sizeof(Ephoto_Flow_Browser));
	
	ef->flow_browser = elm_layout_add(parent);
	elm_layout_file_set(ef->flow_browser,
			    PACKAGE_DATA_DIR "/themes/default/ephoto.edj", 
			    "ephoto/flow/layout");
	//elm_win_resize_object_add(em->win, em->flow_browser);
	evas_object_size_hint_weight_set(ef->flow_browser, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ef->flow_browser, EVAS_HINT_FILL, EVAS_HINT_FILL);

	ef->image = elm_photocam_add(ef->flow_browser);
        evas_object_size_hint_weight_set(ef->image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(ef->image, EVAS_HINT_FILL, EVAS_HINT_FILL);

	ef->image2 = elm_image_add(ef->flow_browser);
	elm_image_smooth_set(ef->image2, EINA_TRUE);
        evas_object_size_hint_weight_set(ef->image2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(ef->image2, EVAS_HINT_FILL, EVAS_HINT_FILL);

	ef->toolbar = elm_toolbar_add(ef->flow_browser);
        elm_toolbar_icon_size_set(ef->toolbar, 24);
        elm_toolbar_homogenous_set(ef->toolbar, EINA_TRUE);
        elm_toolbar_scrollable_set(ef->toolbar, EINA_FALSE);
        evas_object_size_hint_weight_set(ef->toolbar, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(ef->toolbar, EVAS_HINT_FILL, 0.5);
	elm_layout_content_set(ef->flow_browser, "ephoto.toolbar.swallow", ef->toolbar);
	evas_object_show(ef->toolbar);

	o = elm_icon_add(ef->toolbar);
        elm_icon_file_set(o, PACKAGE_DATA_DIR "/images/go_back.png", NULL);
        elm_toolbar_item_add(ef->toolbar, o, "Go Back", _ephoto_go_back, ef);

        o = elm_icon_add(ef->toolbar);
        elm_icon_file_set(o, PACKAGE_DATA_DIR "/images/first.png", NULL);
        elm_toolbar_item_add(ef->toolbar, o, "First", _ephoto_go_first, ef);

	o = elm_icon_add(ef->toolbar);
        elm_icon_file_set(o, PACKAGE_DATA_DIR "/images/previous.png", NULL);
        elm_toolbar_item_add(ef->toolbar, o, "Previous", _ephoto_go_previous, ef);

	o = elm_icon_add(ef->toolbar);
        elm_icon_file_set(o, PACKAGE_DATA_DIR "/images/next.png", NULL);
        elm_toolbar_item_add(ef->toolbar, o, "Next", _ephoto_go_next, ef);

        o = elm_icon_add(ef->toolbar);
        elm_icon_file_set(o, PACKAGE_DATA_DIR "/images/last.png", NULL);
        elm_toolbar_item_add(ef->toolbar, o, "Last", _ephoto_go_last, ef);

	o = elm_icon_add(ef->toolbar);
        elm_icon_file_set(o, PACKAGE_DATA_DIR "/images/play_slideshow.png", NULL);
        elm_toolbar_item_add(ef->toolbar, o, "Slideshow", _ephoto_go_slideshow, ef);

        o = elm_icon_add(ef->toolbar);
        elm_icon_file_set(o, PACKAGE_DATA_DIR "/images/play_slideshow.png", NULL);
        elm_toolbar_item_add(ef->toolbar, o, "RotateCW", _ephoto_go_rotate_clockwise, ef);

        o = elm_icon_add(ef->toolbar);
        elm_icon_file_set(o, PACKAGE_DATA_DIR "/images/play_slideshow.png", NULL);
        elm_toolbar_item_add(ef->toolbar, o, "RotateCCW", _ephoto_go_rotate_counterclockwise, ef);

        o = elm_icon_add(ef->toolbar);
        elm_icon_file_set(o, PACKAGE_DATA_DIR "/images/play_slideshow.png", NULL);
        elm_toolbar_item_add(ef->toolbar, o, "Zoom In", _ephoto_zoom_in, ef);

        o = elm_icon_add(ef->toolbar);
        elm_icon_file_set(o, PACKAGE_DATA_DIR "/images/play_slideshow.png", NULL);
        elm_toolbar_item_add(ef->toolbar, o, "Zoom Out", _ephoto_zoom_out, ef);

	evas_object_event_callback_add(ef->flow_browser, EVAS_CALLBACK_SHOW,
						_ephoto_flow_browser_show_cb, ef);
	evas_object_event_callback_add(ef->flow_browser, EVAS_CALLBACK_DEL,
						_ephoto_flow_browser_del_cb, ef);
	evas_object_data_set(ef->flow_browser, "flow_browser", ef);

	return ef->flow_browser;
}

/*Show the flow browser*/

void
ephoto_flow_browser_image_set(Evas_Object *obj, const char *current_image)
{	
        Ephoto_Flow_Browser *ef;
	Elm_Toolbar_Item *o;
	unsigned int i;

        ef = evas_object_data_get(obj, "flow_browser");

	if (current_image)
        {
		ef->cur_image = current_image;

		evas_object_event_callback_add(ef->flow_browser, EVAS_CALLBACK_KEY_DOWN,
						_ephoto_key_pressed, ef);
		evas_object_event_callback_add(ef->flow_browser, EVAS_CALLBACK_MOUSE_WHEEL,
						_ephoto_mouse_wheel, ef);

		ef->iter = eina_list_data_find_list(em->images, current_image);
		for (i = 0; i < (sizeof (toolbar_items) / sizeof (char*)); ++i)
		{
			o = elm_toolbar_item_find_by_label(ef->toolbar, toolbar_items[i]);
			elm_toolbar_item_disabled_set(o, !ef->iter ? EINA_TRUE : EINA_FALSE);
		}

                DBG("iter: %p", ef->iter);

		_ephoto_go_update(ef);
	}
	else
	{
		for (i = 0; i < (sizeof (toolbar_items) / sizeof (char*)); ++i)
                {
                        o = elm_toolbar_item_find_by_label(ef->toolbar, toolbar_items[i]);
                        elm_toolbar_item_disabled_set(o, EINA_TRUE);
                }
	}
	evas_object_show(ef->toolbar);
	evas_object_show(ef->flow_browser);
	evas_object_focus_set(ef->flow_browser, 1);
}

static void
_ephoto_flow_browser_show_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{

}

/*Delete the flow browser*/
static void 
_ephoto_flow_browser_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{

}

/*A key has been pressed*/
static const struct
{
	const char *name;
	const char *modifiers;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
} keys[] = {
	{ "Left", NULL, _ephoto_go_previous },
	{ "Right", NULL, _ephoto_go_next },
	{ "space", NULL, _ephoto_go_next },
	{ "Escape", NULL, _ephoto_go_back },
	{ "bracketleft", NULL, _ephoto_go_rotate_counterclockwise },
	{ "bracketright", NULL, _ephoto_go_rotate_clockwise },
	{ "Home", NULL, _ephoto_go_first },
	{ "End", NULL, _ephoto_go_last },
	{ "F5", NULL, _ephoto_go_slideshow},
	{ "plus", "Control", _ephoto_zoom_in},
	{ "minus", "Control", _ephoto_zoom_out},
	{ "0", "Control", _ephoto_zoom_regular_size},
        { "e", NULL, _ephoto_go_editor },
	{ NULL, NULL, NULL }
};

static void
_ephoto_key_pressed(void *data, Evas *e, Evas_Object *obj, void *event_data)
{
	Evas_Event_Key_Down *eku;
	int i;

	eku = (Evas_Event_Key_Down *)event_data;
	DBG("Key name: %s", eku->key);
	for (i = 0; keys[i].name; ++i)
		if ((!strcmp(eku->key, keys[i].name)) &&
		    ((keys[i].modifiers == NULL) || (evas_key_modifier_is_set(eku->modifiers, keys[i].modifiers))))
			keys[i].func(data, NULL, NULL);
}

static void
_ephoto_mouse_wheel(void *data, Evas *e, Evas_Object *obj, void *event_data)
{
	Evas_Event_Mouse_Wheel *emw = (Evas_Event_Mouse_Wheel *) event_data;
	if (evas_key_modifier_is_set(emw->modifiers, "Control"))
	{
		if (emw->z < 0)
			_ephoto_zoom_in(data, NULL, NULL);
		else
			_ephoto_zoom_out(data, NULL, NULL);
	}
}

/*Go back to the thumbnail viewer*/
static void 
_ephoto_go_back(void *data, Evas_Object *obj, void *event_info)
{
        Ephoto_Flow_Browser *ef = data;
    
	evas_object_smart_callback_call(ef->flow_browser, "delete,request", NULL);
	/* elm_toolbar_item_unselect_all(ef->toolbar); */
	/* em->thumb_browser = ephoto_create_thumb_browser(em->layout); */
	/* elm_layout_content_set(em->layout, "ephoto.content.swallow", em->thumb_browser); */
    
}

/*Go to the very first image in the list*/
static void 
_ephoto_go_first(void *data, Evas_Object *obj, void *event_info)
{
        Ephoto_Flow_Browser *ef = data;

	ef->iter = em->images;

	ef->cur_image = eina_list_data_get(ef->iter);
	_ephoto_go_update(ef);
}

/*Go to the very last image in the list*/
static void 
_ephoto_go_last(void *data, Evas_Object *obj, void *event_info)
{
        Ephoto_Flow_Browser *ef = data;

	ef->iter = eina_list_last(em->images);
	ef->cur_image = eina_list_data_get(ef->iter);

	_ephoto_go_update(ef);
}

/*Go to the next image in the list*/
static void 
_ephoto_go_next(void *data, Evas_Object *obj, void *event_info)
{
	Ephoto_Flow_Browser *ef = data;

	ef->iter = eina_list_next(ef->iter);
	if (!ef->iter) ef->iter = em->images;

	ef->cur_image = eina_list_data_get(ef->iter);

	_ephoto_go_update(ef);
}

/*Go to the previous image in the list*/
static void 
_ephoto_go_previous(void *data, Evas_Object *obj, void *event_info)
{
	Ephoto_Flow_Browser *ef = data;

	ef->iter = eina_list_prev(ef->iter);
	if (!ef->iter)
		ef->iter = eina_list_last(em->images);

	ef->cur_image = eina_list_data_get(ef->iter);

	_ephoto_go_update(ef);
}

/*Go to the slideshow*/
static void
_ephoto_go_slideshow(void *data, Evas_Object *obj, void *event_info)
{   
        Ephoto_Flow_Browser *ef = data;
        // FIXME
	//ephoto_hide_flow_browser();
	ephoto_show_slideshow(1, ef->cur_image);
	elm_toolbar_item_unselect_all(ef->toolbar);
}

static void
_ephoto_go_rotate_counterclockwise(void *data, Evas_Object *obj, void *event_info)
{
        Ephoto_Flow_Browser *ef = data;
        Evas_Object *o = elm_layout_edje_get(ef->flow_browser);

        switch(ef->orient)
        {
                case EPHOTO_ORIENT_0:
                        ef->orient = EPHOTO_ORIENT_270;
                        edje_object_signal_emit(o, "ef,state,rotate,270", "ef");
                        break;
                case EPHOTO_ORIENT_90:
                        ef->orient = EPHOTO_ORIENT_0;
                        edje_object_signal_emit(o, "ef,state,rotate,0", "ef");
                        break;
                case EPHOTO_ORIENT_180:
                        ef->orient = EPHOTO_ORIENT_90;
                        edje_object_signal_emit(o, "ef,state,rotate,90", "ef");
                        break;
                case EPHOTO_ORIENT_270:
                        ef->orient = EPHOTO_ORIENT_180;
                        edje_object_signal_emit(o, "ef,state,rotate,180", "ef");
                        break;
        }
	elm_toolbar_item_unselect_all(ef->toolbar);
}

static void
_ephoto_go_rotate_clockwise(void *data, Evas_Object *obj, void *event_info)
{
        Ephoto_Flow_Browser *ef = data;
        Evas_Object *o = elm_layout_edje_get(ef->flow_browser);

        switch(ef->orient)
        {
                case EPHOTO_ORIENT_0:
                        ef->orient = EPHOTO_ORIENT_90;
                        edje_object_signal_emit(o, "ef,state,rotate,90", "ef");
                        break;
                case EPHOTO_ORIENT_90:
                        ef->orient = EPHOTO_ORIENT_180;
                        edje_object_signal_emit(o, "ef,state,rotate,180", "ef");
                        break;
                case EPHOTO_ORIENT_180:
                        ef->orient = EPHOTO_ORIENT_270;
                        edje_object_signal_emit(o, "ef,state,rotate,270", "ef");
                        break;
                case EPHOTO_ORIENT_270:
                        ef->orient = EPHOTO_ORIENT_0;
                        edje_object_signal_emit(o, "ef,state,rotate,0", "ef");
                        break;
        }
	elm_toolbar_item_unselect_all(ef->toolbar);
}

static void
_ephoto_go_editor(void *data, Evas_Object *obj, void *event_info)
{
        Ephoto_Flow_Browser *ef = data;
        Ecore_Exe *exe;
        char buf[PATH_MAX];

        snprintf(buf, sizeof(buf), em->config->editor, (char *) ef->cur_image);
        DBG("Editor command: %s", buf);
        exe = ecore_exe_run(buf, NULL);
        ecore_exe_free(exe);
}

/* Zoom in in image */
static void
_ephoto_zoom_in(void *data, Evas_Object *obj, void *event)
{
	Ephoto_Flow_Browser *ef = data;
	double zoom;

	elm_photocam_zoom_mode_set(ef->image, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
	zoom = elm_photocam_zoom_get(ef->image);
	zoom -= 0.4;
	if (zoom < 0.1)
		zoom = 0.1;
	elm_photocam_zoom_set(ef->image, zoom);
	elm_toolbar_item_unselect_all(ef->toolbar);
}

/* Zoom out in image */
static void
_ephoto_zoom_out(void *data, Evas_Object *obj, void *event)
{
	Ephoto_Flow_Browser *ef = data;
	double zoom;

	elm_photocam_zoom_mode_set(ef->image, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
	zoom = elm_photocam_zoom_get(ef->image);
	zoom += 0.4;
	elm_photocam_zoom_set(ef->image, zoom);
	elm_toolbar_item_unselect_all(ef->toolbar);
}

/* Zoom regular size in image */
static void
_ephoto_zoom_regular_size(void *data, Evas_Object *obj, void *event)
{
	Ephoto_Flow_Browser *ef = data;
	elm_photocam_zoom_mode_set(ef->image, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
}
