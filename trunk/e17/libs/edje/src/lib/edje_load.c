#include "Edje.h"
#include "edje_private.h"

static Evas_Hash   *_edje_file_hash = NULL;

static void _edje_collection_free_part_description_free(Edje_Part_Description *desc);
#ifdef EDJE_PROGRAM_CACHE
static int  _edje_collection_free_prog_cache_matches_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);
#endif

/* API Routines */
int
edje_object_file_set(Evas_Object *obj, const char *file, const char *part)
{
   Edje *ed;
   int n;
   
   ed = _edje_fetch(obj);
   if (!ed) return 0;
   if (!file) file = "";
   if (!part) part = "";
   if (((ed->path) && (!strcmp(file, ed->path))) &&
	(ed->part) && (!strcmp(part, ed->part)))
     return 1;
   
   _edje_file_del(ed);
   
   if (ed->path) free(ed->path);
   ed->path = strdup(file);
   if (ed->part) free(ed->part);
   ed->part = strdup(part);
   
   ed->load_error = EDJE_LOAD_ERROR_NONE;
   _edje_file_add(ed);

   if (ed->collection)
     {
	Evas_List *l;
	int errors = 0;
	
	/* check for invalid loops */
	for (l = ed->collection->parts; (l && ! errors); l = l->next)
	  {
	     Edje_Part *ep;
	     Evas_List *hist = NULL;

	     /* Register any color classes in this parts descriptions. */
	     ep = l->data;
	     if ((ep->default_desc) && (ep->default_desc->color_class)) _edje_color_class_member_add(ed, ep->default_desc->color_class);
	     for (hist = ep->other_desc; hist; hist = hist->next)
	       {
		  Edje_Part_Description *desc;

		  desc = hist->data;
		  if (desc->color_class) _edje_color_class_member_add(ed, desc->color_class);
	       }
	     hist = evas_list_append(hist, ep);
	     while (ep->dragable.confine_id >= 0)
	       {
		  ep = evas_list_nth(ed->collection->parts,
				     ep->dragable.confine_id);
		  if (evas_list_find(hist, ep))
		    {
		       printf("EDJE ERROR: confine_to loops. invalidating loop.\n");
		       ep->dragable.confine_id = -1;
		       break;
		    }
		  hist = evas_list_append(hist, ep);
	       }
	     evas_list_free(hist);
	     hist = NULL;
	     hist = evas_list_append(hist, ep);
	     while (ep->clip_to_id >= 0)
	       {
		  ep = evas_list_nth(ed->collection->parts,
				     ep->clip_to_id);
		  if (evas_list_find(hist, ep))
		    {
		       printf("EDJE ERROR: clip_to loops. invalidating loop.\n");
		       ep->clip_to_id = -1;
		       break;
		    }
		  hist = evas_list_append(hist, ep);
	       }
	     evas_list_free(hist);
	     hist = NULL;
	  }
	/* build real parts */
	for (n = 0, l = ed->collection->parts; l; l = l->next, n++)
	  {
	     Edje_Part *ep;
	     Edje_Real_Part *rp;
	     
	     ep = l->data;
	     rp = calloc(1, sizeof(Edje_Real_Part));
	     if (!rp) return 0;
	     rp->part = ep;
	     ed->parts = evas_list_append(ed->parts, rp);
	     rp->param1.description =  ep->default_desc;
	     if (!rp->param1.description)
	       {
		  printf("EDJE ERROR: no default part description!\n");
	       }
	     _edje_text_part_on_add(ed, rp);
	     if (ep->type == EDJE_PART_TYPE_RECTANGLE)
	       rp->object = evas_object_rectangle_add(ed->evas);
	     else if (ep->type == EDJE_PART_TYPE_IMAGE)
	       rp->object = evas_object_image_add(ed->evas);
	     else if (ep->type == EDJE_PART_TYPE_TEXT)
	       {
		  rp->object = evas_object_text_add(ed->evas);
		  evas_object_text_font_source_set(rp->object, ed->path);
	       }
	     else
	       {
		  printf("EDJE ERROR: wrong part type %i!\n", ep->type);
	       }
	     evas_object_smart_member_add(rp->object, ed->obj);
	     if (ep->mouse_events)
	       {
		  evas_object_event_callback_add(rp->object, 
						 EVAS_CALLBACK_MOUSE_IN,
						 _edje_mouse_in_cb,
						 ed);
		  evas_object_event_callback_add(rp->object, 
						 EVAS_CALLBACK_MOUSE_OUT,
						 _edje_mouse_out_cb,
						 ed);
		  evas_object_event_callback_add(rp->object, 
						 EVAS_CALLBACK_MOUSE_DOWN,
						 _edje_mouse_down_cb,
						 ed);
		  evas_object_event_callback_add(rp->object, 
						 EVAS_CALLBACK_MOUSE_UP,
						 _edje_mouse_up_cb,
						 ed);
		  evas_object_event_callback_add(rp->object, 
						 EVAS_CALLBACK_MOUSE_MOVE,
						 _edje_mouse_move_cb,
						 ed);
		  evas_object_event_callback_add(rp->object, 
						 EVAS_CALLBACK_MOUSE_WHEEL,
						 _edje_mouse_wheel_cb,
						 ed);
		  evas_object_data_set(rp->object, "real_part", rp);
		  if (ep->repeat_events)
		    evas_object_repeat_events_set(rp->object, 1);
	       }
	     else
	       evas_object_pass_events_set(rp->object, 1);
	     if (rp->part->clip_to_id < 0)
	       evas_object_clip_set(rp->object, ed->clipper);
	     rp->drag.step.x = ep->dragable.step_x;
	     rp->drag.step.y = ep->dragable.step_y;
	  }
	if (n > 0)
	  {
	     ed->table_parts = malloc(sizeof(Edje_Real_Part *) * n);
	     ed->table_parts_size = n;
	     /* FIXME: check malloc return */
	     n = 0;
	     for (l = ed->parts; l; l = l->next)
	       {
		  Edje_Real_Part *rp;
		  
		  rp = l->data;
		  ed->table_parts[n] = rp;
		  n++;
		  if (rp->param1.description->rel1.id_x >= 0)
		    rp->param1.rel1_to_x = evas_list_nth(ed->parts, rp->param1.description->rel1.id_x);
		  if (rp->param1.description->rel1.id_y >= 0)
		    rp->param1.rel1_to_y = evas_list_nth(ed->parts, rp->param1.description->rel1.id_y);
		  if (rp->param1.description->rel2.id_x >= 0)
		    rp->param1.rel2_to_x = evas_list_nth(ed->parts, rp->param1.description->rel2.id_x);
		  if (rp->param1.description->rel2.id_y >= 0)
		    rp->param1.rel2_to_y = evas_list_nth(ed->parts, rp->param1.description->rel2.id_y);
		  _edje_text_part_on_add_clippers(ed, rp);
		  if (rp->part->clip_to_id >= 0)
		    {
		       rp->clip_to = evas_list_nth(ed->parts, rp->part->clip_to_id);
		       if (rp->clip_to)
			 {
			    evas_object_pass_events_set(rp->clip_to->object, 1);
			    evas_object_clip_set(rp->object, rp->clip_to->object);
			 }
		    }
		  if (rp->part->dragable.confine_id >= 0)
		    rp->confine_to = evas_list_nth(ed->parts, rp->part->dragable.confine_id);
		  
		  rp->swallow_params.min.w = 0;
		  rp->swallow_params.min.w = 0;
		  rp->swallow_params.max.w = -1;
		  rp->swallow_params.max.h = -1;
	       }
	  }
	n = evas_list_count(ed->collection->programs);
	if (n > 0)
	  {
	     ed->table_programs = malloc(sizeof(Edje_Program *) * n);
	     ed->table_programs_size = n;
	     /* FIXME: check malloc return */
	     n = 0;
	     for (l = ed->collection->programs; l; l = l->next)
	       {
		  Edje_Program *pr;
		  
		  pr = l->data;
		  ed->table_programs[n] = pr;
		  n++;
	       }
	  }
	_edje_ref(ed);
	_edje_block(ed);
	_edje_freeze(ed);
	if (ed->collection->script) _edje_embryo_script_init(ed);
	_edje_var_init(ed);
	_edje_emit(ed, "load", "");
	for (l = ed->parts; l; l = l->next)
	  {
	     Edje_Real_Part *rp;
	     
	     rp = l->data;
	     evas_object_show(rp->object);
	     if (_edje_block_break(ed)) break;
	     if (rp->part->dragable.x < 0) rp->drag.val.x = 1.0;
	     if (rp->part->dragable.y < 0) rp->drag.val.x = 1.0;
	     _edje_dragable_pos_set(ed, rp, 1.0, 1.0);
	  }
	ed->dirty = 1;
	if ((ed->parts) && (evas_object_visible_get(obj)))
	  evas_object_show(ed->clipper);
	_edje_recalc(ed);
	_edje_thaw(ed);
	_edje_unblock(ed);
	_edje_unref(ed);
	ed->load_error = EDJE_LOAD_ERROR_NONE;
	return 1;
     }
   else
     {
	return 0;
     }
   ed->load_error = EDJE_LOAD_ERROR_NONE;
   return 1;
}

void
edje_object_file_get(Evas_Object *obj, const char **file, const char **part)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if (!ed)
     {
	if (file) *file = NULL;
	if (part) *part = NULL;
	return;
     }
   if (file) *file = ed->path;
   if (part) *part = ed->part;
}

int
edje_object_load_error_get(Evas_Object *obj)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if (!ed) return EDJE_LOAD_ERROR_NONE;
   return ed->load_error;
}

Evas_List *
edje_file_collection_list(const char *file)
{
   Eet_File *ef = NULL;
   Evas_List *lst = NULL;
   Edje_File *ed_file;
   
   ed_file = evas_hash_find(_edje_file_hash, file);
   if (!ed_file)
     {
	ef = eet_open((char *)file, EET_FILE_MODE_READ);
	if (!ef) return NULL;
	ed_file = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
	if (!ed_file)
	  {
	     eet_close(ef);
	     return NULL;
	  }
	eet_close(ef);
	ed_file->path = strdup(file);
	ed_file->collection_hash = NULL;
	ed_file->references = 1;
	_edje_file_hash = evas_hash_add(_edje_file_hash, ed_file->path, ed_file);
     }
   else
     ed_file->references++;
   if (ed_file->collection_dir)
     {
	Evas_List *l;
	
	for (l = ed_file->collection_dir->entries; l; l = l->next)
	  {
	     Edje_Part_Collection_Directory_Entry *ce;
	     
	     ce = l->data;
	     lst = evas_list_append(lst, strdup(ce->entry));
	  }
     }
   ed_file->references--;   
   if (ed_file->references <= 0) _edje_file_free(ed_file);
   return lst;
}

void
edje_file_collection_list_free(Evas_List *lst)
{
   while (lst)
     {
	if (lst->data) free(lst->data);
	lst = evas_list_remove(lst, lst->data);
     }
}

char *
edje_file_data_get(const char *file, const char *key)
{
   Eet_File *ef = NULL;
   Edje_File *ed_file;
   Evas_List *l;
   char *str = NULL;
   
   ed_file = evas_hash_find(_edje_file_hash, file);
   if (!ed_file)
     {
	ef = eet_open((char *)file, EET_FILE_MODE_READ);
	if (!ef) return NULL;
	ed_file = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
	if (!ed_file)
	  {
	     eet_close(ef);
	     return NULL;
	  }
	eet_close(ef);
	ed_file->path = strdup(file);
	ed_file->collection_hash = NULL;
	ed_file->references = 1;
	_edje_file_hash = evas_hash_add(_edje_file_hash, ed_file->path, ed_file);
     }
   else
     ed_file->references++;
   for (l = ed_file->data; l; l = l->next)
     {
	Edje_Data *di;
	
	di = l->data;
	if (!strcmp(di->key, key))
	  {
	     str = strdup(di->value);
	     break;
	  }
     }
   ed_file->references--;   
   if (ed_file->references <= 0) _edje_file_free(ed_file);
   return str;
}

void
_edje_file_add(Edje *ed)
{
   Eet_File *ef = NULL;
   Evas_List *l;
   int id = -1;

   ed->file = evas_hash_find(_edje_file_hash, ed->path);
   if (ed->file)
     ed->file->references++;
   else
     {
	ef = eet_open(ed->path, EET_FILE_MODE_READ);
	if (!ef)
	  {
	     ed->load_error = EDJE_LOAD_ERROR_UNKNOWN_FORMAT;
	     return;
	  }
   
	ed->file = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
	if (!ed->file)
	  {
	     ed->load_error = EDJE_LOAD_ERROR_CORRUPT_FILE;
	     goto out;
	  }

	if (ed->file->version != EDJE_FILE_VERSION)
	  {
	    _edje_file_free(ed->file);
	    ed->file = NULL;
	    ed->load_error = EDJE_LOAD_ERROR_INCOMPATIBLE_FILE;
	    goto out;
	  }

	ed->file->references = 1;   
	ed->file->path = strdup(ed->path);
	if (!ed->file->collection_dir)
	  {
	     _edje_file_free(ed->file);
	     ed->file = NULL;
	     ed->load_error = EDJE_LOAD_ERROR_CORRUPT_FILE;
	     goto out;
	  }
	_edje_file_hash = evas_hash_add(_edje_file_hash, ed->file->path, ed->file);
     }
   
   ed->collection = evas_hash_find(ed->file->collection_hash, ed->part);
   if (ed->collection)
     {
	ed->collection->references++;
     }
   else
     {
	for (l = ed->file->collection_dir->entries; l; l = l->next)
	  {
	     Edje_Part_Collection_Directory_Entry *ce;
	     
	     ce = l->data;
	     if ((ce->entry) && (!strcmp(ce->entry, ed->part)))
	       {
		  id = ce->id;
		  break;
	       }
	  }
	if (id >= 0)
	  {
	     char buf[256];
	     int  size;
	     void *data;
	     
	     snprintf(buf, sizeof(buf), "collections/%i", id);
	     if (!ef) ef = eet_open(ed->path, EET_FILE_MODE_READ);
	     if (!ef)
	       {
		  ed->load_error = EDJE_LOAD_ERROR_CORRUPT_FILE;
		  goto out;
	       }
	     ed->collection = eet_data_read(ef, 
					    _edje_edd_edje_part_collection, 
					    buf);
	     if (!ed->collection)
	       {
		  ed->load_error = EDJE_LOAD_ERROR_CORRUPT_FILE;
		  goto out;
	       }
	     ed->collection->references = 1;
	     ed->file->collection_hash = evas_hash_add(ed->file->collection_hash, ed->part, ed->collection);
	     snprintf(buf, sizeof(buf), "scripts/%i", id);
	     data = eet_read(ef, buf, &size);
	     if (data)
	       {
		  ed->collection->script = embryo_program_new(data, size);
		  free(data);
	       }
	  }
	else
	  {
	     _edje_file_free(ed->file);
	     ed->file = NULL;
	     ed->load_error = EDJE_LOAD_ERROR_CORRUPT_FILE;	     
	  }
     }
   out:
   if (ef) eet_close(ef);
}

void
_edje_file_del(Edje *ed)
{
   _edje_emit(ed, NULL, NULL); /* clear out signal emissions */
   ed->dont_clear_signals = 1;
   _edje_block_violate(ed);
   _edje_var_shutdown(ed);
   if (ed->collection)
     {
	ed->collection->references--;
	if (ed->collection->references <= 0)
	  {
	     _edje_embryo_script_shutdown(ed);
	     ed->file->collection_hash = evas_hash_del(ed->file->collection_hash, ed->part, ed->collection);
	     _edje_collection_free(ed, ed->collection);
	  }
	ed->collection = NULL;
     }
   if (ed->file)
     {
	_edje_file_free(ed->file);
	ed->file = NULL;
     }
   if (ed->parts)
     {
	while (ed->parts)
	  {
	     Edje_Real_Part *rp;

	     rp = ed->parts->data;
	     ed->parts = evas_list_remove(ed->parts, rp);
	     evas_object_event_callback_del(rp->object, 
					    EVAS_CALLBACK_MOUSE_IN,
					    _edje_mouse_in_cb);
	     evas_object_event_callback_del(rp->object, 
					    EVAS_CALLBACK_MOUSE_OUT,
					    _edje_mouse_out_cb);
	     evas_object_event_callback_del(rp->object, 
					    EVAS_CALLBACK_MOUSE_DOWN,
					    _edje_mouse_down_cb);
	     evas_object_event_callback_del(rp->object, 
					    EVAS_CALLBACK_MOUSE_UP,
					    _edje_mouse_up_cb);
	     evas_object_event_callback_del(rp->object, 
					    EVAS_CALLBACK_MOUSE_MOVE,
					    _edje_mouse_move_cb);
	     evas_object_event_callback_del(rp->object, 
					    EVAS_CALLBACK_MOUSE_WHEEL,
					    _edje_mouse_wheel_cb);
	     _edje_text_real_part_on_del(ed, rp);
	     evas_object_del(rp->object);
	     if (rp->swallowed_object)
	       {
		  evas_object_smart_member_del(rp->swallowed_object);
		  evas_object_event_callback_del(rp->swallowed_object,
						 EVAS_CALLBACK_FREE,
						 _edje_object_part_swallow_free_cb);
		  evas_object_clip_unset(rp->swallowed_object);
		  rp->swallowed_object = NULL;
/* I think it would be better swallowed objects dont get deleted */
/*		  evas_object_del(rp->swallowed_object);*/
	       }
	     if (rp->text.text) free(rp->text.text);
	     if (rp->text.font) free(rp->text.font);
	     if (rp->text.cache.in_str) free(rp->text.cache.in_str);
	     if (rp->text.cache.out_str) free(rp->text.cache.out_str);	     
	     free(rp);
	  }
	ed->parts = NULL;
     }
   if (ed->actions)
     {
	while (ed->actions)
	  {
	     Edje_Running_Program *runp;
	     
	     _edje_anim_count--;
	     runp = ed->actions->data;
	     ed->actions = evas_list_remove(ed->actions, runp);
	     free(runp);
	  }
     }
   _edje_animators = evas_list_remove(_edje_animators, ed);
   if (ed->pending_actions)
     {
	while (ed->pending_actions)
	  {
	     Edje_Pending_Program *pp;
	     
	     pp = ed->pending_actions->data;
	     ed->pending_actions = evas_list_remove(ed->pending_actions, pp);
	     ecore_timer_del(pp->timer);
	     free(pp);
	  }
     }
   if (ed->table_parts) free(ed->table_parts);
   ed->table_parts = NULL;
   ed->table_parts_size = 0;
   if (ed->table_programs) free(ed->table_programs);
   ed->table_programs = NULL;
   ed->table_programs_size = 0;
}

void
_edje_file_free(Edje_File *edf)
{
   edf->references--;
   if (edf->references > 0) return;
   
   _edje_file_hash = evas_hash_del(_edje_file_hash, edf->path, edf);
   
   if (edf->path) free(edf->path);
   if (edf->image_dir)
     {
	while (edf->image_dir->entries)
	  {
	     Edje_Image_Directory_Entry *ie;
	     
	     ie = edf->image_dir->entries->data;
	     edf->image_dir->entries = 
	       evas_list_remove(edf->image_dir->entries, ie);
	     if (ie->entry) free(ie->entry);
	     free(ie);
	  }
	free(edf->image_dir);
     }
   if (edf->collection_dir)
     {
	while (edf->collection_dir->entries)
	  {
	     Edje_Part_Collection_Directory_Entry *ce;
	     
	     ce = edf->collection_dir->entries->data;
	     edf->collection_dir->entries = 
	       evas_list_remove(edf->collection_dir->entries, ce);
	     if (ce->entry) free(ce->entry);
	     free(ce);
	  }
	free(edf->collection_dir);
     }
   if (edf->collection_hash) evas_hash_free(edf->collection_hash);
   free(edf);
}

void
_edje_collection_free(Edje *ed, Edje_Part_Collection *ec)
{
   while (ec->programs)
     {
	Edje_Program *pr;

	pr = ec->programs->data;
	ec->programs = evas_list_remove(ec->programs, pr);
	if (pr->name) free(pr->name);
	if (pr->signal) free(pr->signal);
	if (pr->source) free(pr->source);
	if (pr->state) free(pr->state);
	if (pr->state2) free(pr->state2);
	while (pr->targets)
	  {
	     Edje_Program_Target *prt;
	     
	     prt = pr->targets->data;
	     pr->targets = evas_list_remove(pr->targets, prt);
	     free(prt);
	  }
	while (pr->after)
	  {
	     Edje_Program_After *pa;

	     pa = pr->after->data;
	     pr->after = evas_list_remove(pr->after, pa);
		 free(pa);
	  }
	free(pr);
     }
   while (ec->parts)
     {
	Edje_Part *ep;

	ep = ec->parts->data;
	ec->parts = evas_list_remove(ec->parts, ep);
	_edje_text_part_on_del(ed, ep);
	_edje_color_class_on_del(ed, ep);
	if (ep->name) free(ep->name);
	if (ep->default_desc)
	  {
	     _edje_collection_free_part_description_free(ep->default_desc);
	     ep->default_desc = NULL;
	  }
	while (ep->other_desc)
	  {
	     Edje_Part_Description *desc;
	     
	     desc = ep->other_desc->data;
	     ep->other_desc = evas_list_remove(ep->other_desc, desc);
	     _edje_collection_free_part_description_free(desc);
	  }
	free(ep);
     }
#ifdef EDJE_PROGRAM_CACHE
   if (ec->prog_cache.no_matches) evas_hash_free(ec->prog_cache.no_matches);
   if (ec->prog_cache.matches)
     {
	evas_hash_foreach(ec->prog_cache.matches, 
			  _edje_collection_free_prog_cache_matches_free_cb, 
			  NULL);
	evas_hash_free(ec->prog_cache.matches);
     }
#endif   
   free(ec);
}

static void
_edje_collection_free_part_description_free(Edje_Part_Description *desc)
{
   if (desc->state.name) free(desc->state.name);
   while (desc->image.tween_list)
     {
	Edje_Part_Image_Id *pi;
	
	pi = desc->image.tween_list->data;
	desc->image.tween_list = evas_list_remove(desc->image.tween_list, pi);
	free(pi);
     }
   if (desc->color_class)     free(desc->color_class);
   if (desc->text.text)       free(desc->text.text);
   if (desc->text.text_class) free(desc->text.text_class);
   if (desc->text.font)       free(desc->text.font);
   free(desc);
}

#ifdef EDJE_PROGRAM_CACHE
static int
_edje_collection_free_prog_cache_matches_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   evas_list_free((Evas_List *)data);
   return 1;
   hash = NULL;
   fdata = NULL;
}
#endif
