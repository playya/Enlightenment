#include <fitz.h>
#include <mupdf.h>
#include "pdfapp.h"

enum panning
{
	DONT_PAN = 0,
	PAN_TO_TOP,
	PAN_TO_BOTTOM
};

static void pdfapp_showpage(pdfapp_t *app, int loadpage, int drawpage);

static void pdfapp_warn(pdfapp_t *app, const char *fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	winwarn(app, buf);
}

static void pdfapp_error(pdfapp_t *app, fz_error error)
{
	winerror(app, error);
}

char *pdfapp_usage(pdfapp_t *app)
{
	return
		"   l <\t\t-- rotate left\n"
		"   r >\t\t-- rotate right\n"
		"   u up\t\t-- scroll up\n"
		"   d down\t-- scroll down\n"
		"   = +\t\t-- zoom in\n"
		"   -\t\t-- zoom out\n"
		"   w\t\t-- shrinkwrap\n"
		"\n"
		"   n pgdn space\t-- next page\n"
		"   b pgup back\t-- previous page\n"
		"   right\t\t-- next page\n"
		"   left\t\t-- previous page\n"
		"   N F\t\t-- next 10\n"
		"   B\t\t-- back 10\n"
		"   m\t\t-- mark page for snap back\n"
		"   t\t\t-- pop back to last mark\n"
		"   123g\t\t-- go to page\n"
		"\n"
		"   left drag to pan, right drag to copy text\n";
}

void pdfapp_init(pdfapp_t *app)
{
	fz_error error;

	memset(app, 0, sizeof(pdfapp_t));

	error = fz_newrenderer(&app->rast, pdf_devicergb, 0, 1024 * 512);
	if (error)
		pdfapp_error(app, error);

	app->scrw = 640;
	app->scrh = 480;
	app->zoom = 1.0;
}

void pdfapp_open(pdfapp_t *app, char *filename)
{
	fz_error error;
	fz_obj *obj;
	char *password = "";

	/*
	 * Open PDF and load xref table
	 */

	app->filename = filename;

	app->xref = pdf_newxref();
	error = pdf_loadxref(app->xref, filename);
	if (error)
	{
		fz_catch(error, "trying to repair");
		pdfapp_warn(app, "There was a problem with file \"%s\".\nIt may be corrupted or generated by faulty software.\nTrying to repair the file.", filename);
		error = pdf_repairxref(app->xref, filename);
		if (error)
			pdfapp_error(app, error);
	}

	error = pdf_decryptxref(app->xref);
	if (error)
		pdfapp_error(app, error);

	/*
	 * Handle encrypted PDF files
	 */

	if (pdf_needspassword(app->xref))
	{
		int okay = pdf_authenticatepassword(app->xref, password);
		while (!okay)
		{
			password = winpassword(app, filename);
			if (!password)
				exit(1);
			okay = pdf_authenticatepassword(app->xref, password);
			if (!okay)
				pdfapp_warn(app, "Invalid password.");
		}
	}

	/*
	 * Load meta information
	 * TODO: move this into mupdf library
	 */

	obj = fz_dictgets(app->xref->trailer, "Root");
	app->xref->root = fz_resolveindirect(obj);
	if (!app->xref->root)
		pdfapp_error(app, fz_throw("syntaxerror: missing Root object"));
	fz_keepobj(app->xref->root);

	obj = fz_dictgets(app->xref->trailer, "Info");
	app->xref->info = fz_resolveindirect(obj);
	if (!app->xref->info)
		pdfapp_warn(app, "Could not load PDF meta information.");
	if (app->xref->info)
		fz_keepobj(app->xref->info);

	app->outline = pdf_loadoutline(app->xref);

	app->doctitle = filename;
	if (strrchr(app->doctitle, '\\'))
		app->doctitle = strrchr(app->doctitle, '\\') + 1;
	if (strrchr(app->doctitle, '/'))
		app->doctitle = strrchr(app->doctitle, '/') + 1;
	if (app->xref->info)
	{
		obj = fz_dictgets(app->xref->info, "Title");
		if (obj)
		{
			app->doctitle = pdf_toutf8(obj);
		}
	}

	/*
	 * Start at first page
	 */

	app->pagecount = pdf_getpagecount(app->xref);

	app->shrinkwrap = 1;
	if (app->pageno < 1)
		app->pageno = 1;
	if (app->zoom < 0.1)
		app->zoom = 0.1;
	if (app->zoom > 3.0)
		app->zoom = 3.0;
	app->rotate = 0;
	app->panx = 0;
	app->pany = 0;

	pdfapp_showpage(app, 1, 1);
}

void pdfapp_close(pdfapp_t *app)
{
	if (app->page)
		pdf_droppage(app->page);
	app->page = nil;

	if (app->image)
		fz_droppixmap(app->image);
	app->image = nil;

	if (app->outline)
		pdf_dropoutline(app->outline);
	app->outline = nil;

	if (app->xref->store)
		pdf_dropstore(app->xref->store);
	app->xref->store = nil;

	pdf_closexref(app->xref);
	app->xref = nil;
}

static fz_matrix pdfapp_viewctm(pdfapp_t *app)
{
	fz_matrix ctm;
	ctm = fz_identity();
	ctm = fz_concat(ctm, fz_translate(0, -app->page->mediabox.y1));
	ctm = fz_concat(ctm, fz_scale(app->zoom, -app->zoom));
	ctm = fz_concat(ctm, fz_rotate(app->rotate + app->page->rotate));
	return ctm;
}

static void pdfapp_panview(pdfapp_t *app, int newx, int newy)
{
	if (newx > 0)
		newx = 0;
	if (newy > 0)
		newy = 0;

	if (newx + app->image->w < app->winw)
		newx = app->winw - app->image->w;
	if (newy + app->image->h < app->winh)
		newy = app->winh - app->image->h;

	if (app->winw >= app->image->w)
		newx = (app->winw - app->image->w) / 2;
	if (app->winh >= app->image->h)
		newy = (app->winh - app->image->h) / 2;

	if (newx != app->panx || newy != app->pany)
		winrepaint(app);

	app->panx = newx;
	app->pany = newy;
}

static void pdfapp_showpage(pdfapp_t *app, int loadpage, int drawpage)
{
	char buf[256];
	fz_error error;
	fz_matrix ctm;
	fz_rect bbox;
	fz_obj *obj;

	if (loadpage)
	{
		wincursor(app, WAIT);

		if (app->page)
			pdf_droppage(app->page);
		app->page = nil;

		pdf_flushxref(app->xref, 0);

		obj = pdf_getpageobject(app->xref, app->pageno);
		error = pdf_loadpage(&app->page, app->xref, obj);
		if (error)
			pdfapp_error(app, error);

		sprintf(buf, "%s - %d/%d", app->doctitle,
			app->pageno, app->pagecount);
		wintitle(app, buf);
	}

	if (drawpage)
	{
		wincursor(app, WAIT);

		if (app->image)
			fz_droppixmap(app->image);
		app->image = nil;

		ctm = pdfapp_viewctm(app);
		bbox = fz_transformaabb(ctm, app->page->mediabox);

		error = fz_rendertree(&app->image, app->rast, app->page->tree,
			ctm, fz_roundrect(bbox), 1);
		if (error)
			pdfapp_error(app, error);

		winconvert(app, app->image);
	}

	pdfapp_panview(app, app->panx, app->pany);

	if (app->shrinkwrap)
	{
		int w = app->image->w;
		int h = app->image->h;
		if (app->winw == w)
			app->panx = 0;
		if (app->winh == h)
			app->pany = 0;
		if (w > app->scrw * 90 / 100)
			w = app->scrw * 90 / 100;
		if (h > app->scrh * 90 / 100)
			h = app->scrh * 90 / 100;
		if (w != app->winw || h != app->winh)
			winresize(app, w, h);
	}

	winrepaint(app);

	wincursor(app, ARROW);
}

static void pdfapp_gotouri(pdfapp_t *app, fz_obj *uri)
{
	char *buf;
	buf = fz_malloc(fz_tostrlen(uri) + 1);
	memcpy(buf, fz_tostrbuf(uri), fz_tostrlen(uri));
	buf[fz_tostrlen(uri)] = 0;
	winopenuri(app, buf);
	fz_free(buf);
}

static void pdfapp_gotopage(pdfapp_t *app, fz_obj *obj)
{
	int page;

	page = pdf_findpageobject(app->xref, obj);

	if (app->histlen + 1 == 256)
	{
		memmove(app->hist, app->hist + 1, sizeof(int) * 255);
		app->histlen --;
	}
	app->hist[app->histlen++] = app->pageno;
	app->pageno = page;
	pdfapp_showpage(app, 1, 1);
}

void pdfapp_onresize(pdfapp_t *app, int w, int h)
{
	if (app->winw != w || app->winh != h)
	{
		app->winw = w;
		app->winh = h;
		pdfapp_panview(app, app->panx, app->pany);
		winrepaint(app);
	}
}

void pdfapp_onkey(pdfapp_t *app, int c)
{
	int oldpage = app->pageno;
	enum panning panto = PAN_TO_TOP;

	/*
	 * Save numbers typed for later
	 */

	if (c >= '0' && c <= '9')
	{
		app->number[app->numberlen++] = c;
		app->number[app->numberlen] = '\0';
	}

	switch (c)
	{

		/*
		 * Zoom and rotate
		 */

	case '+':
	case '=':
		app->zoom += 0.1;
		if (app->zoom > 3.0)
			app->zoom = 3.0;
		pdfapp_showpage(app, 0, 1);
		break;
	case '-':
		app->zoom -= 0.1;
		if (app->zoom < 0.1)
			app->zoom = 0.1;
		pdfapp_showpage(app, 0, 1);
		break;
	case 'l':
	case '<':
		app->rotate -= 90;
		pdfapp_showpage(app, 0, 1);
		break;
	case 'r':
	case '>':
		app->rotate += 90;
		pdfapp_showpage(app, 0, 1);
		break;

	case 'a':
		app->rotate -= 15;
		pdfapp_showpage(app, 0, 1);
		break;
	case 's':
		app->rotate += 15;
		pdfapp_showpage(app, 0, 1);
		break;

		/*
		 * Pan view, but dont need to repaint image
		 */

	case 'w':
		app->shrinkwrap = 1;
		app->panx = app->pany = 0;
		pdfapp_showpage(app, 0, 0);
		break;

	case 'd':
		app->pany -= app->image->h / 10;
		pdfapp_showpage(app, 0, 0);
		break;

	case 'u':
		app->pany += app->image->h / 10;
		pdfapp_showpage(app, 0, 0);
		break;

	case ',':
		app->panx += app->image->w / 10;
		pdfapp_showpage(app, 0, 0);
		break;

	case '.':
		app->panx -= app->image->w / 10;
		pdfapp_showpage(app, 0, 0);
		break;

		/*
		 * Page navigation
		 */

	case 'g':
	case '\n':
	case '\r':
		if (app->numberlen > 0)
			app->pageno = atoi(app->number);
		break;

	case 'G':
		app->pageno = app->pagecount;
		break;

	case 'm':
		if (app->histlen + 1 == 256)
		{
			memmove(app->hist, app->hist + 1, sizeof(int) * 255);
			app->histlen --;
		}
		app->hist[app->histlen++] = app->pageno;
		break;

	case 't':
		if (app->histlen > 0)
			app->pageno = app->hist[--app->histlen];
		break;

		/*
		 * Back and forth ...
		 */

	case 'p':
		panto = PAN_TO_BOTTOM;
		if (app->numberlen > 0)
			app->pageno -= atoi(app->number);
		else
			app->pageno--;
		break;

	case 'n':
		panto = PAN_TO_TOP;
		if (app->numberlen > 0)
			app->pageno += atoi(app->number);
		else
			app->pageno++;
		break;

	case 'b':
	case '\b':
		panto = DONT_PAN;
		if (app->numberlen > 0)
			app->pageno -= atoi(app->number);
		else
			app->pageno--;
		break;

	case 'f':
	case ' ':
		panto = DONT_PAN;
		if (app->numberlen > 0)
			app->pageno += atoi(app->number);
		else
			app->pageno++;
		break;

	case 'B':
		panto = PAN_TO_TOP;	app->pageno -= 10; break;
	case 'F':
		panto = PAN_TO_TOP;	app->pageno += 10; break;
	}

	if (c < '0' || c > '9')
		app->numberlen = 0;

	if (app->pageno < 1)
		app->pageno = 1;
	if (app->pageno > app->pagecount)
		app->pageno = app->pagecount;

	if (app->pageno != oldpage)
	{
		switch (panto)
		{
		case PAN_TO_TOP:	app->pany = 0; break;
		case PAN_TO_BOTTOM:	app->pany = -2000; break;
		case DONT_PAN:		break;
		}
		pdfapp_showpage(app, 1, 1);
	}
}

void pdfapp_onmouse(pdfapp_t *app, int x, int y, int btn, int modifiers, int state)
{
	pdf_link *link;
	fz_matrix ctm;
	fz_point p;

	p.x = x - app->panx + app->image->x;
	p.y = y - app->pany + app->image->y;

	ctm = pdfapp_viewctm(app);
	ctm = fz_invertmatrix(ctm);

	p = fz_transformpoint(ctm, p);

	for (link = app->page->links; link; link = link->next)
	{
		if (p.x >= link->rect.x0 && p.x <= link->rect.x1)
			if (p.y >= link->rect.y0 && p.y <= link->rect.y1)
				break;
	}

	if (link)
	{
		wincursor(app, HAND);
		if (btn == 1 && state == 1)
		{
			if (link->kind == PDF_LURI)
				pdfapp_gotouri(app, link->dest);
			else if (link->kind == PDF_LGOTO)
				pdfapp_gotopage(app, link->dest);
			return;
		}
	}
	else
	{
		wincursor(app, ARROW);
	}

	if (state == 1)
	{
		if (btn == 1 && !app->iscopying)
		{
			app->ispanning = 1;
			app->selx = x;
			app->sely = y;
		}
		if (btn == 3 && !app->ispanning)
		{
			app->iscopying = 1;
			app->selx = x;
			app->sely = y;
			app->selr.x0 = x;
			app->selr.x1 = x;
			app->selr.y0 = y;
			app->selr.y1 = y;
		}
		if (btn == 4 || btn == 5) /* scroll wheel */
		{
			int dir = btn == 4 ? 1 : -1;
			app->ispanning = app->iscopying = 0;
			if (modifiers & (1<<2))
			{
				/* zoom in/out if ctrl is pressed */
				app->zoom += 0.1 * dir;
				if (app->zoom > 3.0)
					app->zoom = 3.0;
				if (app->zoom < 0.1)
					app->zoom = 0.1;
				pdfapp_showpage(app, 0, 1);
			}
			else
			{
				/* scroll up/down, or left/right if
				shift is pressed */
				int isx = (modifiers & (1<<0));
				int xstep = isx ? 20 * dir : 0;
				int ystep = !isx ? 20 * dir : 0;
				pdfapp_panview(app, app->panx + xstep, app->pany + ystep);
			}
		}
	}

	else if (state == -1)
	{
		if (app->iscopying)
		{
			app->iscopying = 0;
			app->selr.x0 = MIN(app->selx, x);
			app->selr.x1 = MAX(app->selx, x);
			app->selr.y0 = MIN(app->sely, y);
			app->selr.y1 = MAX(app->sely, y);
			winrepaint(app);
			if (app->selr.x0 < app->selr.x1 && app->selr.y0 < app->selr.y1)
				windocopy(app);
		}
		if (app->ispanning)
			app->ispanning = 0;
	}

	else if (app->ispanning)
	{
		int newx = app->panx + x - app->selx;
		int newy = app->pany + y - app->sely;
		pdfapp_panview(app, newx, newy);
		app->selx = x;
		app->sely = y;
	}

	else if (app->iscopying)
	{
		app->selr.x0 = MIN(app->selx, x);
		app->selr.x1 = MAX(app->selx, x);
		app->selr.y0 = MIN(app->sely, y);
		app->selr.y1 = MAX(app->sely, y);
		winrepaint(app);
	}

}

void pdfapp_oncopy(pdfapp_t *app, unsigned short *ucsbuf, int ucslen)
{
	ucsbuf[0] = 0;
#if 0 /* pdf_loadtextfromtree needs rewriting, so removing this temporarily */
	fz_error error;
	pdf_textline *line, *ln;
	int y, c;
	int i, p;

	int bx0, bx1, by0, by1;

	int x0 = app->image->x + app->selr.x0 - app->panx;
	int x1 = app->image->x + app->selr.x1 - app->panx;
	int y0 = app->image->y + app->selr.y0 - app->pany;
	int y1 = app->image->y + app->selr.y1 - app->pany;

	error = pdf_loadtextfromtree(&line, app->page->tree, pdfapp_viewctm(app));
	if (error)
		pdfapp_error(app, error);

	p = 0;
	for (ln = line; ln; ln = ln->next)
	{
		if (ln->len > 0)
		{
			y = y0 - 1;

			for (i = 0; i < ln->len; i++)
			{
				bx0 = ln->text[i].bbox.x0;
				bx1 = ln->text[i].bbox.x1;
				by0 = ln->text[i].bbox.y0;
				by1 = ln->text[i].bbox.y1;
				c = ln->text[i].c;
				if (c < 32)
					c = '?';
				if (bx1 >= x0 && bx0 <= x1 && by1 >= y0 && by0 <= y1)
					if (p < ucslen - 1)
						ucsbuf[p++] = c;
			}

			if (by1 >= y0 && by0 <= y1)
			{
#ifdef WIN32
				if (p < ucslen - 1)
					ucsbuf[p++] = '\r';
#endif
				if (p < ucslen - 1)
					ucsbuf[p++] = '\n';
			}
		}
	}

	ucsbuf[p] = 0;

	pdf_droptextline(line);
#endif
}

