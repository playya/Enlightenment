/* ENESIM - Direct Rendering Library
 * Copyright (C) 2007-2008 Jorge Luis Zapata
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
#include "Enesim.h"
#include "enesim_private.h"
/* FIXME create the table dynamically */
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
typedef Enesim_Scaler_1D Enesim_Scaler_1D_Lut[ENESIM_FORMATS][ENESIM_FORMATS];
typedef Enesim_Scaler_1D Enesim_Scaler_2D_Lut[ENESIM_FORMATS][ENESIM_QUALITIES][ENESIM_FORMATS];

Enesim_Scaler_1D_Lut *_scalers1d;
Enesim_Scaler_2D_Lut *_scalers2d;
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
void enesim_scaler_init(void)
{
	Enesim_Cpu **cpus;
	int numcpu;
	int i;

	cpus = enesim_cpu_get(&numcpu);
	_scalers1d = malloc(sizeof(Enesim_Scaler_1D_Lut) * numcpu);
	_scalers2d = malloc(sizeof(Enesim_Scaler_2D_Lut) * numcpu);
	for (i = 0; i < numcpu; i++)
	{
		enesim_scaler_argb8888_init(cpus[i]);
	}
}
void enesim_scaler_shutdown(void)
{
	free(_scalers1d);
	free(_scalers2d);
}
/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void enesim_scaler_1d_register(Enesim_Cpu *cpu, Enesim_Scaler_1D scl,
		Enesim_Format sfmt, Enesim_Format dfmt)
{
	unsigned int cpuid;
	Enesim_Scaler_1D_Lut *t;

	cpuid = enesim_cpu_id_get(cpu);
	t = &_scalers1d[cpuid];
	*t[sfmt][dfmt] = scl;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void enesim_scaler_2d_register(Enesim_Cpu *cpu, Enesim_Scaler_1D scl,
		Enesim_Format sfmt, Enesim_Quality qty,
		Enesim_Format dfmt)
{
	unsigned int cpuid;
	Enesim_Scaler_2D_Lut *t;

	cpuid = enesim_cpu_id_get(cpu);
	t = &_scalers2d[cpuid];
	*t[sfmt][qty][dfmt] = scl;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI Eina_Bool enesim_scaler_1d_op_get(Enesim_Operator *op, Enesim_Cpu *cpu,
		Enesim_Format sfmt, Enesim_Format dfmt)
{
	unsigned int cpuid;
	Enesim_Scaler_1D_Lut *t;
	Enesim_Scaler_1D s;

	cpuid = enesim_cpu_id_get(cpu);
	t = &_scalers1d[cpuid];
	s = *t[sfmt][dfmt];
	if (s)
	{
		op->id = ENESIM_OPERATOR_SCALER1D;
		op->cb = s;
		op->cpu = cpu;
		return EINA_TRUE;
	}
	else
		return EINA_FALSE;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI Eina_Bool enesim_scaler_2d_op_get(Enesim_Operator *op, Enesim_Cpu *cpu,
		Enesim_Format sfmt, Enesim_Quality q, Enesim_Format dfmt)
{
	unsigned int cpuid;
	Enesim_Scaler_2D_Lut *t;
	Enesim_Scaler_1D s;

	cpuid = enesim_cpu_id_get(cpu);
	t = &_scalers2d[cpuid];
	s = *t[sfmt][q][dfmt];
	if (s)
	{
		op->id = ENESIM_OPERATOR_SCALER2D;
		op->cb = s;
		op->cpu = cpu;
		return EINA_TRUE;
	}
	else
		return EINA_FALSE;
}

#if 0
EAPI Eina_Bool enesim_renderer_scaler_src_y(Enesim_Renderer *r, int ydst, int *ysrc)
{
	Renderer_Scaler *s = (Renderer_Scaler *)r;

	if (ydst > (s->dst.area.y + s->dst.area.w))
	{
		return EINA_FALSE;
	}
	else
	{
		*ysrc = s->row.values[ydst];
		return EINA_TRUE;
	}
}
#endif
