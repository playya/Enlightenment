#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

static Ewl_Image_Type  ewl_image_type_get(const char *i);

/**
 * @return Returns a pointer to a new image widget on success, NULL on failure.
 * @brief Load an image widget with specified image contents
 *
 * The @a k parameter is primarily used for loading edje groups or keyed data
 * in an image.
 */
Ewl_Widget *
ewl_image_new(void)
{
	Ewl_Image *image;

	DENTER_FUNCTION(DLEVEL_STABLE);

	image = NEW(Ewl_Image, 1);
	if (!image)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	if (!ewl_image_init(image)) {
		ewl_widget_destroy(EWL_WIDGET(image));
		image = NULL;
	}

	DRETURN_PTR(EWL_WIDGET(image), DLEVEL_STABLE);
}

/**
 * @param i: the image widget to initialize
 * @return Returns no value.
 * @brief Initialize an image widget to default values and callbacks
 *
 * Sets the fields and callbacks of @a i to their default values.
 */
int
ewl_image_init(Ewl_Image *i)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("i", i, FALSE);

	w = EWL_WIDGET(i);

	if (!ewl_widget_init(w))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_widget_appearance_set(w, "image");
	ewl_widget_inherit(w, "image");

	ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_NONE);

	/*
	 * Append necessary callbacks.
	 */
	ewl_callback_append(w, EWL_CALLBACK_REALIZE, ewl_image_realize_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_UNREALIZE, ewl_image_unrealize_cb,
			    NULL);
	ewl_callback_prepend(w, EWL_CALLBACK_DESTROY, ewl_image_destroy_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, ewl_image_configure_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_DOWN, ewl_image_mouse_down_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_UP, ewl_image_mouse_up_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_MOVE, ewl_image_mouse_move_cb,
			    NULL);

	i->sw = 1.0;
	i->sh = 1.0;

	i->cs = 0;

	i->tile.x = 0;
	i->tile.y = 0;
	i->tile.w = 0;
	i->tile.h = 0;

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param i: the image widget to get the file of
 * @return Returns the currently set filename
 * @brief get the filename this image uses
 */
char *
ewl_image_file_get(Ewl_Image *i)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("i", i, NULL);
	DCHECK_TYPE_RET("i", i, "image", NULL);

	DRETURN_PTR(i->path, DLEVEL_STABLE);
}

/**
 * @param i: the image widget to change the displayed image
 * @param im: the path to the new image to be displayed by @a i
 * @param key: the key in the file for the image
 * @return Returns no value.
 * @brief Change the image file displayed by an image widget
 *
 * Set the image displayed by @a i to the one found at the path @a im.
 */
void
ewl_image_file_set(Ewl_Image *i, char *im, char *key)
{
	int old_type;
	Ewl_Widget *w;
	Ewl_Embed *emb;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("i", i);
	DCHECK_TYPE("i", i, "image");

	w = EWL_WIDGET(i);
	emb = ewl_embed_widget_find(w);

	IF_FREE(i->path);
	IF_FREE(i->key);

	/*
	 * Determine the type of image to be loaded.
	 */
	old_type = i->type;
	if (im) {
		i->type = ewl_image_type_get(im);
		i->path = strdup(im);
		if (key)
			i->key = strdup(key);
	}
	else
		i->type = EWL_IMAGE_TYPE_NORMAL;

	/*
	 * Load the new image if widget has been realized
	 */
	if (REALIZED(w)) {
		/*
		 * Free the image if it had been loaded.
		 */
		if (i->image) {
			/*
			 * Type is important for using the correct free calls.
			 */
			evas_object_hide(i->image);
			evas_object_clip_unset(i->image);
			ewl_evas_object_destroy(i->image);
			i->image = NULL;
		}

		/*
		 * Now draw the new image
		 */
		ewl_image_realize_cb(w, NULL, NULL);
		ewl_widget_configure(w);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param i: the image to change constrain setting
 * @param size: the minimum constrain size
 * @return Returns no value.
 * @brief Set a size which, if the image is bigger than, scale proportionally
 *
 * Sets a size to scale to proportionally if the image exceeds this size
 */
void ewl_image_constrain_set(Ewl_Image* i, int size)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("i", i);
	DCHECK_TYPE("i", i, "image");

	i->cs = size;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param i: the image to change proportional setting
 * @param p: the boolean indicator of proportionality
 * @return Returns no value.
 * @brief Set boolean to determine how to scale
 *
 * Changes the flag indicating if the image is scaled proportionally.
 */
void
ewl_image_proportional_set(Ewl_Image *i, char p)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("i", i);
	DCHECK_TYPE("i", i, "image");

	i->proportional = p;
	ewl_widget_configure(EWL_WIDGET(i));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param i: the image to scale
 * @param wp: the percentage to scale width
 * @param hp: the percentage to scale height
 * @brief Scale image dimensions by a percentage
 *
 * @return Returns no value.
 * Scales the given image to @a wp percent of preferred width
 * by @a hp percent of preferred height. If @a i->proportional is set to TRUE,
 * the lesser of @a wp and @a hp is applied for both directions.
 */
void
ewl_image_scale(Ewl_Image *i, double wp, double hp)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("i", i);
	DCHECK_TYPE("i", i, "image");

	if (i->proportional) {
		if (wp < hp)
			hp = wp;
		else
			wp = hp;
	}

	i->sw = wp;
	i->sh = hp;

	ewl_object_preferred_inner_w_set(EWL_OBJECT(i), wp * i->ow);
	ewl_object_preferred_inner_h_set(EWL_OBJECT(i), hp * i->oh);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param i: the image to scale
 * @param w: the size to scale width
 * @param h: the size to scale height
 * @return Returns no value.
 * @brief Scale image dimensions to a specific size
 *
 * Scales the given image to @a w by @a hp. If @a i->proportional
 * is set to TRUE, the image is scaled proportional to the lesser scale
 * percentage of preferred size.
 */
void
ewl_image_scale_to(Ewl_Image *i, int w, int h)
{

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("i", i);
	DCHECK_TYPE("i", i, "image");

	i->sw = 1.0;
	i->sh = 1.0;
	i->aw = w;
	i->ah = h;
	ewl_object_preferred_inner_size_set(EWL_OBJECT(i), w, h);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param i: the image to tile
 * @param x: the x position of the top right corner
 * @param y: the y position of the top right corner
 * @param w: the width of the tile
 * @param h: the height of the tile
 * @return Returns no value
 * @brief Tile the image with the given start position and given size
 *
 * Tiles the image across the available area, starting the image at the
 * given position and with the given size.
 */
void
ewl_image_tile_set(Ewl_Image *i, int x, int y, int w, int h)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("i", i);
	DCHECK_TYPE("i", i, "image");

	i->tile.set = 1;
	i->tile.x = x;
	i->tile.y = y;
	i->tile.w = w;
	i->tile.h = h;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_image_realize_cb(Ewl_Widget *w, void *ev_data __UNUSED__,
					void *user_data __UNUSED__)
{
	Ewl_Image *i;
	Ewl_Embed *emb;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	i = EWL_IMAGE(w);
	emb = ewl_embed_widget_find(w);

	/*
	 * Load the image based on the type.
	 */
	if (i->type == EWL_IMAGE_TYPE_EDJE) {
		i->image = edje_object_add(emb->evas);
		if (!i->image)
			DRETURN(DLEVEL_STABLE);

		if (i->path)
			edje_object_file_set(i->image, i->path, i->key);
		edje_object_size_min_get(i->image, &i->ow, &i->oh);
	} else {
		i->image = evas_object_image_add(emb->evas);
		if (!i->image)
			DRETURN(DLEVEL_STABLE);

		if (i->path)
			evas_object_image_file_set(i->image, i->path, i->key);
		evas_object_image_size_get(i->image, &i->ow, &i->oh);
	}



	evas_object_layer_set(i->image, ewl_widget_layer_sum_get(w));
	if (w->fx_clip_box)
		evas_object_clip_set(i->image, w->fx_clip_box);

	evas_object_pass_events_set(i->image, TRUE);
	evas_object_show(i->image);

	if (!i->ow)
		i->ow = 1;
	if (!i->oh)
		i->oh = 1;

	if (i->aw || i->ah) {
		ewl_image_scale_to(i, i->aw, i->ah);
	}
	else {
		ewl_object_preferred_inner_w_set(EWL_OBJECT(i), i->ow);
		ewl_object_preferred_inner_h_set(EWL_OBJECT(i), i->oh);
		ewl_image_scale(i, i->sw, i->sh);
	}

	/*Constrain settings*/
	if (i->cs && (i->ow > i->cs || i->oh > i->cs)) {
		double cp = 0;
		if (i->ow > i->oh) 
			cp = i->cs / (double)i->ow;
	 	else 
			cp = i->cs / (double)i->oh;

		ewl_image_scale(i, cp,cp);
		ewl_image_tile_set(i,0,0,cp*i->ow,cp*i->oh);

	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_image_unrealize_cb(Ewl_Widget *w, void *ev_data __UNUSED__,
						void *user_data __UNUSED__)
{
	Ewl_Image *i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	i = EWL_IMAGE(w);
	if (i->image) {
		evas_object_del(i->image);
		i->image = NULL;
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_image_destroy_cb(Ewl_Widget *w, void *ev_data __UNUSED__,
					  void *user_data __UNUSED__)
{
	Ewl_Image *i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	i = EWL_IMAGE(w);

	IF_FREE(i->path);
	IF_FREE(i->key);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_image_reparent_cb(Ewl_Widget *w, void *ev_data __UNUSED__,
						void *user_data __UNUSED__)
{
	Ewl_Image *i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	i = EWL_IMAGE(w);
	if (!i->image)
		DRETURN(DLEVEL_STABLE);

	evas_object_layer_set(i->image, ewl_widget_layer_sum_get(w));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_image_configure_cb(Ewl_Widget *w, void *ev_data __UNUSED__,
						void *user_data __UNUSED__)
{
	Ewl_Image *i;
	Ewl_Embed *emb;
	int ww, hh;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	i = EWL_IMAGE(w);
	if (!i->image)
		DRETURN(DLEVEL_STABLE);

	emb = ewl_embed_widget_find(w);

	ww = CURRENT_W(w);
	hh = CURRENT_H(w);

	if (i->proportional) {
		double op;

		op = (double)i->ow / (double)i->oh;

		if (i->ow < i->oh) {
			ww /= op;
		}
		else {
			hh *= op;
		}
	}

	/*
	 * set the tile width and height if not set already
	*/
	if (!i->tile.set) {
		i->tile.x = i->tile.y = 0;
		i->tile.w = i->sw * ww;
		i->tile.h = i->sh * hh;
	}

	/*
	 * Move the image into place based on type.
	 */
	if (i->type != EWL_IMAGE_TYPE_EDJE)
		evas_object_image_fill_set(i->image, i->tile.x, i->tile.y,
					i->tile.w, i->tile.h);

	evas_object_move(i->image, CURRENT_X(w), CURRENT_Y(w));
	evas_object_resize(i->image, ww, hh);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Determine the type of the file based on the filename.
 */
static Ewl_Image_Type
ewl_image_type_get(const char *i)
{
	int l;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("i", i, -1);

	l = strlen(i);

	if ((l >= 8 && !(strncasecmp((char *) i + l - 8, ".bits.db", 8)))
	    || (l >= 4 && !(strncasecmp((char *) i + l - 4, ".eet", 4)))
	    || (l >= 4 && !(strncasecmp((char *) i + l - 4, ".edj", 4)))
	    || (l >= 4 && !(strncasecmp((char *) i + l - 4, ".eap", 5))))
		DRETURN_INT(EWL_IMAGE_TYPE_EDJE, DLEVEL_STABLE);

	DRETURN_INT(EWL_IMAGE_TYPE_NORMAL, DLEVEL_STABLE);
}

void
ewl_image_mouse_down_cb(Ewl_Widget *w, void *ev_data,
					void *user_data __UNUSED__)
{
	Ewl_Image *i;
	Ewl_Embed *emb;
	Ewl_Event_Mouse_Down *ev;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	i = EWL_IMAGE(w);
	emb = ewl_embed_widget_find(w);
	ev = ev_data;

	if (i->type == EWL_IMAGE_TYPE_EDJE)
		evas_event_feed_mouse_down(emb->evas, ev->button, 
				EVAS_BUTTON_NONE, 
				(unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), 
				NULL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_image_mouse_up_cb(Ewl_Widget *w, void *ev_data,
					void *user_data __UNUSED__)
{
	Ewl_Image *i;
	Ewl_Embed *emb;
	Ewl_Event_Mouse_Up *ev;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	i = EWL_IMAGE(w);
	emb = ewl_embed_widget_find(w);
	ev = ev_data;

	if (i->type == EWL_IMAGE_TYPE_EDJE)
		evas_event_feed_mouse_up(emb->evas, ev->button, 
				EVAS_BUTTON_NONE, 
				(unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), 
				NULL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_image_mouse_move_cb(Ewl_Widget *w, void *ev_data,
					void *user_data __UNUSED__)
{
	Ewl_Image *i;
	Ewl_Embed *emb;
	Ewl_Event_Mouse_Move *ev;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	i = EWL_IMAGE(w);
	emb = ewl_embed_widget_find(w);
	ev = ev_data;

	if (i->type == EWL_IMAGE_TYPE_EDJE)
		evas_event_feed_mouse_move(emb->evas, ev->x, ev->y, 
				(unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), 
				NULL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

