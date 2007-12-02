#ifndef _ENESIM_EXTENDER_H
#define _ENESIM_EXTENDER_H

typedef struct _Enesim_Extender
{
	int max;
	int min;
} Enesim_Extender;

static inline enesim_extender_reset(Enesim_Extender *e)
{
	e->min = INT_MAX;
	e->max = INT_MIN;
}

static inline enesim_extender_add(Enesim_Extender *e, int start, int end)
{
	if (start < e->min)
		e->min = start;
	if (end > e->max)
		e->max = end;
}

static inline enesim_extender_add_sort(Enesim_Extender *e, int start, int end)
{
	if (start <= end)
		enesim_extender_add(e, start, end);
	else
		enesim_extender_add(e, end, start);
}

#endif
