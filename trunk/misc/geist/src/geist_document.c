#include "geist_document.h"
#include "geist_document_xml.h"

geist_document *
geist_document_new(int w, int h)
{
   geist_document *d = NULL;

   D_ENTER(3);

   d = emalloc(sizeof(geist_document));

   memset(d, 0, sizeof(geist_document));

   d->w = w;
   d->h = h;

   d->im = imlib_create_image(w, h);
   d->pmap = XCreatePixmap(disp, root, w, h, depth);
   d->bg_fill = geist_fill_new_coloured(255, 255, 255, 255);
   d->name = estrdup("New document");
   geist_document_add_layer(d);

   D_RETURN(3, d);
}

void
geist_document_free(geist_document * document)
{
   geist_list *l;

   D_ENTER(3);

   for (l = document->layers; l; l = l->next)
      geist_layer_free((geist_layer *) l->data);

   geist_list_free(document->layers);

   if (document->im)
      geist_imlib_free_image(document->im);
   if (document->pmap)
      XFreePixmap(disp, document->pmap);

   efree(document);

   D_RETURN_(3);
}

void
geist_document_render(geist_document * document)
{
   geist_list *l;

   D_ENTER(3);

   geist_imlib_image_fill_rectangle(document->im, 0, 0, document->w,
                                    document->h, 255, 255, 255, 255);

   geist_fill_render(document->bg_fill, document->im, 0, 0, document->w,
                     document->h);

   for (l = document->layers; l; l = l->next)
      geist_layer_render((geist_layer *) l->data, document->im);

   D_RETURN_(3);
}

geist_list *
geist_document_get_selected_list(geist_document * doc)
{
   geist_list *l, *ll, *ret = NULL;
   geist_object *obj;

   D_ENTER(3);

   for (l = doc->layers; l; l = l->next)
   {
      for (ll = GEIST_LAYER(l->data)->objects; ll; ll = ll->next)
      {
         obj = GEIST_OBJECT(ll->data);

         if (geist_object_get_state(obj, SELECTED))
         {
            D(5, ("selected object found\n"));
            ret = geist_list_add_end(ret, obj);
         }
      }
   }
   D_RETURN(3, ret);
}

void
geist_document_unselect_all(geist_document * doc)
{
   geist_list *sl, *l;

   D_ENTER(3);

   sl = geist_document_get_selected_list(doc);
   D(3, ("selected items count: %d\n", geist_list_length(sl)));
   if (sl)
   {
      geist_object *obj;

      for (l = sl; l; l = l->next)
      {
         obj = GEIST_OBJECT(l->data);
         if (geist_object_get_state(obj, SELECTED))
         {
            geist_object_unset_state(obj, SELECTED);
            geist_object_dirty(obj);
         }
      }
      geist_list_free(sl);
   }

   D_RETURN_(3);
}

void
geist_document_render_selection(geist_document * doc)
{
   geist_list *sl, *l;
   int sel_count;

   D_ENTER(3);

   sl = geist_document_get_selected_list(doc);
   D(3, ("selected items count: %d\n", geist_list_length(sl)));

   if (sl)
   {
      geist_object *obj;

      sel_count = geist_list_length(sl);

      for (l = sl; l; l = l->next)
      {
         obj = GEIST_OBJECT(l->data);
         obj->render_selected(obj, doc->im, (sel_count > 1) ? TRUE : FALSE);
      }
      geist_list_free(sl);
   }

   D_RETURN_(3);
}

void
geist_document_render_selection_partial(geist_document * doc, int x, int y,
                                        int w, int h)
{
   geist_list *sl, *l;
   int sel_count;

   D_ENTER(3);

   sl = geist_document_get_selected_list(doc);
   D(3, ("selected items count: %d\n", geist_list_length(sl)));

   if (sl)
   {
      geist_object *obj;

      sel_count = geist_list_length(sl);

      for (l = sl; l; l = l->next)
      {
         obj = GEIST_OBJECT(l->data);
         if (RECTS_INTERSECT
             (x, y, w, h, obj->x - HALF_SEL_WIDTH, obj->y - HALF_SEL_HEIGHT,
              obj->w + (2 * HALF_SEL_WIDTH), obj->h + (2 * HALF_SEL_HEIGHT)))
         {
            obj->render_selected(obj, doc->im,
                                 (sel_count > 1) ? TRUE : FALSE);
         }
      }
      geist_list_free(sl);
   }

   D_RETURN_(3);
}


void
geist_document_render_pmap(geist_document * doc)
{
   D_ENTER(3);
   geist_imlib_render_image_on_drawable(doc->pmap, doc->im, 0, 0, 1, 1, 0);
   D_RETURN_(3);
}

void
geist_document_render_partial(geist_document * document, int x, int y, int w,
                              int h)
{
   geist_list *l;

   D_ENTER(3);

   D(4, ("Doc render partial, %d,%d %dx%d\n", x, y, w, h));
   geist_imlib_image_fill_rectangle(document->im, x, y, w, h, 255, 255, 255,
                                    255);

   geist_fill_render(document->bg_fill, document->im, x, y, w, h);

   for (l = document->layers; l; l = l->next)
      geist_layer_render_partial((geist_layer *) l->data, document->im, x, y,
                                 w, h);

   D_RETURN_(3);

}

void
geist_document_render_pmap_partial(geist_document * doc, int x, int y, int w,
                                   int h)
{
   D_ENTER(3);
   geist_imlib_render_image_part_on_drawable_at_size(doc->pmap, doc->im, x, y,
                                                     w, h, x, y, w, h, 1, 1,
                                                     0);
   D_RETURN_(3);

}

void
geist_document_add_object(geist_document * doc, geist_object * obj)
{
   geist_list *top;

   D_ENTER(3);

   top = geist_list_last(doc->layers);
   geist_layer_add_object(((geist_layer *) top->data), obj);
   if (GEIST_OBJECT_DOC(obj) == current_doc)
      geist_object_add_to_object_list(obj);
   geist_object_dirty(obj);

   D_RETURN_(3);
}


void
geist_document_reset_object_list(geist_document * d)
{
   geist_list *l, *ll;

   D_ENTER(3);

   gtk_signal_handler_block(GTK_OBJECT(obj_list), obj_sel_handler);
   gtk_signal_handler_block(GTK_OBJECT(obj_list), obj_unsel_handler);
   gtk_clist_freeze(GTK_CLIST(obj_list));
   gtk_clist_clear(GTK_CLIST(obj_list));
   for (l = d->layers; l; l = l->next)
   {
      for (ll = GEIST_LAYER(l->data)->objects; ll; ll = ll->next)
      {
         geist_object_add_to_object_list(GEIST_OBJECT(ll->data));
      }
   }
   gtk_clist_thaw(GTK_CLIST(obj_list));
   gtk_signal_handler_unblock(GTK_OBJECT(obj_list), obj_sel_handler);
   gtk_signal_handler_unblock(GTK_OBJECT(obj_list), obj_unsel_handler);

   D_RETURN_(3);
}

void
geist_document_add_layer(geist_document * doc)
{
   geist_layer *layer;

   D_ENTER(3);

   layer = geist_layer_new();
   layer->doc = doc;
   doc->layers = geist_list_add_end(doc->layers, layer);

   D_RETURN_(3);
}

geist_object *
geist_document_find_clicked_object(geist_document * doc, int x, int y)
{
   geist_list *l;
   geist_object *ret = NULL;

   D_ENTER(3);

   for (l = doc->layers; l; l = l->next)
   {
      if (GEIST_LAYER(l->data)->visible)
         ret = geist_layer_find_clicked_object(GEIST_LAYER(l->data), x, y);
   }

   D_RETURN(3, ret);

}

void
geist_document_render_full(geist_document * d, int selection)
{
   D_ENTER(3);

   geist_document_render(d);
   if (selection)
      geist_document_render_selection(d);
   geist_document_render_pmap(d);

   geist_document_render_to_window(d);

   D_RETURN_(3);
}

void
geist_document_render_updates(geist_document * d)
{
   D_ENTER(3);

   if (d && d->up)
   {
      int x, y, w, h;
      Imlib_Updates u;

      d->up = imlib_updates_merge_for_rendering(d->up, d->w, d->h);
      for (u = d->up; u; u = imlib_updates_get_next(u))
      {
         imlib_updates_get_coordinates(u, &x, &y, &w, &h);
         geist_document_render_partial(d, x, y, w, h);
         geist_document_render_selection_partial(d, x, y, w, h);
         geist_document_render_pmap_partial(d, x, y, w, h);
         geist_document_render_to_window_partial(d, x, y, w, h);
      }
      imlib_updates_free(d->up);
      d->up = NULL;
   }
   D_RETURN_(3);
}

void
geist_document_remove_object(geist_document * d, geist_object * obj)
{
   D_ENTER(3);
   geist_object_dirty(obj);
   geist_layer_remove_object(obj->layer, obj);

   D_RETURN_(3);
}

void
geist_document_dirty_selection(geist_document * doc)
{
   geist_list *l, *list;

   D_ENTER(3);

   list = geist_document_get_selected_list(doc);

   for (l = list; l; l = l->next)
      geist_object_dirty_selection(GEIST_OBJECT(l->data));

   geist_list_free(list);

   D_RETURN_(3);
}

void
geist_document_rename(geist_document * d, char *name)
{
   D_ENTER(3);

   if (d->name)
      efree(d->name);
   d->name = estrdup(name);

   D_RETURN_(3);
}

int
geist_document_save(geist_document * doc, char *filename)
{
   char *ext;

   D_ENTER(3);

   ext = strrchr(filename, '.');
   if (ext)
      ext++;

   if (ext)
   {
      if (!strcasecmp(ext, "xml"))
         geist_document_save_xml(doc, filename);
      else
         geist_document_save_imlib(doc, filename);
   }
   else
      geist_document_save_xml(doc, filename);

   D_RETURN(3, 1);
}

/* TODO Error checking here */
int
geist_document_save_imlib(geist_document * doc, char *filename)
{
   D_ENTER(3);

   /* render all but the selection */
   geist_document_render_full(doc, 0);

   geist_imlib_save_image(doc->im, filename);

   D_RETURN(3, 1);
}

void
geist_document_resize(geist_document * doc, int w, int h)
{
   D_ENTER(3);

   if (w == 0)
      w = 1;
   if (h == 0)
      h = 1;

   if ((w != doc->w) || (h != doc->h))
   {
      doc->w = w;
      doc->h = h;
      if (doc->pmap)
         XFreePixmap(disp, doc->pmap);
      doc->pmap = XCreatePixmap(disp, root, w, h, depth);
      if (doc->im)
         geist_imlib_free_image(doc->im);
      doc->im = imlib_create_image(w, h);
      /* TODO move objects back into document if they are moved off it */
      geist_document_resize_gtk(doc, w,h);
      geist_document_render_full(doc, TRUE);
   }

   D_RETURN_(3);
}
