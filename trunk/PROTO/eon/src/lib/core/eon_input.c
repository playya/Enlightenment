/* EON - Canvas and Toolkit library
 * Copyright (C) 2008-2009 Jorge Luis Zapata
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#include "Eon.h"
#include "eon_private.h"
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
struct _Eon_Input
{
	Eon_Layout *c;
	struct
	{
		unsigned int downx;
		unsigned int downy;
		unsigned int button;
		unsigned int x;
		unsigned int y;
		unsigned int px;
		unsigned int py;
		Eina_Bool inside;
		Eon_Paint *grabbed;
		Eon_Paint *r;
	} pointer;
	/* TODO keep the last modifiers */
	struct
	{

	} keyboard;
};
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
Eon_Input * eon_input_new(Eon_Layout *c)
{
	Eon_Input *i;

	i = calloc(1, sizeof(Eon_Input));
	i->c = c;
	return i;
}

void event_ui_init(Eon_Event_Ui *eui, const char *type, Ekeko_Object *o,
		Ekeko_Object *related, const Eon_Input *i)
{
	ekeko_event_init((Ekeko_Event *)eui, type, o, EINA_FALSE);
	eui->related = related;
	eui->i = i;
}

void event_mouse_move_init(Eon_Event_Mouse *em, Ekeko_Object *o,
		Ekeko_Object *related, const Eon_Input *i,
		unsigned int sx, unsigned int sy, unsigned int px,
		unsigned int py)
{
	event_ui_init((Eon_Event_Ui*)em, EON_EVENT_UI_MOUSE_MOVE, o, related, i);
	em->screen.x = sx;
	em->screen.y = sy;
	em->screen.prev_x = px;
	em->screen.prev_y = py;
}

void event_mouse_in_init(Eon_Event_Mouse *em, Ekeko_Object *o,
		Ekeko_Object *related, const Eon_Input *i)
{
	event_ui_init((Eon_Event_Ui*)em, EON_EVENT_UI_MOUSE_IN, o, related, i);
}
void event_mouse_out_init(Eon_Event_Mouse *em, Ekeko_Object *o,
		Ekeko_Object *related, const Eon_Input *i)
{
	event_ui_init((Eon_Event_Ui*)em, EON_EVENT_UI_MOUSE_OUT, o, related, i);
}

void event_mouse_down_init(Eon_Event_Mouse *em, Ekeko_Object *o,
		Ekeko_Object *related, const Eon_Input *i)
{
	event_ui_init((Eon_Event_Ui*)em, EON_EVENT_UI_MOUSE_DOWN, o, related, i);
}

void event_mouse_up_init(Eon_Event_Mouse *em, Ekeko_Object *o,
		Ekeko_Object *related, const Eon_Input *i)
{
	event_ui_init((Eon_Event_Ui*)em, EON_EVENT_UI_MOUSE_UP, o, related, i);
}

void event_mouse_click_init(Eon_Event_Mouse *em, Ekeko_Object *o,
		Ekeko_Object *related, const Eon_Input *i)
{
	event_ui_init((Eon_Event_Ui*)em, EON_EVENT_UI_MOUSE_CLICK, o, related, i);
}

void eon_key_up_init(Eon_Event_Key *ek, Ekeko_Object *o,
		const Eon_Input *i, Eon_Key key, Eon_Key_Mod mod)
{
	ek->key = key;
	ek->mod = mod;
	event_ui_init((Eon_Event_Ui *)ek, EON_EVENT_UI_KEY_UP, o, o, i);
}

void eon_key_down_init(Eon_Event_Key *ek, Ekeko_Object *o,
		const Eon_Input *i, Eon_Key key, Eon_Key_Mod mod)
{
	ek->key = key;
	ek->mod = mod;
	event_ui_init((Eon_Event_Ui *)ek, EON_EVENT_UI_KEY_DOWN, o, o, i);
}

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
EAPI void eon_input_feed_mouse_in(Eon_Input *i)
{
	Eon_Paint *r;

	if (i->pointer.inside)
		return;
	i->pointer.inside = EINA_TRUE;
	r = eon_layout_renderable_get_at_coord(i->c, i->pointer.x, i->pointer.y);
	if (!r)
		return;
	/* TODO send the event */
}

EAPI void eon_input_feed_mouse_move(Eon_Input *i, unsigned int x, unsigned int y)
{
	Eon_Paint *r;
	unsigned int px, py;

	if (!i->pointer.inside)
		return;

	px = i->pointer.x;
	py = i->pointer.y;
	i->pointer.x = x;
	i->pointer.y = y;

	if (i->pointer.grabbed)
	{
		Eon_Event_Mouse em;

		event_mouse_move_init(&em, (Ekeko_Object *)i->pointer.grabbed,
				NULL, i, x, y, px, py);
		ekeko_event_dispatch((Ekeko_Event *)&em);
		return;
	}
	/* TODO handle the subcanvas
	 * if mouse in an object and canvas(obj) != canvas => in canvas(obj) ?
	 * if mouse out an object and canvas(obj) != canvas => out canvas(obj) ?
	 */
	r = eon_layout_renderable_get_at_coord(i->c, x, y);
	if (r == i->pointer.r)
	{
		/* send move */
		if (r)
		{
			Eon_Event_Mouse em;

			event_mouse_move_init(&em, (Ekeko_Object *)r, NULL, i,
					x, y, px, py);
			ekeko_event_dispatch((Ekeko_Event *)&em);
		}
	}
	else
	{
		/* send out event on i->r */
		if (i->pointer.r)
		{
			Eon_Event_Mouse em;

			event_mouse_out_init(&em, (Ekeko_Object *)i->pointer.r, (Ekeko_Object *)r, i);
			ekeko_event_dispatch((Ekeko_Event *)&em);
		}
		/* send in event on r */
		if (r)
		{
			Eon_Event_Mouse em;

			event_mouse_in_init(&em, (Ekeko_Object *)r, (Ekeko_Object *)r, i);
			ekeko_event_dispatch((Ekeko_Event *)&em);

		}
	}
	/* update the current inside */
	i->pointer.r = r;
}

EAPI void eon_input_feed_mouse_out(Eon_Input *i)
{
	Eon_Paint *r;

	if (!i->pointer.inside)
		return;
	i->pointer.inside = EINA_FALSE;
	r = eon_layout_renderable_get_at_coord(i->c, i->pointer.x, i->pointer.y);
	if (!r)
		return;
	/* TODO send the event */
}

EAPI void eon_input_feed_mouse_down(Eon_Input *i)
{
	Eon_Paint *r;
	Eon_Event_Mouse em;

	if (!i->pointer.inside)
		return;
	r = eon_layout_renderable_get_at_coord(i->c, i->pointer.x, i->pointer.y);
	if (!r)
		return;
	/* store the coordinates where the mouse buton down was done to
	 * trigger the click later
	 */
	i->pointer.grabbed = r;
	i->pointer.downx = i->pointer.x;
	i->pointer.downy = i->pointer.y;
	event_mouse_down_init(&em, (Ekeko_Object *)r, (Ekeko_Object *)r, i);
	ekeko_event_dispatch((Ekeko_Event *)&em);
}

EAPI void eon_input_feed_mouse_up(Eon_Input *i)
{
	Eon_Paint *r;
	Eon_Event_Mouse em;

	/* send the event to the grabbed object */
	r = i->pointer.grabbed;
	if (!r)
		return;

	event_mouse_up_init(&em, (Ekeko_Object *)r, (Ekeko_Object *)r, i);
	ekeko_event_dispatch((Ekeko_Event *)&em);
	/* in case the down coordinates are the same as the current coordinates
	 * send a click event
	 */
	if ((i->pointer.downx == i->pointer.x) &&
			 (i->pointer.downy == i->pointer.y))
	{
		event_mouse_click_init(&em, (Ekeko_Object *)r,
				(Ekeko_Object *)r, i);
		ekeko_event_dispatch((Ekeko_Event *)&em);
	}
	i->pointer.grabbed = NULL;
}

EAPI void eon_input_feed_key_up(Eon_Input *i, Eon_Key key, Eon_Key_Mod mod)
{
	Eon_Paint *r;
	Eon_Event_Key ek;

	r = eon_layout_focus_get(i->c);
	if (!r) return;

	eon_key_up_init(&ek, (Ekeko_Object *)r, i, key, mod);
	ekeko_event_dispatch((Ekeko_Event *)&ek);
}

EAPI void eon_input_feed_key_down(Eon_Input *i, Eon_Key key, Eon_Key_Mod mod)
{
	Eon_Paint *r;
	Eon_Event_Key ek;

	r = eon_layout_focus_get(i->c);
	if (!r) return;

	eon_key_down_init(&ek, (Ekeko_Object *)r, i, key, mod);
	ekeko_event_dispatch((Ekeko_Event *)&ek);
}
