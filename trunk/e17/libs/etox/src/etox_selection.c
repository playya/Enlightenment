#include "Etox_private.h"

Evas_List *active_selections = NULL;

#define SELECTION_LOOP_START(selected) \
do { \
	Evas_Object *bit = NULL; \
	Etox_Line *line; \
	Evas_List *l, *bl; \
	double w, h; \
	line = selected->start.line; \
	l = evas_list_find_list(selected->etox->lines, selected->start.line); \
	bl = evas_list_find_list(line->bits, selected->start.bit); \
	while (bl && bit != selected->end.bit) { \
		bit = bl->data; \
		evas_object_geometry_get(bit, NULL, NULL, &w, NULL); \
		line->w -= w

#define SELECTION_LOOP_END \
		evas_object_geometry_get(bit, NULL, NULL, &w, &h); \
		line->w += w; \
		if (h > line->h) \
			line->h = h; \
		bl = bl->next; \
		if (!bl) { \
			l = l->next; \
			if (l) { \
				line = l->data; \
				bl = line->bits; \
			} \
		} \
	} \
} while (0)


Evas_Object *
etox_split_bit(Etox_Line *line, Evas_Object *bit, int index)
{
	Evas_List *l;
	Evas_Object *point = bit;
	Etox_Selection *selected;

	/*
	 * Split the leading bit, only need to inform selections that end with
	 * this bit.
	 */
	if (index && index < estyle_length(bit)) {
		point = estyle_split(bit, index);
		evas_object_smart_member_add(point, line->et->smart_obj);
		line->bits = evas_list_append_relative(line->bits, point, bit);

		l = active_selections;
		while (l) {
			selected = l->data;
			if (selected->end.bit == bit)
				selected->end.bit = point;
			l = l->next;
		}
	}

	return point;
}

Etox_Selection *
etox_selection_new(Etox *etox, Etox_Line *l1, Etox_Line *l2,
		Evas_Object *s1, Evas_Object *s2, int i1, int i2)
{
	Evas_Object *temp;
	Etox_Selection *selected;

	/*
	 * Split bits on their index boundaries, this updates selections that
	 * contain the bits.
	 */
	temp = etox_split_bit(l1, s1, i1);
	if (s1 == s2) {
		i2 -= i1;
		s2 = temp;
	}
	s1 = temp;

	/*
	 * Split on the ending index, we use the original s2 for the end,
	 * since it's the bit portion before the split.
	 */
	etox_split_bit(l2, s2, i2);

	selected = calloc(1, sizeof(Etox_Selection));
	memset(selected, 0, sizeof(Etox_Selection));

	selected->etox = etox;

	selected->start.line = l1;
	selected->start.bit = s1;

	selected->end.line = l2;
	selected->end.bit = s2;

	active_selections = evas_list_prepend(active_selections, selected);

	etox_layout(etox);

	return selected;
}

/**
 */
void
etox_selection_free(Etox_Selection *selected)
{
        CHECK_PARAM_POINTER("selected", selected);

	active_selections = evas_list_remove(active_selections, selected);
	FREE(selected);
}

/**
 */
void
etox_selection_free_by_etox(Evas_Object *obj)
{
	Etox *etox;
	Evas_List *l, *r = NULL;
	Etox_Selection *selected;

	CHECK_PARAM_POINTER("obj", obj);

	etox = evas_object_smart_data_get(obj);

        /*
         * loop through all active selections. add the ones on etox
         * to a second list, to be removed later.
         */
	for (l = active_selections; l; l = l->next) {
		selected = l->data;
		if (selected->etox == etox) {
                        r = evas_list_append(r, selected);
		}
	}


        for (l = r; l; l = l->next)
        {
          selected = l->data;

          active_selections = evas_list_remove(active_selections, selected);
          free(selected);
        }

        evas_list_free(r);
}

/**
 */
Etox_Selection *
etox_select_coords(Evas_Object *obj, double sx, double sy, double ex, double ey)
{
	int i1, i2;
	Etox *et;
	Etox_Line *sl, *el = NULL;
	Evas_Object *sb, *eb = NULL;
	Etox_Selection *selected = NULL;

	CHECK_PARAM_POINTER_RETURN("obj", obj, NULL);

	et = evas_object_smart_data_get(obj);

	sl = etox_coord_to_line(et, sy);
	if (!sl)
		goto out;

	el = etox_coord_to_line(et, ey);
	if (!el)
		goto out;

	sb = etox_line_coord_to_bit(sl, sx);
	if (!sb)
		goto out;

	eb = etox_line_coord_to_bit(el, ex);
	if (!eb)
		goto out;

	i1 = estyle_text_at_position(sb, sx, sy, NULL, NULL, NULL, NULL);
	i2 = estyle_text_at_position(eb, sx, sy, NULL, NULL, NULL, NULL);

	selected = etox_selection_new(et, sl, el, sb, eb, i1, i2);

out:
	return selected;
}

/**
 * etox_select_index - create a selection based on two indices
 * @et: the etox to choose the selection
 * @si: the starting index of characters to be selected
 * @ei: the ending index of characters to be selected
 *
 * Returns a newly allocated selection on success, NULL on failure.
 */
Etox_Selection *
etox_select_index(Evas_Object * obj, int si, int ei)
{
	Etox *et;
	Etox_Line *sl = NULL, *el = NULL;
	Evas_Object *sb = NULL, *eb = NULL;
	Etox_Selection *selected = NULL;

	CHECK_PARAM_POINTER_RETURN("obj", obj, NULL);

	et = evas_object_smart_data_get(obj);

	/*
	 * First determine the lines containing the indices.
	 */
	sl = etox_index_to_line(et, &si);
	if (!sl)
		goto out;

	el = etox_index_to_line(et, &ei);
	if (!el)
		goto out;

	sb = etox_line_index_to_bit(sl, &si);
	if (!sb)
		goto out;

	eb = etox_line_index_to_bit(el, &ei);
	if (!eb)
		goto out;

	/*
	 * Create the new selection and assign it's fields
	 */
	selected = etox_selection_new(et, sl, el, sb, eb, si, ei);

out:
	return selected;
}

/**
 */
Etox_Selection *
etox_select_str(Evas_Object * obj, char *match, char **last)
{
	Etox *et;

	CHECK_PARAM_POINTER_RETURN("obj", obj, NULL);

	et = evas_object_smart_data_get(obj);

	return NULL;
}

/**
 */
void
etox_selection_bounds(Etox_Selection *selected, double *sx, double *sy,
		double *ex, double *ey)
{
}

/**
 */
void
etox_selection_add_callback(Etox_Selection *selected,
		Evas_Callback_Type callback, void (*func) (void *data, Evas *e,
					       Evas_Object *o, int b, int x,
					       int y), void *data)
{
}

/**
 */
void
etox_selection_del_callback(Etox_Selection *selected,
		Evas_Callback_Type callback)
{
}

void
etox_selection_apply_context(Etox_Selection *selected,
                             Etox_Context *context)
{
  Evas_List *l;
  Etox_Line *line;

  if (selected->start.line == selected->end.line)
  {
    etox_line_apply_context(selected->start.line, context,
                            selected->start.bit, selected->end.bit);
  }

  else
  {
    /* start on the first line */
    l = evas_list_find_list(selected->etox->lines, selected->start.line);

    line = l->data;
    
    for (; l; l = l->next)
    {
      line = l->data;

      /*
       * if start.bit is not on line, then the first bit of line will be
       * used as the starting bit. same for end.bit.
       */
      etox_line_apply_context(line, context, selected->start.bit,
                              selected->end.bit);
      
      if (line == selected->end.line)
        break;
    }
  }
  
  etox_layout(selected->etox);
}

void
etox_selections_update(Evas_Object *bit, Etox_Line *line)
{
	Evas_List *l;

	for (l = active_selections; l; l = l->next)
	{
		Etox_Selection *selected = evas_list_data(l);
		
		if (selected->start.bit == bit)
		{
		  selected->start.line = line;
		}

		if (selected->end.bit == bit)
		{
		  selected->end.line = line;
		}
	}
}
