#include "Etox_private.h"

#include <string.h>

/*
 * etox_line_new - create a new line with the specified alignment
 * @align: the alignment of the current line
 *
 * Returns a pointer to the newly allocated line on success, NULL on failure.
 */
Etox_Line *etox_line_new(char align)
{
	Etox_Line *ret;

	ret = (Etox_Line *) malloc(sizeof(Etox_Line));
	if (ret) {
		memset(ret, 0, sizeof(Etox_Line));
		ret->flags |= align;
		ret->length = 1;
	}

	return ret;
}

/*
 * etox_line_free - free the data structures in a line
 * @line: the line that will be freed
 *
 * Returns no value. Frees all of the data tracked by @line as well as @line
 * itself.
 */
void etox_line_free(Etox_Line * line)
{
	Estyle *bit;

	CHECK_PARAM_POINTER("line", line);

	/*
	 * Only traverse the list if there are bits present.
	 */
	if (line->bits) {

		/*
		 * Free all of the bits on the line.
		 */
		while ((bit = ewd_list_remove_last(line->bits)))
			estyle_free(bit);

		/*
		 * Clean up the remaining list
		 */
		ewd_list_destroy(line->bits);
	}

	FREE(line);
}

/*
 * etox_line_show - display all of the bits in the selected line
 * @line: the line to be displayed
 *
 * Returns no value. Displays the text on the specified line.
 */
void etox_line_show(Etox_Line * line)
{
	Estyle *bit;

	CHECK_PARAM_POINTER("line", line);

	/*
	 * Display all of the bits in the line.
	 */
	ewd_list_goto_first(line->bits);
	while ((bit = ewd_list_next(line->bits)))
		estyle_show(bit);
}


/*
 * etox_line_hide - hide all the bits in the selected line
 * @line: the line to hide
 *
 * Returns no value
 */
void etox_line_hide(Etox_Line * line)
{
	Estyle *bit;

	CHECK_PARAM_POINTER("line", line);

	/*
	 * Hide all the bits in this line
	 */
	ewd_list_goto_first(line->bits);
	while ((bit = ewd_list_next(line->bits)))
		estyle_hide(bit);
}


/*
 * etox_line_append - append a bit to a line
 * @line: the line to append the bit
 * @bit: the bit to append to the line
 *
 * Returns no value. Appends the bit @bit to the line @line and updates
 * display to reflect the change.
 */
void etox_line_append(Etox_Line * line, Estyle * bit)
{
	int x, y, w, h;

	CHECK_PARAM_POINTER("line", line);
	CHECK_PARAM_POINTER("bit", bit);

	/*
	 * Create the list for the bits if none is present
	 */
	if (!line->bits)
		line->bits = ewd_list_new();

	/*
	 * Append the text and update necessary fields
	 */
	ewd_list_append(line->bits, bit);
	estyle_geometry(bit, &x, &y, &w, &h);

	line->w += w;
	if (h > line->h)
		line->h = h;
	line->length += estyle_length(bit);
}

/*
 * etox_line_prepend - prepend a bit to a line
 * @line: the line to prepend the bit
 * @bit: the bit to prepend to the line
 *
 * Returns no value. Prepends the bit @bit to the line @line and updates
 * display to reflect the change.
 */
void etox_line_prepend(Etox_Line * line, Estyle * bit)
{
	int x, y, w, h;

	CHECK_PARAM_POINTER("line", line);
	CHECK_PARAM_POINTER("bit", bit);

	/*
	 * Create the list for the bits if none is present
	 */
	if (!line->bits)
		line->bits = ewd_list_new();

	/*
	 * Prepend the text and update necessary fields
	 */
	ewd_list_prepend(line->bits, bit);
	estyle_geometry(bit, &x, &y, &w, &h);

	line->w += w;
	line->length += estyle_length(bit);
}

/*
 * etox_line_remove - remove a bit from the line
 * @line: the line to remove the bit
 * @bit: the bit to be from @line
 *
 * Removes @bit from @line and updates the appearance of surrounding bits to
 * reflect this change.
 */
void etox_line_remove(Etox_Line * line, Estyle * bit)
{
	CHECK_PARAM_POINTER("line", line);
	CHECK_PARAM_POINTER("bit", bit);

	ewd_list_goto(line->bits, bit);
	ewd_list_remove(line->bits);
	line->length -= estyle_length(bit);
	etox_line_minimize(line);
}

/*
 * etox_line_layout - layout the bits in a line across the etox
 * @line: the line that has the list of bits and bounding geometry
 *
 * Returns no value. Places the bits in @line across the screen and wraps them
 * appropriately around any fixed bits.
 */
void etox_line_layout(Etox_Line * line)
{
	int x;
	Estyle *bit;
	int tx, ty, tw, th;

	CHECK_PARAM_POINTER("line", line);

	if (!line->bits)
		return;

	ewd_list_goto_first(line->bits);

	/*
	 * Determine the horizontal alignment of the text and set the starting
	 * x coordinate appropriately.
	 */
	if (line->flags & ETOX_ALIGN_LEFT) {
		x = line->x;
	} else if (line->flags & ETOX_ALIGN_RIGHT) {
		x = line->et->x + line->et->w - line->w;
	} else {
		x = line->et->x + (line->et->w / 2) - (line->w / 2);
	}

	/*
	 * Determine the veritcal alignment and perform the layout of the
	 * bits.
	 */
	while ((bit = ewd_list_next(line->bits))) {
		if (!estyle_fixed(bit)) {

			estyle_geometry(bit, &tx, &ty, &tw, &th);
			if (line->h < th)
				line->h = th;

			/*
			 * Adjust the y position based on alignment.
			 */
			if (line->flags & ETOX_ALIGN_TOP)
				ty = line->y;
			else if (line->flags & ETOX_ALIGN_BOTTOM)
				ty = line->y + line->h - th;
			else
				ty = line->y + (line->h / 2) - (th / 2);

			/*
			 * Move the evas object into place.
			 */
			estyle_move(bit, x, ty);
		}

		/*
		 * Move horizontally to place the next bit.
		 */
		x += tw;
	}
}

/*
 * etox_line_minimize - reduce the number of bits on a line
 */
void etox_line_minimize(Etox_Line * line)
{
	Estyle *bit, *last_bit = NULL;

	CHECK_PARAM_POINTER("line", line);

	ewd_list_goto_first(line->bits);
	while ((bit = ewd_list_current(line->bits))) {

		/*
		 * Attempt to merge the bits if possible, remove the second
		 * one if successful.
		 */
		if (estyle_merge(last_bit, bit))
			ewd_list_remove(line->bits);

		ewd_list_next(line->bits);
		last_bit = bit;
	}
}

/*
 * etox_line_merge - merge two lines into the first line, free the second
 * @line1: the destination of the merged lines
 * @line2: the line that will be merged with line1
 *
 * Returns no value. Moves the bits from line2 into line 1.
 */
void etox_line_merge(Etox_Line * line1, Etox_Line * line2)
{
	Estyle *bit;

	CHECK_PARAM_POINTER("line1", line1);
	CHECK_PARAM_POINTER("line2", line2);

	/*
	 * Move the bits from line2 to line1.
	 */
	while ((bit = ewd_list_remove_first(line2->bits)))
		etox_line_append(line1, bit);

	/*
	 * Adjust the height and length of the merged line.
	 */
	if (line2->h > line1->h)
		line1->h = line2->h;
	line1->length += line2->length;

	/*
	 * Destroy the line that was merged.
	 */
	etox_line_free(line2);
}

/*
 * etox_line_get_text - retrieve the text from a specified line into a buffer
 * @line: the line to retrieve text
 * @buf: the char buffer to store the found text, must have enough space
 *
 * Returns no value. Saves the text from the line @line into the char buffer
 * @buf.
 */
void etox_line_get_text(Etox_Line *line, char *buf)
{
	char *temp;
	Estyle *es;

	CHECK_PARAM_POINTER("line", line);
	CHECK_PARAM_POINTER("buf", buf);

	ewd_list_goto_first(line->bits);

	/*
	 * Examine each bit on the list of bits and cat it's text onto the end
	 * of the buffer. Then append a \n to the buffer at the end of the
	 * line.
	 */
	while ((es = ewd_list_next(line->bits))) {
		temp = estyle_get_text(es);
		strcat(buf, temp);
	}

	strcat(buf, "\n");
}
