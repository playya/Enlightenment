#include "Edje.h"
#include "edje_private.h"

static void _edje_emit_cb(Edje *ed, char *sig, char *src);

static double       _edje_frametime = 1.0 / 60.0;

int          _edje_anim_count = 0;
Ecore_Timer *_edje_timer = NULL;
Evas_List   *_edje_animators = NULL;

/************************** API Routines **************************/

/* FIXDOC: Expand */
/** Set the frametime
 * @param t The frametime
 *
 * Sets the frametime in seconds, by default this is 1/60.
 */
void
edje_frametime_set(double t)
{
   if (t == _edje_frametime) return;
   _edje_frametime = t;
   if (_edje_timer)
     {
	ecore_timer_del(_edje_timer);
	_edje_timer = ecore_timer_add(_edje_frametime, _edje_timer_cb, NULL);
     }
   _edje_var_anim_frametime_reset();
}

/* FIXDOC: Expand */
/** Get the frametime
 * @return The frametime
 *
 * Returns the frametime in seconds, by default this is 1/60.
 */
double
edje_frametime_get(void)
{
   return _edje_frametime;
}

/* FIXDOC: Expand */
/** Adds a callback for the object.
 * @param obj A valid Evas_Object handle
 * @param emission Signal to activate callback FIXDOC: Naming Convention?
 * @param source Source of signal
 * @param func The function to be executed when the callback is signaled
 *
 * Creates a callback for the object to execute the given function.
 */
void
edje_object_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source), void *data)
{
   Edje *ed;
   Edje_Signal_Callback *escb;
   
   if ((!emission) || (!source) || (!func)) return;
   ed = _edje_fetch(obj);
   if (!ed) return;
   if (ed->delete_me) return;
   escb = calloc(1, sizeof(Edje_Signal_Callback));
   escb->signal = strdup(emission);
   escb->source = strdup(source);
   escb->func = func;
   escb->data = data;
   ed->callbacks = evas_list_append(ed->callbacks, escb);
   if (ed->walking_callbacks)
     {
	escb->just_added = 1;
	ed->just_added_callbacks = 1;
     }
}

/* FIXDOC: Expand */
/** Delete an object's callback
 * @param obj A valid Evas_Object handle
 * @param emission ? FIXDOC
 * @param source ? FIXDOC
 *
 * Deletes an existing callback
 */
void *
edje_object_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source))
{
   Edje *ed;
   Evas_List *l;
   
   if ((!emission) || (!source) || (!func)) return NULL;
   ed = _edje_fetch(obj);
   if (!ed) return NULL;
   if (ed->delete_me) return NULL;
   for (l = ed->callbacks; l; l = l->next)
     {
	Edje_Signal_Callback *escb;
	
	escb = l->data;
	if ((escb->func == func) && 
	    (!strcmp(escb->signal, emission)) &&
	    (!strcmp(escb->source, source)))
	  {
	     void *data;
	     
	     data = escb->data;
	     if (ed->walking_callbacks)
	       {
		  escb->delete_me = 1;
		  ed->delete_callbacks = 1;
	       }
	     else
	       {
		  ed->callbacks = evas_list_remove_list(ed->callbacks, l);
		  free(escb->signal);
		  free(escb->source);
		  free(escb);
	       }
	     return data;
	  }
     }
   return NULL;
}

/* FIXDOC: Verify/Expand */
/** Send a signal to the Edje
 * @param A vaild Evas_Object handle
 * @param emission The signal
 * @param source The signal source
 *
 * This sends a signal to the edje.  These are defined in the programs section of an edc.
 */
void
edje_object_signal_emit(Evas_Object *obj, const char *emission, const char *source)
{
   Edje *ed;

   if ((!emission) || (!source)) return;
   ed = _edje_fetch(obj);
   if (!ed) return;
   if (ed->delete_me) return;
   _edje_emit(ed, (char *)emission, (char *)source);
}

/* FIXDOC: Verify/Expand */
/** Set the Edje to play or pause
 * @param obj A vaild Evas_Object handle
 * @param play Play instruction (1 to play, 0 to pause)
 *
 * This sets the Edje to play or pause depending on the parameter.  This has no effect if the Edje is already in that state.
 */
void
edje_object_play_set(Evas_Object *obj, int play)
{
   Edje *ed;
   double t;
   Evas_List *l;
   
   ed = _edje_fetch(obj);
   if (!ed) return;
   if (ed->delete_me) return;
   if (play)
     {
	if (!ed->paused) return;
	ed->paused = 0;
	t = ecore_time_get() - ed->paused_at;
	for (l = ed->actions; l; l = l->next)
	  {
	     Edje_Running_Program *runp;
	     
	     runp = l->data;
	     runp->start_time += t;
	  }
     }
   else
     {
	if (ed->paused) return;
	ed->paused = 1;
	ed->paused_at = ecore_time_get();
     }
}

/* FIXDOC: Verify/Expand */
/** Get the Edje play/pause state
 * @param obj A valid Evas_Object handle
 * @return 0 if Edje not connected, Edje delete_me, or Edje paused\n
 * 1 if Edje set to play
 */
int 
edje_object_play_get(Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   if (ed->delete_me) return 0;
   if (ed->paused) return 0;
   return 1;
}

/* FIXDOC: Verify/Expand */
/** Set Animation state
 * @param obj A valid Evas_Object handle
 * @param on Animation State
 *
 * Stop or start an Edje animation.
 */
void
edje_object_animation_set(Evas_Object *obj, int on)
{
   Edje *ed;
   Evas_List *l;
   
   ed = _edje_fetch(obj);
   if (!ed) return;   
   if (ed->delete_me) return;
   _edje_block(ed);
   ed->no_anim = !on;
   _edje_freeze(ed);
   if (!on)
     {
	Evas_List *newl = NULL;
	
	for (l = ed->actions; l; l = l->next)
	  newl = evas_list_append(newl, l->data);
	while (newl)
	  {
	     Edje_Running_Program *runp;
	     
	     runp = newl->data;
	     newl = evas_list_remove(newl, newl->data);
	     _edje_program_run_iterate(runp, runp->start_time + runp->program->tween.time);
	     if (_edje_block_break(ed))
	       {
		  evas_list_free(newl);
		  goto break_prog;
	       }
	  }
     }
   else
     {
	_edje_emit(ed, "load", "");	
	if (evas_object_visible_get(obj))
	  {
	     evas_object_hide(obj);
	     evas_object_show(obj);
	  }
     }
   break_prog:
   _edje_thaw(ed);
   _edje_unblock(ed);
}

/* FIXDOC: Verify/Expand */
/** Get the animation state
 * @param obj A valid Evas_Object handle
 * @return 0 on Error or if not animated\n
 * 1 if animated
 */
int
edje_object_animation_get(Evas_Object *obj)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if (!ed) return 0;
   if (ed->delete_me) return 0;
   if (ed->no_anim) return 0;
   return 1;
}

/* Private Routines */

int
_edje_program_run_iterate(Edje_Running_Program *runp, double tim)
{
   double t, total;
   Evas_List *l;
   Edje *ed;

   ed = runp->edje;
   if (ed->delete_me) return 0;
   _edje_block(ed);
   _edje_ref(ed);
   _edje_freeze(ed);
   t = tim - runp->start_time;
   total = runp->program->tween.time;
   t /= total;
   if (t > 1.0) t = 1.0;
   for (l = runp->program->targets; l; l = l->next)
     {
	Edje_Real_Part *rp;
	Edje_Program_Target *pt;
	
	pt = l->data;
	if (pt->id >= 0)
	  {
	     rp = ed->table_parts[pt->id % ed->table_parts_size];
	     if (rp) _edje_part_pos_set(ed, rp, 
					runp->program->tween.mode, t);
	  }
     }
   if (t >= 1.0)
     {
	for (l = runp->program->targets; l; l = l->next)
	  {
	     Edje_Real_Part *rp;
	     Edje_Program_Target *pt;
	     
	     pt = l->data;
	     if (pt->id >= 0)
	       {
		  rp = ed->table_parts[pt->id % ed->table_parts_size];
		  if (rp)
		    {
		       _edje_part_description_apply(ed, rp, 
						    runp->program->state, 
						    runp->program->value,
						    NULL,
						    0.0);
		       _edje_part_pos_set(ed, rp, 
					  runp->program->tween.mode, 0.0);
		       rp->program = NULL;
		    }
	       }
	  }
	_edje_recalc(ed);
	runp->delete_me = 1;
	if (!ed->walking_actions)
	  {
	     _edje_anim_count--;
	     ed->actions = evas_list_remove(ed->actions, runp);
	     if (!ed->actions)
	       _edje_animators = evas_list_remove(_edje_animators, ed);
	  }
	_edje_emit(ed, "program,stop", runp->program->name);
	if (_edje_block_break(ed))
	  {
	     if (!ed->walking_actions) free(runp);
	     goto break_prog;
	  }
	for (l = runp->program->after; l; l = l->next)
	  {
	     Edje_Program *pr;
	     Edje_Program_After *pa = l->data;
	     
	     if (pa->id >= 0)
	       {
		  pr = ed->table_programs[pa->id % ed->table_programs_size];
		  if (pr) _edje_program_run(ed, pr, 0, "", "");
		  if (_edje_block_break(ed))
		    {
		       if (!ed->walking_actions) free(runp);
		       goto break_prog;
		    }
	       }
	  }
	_edje_thaw(ed);
	_edje_unref(ed);
	if (!ed->walking_actions) free(runp);
	_edje_unblock(ed);
	return  0;
     }
   break_prog:
   _edje_recalc(ed);
   _edje_thaw(ed);
   _edje_unref(ed);
   _edje_unblock(ed);
   return 1;
}

void
_edje_program_end(Edje *ed, Edje_Running_Program *runp)
{
   Evas_List *l;
   char *pname = NULL;

   if (ed->delete_me) return;
   _edje_ref(ed);
   _edje_freeze(ed);
   for (l = runp->program->targets; l; l = l->next)
     {
	Edje_Real_Part *rp;
	Edje_Program_Target *pt;
	
	pt = l->data;
	if (pt->id >= 0)
	  {
	     rp = ed->table_parts[pt->id % ed->table_parts_size];
	     if (rp)
	       {
		  _edje_part_description_apply(ed, rp, 
					       runp->program->state, 
					       runp->program->value,
					       NULL,
					       0.0);
		  _edje_part_pos_set(ed, rp, 
				     runp->program->tween.mode, 0.0);
		  rp->program = NULL;
	       }
	  }
     }
   _edje_recalc(ed);
   runp->delete_me = 1;
   pname = runp->program->name;
   if (!ed->walking_actions)
     {
	_edje_anim_count--;
	ed->actions = evas_list_remove(ed->actions, runp);
	free(runp);
	if (!ed->actions)
	  {
	     _edje_animators = evas_list_remove(_edje_animators, ed);
	  }
     }
   _edje_emit(ed, "program,stop", pname);
   _edje_thaw(ed);
   _edje_unref(ed);   
}
   
void
_edje_program_run(Edje *ed, Edje_Program *pr, int force, char *ssig, char *ssrc)
{
   Evas_List *l;
   /* limit self-feeding loops in programs to 64 levels */
   static int recursions = 0;
   static int recursion_limit = 0;

   if (ed->delete_me) return;
   if ((pr->in.from > 0.0) && (pr->in.range >= 0.0) && (!force))
     {
	Edje_Pending_Program *pp;
	double r = 0.0;
	
	pp = calloc(1, sizeof(Edje_Pending_Program));
	if (!pp) return;
	if (pr->in.range > 0.0) r = ((double)rand() / RAND_MAX);
	pp->timer = ecore_timer_add(pr->in.from + (pr->in.range * r), 
				    _edje_pending_timer_cb, pp);
	if (!pp->timer)
	  {
	     free(pp);
	     return;
	  }
	pp->edje = ed;
	pp->program = pr;
	ed->pending_actions = evas_list_append(ed->pending_actions, pp);
	return;
     }
   if ((recursions >= 64) || (recursion_limit))
     {
	printf("EDJE ERROR: programs recursing up to recursion limit of %i. Disabled.\n",
	       64);
	recursion_limit = 1;
	return;
     }
   recursions++;
   _edje_block(ed);
   _edje_ref(ed);
   _edje_freeze(ed);
   if (pr->action == EDJE_ACTION_TYPE_STATE_SET)
     {
	if ((pr->tween.time > 0.0) && (!ed->no_anim))
	  {
	     Edje_Running_Program *runp;
	     
	     runp = calloc(1, sizeof(Edje_Running_Program));
	     for (l = pr->targets; l; l = l->next)
	       {
		  Edje_Real_Part *rp;
		  Edje_Program_Target *pt;
		  
		  pt = l->data;
		  if (pt->id >= 0)
		    {
		       rp = ed->table_parts[pt->id % ed->table_parts_size];
		       if (rp)
			 {
			    if (rp->program)
			      _edje_program_end(ed, rp->program);
			    _edje_part_description_apply(ed, rp, 
							 rp->param1.description->state.name,
							 rp->param1.description->state.value, 
							 pr->state, 
							 pr->value);
			    _edje_part_pos_set(ed, rp, pr->tween.mode, 0.0);
			    rp->program = runp;
			 }
		    }
	       }
	     _edje_emit(ed, "program,start", pr->name);
	     if (_edje_block_break(ed))
	       {
		  ed->actions = evas_list_append(ed->actions, runp);
		  goto break_prog;
	       }
	     if (!ed->actions)
	       _edje_animators = evas_list_append(_edje_animators, ed);
	     ed->actions = evas_list_append(ed->actions, runp);
	     runp->start_time = ecore_time_get();
	     runp->edje = ed;
	     runp->program = pr;
	     if (!_edje_timer)
	       _edje_timer = ecore_timer_add(_edje_frametime, _edje_timer_cb, NULL);
	     _edje_anim_count++;
	  }
	else
	  {
	     for (l = pr->targets; l; l = l->next)
	       {
		  Edje_Real_Part *rp;
		  Edje_Program_Target *pt;
		  
		  pt = l->data;
		  if (pt->id >= 0)
		    {
		       rp = ed->table_parts[pt->id % ed->table_parts_size];
		       if (rp)
			 {
			    if (rp->program)
			      _edje_program_end(ed, rp->program);
			    _edje_part_description_apply(ed, rp, 
							 pr->state, 
							 pr->value,
							 NULL,
							 0.0);
			    _edje_part_pos_set(ed, rp, pr->tween.mode, 0.0);
			 }
		    }
	       }
	     _edje_emit(ed, "program,start", pr->name);
	     if (_edje_block_break(ed)) goto break_prog;
	     _edje_emit(ed, "program,stop", pr->name);
	     if (_edje_block_break(ed)) goto break_prog;

	     for (l = pr->after; l; l = l->next)
	       {
		  Edje_Program *pr2;
		  Edje_Program_After *pa = l->data;
		 
		  if (pa->id >= 0)
		    {
		       pr2 = ed->table_programs[pa->id % ed->table_programs_size];
		       if (pr2) _edje_program_run(ed, pr2, 0, "", "");
		       if (_edje_block_break(ed)) goto break_prog;
		    }
	       }
	     _edje_recalc(ed);
	  }
     }
   else if (pr->action == EDJE_ACTION_TYPE_ACTION_STOP)
     {
	_edje_emit(ed, "program,start", pr->name);
	for (l = pr->targets; l; l = l->next)
	  {
	     Edje_Program_Target *pt;
	     Evas_List *ll;
	     
	     pt = l->data;
	     for (ll = ed->actions; ll; ll = ll->next)
	       {
		  Edje_Running_Program *runp;
		  
		  runp = ll->data;
		  if (pt->id == runp->program->id)
		    {
		       _edje_program_end(ed, runp);
		       goto done;
		    }
	       }
	     for (ll = ed->pending_actions; ll; ll = ll->next)
	       {
		  Edje_Pending_Program *pp;
		  
		  pp = ll->data;
		  if (pt->id == pp->program->id)
		    {
		       ed->pending_actions = evas_list_remove(ed->pending_actions, pp);
		       ecore_timer_del(pp->timer);
		       free(pp);
		       goto done;
		    }
	       }
	     done:
	        continue;
	  }
	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
     }
   else if (pr->action == EDJE_ACTION_TYPE_SIGNAL_EMIT)
     {
	_edje_emit(ed, "program,start", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	_edje_emit(ed, pr->state, pr->state2);
	if (_edje_block_break(ed)) goto break_prog;
	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
     }
   else if (pr->action == EDJE_ACTION_TYPE_DRAG_VAL_SET)
     {
	_edje_emit(ed, "program,start", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	for (l = pr->targets; l; l = l->next)
	  {
	     Edje_Real_Part *rp;
	     Edje_Program_Target *pt;
	     
	     pt = l->data;
	     if (pt->id >= 0)
	       {
		  rp = ed->table_parts[pt->id % ed->table_parts_size];
		  if ((rp) && (rp->drag.down.count == 0))
		    {
		       rp->drag.val.x = pr->value;
		       rp->drag.val.y = pr->value2;
		       if      (rp->drag.val.x < 0.0) rp->drag.val.x = 0.0;
		       else if (rp->drag.val.x > 1.0) rp->drag.val.x = 1.0;
		       if      (rp->drag.val.y < 0.0) rp->drag.val.y = 0.0;
		       else if (rp->drag.val.y > 1.0) rp->drag.val.y = 1.0;
		       _edje_dragable_pos_set(ed, rp, rp->drag.val.x, rp->drag.val.y);
		       _edje_emit(ed, "drag,set", rp->part->name);
		       if (_edje_block_break(ed)) goto break_prog;
		    }
	       }
	  }
	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
     }
   else if (pr->action == EDJE_ACTION_TYPE_DRAG_VAL_STEP)
     {
	_edje_emit(ed, "program,start", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	for (l = pr->targets; l; l = l->next)
	  {
	     Edje_Real_Part *rp;
	     Edje_Program_Target *pt;
	     
	     pt = l->data;
	     if (pt->id >= 0)
	       {
		  rp = ed->table_parts[pt->id % ed->table_parts_size];
		  if ((rp) && (rp->drag.down.count == 0))
		    {
		       rp->drag.val.x += pr->value * rp->drag.step.x * rp->part->dragable.x;
		       rp->drag.val.y += pr->value2 * rp->drag.step.y * rp->part->dragable.y;
		       if      (rp->drag.val.x < 0.0) rp->drag.val.x = 0.0;
		       else if (rp->drag.val.x > 1.0) rp->drag.val.x = 1.0;
		       if      (rp->drag.val.y < 0.0) rp->drag.val.y = 0.0;
		       else if (rp->drag.val.y > 1.0) rp->drag.val.y = 1.0;
		       _edje_dragable_pos_set(ed, rp, rp->drag.val.x, rp->drag.val.y);
		       _edje_emit(ed, "drag,step", rp->part->name);
		       if (_edje_block_break(ed)) goto break_prog;
		    }
	       }
	  }
	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
     }
   else if (pr->action == EDJE_ACTION_TYPE_DRAG_VAL_PAGE)
     {
	_edje_emit(ed, "program,start", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	for (l = pr->targets; l; l = l->next)
	  {
	     Edje_Real_Part *rp;
	     Edje_Program_Target *pt;
	     
	     pt = l->data;
	     if (pt->id >= 0)
	       {
		  rp = ed->table_parts[pt->id % ed->table_parts_size];
		  if ((rp) && (rp->drag.down.count == 0))
		    {
		       rp->drag.val.x += pr->value * rp->drag.page.x * rp->part->dragable.x;
		       rp->drag.val.y += pr->value2 * rp->drag.step.y * rp->part->dragable.y;
		       if      (rp->drag.val.x < 0.0) rp->drag.val.x = 0.0;
		       else if (rp->drag.val.x > 1.0) rp->drag.val.x = 1.0;
		       if      (rp->drag.val.y < 0.0) rp->drag.val.y = 0.0;
		       else if (rp->drag.val.y > 1.0) rp->drag.val.y = 1.0;
		       _edje_dragable_pos_set(ed, rp, rp->drag.val.x, rp->drag.val.y);
		       _edje_emit(ed, "drag,page", rp->part->name);
		       if (_edje_block_break(ed)) goto break_prog;
		    }
	       }
	  }
	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
     }
   else if (pr->action == EDJE_ACTION_TYPE_SCRIPT)
     {
	char fname[128];
	
	_edje_emit(ed, "program,start", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	snprintf(fname, sizeof(fname), "_p%i", pr->id);
	_edje_embryo_test_run(ed, fname, ssig, ssrc);
	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	_edje_recalc(ed);
     }
   else
     {
	_edje_emit(ed, "program,start", pr->name);
	_edje_emit(ed, "program,stop", pr->name);
     }
   if (!((pr->action == EDJE_ACTION_TYPE_STATE_SET) 
	 /* hmm this fucks somethgin up. must look into it later */
	 /* && (pr->tween.time > 0.0) && (!ed->no_anim))) */
	 ))
     {
	for (l= pr->after; l; l = l->next)
	  {
	     Edje_Program *pr2;
	     Edje_Program_After *pa = l->data;
	     
	     if (pa->id >= 0)
	       {
		  pr2 = ed->table_programs[pa->id % ed->table_programs_size];
		  if (pr2) _edje_program_run(ed, pr2, 0, "", "");
		  if (_edje_block_break(ed)) goto break_prog;
	       }
	  }
     }
   break_prog:
   _edje_thaw(ed);
   _edje_unref(ed);
   recursions--;
   if (recursions == 0) recursion_limit = 0;
   _edje_unblock(ed);
}

void
_edje_emit(Edje *ed, char *sig, char *src)
{
   Edje_Message_Signal emsg;
   
   if (ed->delete_me) return;
   emsg.sig = sig;
   emsg.src = src;
   _edje_message_send(ed, EDJE_QUEUE_SCRIPT, EDJE_MESSAGE_SIGNAL, 0, &emsg);
}

/* FIXME: what if we delete the evas object??? */
void
_edje_emit_handle(Edje *ed, char *sig, char *src)
{
   Evas_List *l;

   if (ed->delete_me) return;
//   printf("EDJE EMIT: signal: \"%s\" source: \"%s\"\n", sig, src);
   _edje_block(ed);
   _edje_ref(ed);
   _edje_freeze(ed);
   if (ed->collection)
     {
	Edje_Part_Collection *ec;
#ifdef EDJE_PROGRAM_CACHE
	char *tmps;
	int l1, l2;
#endif	     
	int done;
	
	ec = ed->collection;
#ifdef EDJE_PROGRAM_CACHE
	l1 = strlen(sig);
	l2 = strlen(src);
	tmps = malloc(l1 + l2 + 2);
	
	if (tmps)
	  {
	     strcpy(tmps, sig);
	     tmps[l1] = '\377';
	     strcpy(&(tmps[l1 + 1]), src);
	  }
#endif	     
	done = 0;
	
#ifdef EDJE_PROGRAM_CACHE
	if (tmps)
	  {
	     Evas_List *matches;
	     
	     if (evas_hash_find(ec->prog_cache.no_matches, tmps))
	       {
		  done = 1;
	       }
	     else if ((matches = evas_hash_find(ec->prog_cache.matches, tmps)))
	       {
		  for (l = matches; l; l = l->next)
		    {
		       Edje_Program *pr;
		       
		       pr = l->data;
		       _edje_program_run(ed, pr, 0, sig, src);
		       if (_edje_block_break(ed))
			 {
			    if (tmps) free(tmps);
			    goto break_prog;
			 }
		    }
		  done = 1;
	       }
	  }
#endif
	if (!done)
	  {
#ifdef EDJE_PROGRAM_CACHE
	     int matched = 0;
	     Evas_List *matches = NULL;
#endif
	     
	     for (l = ed->collection->programs; l; l = l->next)
	       {
		  Edje_Program *pr;
		  
		  pr = l->data;
		  if ((pr->signal) &&
		      (pr->source) &&
		      (_edje_glob_match(sig, pr->signal)) &&
		      (_edje_glob_match(src, pr->source)))
		    {
#ifdef EDJE_PROGRAM_CACHE
		       matched++;
#endif			    
		       _edje_program_run(ed, pr, 0, sig, src);
		       if (_edje_block_break(ed))
			 {
#ifdef EDJE_PROGRAM_CACHE
			    if (tmps) free(tmps);
			    evas_list_free(matches);
#endif				 
			    goto break_prog;
			 }
#ifdef EDJE_PROGRAM_CACHE
		       matches = evas_list_append(matches, pr);
#endif			    
		    }
	       }
#ifdef EDJE_PROGRAM_CACHE
	     if (tmps)
	       {
		  if (matched == 0)
		    ec->prog_cache.no_matches = 
		    evas_hash_add(ec->prog_cache.no_matches, tmps, ed);
		  else
		    ec->prog_cache.matches =
		    evas_hash_add(ec->prog_cache.matches, tmps, matches);
	       }
#endif		    
	  }
	_edje_emit_cb(ed, sig, src);
	if (_edje_block_break(ed))
	  {
#ifdef EDJE_PROGRAM_CACHE
	     if (tmps) free(tmps);
#endif		  
	     goto break_prog;
	  }
#ifdef EDJE_PROGRAM_CACHE
	if (tmps) free(tmps);
	tmps = NULL;
#endif	     
     }
   break_prog:
   _edje_thaw(ed);
   _edje_unref(ed);
   _edje_unblock(ed);
}

/* FIXME: what if we delete the evas object??? */
static void
_edje_emit_cb(Edje *ed, char *sig, char *src)
{
   Evas_List *l;
   
   if (ed->delete_me) return;
   _edje_ref(ed);
   _edje_freeze(ed);   
   _edje_block(ed);
   ed->walking_callbacks = 1;
   for (l = ed->callbacks; l; l = l->next)
     {
	Edje_Signal_Callback *escb;
	
	escb = l->data;
	if ((!escb->just_added) &&
	    (!escb->delete_me) &&
	    (_edje_glob_match(sig, escb->signal)) &&
	    (_edje_glob_match(src, escb->source)))
	  escb->func(escb->data, ed->obj, sig, src);
	if (_edje_block_break(ed)) goto break_prog;
     }
   ed->walking_callbacks = 0;
   if ((ed->delete_callbacks) || (ed->just_added_callbacks))
     {
	ed->delete_callbacks = 0;
	ed->just_added_callbacks = 0;
	for (l = ed->callbacks; l;)
	  {
	     Edje_Signal_Callback *escb;
	     Evas_List *next_l;
	     
	     escb = l->data;		       
	     next_l = l->next;
	     if (escb->just_added)
	       escb->just_added = 0;
	     if (escb->delete_me)
	       {
		  ed->callbacks = evas_list_remove_list(ed->callbacks, l);
		  free(escb->signal);
		  free(escb->source);
		  free(escb);
	       }
	     l = next_l;
	  }
     }
   break_prog:
   _edje_unblock(ed);
   _edje_thaw(ed);
   _edje_unref(ed);
}
