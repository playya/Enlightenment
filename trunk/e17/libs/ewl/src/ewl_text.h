
#ifndef __EWL_TEXT_H
#define __EWL_TEXT_H

typedef struct _ewl_text Ewl_Text;

#define EWL_TEXT(text) ((Ewl_Text *) text)

struct _ewl_text
{
	Ewl_Widget widget;
	Etox *tox;
	Etox_Color color;
	Etox_Style style;

	char *text;
	char *font;
	int font_size;
	int padding;
};

Ewl_Widget *ewl_text_new();

void ewl_text_set_text(Ewl_Text * t, char *text);
char *ewl_text_get_text(Ewl_Text * t);
void ewl_text_set_font(Ewl_Text * t, char *font);
char *ewl_text_get_font(Ewl_Text * t);
void ewl_text_set_font_size(Ewl_Text * t, int size);
int ewl_text_get_font_size(Ewl_Text * t);
void ewl_text_set_color(Ewl_Text * t, int r, int g, int b, int a);
void ewl_text_get_color(Ewl_Text * t, int *r, int *g, int *b, int *a);
void ewl_text_get_text_geometry(Ewl_Text * t, int *xx, int *yy,
				int *ww, int *hh);
void ewl_text_set_style(Ewl_Text * t, char *s);
void ewl_text_get_letter_geometry(Ewl_Text * t, int index, int *xx,
				  int *yy, int *ww, int *hh);
void ewl_text_get_letter_geometry_at(Ewl_Text * t, int x, int y,
				     int *tx, int *ty, int *tw, int *th);
void ewl_text_set_alignment(Ewl_Text * t, Ewl_Alignment a);
int ewl_text_get_index_at(Ewl_Text * t, int x, int y);

#endif
