
#include <Ewl.h>


static void ewl_filedialog_save_init(Ewl_Filedialog * fd,
				     Ewl_Callback_Function cb);
static void ewl_filedialog_open_init(Ewl_Filedialog * fd,
				     Ewl_Callback_Function cb);
static void ewl_filedialog_change_path(Ewl_Widget * w, void *ev_data, 
						 void *user_data);



typedef struct _open_dialog Open_Dialog;
struct _open_dialog {
	Ewl_Widget     *box;	/* box to hold the buttons */

	Ewl_Widget     *open;	/* open button */
	Ewl_Widget     *cancel;	/* cancel button */
};

typedef struct _save_dialog Save_Dialog;
struct _save_dialog {
	Ewl_Widget     *inputbox;	/* box to hold text input widgets */
	Ewl_Widget     *buttonbox;	/* box to hold buttons */

	Ewl_Widget     *save;	/* save button */
	Ewl_Widget     *cancel;	/* cancel button */
};


/**
 * ewl_filedialog_new - create a new filedialog
 * @follows: the widget this dialog follows
 * @cb: callback to be called when open/save button is pushed
 *
 * Returns a pointer to a newly allocated filedialog in success, NULL on
 * failure.
 */
Ewl_Widget     *ewl_filedialog_new(Ewl_Widget * follows,
				   Ewl_Filedialog_Type type,
				   Ewl_Callback_Function cb)
{
	Ewl_Filedialog *fd;

	DENTER_FUNCTION(DLEVEL_STABLE);

	fd = NEW(Ewl_Filedialog, 1);
	if (!fd)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	memset(fd, 0, sizeof(Ewl_Filedialog));

	ewl_filedialog_init(fd, follows, type, cb);

	DRETURN_PTR(EWL_WIDGET(fd), DLEVEL_STABLE);
}


/**
 * ewl_filedialog_init - initialize a new filedialog
 * @fd: the filedialog
 * @follows: widget to follow for the floater
 * @type: the filedialog type
 * @cb: the callback to call when open/save button is pushed
 *
 * Returns nothing. Iinitialize the filedialog to default values.
 */
void
ewl_filedialog_init(Ewl_Filedialog * fd, Ewl_Widget * follows,
		    Ewl_Filedialog_Type type, Ewl_Callback_Function cb)
{
	Ewl_Widget     *w;
	
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR ("fd", fd);

	w = EWL_WIDGET(fd);
	
	ewl_floater_init(EWL_FLOATER(w), follows);
	fd->type = type;

	ewl_widget_set_appearance (EWL_WIDGET (w), "filedialog");

	if (type == EWL_FILEDIALOG_TYPE_OPEN)
		ewl_filedialog_open_init(EWL_FILEDIALOG (fd), cb);
	else
		ewl_filedialog_save_init(EWL_FILEDIALOG (fd), cb);


	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_filedialog_change_labels (Ewl_Widget * w, void *ev_data, 
		void *user_data) 
{
	char *ptr;
	Ewl_Filedialog *fd = user_data; 

	DENTER_FUNCTION(DLEVEL_STABLE);
	
	ptr = ewl_fileselector_get_path (EWL_FILESELECTOR (fd->selector));
	ewl_text_set_text (EWL_TEXT (fd->path_label), ptr);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_filedialog_change_entry (Ewl_Widget * w, void *ev_data,
		void *user_data)
{
	Ewl_Filedialog *fd = user_data;
	Ewl_Fileselector *fs = EWL_FILESELECTOR (fd->selector);

	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_entry_set_text (EWL_ENTRY (fd->entry), fs->item);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


static void
ewl_filedialog_open_init(Ewl_Filedialog * fd, Ewl_Callback_Function cb)
{
	Open_Dialog    *od;
	Ewl_Widget     *vbox;

	DENTER_FUNCTION(DLEVEL_STABLE);

	od = NEW(Open_Dialog, 1);

	if (!od)
		return;

	vbox = ewl_vbox_new ();
	ewl_container_append_child(EWL_CONTAINER(fd), vbox);
	ewl_widget_show (vbox);

	fd->path_label = ewl_text_new ("");
	ewl_container_append_child(EWL_CONTAINER(vbox), fd->path_label);
	ewl_widget_show (fd->path_label);

	fd->selector = ewl_fileselector_new(cb);
	ewl_object_set_fill_policy(EWL_OBJECT(fd->selector),
				   EWL_FLAG_FILL_SHRINK);
	ewl_container_append_child(EWL_CONTAINER(vbox), fd->selector);
	ewl_callback_append (EWL_WIDGET (fd->selector),
			EWL_CALLBACK_VALUE_CHANGED, ewl_filedialog_change_labels, fd);
	ewl_callback_append (EWL_WIDGET (fd->selector),
			EWL_CALLBACK_CLICKED, ewl_filedialog_change_entry, fd);
	ewl_widget_show(fd->selector);

	od->box = ewl_box_new(EWL_ORIENTATION_HORIZONTAL);
	ewl_box_set_spacing(EWL_BOX(od->box), 4);
	ewl_object_set_padding(EWL_OBJECT(od->box), 10, 10, 10, 10);
	ewl_object_set_fill_policy(EWL_OBJECT(od->box),
				   EWL_FLAG_FILL_VSHRINK);
	ewl_object_set_alignment(EWL_OBJECT(od->box), EWL_FLAG_ALIGN_RIGHT);
	ewl_container_append_child(EWL_CONTAINER(vbox), od->box);
	ewl_widget_show(od->box);

	od->open = ewl_button_new("Open");
	ewl_object_set_fill_policy(EWL_OBJECT(od->open),
				   EWL_FLAG_FILL_SHRINK);
	ewl_callback_append(od->open, EWL_CALLBACK_CLICKED, cb, fd->selector);
	ewl_container_append_child(EWL_CONTAINER(od->box), od->open);
	ewl_widget_show(od->open);

	od->cancel = ewl_button_new("Cancel");
	ewl_object_set_fill_policy(EWL_OBJECT(od->cancel),
				   EWL_FLAG_FILL_SHRINK);
	ewl_callback_append(od->cancel, EWL_CALLBACK_CLICKED,
			    ewl_filedialog_destroy_cb, NULL);
	ewl_container_append_child(EWL_CONTAINER(od->box), od->cancel);
	ewl_widget_show(od->cancel);

	fd->dialog = (void *) od;

	fd->entry = ewl_entry_new ("");
	ewl_container_append_child(EWL_CONTAINER(vbox), fd->entry);
	ewl_callback_append (fd->entry, EWL_CALLBACK_VALUE_CHANGED,
			ewl_filedialog_change_path, fd);
	ewl_widget_show (fd->entry);
	
	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_filedialog_change_path(Ewl_Widget * w, void *ev_data, void *user_data)
{
	printf ("Changing path to: %s\n", ewl_entry_get_text (EWL_ENTRY (w)));
}


static void
ewl_filedialog_save_init(Ewl_Filedialog * fd, Ewl_Callback_Function cb)
{
	Save_Dialog    *sd;

	DENTER_FUNCTION(DLEVEL_STABLE);

	sd = NEW(Save_Dialog, 1);
	if (!sd)
		return;

	fd->selector = ewl_fileselector_new(cb);
	ewl_object_set_fill_policy(EWL_OBJECT(fd->selector),
				   EWL_FLAG_FILL_FILL);
	ewl_container_append_child(EWL_CONTAINER(fd), fd->selector);


	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_filedialog_destroy_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Widget     *fd;

	/* Destroy the filedialog */
	fd = w->parent->parent;
	ewl_widget_destroy(fd);
}
