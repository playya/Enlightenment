
#ifndef __EWL_ENTRY_H__
#define __EWL_ENTRY_H__

typedef struct _ewl_entry Ewl_Entry;
#define EWL_ENTRY(entry) ((Ewl_Entry *) entry)

struct _ewl_entry {
	Ewl_Box box;

	Ewl_Widget * text;
};

Ewl_Widget *ewl_entry_new(void);
void ewl_entry_set_text(Ewl_Widget * w, char *t);
char *ewl_entry_get_text(Ewl_Widget * w);

#endif /* __EWL_ENTRY_H__ */
