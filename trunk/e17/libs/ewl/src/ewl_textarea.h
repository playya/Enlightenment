
#ifndef __EWL_TEXTAREA_H__
#define __EWL_TEXTAREA_H__

typedef struct _ewl_textarea Ewl_TextArea;

#define EWL_TEXTAREA(textarea) ((Ewl_TextArea *) textarea)

struct _ewl_textarea {
	Ewl_Widget      widget;

	char           *text;

	Etox           *etox;
};

Ewl_Widget     *ewl_textarea_new(char *text);
void            ewl_textarea_init(Ewl_TextArea * ta, char *text);

void            ewl_textarea_set_text(Ewl_TextArea * ta, char *text);
char           *ewl_textarea_get_text(Ewl_TextArea * ta);

Etox           *ewl_textarea_get_etox(Ewl_TextArea * ta);

#endif /* __EWL_TEXTAREA_H__ */
