
/*\
|*|
|*| The container is a sub class for each widget, Ewl_Widget structure holds it
|*| and is used to keep all subwidgets for a widget.
|*|
\*/

#ifndef __EWL_CONTAINER_H__
#define __EWL_CONTAINER_H__

typedef struct _ewl_container Ewl_Container;
#define EWL_CONTAINER(widget) ((Ewl_Container *) widget)

struct _ewl_container
{
	Ewl_Widget widget;
	Ewd_List *children;
	Ewd_Free_Cb free_cb;
	Evas_Object clip_box;
};

void ewl_container_init(Ewl_Container * widget, int type, int min_w,
			int min_h, int max_w, int max_h);
void ewl_container_append_child(Ewl_Container * parent, Ewl_Widget * child);
void ewl_container_prepend_child(Ewl_Container * parent, Ewl_Widget * child);
void ewl_container_insert_child(Ewl_Container * parent,
				Ewl_Widget * child, int index);
Ewl_Widget *ewl_container_get_child_at(Ewl_Container * widget, int x, int y);
Ewl_Widget *ewl_container_get_child_at_recursive(Ewl_Container * widget,
						 int x, int y);
void ewl_container_clip_box_create(Ewl_Container * widget);
void ewl_container_clip_box_resize(Ewl_Container * widget);
void ewl_container_set_clip(Ewl_Container * widget);
void ewl_container_show_clip(Ewl_Container * widget);
void ewl_container_hide_clip(Ewl_Container * widget);

#define RECURSIVE(w) EWL_CONTAINER(w)->recursive

#endif /* __EWL_CONTAINER_H__ */
