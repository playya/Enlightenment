#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

/* XXX may want to make this configurable, possibly per icon? */
#define EWL_ICON_COMPRESS_SIZE 10

static void ewl_icon_cb_label_mouse_down(Ewl_Widget *w, void *ev, 
							void *data);
static void ewl_icon_cb_entry_focus_out(Ewl_Widget *w, void *ev,
							void *data);
static void ewl_icon_cb_entry_value_changed(Ewl_Widget *w, void *ev,
							void *data);

static void ewl_icon_update_label(Ewl_Icon *icon);

/**
 * @return Returns a new Ewl_Icon widget, or NULL on failure
 * @brief Creates and initializes a new Ewl_Icon widget 
 */
Ewl_Widget *
ewl_icon_new(void)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);

	w = NEW(Ewl_Icon, 1);
	if (!w)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	if (!ewl_icon_init(EWL_ICON(w)))
	{
		ewl_widget_destroy(w);
		w = NULL;
	}

	DRETURN_PTR(w, DLEVEL_STABLE);
}

/**
 * @param icon: The widget to initialize
 * @return Returns TRUE on successful initialization, FALSE otherwise
 * @brief Initializes the given Ewl_Icon widget
 */
int
ewl_icon_init(Ewl_Icon *icon)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, FALSE);

	if (!ewl_box_init(EWL_BOX(icon)))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_box_orientation_set(EWL_BOX(icon), EWL_ORIENTATION_VERTICAL);
	ewl_box_spacing_set(EWL_BOX(icon), 4);

	ewl_widget_appearance_set(EWL_WIDGET(icon), EWL_ICON_TYPE);
	ewl_widget_inherit(EWL_WIDGET(icon), EWL_ICON_TYPE);

	ewl_callback_prepend(EWL_WIDGET(icon), EWL_CALLBACK_DESTROY,
					ewl_icon_cb_destroy, NULL);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param icon: The icon to set the type of
 * @param type The type to set on this icon
 * @return Returns no value.
 * @brief Set the type of the icon
 */
void
ewl_icon_type_set(Ewl_Icon *icon, Ewl_Icon_Type type)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("icon", icon);
	DCHECK_TYPE("icon", icon, EWL_ICON_TYPE);

	if (icon->type == type)
		DRETURN(DLEVEL_STABLE);

	icon->type = type;

	/* if we are no longer extended then clear out the current extended
	 * data */
	if (icon->extended)
	{
		if (type == EWL_ICON_TYPE_SHORT)
			ewl_widget_hide(icon->extended);
		else
			ewl_widget_show(icon->extended);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param icon: The icon to get the type from
 * @return Returns the Ewl_Icon_Type of the icon
 * @brief Retrieve the type of the icon
 */
Ewl_Icon_Type 
ewl_icon_type_get(Ewl_Icon *icon)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, EWL_ICON_TYPE_SHORT);
	DCHECK_TYPE_RET("icon", icon, EWL_ICON_TYPE, EWL_ICON_TYPE_SHORT);

	DRETURN_INT(icon->type, DLEVEL_STABLE);
}

/**
 * @param icon: The Ewl_Icon to set the image into
 * @param file: The file with the image
 * @param key: The key inside the file if applicable
 * @return Returns no value
 * @brief set the image to use in the icon
 */
void
ewl_icon_image_set(Ewl_Icon *icon, const char *file, const char *key)
{
	Ewl_Widget *img;
	int constrain = 16;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("icon", icon);
	DCHECK_PARAM_PTR("file", file);
	DCHECK_TYPE("icon", icon, EWL_ICON_TYPE);

	if (icon->preview)
	{
		if (ewl_widget_type_is(icon->preview, EWL_IMAGE_TYPE))
			constrain = ewl_icon_constrain_get(icon);
		ewl_widget_destroy(icon->preview);
	}

	img = ewl_image_new();
	ewl_image_file_set(EWL_IMAGE(img), file, key);

	icon->preview = ewl_image_thumbnail_get(EWL_IMAGE(img));
	ewl_image_proportional_set(EWL_IMAGE(icon->preview), TRUE);
	ewl_icon_constrain_set(icon, constrain);
	ewl_image_file_set(EWL_IMAGE(icon->preview), 
					ewl_icon_theme_icon_path_get(
						EWL_ICON_IMAGE_LOADING, NULL),
					EWL_ICON_IMAGE_LOADING);
	ewl_object_alignment_set(EWL_OBJECT(icon->preview), 
						EWL_FLAG_ALIGN_CENTER);
	ewl_widget_internal_set(icon->preview, TRUE);
	ewl_container_child_prepend(EWL_CONTAINER(icon), icon->preview);
	ewl_widget_show(icon->preview);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param icon: The Ewl_Icon to get the image file from
 * @return Returns the image file associated with this icon, or NULL if
 * none.
 * @brief Retrieve the image to used in the icon
 */
const char *
ewl_icon_image_file_get(Ewl_Icon *icon)
{
	const char *file = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, NULL);
	DCHECK_TYPE_RET("icon", icon, EWL_ICON_TYPE, NULL);

	if (icon->preview)
		file = ewl_image_file_path_get(EWL_IMAGE(icon->preview));

	DRETURN_PTR(file, DLEVEL_STABLE);
}

/**
 * @param icon: The icon to set if it is editable or not
 * @param e: The value to set as the editable flag
 * @return Returns no value.
 * @brief Set if the icon is editable or not
 */
void
ewl_icon_editable_set(Ewl_Icon *icon, unsigned int e)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("icon", icon);
	DCHECK_TYPE("icon", icon, EWL_ICON_TYPE);

	if (icon->editable == e)
		DRETURN(DLEVEL_STABLE);

	icon->editable = e;
	if (icon->editable && icon->label)
		ewl_callback_append(icon->label, EWL_CALLBACK_MOUSE_DOWN,
					ewl_icon_cb_label_mouse_down, icon);
	else if (icon->label)
		ewl_callback_del(icon->label, EWL_CALLBACK_MOUSE_DOWN,
					ewl_icon_cb_label_mouse_down);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param icon: The icon to check if it's editable
 * @return Returns TRUE if the icon is editable, FALSE otherwise
 * @brief Retrieve if the icon is editable or not
 */
unsigned int
ewl_icon_editable_get(Ewl_Icon *icon)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, FALSE);
	DCHECK_TYPE_RET("icon", icon, EWL_ICON_TYPE, FALSE);

	DRETURN_INT(icon->editable, DLEVEL_STABLE);
}

/**
 * @param icon: The icon to set the label onto
 * @param label: The label to set on the icon
 * @return Returns no value
 * @brief Set the label of the icon
 */
void
ewl_icon_label_set(Ewl_Icon *icon, const char *label)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("icon", icon);
	DCHECK_TYPE("icon", icon, EWL_ICON_TYPE);

	if (!label)
	{
		if (icon->label)
		{
			ewl_text_text_set(EWL_TEXT(icon->label), NULL);
			IF_FREE(icon->label_text);
		}

		DRETURN(DLEVEL_STABLE);
	}

	if (!icon->label)
	{
		icon->label = ewl_text_new();
		ewl_object_alignment_set(EWL_OBJECT(icon->label), 
						EWL_FLAG_ALIGN_CENTER);

		if (icon->editable)
			ewl_callback_append(icon->label, 
					EWL_CALLBACK_MOUSE_DOWN,
					ewl_icon_cb_label_mouse_down, icon);

		ewl_widget_show(icon->label);

		/* if we have a preview make sure we are after it, but
		 * before anything that is after the preview */
		if (icon->preview && icon->extended)
		{
			int idx;
			idx = ewl_container_child_index_get(EWL_CONTAINER(icon),
							icon->preview);
			ewl_container_child_insert_internal(EWL_CONTAINER(icon),
						icon->label, idx + 1);
		}
		else
			ewl_container_child_append(EWL_CONTAINER(icon),
								icon->label);
	}

	icon->label_text = strdup(label);
	ewl_icon_update_label(icon);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param icon: The icon to get the label from
 * @return Returns the icons label or NULL if none set
 * @brief Retrieve the label from the icon
 */
const char *
ewl_icon_label_get(Ewl_Icon *icon)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, NULL);
	DCHECK_TYPE_RET("icon", icon, EWL_ICON_TYPE, NULL);

	if (!icon->label)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	DRETURN_PTR(icon->label_text, DLEVEL_STABLE);
}

/**
 * @param icon: The icon to set the menu into
 * @param menu: The mneu to set on the icon
 * @return Returns no value
 * @brief Set the menu for the icon
 */
void
ewl_icon_menu_set(Ewl_Icon *icon, Ewl_Widget *menu)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("icon", icon);
	DCHECK_PARAM_PTR("menu", menu);
	DCHECK_TYPE("icon", icon, EWL_ICON_TYPE);
	DCHECK_TYPE("menu", menu, EWL_MENU_TYPE);

	printf("FIXME: MENUS NOT HOOKED INTO ICONS YET\n");

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/** 
 * @param icon: The icon to get the menu from
 * @return Returns the menu set on this icon, or NULL if none set 
 * @brief Retrieve the menu from the icon
 */
Ewl_Widget *
ewl_icon_menu_get(Ewl_Icon *icon)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, NULL);
	DCHECK_TYPE_RET("icon", icon, EWL_ICON_TYPE, NULL);

	printf("FIXME: MENUS NOT HOOKED INTO ICONS YET\n");

	DRETURN_PTR(NULL, DLEVEL_STABLE);
}

/**
 * @param icon: The icon to set the extended data into
 * @param ext: The extended data to set in the icon
 * @return Returns no value
 * @brief Set the extended data into the icon
 *
 * @note The widget passed in here becomes internal to the icon, you should
 * not delete it after this. You can pack widgets as needed but the icon
 * will handle the show/hide of the widget after this
 */
void
ewl_icon_extended_data_set(Ewl_Icon *icon, Ewl_Widget *ext)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("icon", icon);
	DCHECK_TYPE("icon", icon, EWL_ICON_TYPE);

	if (icon->extended)
		ewl_widget_destroy(icon->extended);
	
	icon->extended = ext;
	ewl_widget_internal_set(icon->extended, TRUE);
	ewl_container_child_append(EWL_CONTAINER(icon), icon->extended);	

	if (icon->type == EWL_ICON_TYPE_SHORT)
		ewl_widget_hide(icon->extended);
	else
		ewl_widget_show(icon->extended);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param icon: The icon to get the extended data from
 * @return Returns the extended data on the icon, or NULL if none set 
 * @brief Retrieve the extended data from the icon
 */
Ewl_Widget *
ewl_icon_extended_data_get(Ewl_Icon *icon)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, NULL);
	DCHECK_TYPE_RET("icon", icon, EWL_ICON_TYPE, NULL);

	DRETURN_PTR(icon->extended, DLEVEL_STABLE);
}

/**
 * @param icon: The Ewl_Icon to constrain
 * @param val: The val to constrain too
 * @return Returns no value.
 * @brief Set the constrain value on the icon
 */
void
ewl_icon_constrain_set(Ewl_Icon *icon, unsigned int val)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("icon", icon);
	DCHECK_TYPE("icon", icon, EWL_ICON_TYPE);

	if (icon->preview)
		ewl_image_constrain_set(EWL_IMAGE(icon->preview), val);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/** 
 * @param icon: The icon to get the constrain from
 * @return Returns the current constrain value of the icon 
 * @brief Retrieve the constrain value set on the icon
 */
unsigned int
ewl_icon_constrain_get(Ewl_Icon *icon)
{
	unsigned int constrain = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, 0);
	DCHECK_TYPE_RET("icon", icon, EWL_ICON_TYPE, 0);

	if (!ewl_widget_type_is(icon->preview, EWL_IMAGE_TYPE))
		DRETURN_INT(constrain, DLEVEL_STABLE);

	constrain = ewl_image_constrain_get(EWL_IMAGE(icon->preview));

	DRETURN_INT(constrain, DLEVEL_STABLE);
}

/**
 * @param icon: The icon to work with
 * @param compress: The compression setting to use
 * @return Returns no value
 * @brief Sets the compressions setting for the icon to the given value
 */
void
ewl_icon_label_compressed_set(Ewl_Icon *icon, unsigned int compress)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("icon", icon);
	DCHECK_TYPE("icon", icon, EWL_ICON_TYPE);

	/* nothing to do if no compression change */
	if (compress == icon->compress_label)
		DRETURN(DLEVEL_STABLE);

	icon->compress_label = !!compress;
	ewl_icon_update_label(icon);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param icon: The ewl_icon to work with
 * @return Returns the current compression setting for the icon
 * @brief Retrieves the current compressiion setting for the icon
 */
unsigned int
ewl_icon_label_compressed_get(Ewl_Icon *icon)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, FALSE);
	DCHECK_TYPE_RET("icon", icon, EWL_ICON_TYPE, FALSE);

	DRETURN_INT(icon->compress_label, DLEVEL_STABLE);
}

/**
 * @param icon: The icon to work with
 * @param txt: The text to set as the alternate text
 * @return Returns no value
 * @brief Sets the given text as the alternate text for the icon
 */
void
ewl_icon_alt_text_set(Ewl_Icon *icon, const char *txt)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("icon", icon);
	DCHECK_TYPE("icon", icon, EWL_ICON_TYPE);

	IF_FREE(icon->alt_text);
	if (!txt && icon->preview && 
			ewl_widget_type_is(icon->preview, EWL_LABEL_TYPE))
	{
		ewl_widget_destroy(icon->preview);
		icon->preview = NULL;

		DRETURN(DLEVEL_STABLE);
	}

	icon->alt_text = strdup(txt);
	if (!icon->preview)
	{
		icon->preview = ewl_label_new();
		ewl_label_text_set(EWL_LABEL(icon->preview), icon->alt_text);
		ewl_container_child_prepend(EWL_CONTAINER(icon), icon->preview);
		ewl_widget_show(icon->preview);
	}
	else if (ewl_widget_type_is(icon->preview, EWL_LABEL_TYPE))
		ewl_label_text_set(EWL_LABEL(icon->preview), icon->alt_text);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param icon: The icon to work with
 * @return Returns the alternate text set on the icon
 * @brief Retrieves the alternate text set on the icon
 */
const char *
ewl_icon_alt_text_get(Ewl_Icon *icon)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, NULL);
	DCHECK_TYPE_RET("icon", icon, EWL_ICON_TYPE, NULL);

	DRETURN_PTR(icon->alt_text, DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @param ev: UNUSED
 * @param data: UNUSED
 * @return Returns no value
 * @brief The destroy callback
 */
void
ewl_icon_cb_destroy(Ewl_Widget *w, void *ev __UNUSED__, void *data __UNUSED__)
{
	Ewl_Icon *icon;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, EWL_WIDGET_TYPE);

	icon = EWL_ICON(w);
	IF_FREE(icon->label_text);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_icon_cb_label_mouse_down(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__, 
						void *data)
{
	Ewl_Icon *icon;
	Ewl_Widget *entry;
	Ewl_Embed *emb;
	int x, y;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("data", data);
	DCHECK_TYPE("data", data, EWL_ICON_TYPE);

	icon = data;
	emb = ewl_embed_widget_find(EWL_WIDGET(icon));

	ewl_widget_hide(icon->label);

	entry = ewl_entry_new();
	ewl_text_text_set(EWL_TEXT(entry), icon->label_text);
	ewl_container_child_append(EWL_CONTAINER(emb), entry);

	/* put the entry in the same spot as the label */
	ewl_object_current_geometry_get(EWL_OBJECT(icon->label), &x, &y, 
							NULL, NULL);
	ewl_object_position_request(EWL_OBJECT(entry), x, y);
	ewl_widget_show(entry);

	ewl_callback_append(entry, EWL_CALLBACK_FOCUS_OUT,
				ewl_icon_cb_entry_focus_out, icon);
	ewl_callback_append(entry, EWL_CALLBACK_VALUE_CHANGED,
				ewl_icon_cb_entry_value_changed, icon);

	ewl_embed_focused_widget_set(emb, entry);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_icon_cb_entry_focus_out(Ewl_Widget *w, void *ev __UNUSED__, 
						void *data __UNUSED__)
{
	Ewl_Icon *icon;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("data", data);
	DCHECK_TYPE("w", w, EWL_WIDGET_TYPE);
	DCHECK_TYPE("data", data, EWL_ICON_TYPE);

	icon = data;

	ewl_widget_show(icon->label);
	ewl_widget_destroy(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_icon_cb_entry_value_changed(Ewl_Widget *w, void *ev __UNUSED__, void *data)
{
	Ewl_Icon *icon;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("data", data);
	DCHECK_TYPE("w", w, EWL_WIDGET_TYPE);
	DCHECK_TYPE("data", data, EWL_ICON_TYPE);

	icon = data;
	ewl_icon_label_set(icon, ewl_text_text_get(EWL_TEXT(w)));

	ewl_widget_show(icon->label);
	ewl_widget_destroy(w);

	ewl_callback_call(EWL_WIDGET(icon), EWL_CALLBACK_VALUE_CHANGED);
}

static void
ewl_icon_update_label(Ewl_Icon *icon)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("icon", icon);
	DCHECK_TYPE("icon", icon, EWL_ICON_TYPE);

	/* nothing to do if no label set */
	if (!icon->label_text)
		DRETURN(DLEVEL_STABLE);

	if (icon->compress_label && 
			(strlen(icon->label_text) > EWL_ICON_COMPRESS_SIZE))
	{
		char *c;

		c = NEW(char, EWL_ICON_COMPRESS_SIZE + 4);
		strncpy(c, icon->label_text, EWL_ICON_COMPRESS_SIZE);
		strcat(c, "...");

		ewl_text_text_set(EWL_TEXT(icon->label), c);
		FREE(c);
	}
	else
		ewl_text_text_set(EWL_TEXT(icon->label), icon->label_text);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


