#include "fitz.h"
#include "mupdf.h"

void
pdf_set_font_wmode(pdf_font_desc *font, int wmode)
{
	font->wmode = wmode;
}

void
pdf_set_default_hmtx(pdf_font_desc *font, int w)
{
	font->dhmtx.w = w;
}

void
pdf_set_default_vmtx(pdf_font_desc *font, int y, int w)
{
	font->dvmtx.y = y;
	font->dvmtx.w = w;
}

void
pdf_add_hmtx(pdf_font_desc *font, int lo, int hi, int w)
{
	if (font->hmtx_len + 1 >= font->hmtx_cap)
	{
		font->hmtx_cap = font->hmtx_cap + 16;
		font->hmtx = fz_realloc(font->hmtx, font->hmtx_cap, sizeof(pdf_hmtx));
	}

	font->hmtx[font->hmtx_len].lo = lo;
	font->hmtx[font->hmtx_len].hi = hi;
	font->hmtx[font->hmtx_len].w = w;
	font->hmtx_len++;
}

void
pdf_add_vmtx(pdf_font_desc *font, int lo, int hi, int x, int y, int w)
{
	if (font->vmtx_len + 1 >= font->vmtx_cap)
	{
		font->vmtx_cap = font->vmtx_cap + 16;
		font->vmtx = fz_realloc(font->vmtx, font->vmtx_cap, sizeof(pdf_vmtx));
	}

	font->vmtx[font->vmtx_len].lo = lo;
	font->vmtx[font->vmtx_len].hi = hi;
	font->vmtx[font->vmtx_len].x = x;
	font->vmtx[font->vmtx_len].y = y;
	font->vmtx[font->vmtx_len].w = w;
	font->vmtx_len++;
}

static int cmph(const void *a0, const void *b0)
{
	pdf_hmtx *a = (pdf_hmtx*)a0;
	pdf_hmtx *b = (pdf_hmtx*)b0;
	return a->lo - b->lo;
}

static int cmpv(const void *a0, const void *b0)
{
	pdf_vmtx *a = (pdf_vmtx*)a0;
	pdf_vmtx *b = (pdf_vmtx*)b0;
	return a->lo - b->lo;
}

void
pdf_end_hmtx(pdf_font_desc *font)
{
	if (!font->hmtx)
		return;
	qsort(font->hmtx, font->hmtx_len, sizeof(pdf_hmtx), cmph);
}

void
pdf_end_vmtx(pdf_font_desc *font)
{
	if (!font->vmtx)
		return;
	qsort(font->vmtx, font->vmtx_len, sizeof(pdf_vmtx), cmpv);
}

pdf_hmtx
pdf_get_hmtx(pdf_font_desc *font, int cid)
{
	int l = 0;
	int r = font->hmtx_len - 1;
	int m;

	if (!font->hmtx)
		goto notfound;

	while (l <= r)
	{
		m = (l + r) >> 1;
		if (cid < font->hmtx[m].lo)
			r = m - 1;
		else if (cid > font->hmtx[m].hi)
			l = m + 1;
		else
			return font->hmtx[m];
	}

notfound:
	return font->dhmtx;
}

pdf_vmtx
pdf_get_vmtx(pdf_font_desc *font, int cid)
{
	pdf_hmtx h;
	pdf_vmtx v;
	int l = 0;
	int r = font->vmtx_len - 1;
	int m;

	if (!font->vmtx)
		goto notfound;

	while (l <= r)
	{
		m = (l + r) >> 1;
		if (cid < font->vmtx[m].lo)
			r = m - 1;
		else if (cid > font->vmtx[m].hi)
			l = m + 1;
		else
			return font->vmtx[m];
	}

notfound:
	h = pdf_get_hmtx(font, cid);
	v = font->dvmtx;
	v.x = h.w / 2;
	return v;
}
