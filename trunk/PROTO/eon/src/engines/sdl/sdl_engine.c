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

#include "Eon_Enesim.h"

#include "Ecore_Input.h"
#include "Ecore_Sdl.h"
#include "SDL.h"
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
#define SINGLE_BUFFER

typedef struct _Engine_SDL_Document
{
	Eon_Document *doc;
	SDL_Surface *s; /* main surface */
	 // FIXME remove this?
	Eon_Canvas *root;
	/* options */
	char resizable:1;
	char dbuffer:1;
	Ecore_Event_Handler *handler_rz;
} Engine_SDL_Document;

typedef struct _Engine_SDL_Layout
{
	Engine_SDL_Document *sdoc;
	/* mouse, key, etc handlers */
	Ecore_Event_Handler *handlers[7];
} Engine_SDL_Layout;

static void _sdl_surface_new(Engine_SDL_Document *sdoc, int w, int h)
{
	Uint32 flags = SDL_SRCALPHA;

	if (sdoc->resizable)
		flags |= SDL_RESIZABLE;
	/* the destination surface */
	printf("[SDL] Setting video mode to %d %d\n", w, h);
#ifdef SINGLE_BUFFER
	sdoc->s = SDL_SetVideoMode(w, h, 32, flags);
#else
	sdoc->s = SDL_SetVideoMode(w, h, 32, flags | SDL_DOUBLEBUF);
#endif
}

static int _sdl_event(void *data)
{
	ecore_sdl_feed_events();
	return 1;
}

static int _resize_cb(void *data, int type, void *event)
{
	Ecore_Sdl_Event_Video_Resize *e = event;
	Engine_SDL_Document *sdoc = data;

	eon_document_resize(sdoc->doc, e->w, e->h);
	_sdl_surface_new(sdoc, e->w, e->h);

	return 1;
}
static int _mouse_down_cb(void *data, int type, void *event)
{
	Eon_Input *input = (Eon_Input *)data;
	eon_input_feed_mouse_down(input);
	return 1;
}

static int _mouse_up_cb(void *data, int type, void *event)
{
	Eon_Input *input = (Eon_Input *)data;
	eon_input_feed_mouse_up(input);
	return 1;
}

static int _mouse_move_cb(void *data, int type, void *event)
{
	Eon_Input *input = (Eon_Input *)data;
	Ecore_Event_Mouse_Move *e = event;

	eon_input_feed_mouse_move(input, e->x, e->y);
	return 1;
}

static int _key_down_cb(void *data, int type, void *event)
{
	Eon_Input *input = (Eon_Input *)data;

	eon_input_feed_key_down(input, 0, 0);
	return 1;
}

static int _key_up_cb(void *data, int type, void *event)
{
	Eon_Input *input = (Eon_Input *)data;

	eon_input_feed_key_up(input, 0, 0);
	return 1;
}

static int _in_cb(void *data, int type, void *event)
{
	Eon_Input *input = (Eon_Input *)data;
	return 1;
}
static int _out_cb(void *data, int type, void *event)
{
	Eon_Input *input = (Eon_Input *)data;
	return 1;
}

static void * document_create(Eon_Document *d, int w, int h, const char *options)
{
	Engine_SDL_Document *sdoc;
	Uint32 flags = SDL_SRCALPHA;

	printf("[SDL] Initializing SDL\n");
	sdoc = calloc(1, sizeof(Engine_SDL_Document));
	if (options)
	{
		if (!strcmp(options, "resizable"))
			sdoc->resizable = 1;
	}
	sdoc->doc = d;
	ecore_sdl_init(NULL);
	SDL_Init(SDL_INIT_VIDEO);
	_sdl_surface_new(sdoc, w, h);
	/* the event feeder evas/ecore has a very weird way to feed sdl events! */
	ecore_timer_add(0.008, _sdl_event, NULL);
	/* called whenever the wm changes the window */
	ecore_event_handler_add(ECORE_SDL_EVENT_RESIZE, _resize_cb, sdoc);

	return sdoc;
}

static void document_delete(void *data)
{
	ecore_sdl_shutdown();
}

static void _lock(void *s)
{
	SDL_Surface *src = s;

	SDL_LockSurface(src);
}

static void _unlock(void *s)
{
	SDL_Surface *src = s;

	SDL_UnlockSurface(src);
}

static void _blit(void *src, Eina_Rectangle *srect, void *context, void *dst, Eina_Rectangle *drect)
{
	SDL_Rect ssrect, sdrect;

	ssrect.x = srect->x;
	ssrect.y = srect->y;
	ssrect.w = srect->w;
	ssrect.h = srect->h;

	sdrect.x = drect->x;
	sdrect.y = drect->y;
	sdrect.w = drect->w;
	sdrect.h = drect->h;

#ifdef EON_DEBUG
	printf("[SDL] rendering into %p from %p (%d %d %d %d to %d %d %d %d)\n",
			dst, src, srect->x, srect->y, srect->w, srect->h,
			drect->x, drect->y, drect->w, drect->h);
#endif
	SDL_BlitSurface(src, &ssrect, dst, &sdrect);
}

static Eina_Bool _flush(void *src, Eina_Rectangle *srect)
{
	SDL_Surface *s = src;

#ifdef EON_DEBUG
	printf("[SDL] Flushing surface %p\n", s);
#endif
#ifdef SINGLE_BUFFER
	SDL_UpdateRect(s, srect->x, srect->y, srect->w, srect->h);
	return EINA_FALSE;
#else
	SDL_Flip(s);
	return EINA_TRUE;
#endif

}

static void _root_canvas_create(Eon_Layout *c)
{
	Eon_Input *input;

	/* Ecore_sdl interval isnt enough */
	SDL_EnableKeyRepeat(10, 10);
	/* add the input */
	input = eon_layout_input_new((Eon_Layout *)c);
	/* add the callbacks */
	ecore_event_handler_add(ECORE_SDL_EVENT_GOT_FOCUS, _in_cb, input);
	ecore_event_handler_add(ECORE_SDL_EVENT_LOST_FOCUS, _out_cb, input);
	ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, _mouse_down_cb, input);
	ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, _mouse_up_cb, input);
	ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, _mouse_move_cb, input);
	ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _key_down_cb, input);
	ecore_event_handler_add(ECORE_EVENT_KEY_UP, _key_up_cb, input);
	/* FIXME Ecore_SDL doesnt support the in/out events, make it always in */
	eon_input_feed_mouse_in(input);
}

static void layout_delete(void *data)
{
	Eon_Enesim_Layout *l = data;
	Engine_SDL_Layout *sdl_layout = l->data;

	eon_engine_enesim_layout_delete(l);
	free(sdl_layout);
}

static void * layout_create(Eon_Layout *l, void *dd, int w, int h)
{
	Engine_SDL_Layout *sdl_layout;

	sdl_layout = malloc(sizeof(Engine_SDL_Layout));
	sdl_layout->sdoc = dd;

	return eon_engine_enesim_layout_create(l, w, h, sdl_layout);
}

static Eina_Bool layout_flush(void *src, Eina_Rectangle *srect)
{
	Eon_Enesim_Layout *l = (Eon_Enesim_Layout *)src;
	Engine_SDL_Layout *sdl_layout = l->data;
	Enesim_Surface *es = l->s;
	Enesim_Converter_1D conv;
	Enesim_Converter_Data cdata;
	uint32_t *sdata;
	uint8_t *sdldata;

	SDL_Surface *s;

	int h = srect->h;
	int stride;
	int inc;
	int numcpus;
	int soffset;
	int coffset;

#if 1
	printf("Flushing the canvas %p %d %d %d %d\n", es, srect->x, srect->y, srect->w, srect->h);
#endif
	/* setup the pointers */
	s = sdl_layout->sdoc->s;
	stride = enesim_surface_stride_get(es);

	sdata = enesim_surface_data_get(es);
	sdldata = s->pixels;

	soffset = (stride * srect->y) + srect->x;
	coffset = (s->pitch * srect->y) + (srect->x * 4);

	sdldata += coffset;
	sdata += soffset;
	cdata.argb8888.plane0 = (uint32_t *)sdldata;
	cdata.argb8888.plane0_stride = s->pitch / 4;
	/* convert */
	conv = enesim_converter_span_get(ENESIM_CONVERTER_ARGB8888, ENESIM_ANGLE_0, ENESIM_FORMAT_ARGB8888);
	_lock(s);
	while (h--)
	{
		conv(&cdata, srect->w, sdata);
		sdata += stride;
		cdata.argb8888.plane0 += cdata.argb8888.plane0_stride;
	}
	_unlock(s);
	return _flush(s, srect);
}

static Eon_Engine _sdl_engine = {
	.document_create = document_create,
	.document_delete = document_delete,
	.layout_flush = layout_flush,
	.layout_create = layout_create,
	.layout_delete = layout_delete,
};
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
void engine_sdl_init(void)
{
	eon_engine_enesim_setup(&_sdl_engine);
	eon_engine_register("sdl", &_sdl_engine);
}

void engine_sdl_shutdown(void)
{

}
/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

