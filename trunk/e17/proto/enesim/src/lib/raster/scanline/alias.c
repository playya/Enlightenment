#include <stdlib.h>

#include "Edata.h"
#include "Enesim.h"
#include "enesim_private.h"
#include "scanline.h"
#include "alias.h"

/**
 * Scanline_Aliased scanline
 *
 */

static void _a_alloc(Scanline_Alias *n, int num)
{
	n->sls = realloc(n->sls, num * sizeof(Scanline_Alias_Sl));
}

static void _a_free(Scanline_Alias *n)
{
	free(n->sls);
}

static void _sl_free(Scanline_Alias *n)
{
	edata_array_free(n->a);
	free(n);
}

static void _sl_add(Scanline_Alias *n, int y, int x0, int x1, int coverage)
{
	edata_array_element_new(n->a);
	n->sls[n->num_sls].y = y;
	n->sls[n->num_sls].x = x0;
	n->sls[n->num_sls].w = x1 - x0 + 1;
	n->num_sls++;
}

Enesim_Scanline_Func naa = {
	.free	= ENESIM_SCANLINE_FREE(_sl_free),
	.add 	= ENESIM_SCANLINE_ADD(_sl_add)
};

/*============================================================================*
 *                                   API                                      * 
 *============================================================================*/
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI Enesim_Scanline * enesim_scanline_alias_new(void)
{
	Enesim_Scanline *sl;
	Scanline_Alias *n;

	n = calloc(1, sizeof(Scanline_Alias));
	n->a = edata_array_new(n, EDATA_ARRAY_ALLOC(_a_alloc),
		EDATA_ARRAY_FREE(_a_free));

	sl = enesim_scanline_new();
	sl->funcs = &naa;
	sl->data = n;
	return sl;
}
