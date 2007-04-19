#include "ephoto.h"

/*Ephoto Image Manipulation*/
static void add_standard_edit_tools(Ewl_Widget *c);
static void previous_image(Ewl_Widget *w, void *event, void *data);
static void next_image(Ewl_Widget *w, void *event, void *data);
static void flip_image_horizontal(Ewl_Widget *w, void *event, void *data);
static void flip_image_vertical(Ewl_Widget *w, void *event, void *data);
static void rotate_image_left(Ewl_Widget *w, void *event, void *data);
static void rotate_image_right(Ewl_Widget *w, void *event, void *data);

/*Add the edit view*/
Ewl_Widget *add_edit_view(Ewl_Widget *c)
{
	Ewl_Widget *button, *vbox, *hbox, *nb, *standard, *enhance, *advanced;

        vbox = add_box(c, EWL_ORIENTATION_VERTICAL, 5);
        ewl_object_fill_policy_set(EWL_OBJECT(vbox), EWL_FLAG_FILL_ALL);

	hbox = add_box(vbox, EWL_ORIENTATION_HORIZONTAL, 2);
	ewl_object_fill_policy_set(EWL_OBJECT(hbox), EWL_FLAG_FILL_ALL);

	nb = ewl_notebook_new();
	ewl_object_fill_policy_set(EWL_OBJECT(nb), EWL_FLAG_FILL_VFILL | EWL_FLAG_FILL_HSHRINK);
	ewl_container_child_append(EWL_CONTAINER(hbox), nb);
	ewl_widget_show(nb);

	standard = add_box(nb, EWL_ORIENTATION_VERTICAL, 2);
	ewl_object_maximum_w_set(EWL_OBJECT(standard), 172);
	ewl_object_minimum_w_set(EWL_OBJECT(standard), 172);
	ewl_object_fill_policy_set(EWL_OBJECT(standard), EWL_FLAG_FILL_VFILL);
	ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(nb), standard, "Standard Tools");	

	add_standard_edit_tools(standard);

	enhance = add_box(nb, EWL_ORIENTATION_VERTICAL, 2);
        ewl_object_maximum_w_set(EWL_OBJECT(enhance), 172);
        ewl_object_minimum_w_set(EWL_OBJECT(enhance), 172);
        ewl_object_fill_policy_set(EWL_OBJECT(enhance), EWL_FLAG_FILL_VFILL);
        ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(nb), enhance, "Enhancing Tools");

	advanced = add_box(nb, EWL_ORIENTATION_VERTICAL, 2);
        ewl_object_maximum_w_set(EWL_OBJECT(advanced), 172);
        ewl_object_minimum_w_set(EWL_OBJECT(advanced), 172);
        ewl_object_fill_policy_set(EWL_OBJECT(advanced), EWL_FLAG_FILL_VFILL);
        ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(nb), advanced, "Advanced Tools");

        em->eimage = add_image(hbox, NULL, 0, NULL, NULL);
        ewl_object_alignment_set(EWL_OBJECT(em->eimage), EWL_FLAG_ALIGN_CENTER);
        ewl_object_fill_policy_set(EWL_OBJECT(em->eimage), EWL_FLAG_FILL_SHRINK);

	hbox = add_box(vbox, EWL_ORIENTATION_HORIZONTAL, 2);
	ewl_object_alignment_set(EWL_OBJECT(hbox), EWL_FLAG_ALIGN_CENTER);
	ewl_object_fill_policy_set(EWL_OBJECT(hbox), EWL_FLAG_FILL_SHRINK);

	button = add_button(hbox, "Return to main view", NULL, show_main_view, NULL);

        button = add_button(hbox, NULL, PACKAGE_DATA_DIR "/images/media-seek-backward.png", previous_image, NULL);
        ewl_button_image_size_set(EWL_BUTTON(button), 25, 25);
        ewl_attach_tooltip_text_set(button, "Previous Image");

        button = add_button(hbox, NULL, PACKAGE_DATA_DIR "/images/media-seek-forward.png", next_image, NULL);
        ewl_button_image_size_set(EWL_BUTTON(button), 25, 25);
        ewl_attach_tooltip_text_set(button, "Next Image");

	return vbox;
}

/*Show the edit view*/
void show_edit_view(Ewl_Widget *w, void *event, void *data)
{
        ewl_notebook_visible_page_set(EWL_NOTEBOOK(em->main_nb), em->edit_vbox);
	ewl_image_file_path_set(EWL_IMAGE(em->eimage), ecore_dlist_current(em->images));
	return;
}

/*Add edit tools to container c*/
static void add_standard_edit_tools(Ewl_Widget *c)
{
	Ewl_Widget *button;

        button = add_button(c, "Get Exif", PACKAGE_DATA_DIR "/images/get_exif.png", NULL, NULL);
        ewl_button_image_size_set(EWL_BUTTON(button), 30, 30);
	ewl_button_label_set(EWL_BUTTON(button), "You do not have libexif 0.6.13");
	ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_LEFT);
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_HFILL);
#ifdef BUILD_EXIF_SUPPORT
        ewl_callback_append(button, EWL_CALLBACK_CLICKED, display_exif_dialog, NULL);
        ewl_button_label_set(EWL_BUTTON(button), "View Exif Data");
#endif

	button = add_button(c, "Rotate Left", PACKAGE_DATA_DIR "/images/undo.png", rotate_image_left, NULL);
	ewl_button_image_size_set(EWL_BUTTON(button), 30, 30);
	ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_LEFT);
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_HFILL);

        button = add_button(c, "Rotate Right", PACKAGE_DATA_DIR "/images/redo.png", rotate_image_right, NULL);
        ewl_button_image_size_set(EWL_BUTTON(button), 30, 30);
	ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_LEFT);
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_HFILL);

        button = add_button(c, "Flip Horizontally", PACKAGE_DATA_DIR "/images/go-next.png", flip_image_horizontal, NULL);
        ewl_button_image_size_set(EWL_BUTTON(button), 30, 30);
	ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_LEFT);
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_HFILL);

        button = add_button(c, "Flip Vertically", PACKAGE_DATA_DIR "/images/go-down.png", flip_image_vertical, NULL);
	ewl_button_image_size_set(EWL_BUTTON(button), 30, 30);
	ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_LEFT);
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_HFILL);

	return;
}

/*Go to the previous image*/
static void previous_image(Ewl_Widget *w, void *event, void *data)
{
        char *image;

        ecore_dlist_previous(em->images);
        image = ecore_dlist_current(em->images);
	if(!image)
	{
		ecore_dlist_goto_last(em->images);
		image = ecore_dlist_current(em->images);
	}
        ewl_image_file_path_set(EWL_IMAGE(em->eimage), image);
        ewl_widget_configure(em->eimage->parent);

        return;
}


/*Go to the next image*/
static void next_image(Ewl_Widget *w, void *event, void *data)
{
	char *image;

	ecore_dlist_next(em->images);
	image = ecore_dlist_current(em->images);
	if(!image)
	{
		ecore_dlist_goto_first(em->images);
		image = ecore_dlist_current(em->images);
	}
	ewl_image_file_path_set(EWL_IMAGE(em->eimage), image);
	ewl_widget_configure(em->eimage->parent);

	return;
}

/*Flip the image 180 degrees horizontally*/
static void flip_image_horizontal(Ewl_Widget *w, void *event, void *data)
{
	unsigned int *image_data;
	int nw, nh;
	Ewl_Image *image;

	evas_object_image_size_get(EWL_IMAGE(em->eimage)->image, &nw, &nh);
	image_data = flip_horizontal(em->eimage);
	update_image(em->eimage, nw, nh, image_data);
	ewl_image_size_set(EWL_IMAGE(em->eimage), nh, nw);
	image = (Ewl_Image *)em->eimage;
        image->ow = nw;
        image->oh = nh;
	ewl_object_preferred_inner_size_set(EWL_OBJECT(em->eimage), nw, nh);
	ewl_widget_configure(em->eimage);
	ewl_widget_configure(em->eimage->parent);

	return;
}

/*Flip the image 180 degrees vertically*/
static void flip_image_vertical(Ewl_Widget *w, void *event, void *data)
{
	unsigned int *image_data;
	int nw, nh;
	Ewl_Image *image;

	evas_object_image_size_get(EWL_IMAGE(em->eimage)->image, &nw, &nh);
	image_data = flip_vertical(em->eimage);
	update_image(em->eimage, nw, nh, image_data);
	ewl_image_size_set(EWL_IMAGE(em->eimage), nh, nw);
	image = (Ewl_Image *)em->eimage;
        image->ow = nw;
        image->oh = nh;
	ewl_object_preferred_inner_size_set(EWL_OBJECT(em->eimage), nh, nw);
	ewl_widget_configure(em->eimage);
	ewl_widget_configure(em->eimage->parent);

	return;
}

/*Rotate the image 90 degrees to the left*/
static void rotate_image_left(Ewl_Widget *w, void *event, void *data)
{
	unsigned int *image_data;
	int nw, nh;
	Ewl_Image *image;

	evas_object_image_size_get(EWL_IMAGE(em->eimage)->image, &nh, &nw);
	image_data = rotate_left(em->eimage);
	update_image(em->eimage, nw, nh, image_data);
	ewl_image_size_set(EWL_IMAGE(em->eimage), nh, nw);
	image = (Ewl_Image *)em->eimage;
        image->ow = nw;
        image->oh = nh;
        ewl_object_preferred_inner_size_set(EWL_OBJECT(em->eimage), nw, nh);
	ewl_widget_configure(em->eimage);
	ewl_widget_configure(em->eimage->parent);

	return;
}

/*Rotate the image 90 degrees to the right*/
static void rotate_image_right(Ewl_Widget *w, void *event, void *data)
{
	unsigned int *image_data;
	int nw, nh;
	Ewl_Image *image;

	evas_object_image_size_get(EWL_IMAGE(em->eimage)->image, &nh, &nw);
	image_data = rotate_right(em->eimage);
	update_image(em->eimage, nw, nh, image_data);
	image = (Ewl_Image *)em->eimage;
        image->ow = nw;
        image->oh = nh;
        ewl_object_preferred_inner_size_set(EWL_OBJECT(em->eimage), nw, nh);
	ewl_widget_configure(em->eimage);
	ewl_widget_configure(em->eimage->parent);

	return;
}

