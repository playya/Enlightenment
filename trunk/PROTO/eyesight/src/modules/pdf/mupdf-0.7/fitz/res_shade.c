#include "fitz.h"

fz_shade *
fz_keepshade(fz_shade *shade)
{
	shade->refs ++;
	return shade;
}

void
fz_dropshade(fz_shade *shade)
{
	if (shade && --shade->refs == 0)
	{
		if (shade->cs)
			fz_dropcolorspace(shade->cs);
		fz_free(shade->mesh);
		fz_free(shade);
	}
}

fz_rect
fz_boundshade(fz_shade *shade, fz_matrix ctm)
{
	float *v;
	fz_rect r;
	fz_point p;
	int i, ncomp, nvert;

	ctm = fz_concat(shade->matrix, ctm);
	ncomp = shade->usefunction ? 3 : 2 + shade->cs->n;
	nvert = shade->meshlen / ncomp;
	v = shade->mesh;

	if (nvert == 0)
		return fz_emptyrect;

	p.x = v[0];
	p.y = v[1];
	v += ncomp;
	p = fz_transformpoint(ctm, p);
	r.x0 = r.x1 = p.x;
	r.y0 = r.y1 = p.y;

	for (i = 1; i < nvert; i++)
	{
		p.x = v[0];
		p.y = v[1];
		p = fz_transformpoint(ctm, p);
		v += ncomp;
		if (p.x < r.x0) r.x0 = p.x;
		if (p.y < r.y0) r.y0 = p.y;
		if (p.x > r.x1) r.x1 = p.x;
		if (p.y > r.y1) r.y1 = p.y;
	}

	return r;
}

void
fz_debugshade(fz_shade *shade)
{
	int i, j, n;
	float *vertex;
	int triangle;

	printf("shading {\n");

	printf("\tbbox [%g %g %g %g]\n",
		shade->bbox.x0, shade->bbox.y0,
		shade->bbox.x1, shade->bbox.y1);

	printf("\tcolorspace %s\n", shade->cs->name);

	printf("\tmatrix [%g %g %g %g %g %g]\n",
			shade->matrix.a, shade->matrix.b, shade->matrix.c,
			shade->matrix.d, shade->matrix.e, shade->matrix.f);

	if (shade->usebackground)
	{
		printf("\tbackground [");
		for (i = 0; i < shade->cs->n; i++)
			printf("%s%g", i == 0 ? "" : " ", shade->background[i]);
		printf("]\n");
	}

	if (shade->usefunction)
	{
		printf("\tfunction\n");
		n = 3;
	}
	else
		n = 2 + shade->cs->n;

	printf("\tvertices: %d\n", shade->meshlen);

	vertex = shade->mesh;
	triangle = 0;
	i = 0;
	while (i < shade->meshlen)
	{
		printf("\t%d:(%g, %g): ", triangle, vertex[0], vertex[1]);

		for (j = 2; j < n; j++)
			printf("%s%g", j == 2 ? "" : " ", vertex[j]);
		printf("\n");

		vertex += n;
		i++;
		if (i % 3 == 0)
			triangle++;
	}

	printf("}\n");
}
