#include "e.h"
#include "view_layout.h"
#include "util.h"

static void e_view_layout_cleanup(E_View_Layout *layout);

E_View_Layout *
e_view_layout_new(E_View *v)
{
   E_View_Layout *layout;
   Ebits_Object bits;
   char buf[PATH_MAX];

   D_ENTER;

   layout = NEW(E_View_Layout, 1);
   ZERO(layout, E_View_Layout, 1);

   e_object_init(E_OBJECT(layout), (E_Cleanup_Func) e_view_layout_cleanup);

   layout->view = v;

   D_RETURN_(layout);
}

static void
e_view_layout_cleanup(E_View_Layout *layout)
{
   Evas_List l;

   D_ENTER;

   /* cleanup the elements */
   for (l = layout->elements; l; l = l->next)
   {
      E_View_Layout_Element *el = l->data;
      if (el)
      {
	D("cleanup element: %s\n", el->name);
        if (el->name) FREE(el->name);
        FREE(el);
      }
   }
   evas_list_free(layout->elements);
   
   /* free the bits */
   if (layout->bits) ebits_free(layout->bits);

   IF_FREE(layout->file);
   IF_FREE(layout->prev_file);

   /* cleanup the base object */
   e_object_cleanup(E_OBJECT(layout));

   D_RETURN;
}

void
e_view_layout_realize(E_View_Layout *layout)
{
   Ebits_Object bits;
   Evas_List l;
   char buf[PATH_MAX];

   D_ENTER;
  
   /* check for custom layout bits */
   snprintf(buf, PATH_MAX, "%s/.e_layout/layout.bits.db",
	    layout->view->model->dir);
   /* keep track of file loaded */ 
   IF_FREE(layout->file);
   e_strdup(layout->file, buf);
   bits = ebits_load(layout->file);

   /* if custom doesn't exist, load default layout */
   if (!bits)
   {
      if (layout->view->model->is_desktop)
        snprintf(buf, PATH_MAX, "%s/desktop.bits.db", e_config_get("layout"));
      else
        snprintf(buf, PATH_MAX, "%s/view.bits.db", e_config_get("layout"));
      /* keep track of which file was loaded */
      IF_FREE(layout->file);
      e_strdup(layout->file, buf);
      bits = ebits_load(layout->file);
   }

   D("loaded layout: %s\n", layout->file);
   if (bits)
   {
     D("layout bits loaded!\n")
     layout->bits = bits;
     layout->mod_time = ecore_get_time();
     if (layout->view->evas)
     {
	ebits_add_to_evas(layout->bits, layout->view->evas);
	ebits_move(layout->bits, 0, 0);
	ebits_resize(layout->bits, layout->view->size.w, layout->view->size.h);
	D("add layout- w:%i, h:%i\n", layout->view->size.w, layout->view->size.h);
	for (l = ebits_get_bit_names(layout->bits); l; l = l->next)
	{
	  char *name = l->data;
	  
	  e_view_layout_add_element(layout, name);
	}
     }
   }
   else
   {
      D("ERROR: can't load layout\n");
   }

   IF_FREE(layout->prev_file);
   e_strdup(layout->prev_file, layout->prev_file);

   D_RETURN;
}
   
int
e_view_layout_add_element(E_View_Layout *layout, char *name)
{
   E_View_Layout_Element *el;
   Evas_List l;
   double x, y, w, h;

   D_ENTER;

   el = NEW(E_View_Layout_Element, 1);
   ZERO(el, E_View_Layout_Element, 1);

   e_strdup(el->name, name);

   for (l = ebits_get_bit_names(layout->bits); l; l = l->next)
   {
      char *name = l->data;
      if (!strcmp(name, el->name))
      {
	 ebits_get_named_bit_geometry(layout->bits, el->name, &x, &y, &w, &h);

	 el->x = x;
	 el->y = y;
	 el->w = w;
	 el->h = h;

	 D("add element: %s, %f, %f, %f, %f\n", el->name, x, y, w, h);
   
	 layout->elements = evas_list_append(layout->elements, el);
	 D_RETURN_(1);
      }
   }

   D("no element of with this name\n");
   FREE(el->name);
   FREE(el);
   D_RETURN_(0);
}

int
e_view_layout_delete_element(E_View_Layout *layout, char *name)
{
   Evas_List l;

   D_ENTER;

   for (l = layout->elements; l; l = l->next)
   {
      E_View_Layout_Element *el = l->data;
      
      if (!strcmp(name, el->name))
      {
	 FREE(el->name);
	 layout->elements = evas_list_remove(layout->elements, el);
	 FREE(el);
	 
	 D_RETURN_(1);
      }
   }

   D("no element of with this name\n");
   D_RETURN_(0);
}
int
e_view_layout_get_element_geometry(E_View_Layout *layout, char *name,
                                   double *x, double *y, double *w, double *h)
{
   Evas_List l;

   D_ENTER;

   for (l = layout->elements; l; l = l->next)
   {
      E_View_Layout_Element *el = l->data;

      if (!strcmp(name, el->name))
      {

	 if (x) *x = el->x;
	 if (y) *y = el->y;
	 if (w) *w = el->w;
         if (h) *h = el->h;


	 D_RETURN(1);
      }
   }

   D_RETURN(0);
}

void
e_view_layout_update(E_View_Layout *layout)
{
   Evas_List l;

   D_ENTER;

   if (!layout->bits)
     D_RETURN;
   /* move/resize bits */
   ebits_move(layout->bits, 0, 0);
   ebits_resize(layout->bits, layout->view->size.w, layout->view->size.h);
   D("update layout- w:%i, h:%i\n", layout->view->size.w, layout->view->size.h);

   /* update elements */
   for (l = layout->elements; l; l = l->next)
   {
      E_View_Layout_Element *el = l->data;
      double x, y, w, h;

      ebits_get_named_bit_geometry(layout->bits, el->name, &x, &y, &w, &h);

      el->x = x;
      el->y = y;
      el->w = w;
      el->h = h;
   }

   D_RETURN;
}
