/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

/* FIXME:
 * 
 * things to add:
 * 
 * * finish off current api where it is unfinished
 * * get native extents
 * * styles (outline, glow, etxra glow, shadow, soft shadow, etc.)
 * * if a word (or char) doesnt fit at all do something sensible
 * * anchors (to query text extents)
 * * tabs (indents)
 * * left and right margins
 * * get all the current format params at any point
 * * freeze thaw api
 * 
 * tough ones:
 * * overflow objects (overflow from this textblock can go into another)
 * * on change figure out what node the change is in and figure out what line (nodes) it affects and only modify those nodes on that line or maybe others until changes dont happen further down
 * * obstacle objects to wrap around
 * * kerning control
 * * fully justified text (space chars evenly to fit the line)
 * 
 * really tough ones:
 * * fix core text code to properly handle unicode codeponts - handle ALL unicode domains properly.
 * * right to left text
 * * top down (japanese/chinese) text layout
 * 
 * insanely tough ones:
 * * for ultra-huge documents determine paging sections of the text in/out and being able to provide callbacks form the user api that can provide the text
 * 
 */

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

/* private magic number for textblock objects */
static const char o_type[] = "textblock";

/* private struct for textblock object internal data */
typedef struct _Evas_Object_Textblock      Evas_Object_Textblock;
typedef struct _Layout                     Layout;
typedef struct _Node                       Node;
typedef struct _Layout_Node                Layout_Node;

/* the current state of the formatting */
struct _Layout
{
   struct {
      char             *name;
      char             *source;
      Evas_Font_Size    size;
      void             *font;
   } font;
   struct {
      unsigned char     r, g, b, a;
   } color, 
     underline_color, 
     double_underline_color, 
     outline_color, 
     shadow_color, 
     glow_color, 
     outer_glow_color, 
     backing_color,
     strikethrough_color
     ;
   struct {
      int               inset, x, y, 
	                ascent, descent, 
	                mascent, mdescent,
	                advance;
   } line;
   double               align, valign;
   unsigned char        word_wrap : 1;
   unsigned char        underline : 1;
   unsigned char        second_underline : 1;
   unsigned char        strikethrough : 1;
   unsigned char        backing : 1;
};

/* a node of formatting data */
struct _Node
{
   Evas_Object_List _list_data;

   char *format; /* format data */
   char *text; /* text data until the next node */
   int   text_len; /* length of the text */
};

/* a node of formatting data */
struct _Layout_Node
{
   Evas_Object_List _list_data;

   /* the current state */
   Layout        layout;
   char         *text; /* text data until the next node */
   int           text_pos, text_len;
   int           w, h;
   Node         *source_node;
   int           source_pos;
   int           line;
   unsigned char line_start : 1;
   unsigned char line_end : 1;
};

struct _Evas_Object_Textblock
{
   DATA32                 magic;
   struct {
      int                 dummy;
   } cur, prev;
   char                   changed : 1;
   
   int                    pos, len, lines;
   Evas_Format_Direction  format_dir;
   Node                  *nodes;
   Layout_Node           *layout_nodes;
   Evas_Coord             last_w, last_h;
   struct {
      unsigned char       dirty : 1;
      Evas_Coord          w, h;
   } native, format;
   Evas_List             *font_hold;
   void                  *engine_data;
};

static void
evas_object_textblock_layout_init(Layout *layout)
{
   layout->font.name = NULL;
   layout->font.source = NULL;
   layout->font.size = 0;
   layout->font.font = NULL;
   layout->color.r = 255;
   layout->color.g = 255;
   layout->color.b = 255;
   layout->color.a = 255;
   layout->underline_color = layout->color;
   layout->double_underline_color = layout->color;
   layout->outline_color = layout->color;
   layout->shadow_color = layout->color;
   layout->glow_color = layout->color;
   layout->outer_glow_color = layout->color;
   layout->backing_color = layout->color;
   layout->strikethrough_color = layout->color;
   layout->line.inset = 0;
   layout->line.x = 0;
   layout->line.y = 0;
   layout->line.ascent = 0;
   layout->line.descent = 0;
   layout->line.mascent = 0;
   layout->line.mdescent = 0;
   layout->line.advance = 0;
   layout->align = 0.0;
   layout->valign = -1.0;
   layout->word_wrap = 0;
   layout->underline = 0;
   layout->second_underline = 0;
   layout->strikethrough = 0;
   layout->backing = 0;
}

static char *
evas_object_textblock_format_merge(char *ofmt, char *fmt)
{
   return strdup(fmt);
}

static int
evas_object_textblock_hex_string_get(char ch)
{
   if ((ch >= '0') && (ch <= '9')) return (ch - '0');
   else if ((ch >= 'A') && (ch <= 'F')) return (ch - 'A' + 10);
   else if ((ch >= 'a') && (ch <= 'f')) return (ch - 'a' + 10);
   return 0;
}

static void
evas_object_textblock_layout_format_apply(Layout *layout, char *key, char *data)
{
   if (!strcmp(key, "font"))
     {
	if (layout->font.name) free(layout->font.name);
	layout->font.name = strdup(data);
     }
   else if (!strcmp(key, "font_source"))
     {
	if (layout->font.source)
	  {
	     free(layout->font.source);
	     layout->font.source = NULL;
	  }
	if (!((data[0] == 0) ||
	      (!strcmp(data, "0")) ||
	      (!strcmp(data, "NULL")) ||
	      (!strcmp(data, "null")) ||
	      (!strcmp(data, "(null)"))))
	  layout->font.source = strdup(data);
     }
   else if (!strcmp(key, "size"))
     {
	layout->font.size = atoi(data);
     }
   else if (!strcmp(key, "align"))
     {
	if (!strcmp(data, "left")) layout->align = 0.0;
	else if (!strcmp(data, "middle")) layout->align = 0.5;
	else if (!strcmp(data, "center")) layout->align = 0.5;
	else if (!strcmp(data, "right")) layout->align = 1.0;
	else
	  {
	     layout->align = atof(data);
	     if (layout->align < 0.0) layout->align = 0.0;
	     else if (layout->align > 1.0) layout->align = 1.0;
	  }
     }
   else if (!strcmp(key, "valign"))
     {
	if (!strcmp(data, "top")) layout->valign = 0.0;
	else if (!strcmp(data, "middle")) layout->valign = 0.5;
	else if (!strcmp(data, "center")) layout->valign = 0.5;
	else if (!strcmp(data, "bottom")) layout->valign = 1.0;
	else if (!strcmp(data, "baseline")) layout->valign = -1.0;
	else if (!strcmp(data, "base")) layout->valign = -1.0;
	else
	  {
	     layout->valign = atof(data);
	     if (layout->valign < 0.0) layout->valign = 0.0;
	     else if (layout->valign > 1.0) layout->valign = 1.0;
	  }
     }
   else if ((!strcmp(key, "color")) ||
	    (!strcmp(key, "underline_color")) ||
	    (!strcmp(key, "double_underline_color")) ||
	    (!strcmp(key, "outline_color")) ||
	    (!strcmp(key, "shadow_color")) ||
	    (!strcmp(key, "glow_color")) ||
	    (!strcmp(key, "outer_glow_color")) ||
	    (!strcmp(key, "backing_color")) ||
	    (!strcmp(key, "strikethrough_color")))
     {
	/* #RRGGBB[AA] or #RGB[A] */
	if (data[0] == '#')
	  {
	     int r, g, b, a;
	     
	     if (strlen(data) == 7) /* #RRGGBB */
	       {
		  r = (evas_object_textblock_hex_string_get(data[1]) << 4) |
		    (evas_object_textblock_hex_string_get(data[2]));
		  g = (evas_object_textblock_hex_string_get(data[3]) << 4) |
		    (evas_object_textblock_hex_string_get(data[4]));
		  b = (evas_object_textblock_hex_string_get(data[5]) << 4) |
		    (evas_object_textblock_hex_string_get(data[6]));
		  a = 0xff;
	       }
	     else if (strlen(data) == 9) /* #RRGGBBAA */
	       {
		  r = (evas_object_textblock_hex_string_get(data[1]) << 4) |
		    (evas_object_textblock_hex_string_get(data[2]));
		  g = (evas_object_textblock_hex_string_get(data[3]) << 4) |
		    (evas_object_textblock_hex_string_get(data[4]));
		  b = (evas_object_textblock_hex_string_get(data[5]) << 4) |
		    (evas_object_textblock_hex_string_get(data[6]));
		  a = (evas_object_textblock_hex_string_get(data[7]) << 4) |
		    (evas_object_textblock_hex_string_get(data[8]));
	       }
	     else if (strlen(data) == 4) /* #RGB */
	       {
		  r = evas_object_textblock_hex_string_get(data[1]);
		  r = (r << 4) | r;
		  g = evas_object_textblock_hex_string_get(data[2]);
		  g = (g << 4) | g;
		  b = evas_object_textblock_hex_string_get(data[3]);
		  b = (b << 4) | b;
		  a = 0xff;
	       }
	     else if (strlen(data) == 5) /* #RGBA */
	       {
		  r = evas_object_textblock_hex_string_get(data[1]);
		  r = (r << 4) | r;
		  g = evas_object_textblock_hex_string_get(data[2]);
		  g = (g << 4) | g;
		  b = evas_object_textblock_hex_string_get(data[3]);
		  b = (b << 4) | b;
		  a = evas_object_textblock_hex_string_get(data[4]);
		  a = (a << 4) | a;
	       }
	     if (!strcmp(key, "color"))
	       {
		  layout->color.r = r;
		  layout->color.g = g;
		  layout->color.b = b;
		  layout->color.a = a;
	       }
	     else if (!strcmp(key, "underline_color"))
	       {
		  layout->underline_color.r = r;
		  layout->underline_color.g = g;
		  layout->underline_color.b = b;
		  layout->underline_color.a = a;
	       }
	     else if (!strcmp(key, "double_underline_color"))
	       {
		  layout->double_underline_color.r = r;
		  layout->double_underline_color.g = g;
		  layout->double_underline_color.b = b;
		  layout->double_underline_color.a = a;
	       }
	     else if (!strcmp(key, "outline_color"))
	       {
		  layout->outline_color.r = r;
		  layout->outline_color.g = g;
		  layout->outline_color.b = b;
		  layout->outline_color.a = a;
	       }
	     else if (!strcmp(key, "shadow_color"))
	       {
		  layout->shadow_color.r = r;
		  layout->shadow_color.g = g;
		  layout->shadow_color.b = b;
		  layout->shadow_color.a = a;
	       }
	     else if (!strcmp(key, "glow_color"))
	       {
		  layout->glow_color.r = r;
		  layout->glow_color.g = g;
		  layout->glow_color.b = b;
		  layout->glow_color.a = a;
	       }
	     else if (!strcmp(key, "outer_glow_color"))
	       {
		  layout->outer_glow_color.r = r;
		  layout->outer_glow_color.g = g;
		  layout->outer_glow_color.b = b;
		  layout->outer_glow_color.a = a;
	       }
	     else if (!strcmp(key, "backing_color"))
	       {
		  layout->backing_color.r = r;
		  layout->backing_color.g = g;
		  layout->backing_color.b = b;
		  layout->backing_color.a = a;
	       }
	     else if (!strcmp(key, "strikethrough_color"))
	       {
		  layout->strikethrough_color.r = r;
		  layout->strikethrough_color.g = g;
		  layout->strikethrough_color.b = b;
		  layout->strikethrough_color.a = a;
	       }
	  }
     }
   else if (!strcmp(key, "wrap"))
     {
	if (!strcmp(data, "word")) layout->word_wrap = 1;
	else layout->word_wrap = 0;
     }
   else if (!strcmp(key, "underline"))
     {
	if (!strcmp(data, "off"))
	  {
	     layout->underline = 0;
	     layout->second_underline = 0;
	  }
	else if ((!strcmp(data, "on")) ||
		 (!strcmp(data, "single")))
	  {
	     layout->underline = 1;
	     layout->second_underline = 0;
	  }
	else if (!strcmp(data, "double"))
	  {
	     layout->underline = 1;
	     layout->second_underline = 1;
	  }
     }
   else if (!strcmp(key, "strikethrough"))
     {
	if (!strcmp(data, "off"))
	  layout->strikethrough = 0;
	else if (!strcmp(data, "on"))
	  layout->strikethrough = 1;
     }
   else if (!strcmp(key, "backing"))
     {
	if (!strcmp(data, "off"))
	  layout->backing = 0;
	else if (!strcmp(data, "on"))
	  layout->backing = 1;
     }
}

static void
evas_object_textblock_layout_format_modify(Layout *layout, const char *format)
{
   const char *p, *k1 = NULL, *k2 = NULL, *d1 = NULL, *d2 = NULL;
   int inquote = 0, inescape = 0;
   
   if (!format) return;
   p = format - 1;
   do
     {
	p++;
	/* we dont have the start of a key yet */
	if (!k1)
	  {
	     if (isalnum(*p)) k1 = p;
	  }
	else if (!k2)
	  {
	     if (*p == '=') k2 = p;
	  }
	else if (!d1)
	  {
	     if (*p == '\'') inquote = 1;
	     else d1 = p;
	  }
	else if (!d2)
	  {
	     if (inquote)
	       {
		  if (!inescape)
		    {
		       if (*p == '\\') inescape = 1;
		       else if (*p == '\'') d2 = p;
		    }
		  else
		    inescape = 0;
	       }
	     else
	       {
		  if ((isblank(*p)) || (*p == 0)) d2 = p;
	       }
	     if (d2)
	       {
		  char *key, *data;
		  
		  key = malloc(k2 - k1 + 1);
		  data = malloc(d2 - d1 + 1);
		  strncpy(key, k1, k2 - k1);
		  key[k2 - k1] = 0;
		  if (inquote)
		    {
		       const char *p2;
		       char *dst;
		       
		       inescape = 0;
		       p2 = d1;
		       dst = data;
		       while (p2 != d2)
			 {
			    if (!inescape)
			      {
				 if (*p == '\\') inescape = 1;
			      }
			    else
			      {
				 *dst = *p;
				 dst++;
				 inescape = 0;
			      }
			    p2++;
			 }
		       *dst = 0;
		    }
		  else
		    {
		       strncpy(data, d1, d2 - d1);
		       data[d2 - d1] = 0;
		    }
		  k1 = k2 = d1 = d2 = NULL;
		  inquote = 0;
		  inescape = 0;
		  evas_object_textblock_layout_format_apply(layout, key, data);
		  free(key);
		  free(data);
	       }
	  }
     }
   while (*p);
}

static void
evas_object_textblock_layout_copy(Layout *layout, Layout *layout_dst)
{
   *layout_dst = *layout;
   if (layout->font.name) layout_dst->font.name = strdup(layout->font.name);
   if (layout->font.source) layout_dst->font.source = strdup(layout->font.source);
}

static void
evas_object_textblock_layout_clear(Evas_Object *obj, Layout *layout)
{
   if (layout->font.name) free(layout->font.name);
   if (layout->font.source) free(layout->font.source);
   if (layout->font.font) ENFN->font_free(ENDT, layout->font.font);
   memset(layout, 0, sizeof(Layout));
}

static void
evas_object_textblock_layout_fonts_hold(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Evas_Object_List *l;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   for (l = (Evas_Object_List *)o->layout_nodes; l; l = l->next)
     {
	Layout_Node *lnode;
	
	lnode = (Layout_Node *)l;
	if (lnode->layout.font.font)
	  {
	     o->font_hold = evas_list_append(o->font_hold,
					     lnode->layout.font.font);
	     lnode->layout.font.font = NULL;
	  }
     }
}

static void
evas_object_textblock_layout_fonts_hold_clean(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   
   while (o->font_hold)
     {
	ENFN->font_free(ENDT, o->font_hold->data);
	o->font_hold = evas_list_remove_list(o->font_hold, o->font_hold);
     }
}

static void
evas_object_textblock_layout_clean(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   while (o->layout_nodes)
     {
	Layout_Node *lnode;
	
	lnode = (Layout_Node *)o->layout_nodes;
	o->layout_nodes = evas_object_list_remove(o->layout_nodes, lnode);
	evas_object_textblock_layout_clear(obj, &lnode->layout);
	if (lnode->text) free(lnode->text);
	free(lnode);
     }
}

static void
evas_object_textblock_contents_clean(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   while (o->nodes)
     {
	Node *node;
	
	node = (Node *)o->nodes;
	o->nodes = evas_object_list_remove(o->nodes, node);
	if (node->format) free(node->format);
	if (node->text) free(node->text);
	free(node);
     }
}

static int
evas_object_textblock_char_is_white(int c)
{
   /*
    * unicode list of whitespace chars
    * 
    * 0009..000D <control-0009>..<control-000D>
    * 0020 SPACE
    * 0085 <control-0085>
    * 00A0 NO-BREAK SPACE
    * 1680 OGHAM SPACE MARK
    * 180E MONGOLIAN VOWEL SEPARATOR
    * 2000..200A EN QUAD..HAIR SPACE
    * 2028 LINE SEPARATOR
    * 2029 PARAGRAPH SEPARATOR
    * 202F NARROW NO-BREAK SPACE
    * 205F MEDIUM MATHEMATICAL SPACE
    * 3000 IDEOGRAPHIC SPACE
    */
   if (
       (c == 0x20) ||
       ((c >= 0x9) && (c <= 0xd)) ||
       (c == 0x85) ||
       (c == 0xa0) ||
       (c == 0x1680) ||
       (c == 0x180e) ||
       ((c >= 0x2000) && (c <= 0x200a)) ||
       (c == 0x2028) ||
       (c == 0x2029) ||
       (c == 0x202f) ||
       (c == 0x205f) ||
       (c == 0x3000)
       )
     return 1;
   return 0;
}

static void
evas_object_textblock_layout(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Layout layout;
   Evas_Object_List *l, *ll;
   Evas_Coord w, h;
   Layout_Node *line_start = NULL;
   int text_pos = 0, fh = 0, last_mdescent = 0, line = 0, last_line = 0;

   o = (Evas_Object_Textblock *)(obj->object_data);
   evas_object_textblock_layout_init(&layout);
   w = obj->cur.geometry.w;
   h = obj->cur.geometry.h;
   o->last_w = w;
   o->last_h = h;
   /* format width is object width */
//   printf("RE-LAYOUT %ix%i!\n", w, h);
   for (l = (Evas_Object_List *)o->nodes; l; l = l->next)
     {
	Layout_Node *lnode;
	Node *node;
	
	/* FIXME: we cant do this - we need to be able to qury text
	 * overflow amounts */
	/*
	if (layout.line.y >= h) goto breakout;
	 */
	node = (Node *)l;
//	printf("NODE: FMT:\"%s\" TXT:\"%s\"\n", node->format, node->text);
	if (node->format)
	  {
	     /* first handle newline, tab etc. etc */
	     if (node->format[0] == '\n')
	       {
		  if (lnode)
		    lnode->line_end = 1;
		  layout.line.x = 0;
		  if ((layout.line.y + layout.line.mascent + layout.line.mdescent) > h)
		    {
		       /* FIXME: this node would overflow to the next textblock */
		    }
		  line++;
		  layout.line.y += layout.line.mascent + layout.line.mdescent;
		  fh = layout.line.y;
		  last_mdescent = 3;
		  line_start = NULL;
	       }
	     else
	       evas_object_textblock_layout_format_modify(&layout, node->format);
	  }
	if (node->text)
	  {
	     int inset = 0, hadvance = 0, vadvance = 0;
	     int ascent = 0, descent = 0, tw = 0, th = 0;
	     int chrpos = -1, nchrpos = -1, x, y, cx, cy, cw, ch;
	     void *font = NULL;
	     char *text;
	     int adj, lastnode;
	     int srcpos = 0;

	     text = strdup(node->text);
	     new_node:
	     /* FIXME: we cant do this - we need to be able to qury text
	      * overflow amounts */
	     /*
	     if (layout.line.y >= h)
	       {
		  free(text);
		  goto breakout;
	       }
	      */
	     lnode = calloc(1, sizeof(Layout_Node));
	     lnode->source_node = node;
	     lnode->source_pos = srcpos;
	     lnode->line = line;
	     last_line = line;
	     evas_object_textblock_layout_copy(&layout, &(lnode->layout));
	     if (lnode->layout.font.name)
	       font = evas_font_load(obj->layer->evas, lnode->layout.font.name, lnode->layout.font.source, lnode->layout.font.size);
	     /* if this is at the start of the line... */
	     if (layout.line.x == 0)
	       {
		  if (font) inset = ENFN->font_inset_get(ENDT, font, text);
		  layout.line.inset = inset;
		  layout.line.x = -inset;
		  layout.line.mascent = 0;
		  layout.line.mdescent = 0;
		  line_start = lnode;
		  lnode->line_start = 1;
	       }
	     lnode->layout.font.font = font;
	     if (font) ascent = ENFN->font_max_ascent_get(ENDT, font);
	     if (font) descent = ENFN->font_max_descent_get(ENDT, font);
	     lnode->layout.line.ascent = ascent;
	     lnode->layout.line.descent = descent;
	     layout.line.ascent = ascent;
	     layout.line.descent = descent;
	     if (layout.line.mascent < ascent) layout.line.mascent = ascent;
	     if (layout.line.mdescent < descent) layout.line.mdescent = descent;
	     if (font) chrpos = ENFN->font_char_at_coords_get(ENDT, font, text, 
							      w - layout.line.x, 0, 
							      &cx, &cy, &cw, &ch);
	     /* if the text fits... just add */
	     if (chrpos < 0)
	       {
		  int more_text_on_line = 0;
		  if (font) ENFN->font_string_size_get(ENDT, font, text, &tw, &th);
		  lnode->w = tw;
		  lnode->h = th;
		  lnode->text = text;
		  lnode->text_len = strlen(lnode->text);
		  lnode->text_pos = text_pos;
		  text_pos += lnode->text_len;
		  if (font) hadvance = ENFN->font_h_advance_get(ENDT, font, text);
		  o->layout_nodes = evas_object_list_append(o->layout_nodes, lnode);
		  /* and advance */
		  /* fix up max ascent/descent for the line */
		  adj = (double)(w - (lnode->layout.line.x + tw + layout.line.inset)) * layout.align;
		  adj -= line_start->layout.line.x;
		  layout.line.x += hadvance;
		  lnode->layout.line.advance = hadvance;
		  for (ll = l->next; ll; ll = ll->next)
		    {
		       Node *node2;
		       
		       node2 = (Node *)ll;
		       if (node2->text)
			 {
			    more_text_on_line = 1;
			    break;
			 }
		       else if ((node2->format) && (node2->format[0] == '\n'))
			 {
			    more_text_on_line = 0;
			    break;
			 }
		    }
		  if (!more_text_on_line)
		    {
		       for (ll = (Evas_Object_List *)lnode; ll; ll = ll->prev)
			 {
			    Layout_Node *lnode2;
			    
			    lnode2 = (Layout_Node *)ll;
			    lnode2->layout.line.x += adj;
			    lnode2->layout.line.mascent = layout.line.mascent;
			    lnode2->layout.line.mdescent = layout.line.mdescent;
			    if (ll == (Evas_Object_List *)line_start) break;
			 }
		       lnode->line_end = 1;
		       fh = layout.line.y + layout.line.mascent + layout.line.mdescent;
		    }
	       }
	     /* text doesnt fit */
	     else
	       {
		  nchrpos = chrpos;
		  /* handle word wrap */
		  if (layout.word_wrap)
		    {
		       int ppos, pos, chr;
		       
		       pos = chrpos;
		       chr = evas_common_font_utf8_get_prev(text, &pos);
		       ppos = pos = chrpos;
		       while ((!evas_object_textblock_char_is_white(chr))
			      &&
			      (pos >= 0) && 
			      (chr > 0))
			 {
			    ppos = pos;
			    chr = evas_common_font_utf8_get_prev(text, &pos);
			 }
		       chr = evas_common_font_utf8_get_next(text, &ppos);
		       if (ppos < 0) ppos = 0;
		       chrpos = ppos;
		       while ((evas_object_textblock_char_is_white(chr))
			      &&
			      (pos >= 0) && 
			      (chr > 0))
			 {
			    ppos = pos;
			    chr = evas_common_font_utf8_get_prev(text, &pos);
			 }
		       chr = evas_common_font_utf8_get_next(text, &ppos);
		       if (ppos < 0) ppos = 0;
		       nchrpos = ppos;
		    }
		  /* if the first char in the line can't fit!!! */
		  if ((chrpos == 0) && (lnode == line_start))
		    {
		       /* the first char can't fit. put it in there anyway */
		       /* FIXME */
		       free(text);
		       if (lnode->text) free(lnode->text);
		       evas_object_textblock_layout_clear(obj, &(lnode->layout));
		       free(lnode);
		    }
		  else
		    {
		       char *text1, *text2;
		       
		       /* byte chrpos is over... so cut there */
		       text1 = malloc(nchrpos + 1);
		       strncpy(text1, text, nchrpos);
		       text1[nchrpos] = 0;
		       text2 = strdup(text + chrpos);
		       lnode->text = text1;
		       lnode->text_len = strlen(lnode->text);
		       lnode->text_pos = text_pos;
		       text_pos += chrpos;
		       free(text);
		       srcpos += chrpos;
		       text = text1;
		       if (font) ENFN->font_string_size_get(ENDT, font, text, &tw, &th);
		       if (font) hadvance = ENFN->font_h_advance_get(ENDT, font, text);
		       lnode->w = tw;
		       lnode->h = th;
		       lnode->layout.line.advance = hadvance;
		       o->layout_nodes = evas_object_list_append(o->layout_nodes, lnode);
		       adj = (double)(w - (lnode->layout.line.x + tw + layout.line.inset)) * layout.align;
		       adj -= line_start->layout.line.x;
		       for (ll = (Evas_Object_List *)lnode; ll; ll = ll->prev)
			 {
			    Layout_Node *lnode2;
			    
			    lnode2 = (Layout_Node *)ll;
			    lnode2->layout.line.x += adj;
			    lnode2->layout.line.mascent = layout.line.mascent;
			    lnode2->layout.line.mdescent = layout.line.mdescent;
			    if (ll == (Evas_Object_List *)line_start) break;
			 }
		       lnode->line_end = 1;
		       layout.line.inset = 0;
		       layout.line.x = 0;
		       if ((layout.line.y + layout.line.mascent + layout.line.mdescent) > h)
			 {
			    /* FIXME: this node would overflow to the next textblock */
			 }
		       line++;
		       layout.line.y += layout.line.mascent + layout.line.mdescent;
		       fh = layout.line.y;
		       line_start = NULL;
		       text = text2;
		       /* still more text to go */
		       if ((layout.line.mascent + layout.line.mdescent) > 0)
			 goto new_node;
		       else
			 free(text);
		    }
	       }
	  }
     }
   breakout:
   evas_object_textblock_layout_clear(obj, &layout);
   o->lines = last_line + 1;
   o->format.w = w;
   if (last_mdescent < 3) fh += 3 - last_mdescent;
   o->format.h = fh;
   o->format.dirty = 0;
}

static void
evas_object_textblock_format_calc(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Layout layout;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   evas_object_textblock_layout_fonts_hold(obj);
   evas_object_textblock_layout_clean(obj);
   evas_object_textblock_layout(obj);
   evas_object_textblock_layout_fonts_hold_clean(obj);
   o->changed = 0;
}

static void
evas_object_textblock_native_calc(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Layout layout;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   /* FIXME: takes nodes and produce layotu nodes ignoring object size */
}

static Node *
evas_object_textblock_node_pos_get(Evas_Object *obj, int pos, int *pstart)
{
   Evas_Object_Textblock *o;
   Evas_Object_List *l;
   int p, ps;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   ps = p = 0;
   for (l = (Evas_Object_List *)o->nodes; l; l = l->next)
     {
	Node *node;
	
	node = (Node *)l;
	if (node->text)
	  {
	     ps = p;
	     p += node->text_len;
	     if (p > pos)
	       {
		  *pstart = ps;
		  return node;
	       }
	  }
     }
   return NULL;
}

static Layout_Node *
evas_object_textblock_layout_node_pos_get(Evas_Object *obj, int pos, int *pstart)
{
   Evas_Object_Textblock *o;
   Evas_Object_List *l;
   int p;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   p = 0;
   for (l = (Evas_Object_List *)o->layout_nodes; l; l = l->next)
     {
	Layout_Node *lnode;
	
	lnode = (Layout_Node *)l;
	if (lnode->text)
	  {
	     p = lnode->text_pos + lnode->text_len;
	     if (p > pos)
	       {
		  *pstart = lnode->text_pos;
		  return lnode;
	       }
	  }
     }
   return NULL;
}

static Layout_Node *
evas_object_textblock_layout_node_line_get(Evas_Object *obj, int line)
{
   Evas_Object_Textblock *o;
   Evas_Object_List *l;
   int p, ps;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   ps = p = 0;
   for (l = (Evas_Object_List *)o->layout_nodes; l; l = l->next)
     {
	Layout_Node *lnode;
	
	lnode = (Layout_Node *)l;
	if (lnode->text)
	  {
	     ps = p;
	     p += lnode->text_len;
	     if (lnode->line == line) return lnode;
	  }
     }
   return NULL;
}

/* private methods for textblock objects */
static void evas_object_textblock_init(Evas_Object *obj);
static void *evas_object_textblock_new(void);
static void evas_object_textblock_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_textblock_free(Evas_Object *obj);
static void evas_object_textblock_render_pre(Evas_Object *obj);
static void evas_object_textblock_render_post(Evas_Object *obj);

static int evas_object_textblock_is_opaque(Evas_Object *obj);
static int evas_object_textblock_was_opaque(Evas_Object *obj);

static void evas_object_textblock_coords_recalc(Evas_Object *obj);

static Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_textblock_free,
     evas_object_textblock_render,
     evas_object_textblock_render_pre,
     evas_object_textblock_render_post,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_textblock_is_opaque,
     evas_object_textblock_was_opaque,
     NULL,
     NULL,
     evas_object_textblock_coords_recalc
};

/* the actual api call to add a textblock */

/**
 * Adds a textblock to the given evas.
 * @param   e The given evas.
 * @return  The new textblock object.
 * @todo Find a documentation group to put this under.
 */
Evas_Object *
evas_object_textblock_add(Evas *e)
{
   Evas_Object *obj;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new();
   evas_object_textblock_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

void
evas_object_textblock_clear(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   evas_object_textblock_contents_clean(obj);
   evas_object_textblock_layout_clean(obj);
   o->len = 0;
   o->pos = 0;
   o->changed = 1;
   evas_object_change(obj);
}

void
evas_object_textblock_cursor_pos_set(Evas_Object *obj, int pos)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   if (pos < 0) pos = 0;
   else if (pos > o->len) pos = o->len;
   o->pos = pos;
}

void
evas_object_textblock_cursor_pos_next(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   /* FIXME: DO */
}

void
evas_object_textblock_cursor_pos_prev(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   /* FIXME: DO */
}

int
evas_object_textblock_cursor_pos_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return 0;
   MAGIC_CHECK_END();
   return o->pos;
}

int
evas_object_textblock_length_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return 0;
   MAGIC_CHECK_END();
   return o->len;
}

int
evas_object_textblock_cursor_line_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Layout_Node *lnode;
   int ps;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return 0;
   MAGIC_CHECK_END();
   if (o->format.dirty)
     {
	evas_object_textblock_format_calc(obj);
	o->format.dirty = 0;
     }
   lnode = evas_object_textblock_layout_node_pos_get(obj, o->pos, &ps);
   if (lnode) return lnode->line;
   return 0;
}

int
evas_object_textblock_lines_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return 0;
   MAGIC_CHECK_END();
   return o->lines;
}

int
evas_object_textblock_line_start_pos_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Layout_Node *lnode;
   int ps;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return -1;
   MAGIC_CHECK_END();
   if (o->format.dirty)
     {
	evas_object_textblock_format_calc(obj);
	o->format.dirty = 0;
     }
   lnode = evas_object_textblock_layout_node_pos_get(obj, o->pos, &ps);
   if (lnode)
     {
	Evas_Object_List *l;
	
	for (l = (Evas_Object_List *)lnode; l; l = l->prev)
	  {
	     lnode = (Layout_Node *)l;
	     if ((lnode->text) && (lnode->line_start))
	       return lnode->text_pos;
	  }
     }
   return -1;
}

int
evas_object_textblock_line_end_pos_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Layout_Node *lnode;
   int ps;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return -1;
   MAGIC_CHECK_END();
   if (o->format.dirty)
     {
	evas_object_textblock_format_calc(obj);
	o->format.dirty = 0;
     }
   lnode = evas_object_textblock_layout_node_pos_get(obj, o->pos, &ps);
   if (lnode)
     {
	Evas_Object_List *l;
	
	for (l = (Evas_Object_List *)lnode; l; l = l->next)
	  {
	     lnode = (Layout_Node *)l;
	     if ((lnode->text) && (lnode->line_end))
	       {
		  int index;
		  
		  index = evas_common_font_utf8_get_last(lnode->text, lnode->text_len);
		  return lnode->text_pos + index;
	       }
	  }
     }
   return -1;
}

void
evas_object_textblock_line_get(Evas_Object *obj, int line, Evas_Coord *lx, Evas_Coord *ly, Evas_Coord *lw, Evas_Coord *lh)
{
   Evas_Object_Textblock *o;
   Layout_Node *lnode;
   int ps;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   if (o->format.dirty)
     {
	evas_object_textblock_format_calc(obj);
	o->format.dirty = 0;
     }
   lnode = evas_object_textblock_layout_node_pos_get(obj, o->pos, &ps);
   if (lnode)
     {
	Evas_Object_List *l;
	Layout_Node *lnode_start = NULL, *lnode_end = NULL;
	
	for (l = (Evas_Object_List *)lnode; l; l = l->prev)
	  {
	     lnode_start = (Layout_Node *)l;
	     if (lnode_start->line_start)
	       break;
	  }
	for (l = (Evas_Object_List *)lnode; l; l = l->next)
	  {
	     lnode_end = (Layout_Node *)l;
	     if (lnode_end->line_end)
	       break;
	  }
	if ((lnode_start) && (lnode_end))
	  {
	     if (lx) *lx = lnode_start->layout.line.x;
	     if (ly) *ly = lnode_start->layout.line.y;
	     if (lw) *lw = lnode_end->layout.line.x - lnode_start->layout.line.x + lnode_end->layout.line.advance;
	     if (lh) *lh = lnode_start->layout.line.mascent + lnode_start->layout.line.mdescent;
	  }
     }
}

void
evas_object_textblock_char_pos_get(Evas_Object *obj, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Textblock *o;
   Layout_Node *lnode;
   int ps;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   if (o->format.dirty)
     {
	evas_object_textblock_format_calc(obj);
	o->format.dirty = 0;
     }
   lnode = evas_object_textblock_layout_node_pos_get(obj, o->pos, &ps);
   if (lnode)
     {
	int ret, x = 0, y = 0, w = 0, h = 0;
	
	ret = ENFN->font_char_coords_get(ENDT, lnode->layout.font.font,
					 lnode->text,
					 pos - lnode->text_pos,
					 &x, &y, &w, &h);
	y = lnode->layout.line.y;
        x += lnode->layout.line.x;
	h = lnode->layout.line.mascent + lnode->layout.line.mdescent;
	if (cx) *cx = x;
        if (cy) *cy = y;
        if (cw) *cw = w;
        if (ch) *ch = h;
     }
}

int
evas_object_textblock_char_coords_get(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Textblock *o;
   Layout_Node *lnode;
   int ps;
   Evas_Object_List *l;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return -1;
   MAGIC_CHECK_END();
   if (o->format.dirty)
     {
	evas_object_textblock_format_calc(obj);
	o->format.dirty = 0;
     }
   for (l = (Evas_Object_List *)o->layout_nodes; l; l = l->next)
     {
	lnode = (Layout_Node *)l;
	
	if ((lnode->text) &&
	    (x >= lnode->layout.line.x) && 
	    (x < (lnode->layout.line.x + lnode->layout.line.advance)) &&
	    (y >= lnode->layout.line.y) && 
	    (y < (lnode->layout.line.y + lnode->layout.line.mascent + lnode->layout.line.mdescent)))
	  {
	     int ret, rx = 0, ry = 0, rw = 0, rh = 0;
	     
	     ret = ENFN->font_char_at_coords_get(ENDT, lnode->layout.font.font,
						 lnode->text,
						 x - lnode->layout.line.x,
						 0,
						 &rx, &ry, &rw, &rh);
	     if (ret < 0)
	       {
		  if ((x - lnode->layout.line.x) <
		      (lnode->layout.line.advance / 2))
		    {
		       ret = ENFN->font_char_coords_get(ENDT, lnode->layout.font.font,
							lnode->text,
							0,
							&rx, &ry, &rw, &rh);
		       ret = 0;
		    }
		  else
		    {
		       int pos;
		       
		       pos = evas_common_font_utf8_get_last(lnode->text, lnode->text_len);
		       ret = ENFN->font_char_coords_get(ENDT, lnode->layout.font.font,
							lnode->text,
							pos,
							&rx, &ry, &rw, &rh);
		       ret = pos;
		    }
	       }
	     ry = lnode->layout.line.y;
	     rx += lnode->layout.line.x;
	     rh = lnode->layout.line.mascent + lnode->layout.line.mdescent;
	     if (cx) *cx = rx;
	     if (cy) *cy = ry;
	     if (cw) *cw = rw;
	     if (ch) *ch = rh;
	     return ret + lnode->text_pos;
	  }
     }
   return -1;
}

void
evas_object_textblock_text_insert(Evas_Object *obj, const char *text)
{
   Evas_Object_Textblock *o;
   Node *node;
   int ps;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   if (!text) return;
   node = evas_object_textblock_node_pos_get(obj, o->pos, &ps);
   /* at the end - just append */
   if (!node)
     {
	if (!o->nodes)
	  {
	     node = calloc(1, sizeof(Node));
	     node->text = strdup(text);
	     node->text_len = strlen(node->text);
	     o->pos = node->text_len;
	     o->len = node->text_len;
	  }
	else
	  {
	     int len;
	     char *ntext;
	     
	     node = (Node *)(((Evas_Object_List *)(o->nodes))->last);
	     len = strlen(text);
	     ntext = malloc(node->text_len + len + 1);
	     if (node->text) strcpy(ntext, node->text);
	     strcpy(ntext + node->text_len, text);
	     if (node->text) free(node->text);
	     node->text = ntext;
	     node->text_len += len;
	     o->pos += len;
	     o->len += len;
	  }
     }
   else
     {
	int len;
	char *ntext;
	
	len = strlen(text);
	ntext = malloc(node->text_len + len + 1);
	if (node->text) strncpy(ntext, node->text, o->pos - ps);
	strcpy(ntext + o->pos - ps, text);
	if (node->text) strcpy(ntext + o->pos - ps + len, node->text + o->pos - ps);
	if (node->text) free(node->text);
	node->text = ntext;
	node->text_len += len;
	o->pos += len;
	o->len += len;
     }
   o->native.dirty = 1;
   o->format.dirty = 1;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Gets length bytes from the textblock from the current cursor position
 * @param   obj The given textblock.
 * @param   len The number of characters to get from the textblock.
 * @return  Returns NULL on failure, or as many of len characters as were
 * available. The returned memory must be free'd by the caller.
 */
char *
evas_object_textblock_text_get(Evas_Object *obj, int len)
{
   Evas_Object_Textblock *o;
   char *ret = NULL;
   int my_len = len;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return NULL;
   MAGIC_CHECK_END();

   if (len <= 0) return NULL;
   if (o->pos > o->len) return NULL;
   if (len > (o->len - o->pos)) my_len = o->len - o->pos;

   ret = malloc(sizeof(char) * (my_len + 1));
   if (ret)
     {
	Node *node;
	int ps = 0;

	node = evas_object_textblock_node_pos_get(obj, o->pos, &ps);
	if (node)
	  {
	     if ((node->text_len - (o->pos - ps)) >= my_len)
	       memcpy(ret, node->text + (o->pos - ps), my_len);
	     else
	       {
		  int remaining = my_len - (node->text_len - (o->pos - ps));
		  int count = (node->text_len - (o->pos - ps));

		  memcpy(ret, node->text + (o->pos - ps), node->text_len - (o->pos - ps));

		  while(remaining > 0)
		    {
		       node = evas_object_textblock_node_pos_get(obj, o->pos + count, &ps);
		       if (node)
			 {
			    int amt = 0;
			    if (node->text_len >= remaining)
			      amt = remaining;
			    else
			      amt = node->text_len;

			    memcpy(ret + count, node->text, amt);
			    remaining -= amt;
			    count += amt;
			 }
		       else
			 { 
			    /* we ran out of nodes ... */
			    break;
			 }
		    }
	       }
	  }
        ret[my_len] = '\0';
     }
   return ret;
}

/**
 * Removes length bytes from the textblock from the current cursor position.
 * @param   obj The given textblock.
 * @param   len The number of bytes to remove
 * @return  Returns no value.
 */
void
evas_object_textblock_text_del(Evas_Object *obj, int len)
{
   Evas_Object_Textblock *o;
   Node *node;
   int my_len, ps = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   o->native.dirty = 1;
   o->format.dirty = 1;
   o->changed = 1;
   evas_object_change(obj);

   if (len <= 0) return;
   if (o->pos >= o->len) return;

   /* deleting everything */
   if ((o->pos == 0) && (len >= o->len))
     {
	evas_object_textblock_clear(obj);
	return;
     }

   /* make sure we have enough to remove */
   my_len = len;
   if ((o->len - o->pos) < len) my_len = o->len - o->pos;

   node = evas_object_textblock_node_pos_get(obj, o->pos, &ps);
   if (node)
     {
	int remaining;
	char *tmp;

	remaining = my_len;

	if (remaining <= (node->text_len - (o->pos - ps)))
	  {
	     tmp = node->text;
	     node->text = malloc(sizeof(char) * (node->text_len - my_len + 1));

	     /* any begining text */
	     if ((o->pos - ps) > 0)
	       strncpy(node->text, tmp, o->pos - ps);

	     /* any ending text */
	     if (((o->pos - ps) + remaining) < node->text_len)
	       strncpy(node->text + (o->pos - ps), tmp + remaining + (o->pos - ps), 
		     node->text_len - remaining - (o->pos - ps));

	     free(tmp);
	     node->text_len -= remaining;
	     o->len -= remaining;

	     /* is the node now empty? */
	     if (node->text_len == 0)
	       {
		  o->nodes = evas_object_list_remove(o->nodes, node);
		  if (node->format) free(node->format);
		  if (node->text) free(node->text);
		  free(node);
	       }
	  }
	else
	  {
	     tmp = node->text;
	     node->text = malloc(sizeof(char) * (o->pos - ps + 1));

	     /* save the start part */
	     strncpy(node->text, tmp, o->pos - ps);

	     o->len -= node->text_len - (o->pos - ps);
	     remaining -= node->text_len - (o->pos - ps);

	     node->text_len = o->pos - ps;
	     node->text[node->text_len] = '\0';
	     free(tmp);

	     ps += node->text_len;
	     while(remaining > 0)
	       {
		  node = evas_object_textblock_node_pos_get(obj, ps, &ps);
		  if (!node)
		    {
		       /* ran out of nodes ... */
		       break;
		    }

		  if (remaining < node->text_len)
		    {
		       tmp = node->text;
		       node->text = malloc(sizeof(char) * (node->text_len - remaining + 1));
		       strncpy(node->text, tmp + remaining, node->text_len - remaining);
		       node->text_len -= remaining;
		       node->text[node->text_len] = '\0';
		       free(tmp);

		       o->len -= remaining;
		       remaining -= remaining;
		    }
		  else
		    {
		       o->nodes = evas_object_list_remove(o->nodes, node);
		       o->len -= node->text_len;
		       remaining -= node->text_len;

		       if (node->format) free(node->format);
		       if (node->text) free(node->text);
		       free(node);
		    }
	       }
	  }
     }
}

void
evas_object_textblock_format_insert(Evas_Object *obj, const char *format)
{
   Evas_Object_Textblock *o;
   Node *node;
   int ps;
   char *nformat;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   node = evas_object_textblock_node_pos_get(obj, o->pos, &ps);
   /* at the end - just append */
   if (!node)
     {
	nformat = evas_object_textblock_format_merge(NULL, (char *)format);
	if (nformat)
	  {
	     node = calloc(1, sizeof(Node));
	     node->format = nformat;
	     o->nodes = evas_object_list_append(o->nodes, node);
	  }
     }
   else
     {
	char *ntext1, *ntext2;
	
	ntext1 = malloc(o->pos - ps + 1);
	ntext2 = malloc(node->text_len - (o->pos - ps) + 1);
	strncpy(ntext1, node->text, o->pos - ps);
	ntext1[o->pos - ps] = 0;
	strcpy(ntext2, node->text + o->pos - ps);
	free(node->text);
	node->text = ntext1;
	node->text_len = o->pos - ps;
	nformat = evas_object_textblock_format_merge(NULL, (char *)format);
	if (nformat)
	  {
	     node = calloc(1, sizeof(Node));
	     node->format = nformat;
	     o->nodes = evas_object_list_append(o->nodes, node);
	  }
     }
   o->native.dirty = 1;
   o->format.dirty = 1;
   o->changed = 1;
   evas_object_change(obj);
}

int
evas_object_textblock_format_next_pos_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return -1;
   MAGIC_CHECK_END();
   /* FIXME: DO */
   return -1;
}

int
evas_object_textblock_format_prev_pos_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return -1;
   MAGIC_CHECK_END();
   /* FIXME: DO */
   return -1;
}

char *
evas_object_textblock_format_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return NULL;
   MAGIC_CHECK_END();
   /* FIXME: DO */
   return NULL;
}

char *
evas_object_textblock_current_format_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return NULL;
   MAGIC_CHECK_END();
   /* FIXME: DO */
   return NULL;
}

void
evas_object_textblock_format_del(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   /* FIXME: DO */
   o->native.dirty = 1;
   o->format.dirty = 1;
   o->changed = 1;
   evas_object_change(obj);
}

void
evas_object_textblock_format_direction_set(Evas_Object *obj, Evas_Format_Direction dir)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   if (o->format_dir == dir) return;
   /* FIXME: DO */
   o->native.dirty = 1;
   o->format.dirty = 1;
   o->changed = 1;
   evas_object_change(obj);
}

Evas_Format_Direction
evas_object_textblock_format_direction_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_FORMAT_DIRECTION_VERTICAL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return EVAS_FORMAT_DIRECTION_VERTICAL;
   MAGIC_CHECK_END();
   return o->format_dir;
}

void
evas_object_textblock_format_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (o->format.dirty)
     {
	evas_object_textblock_format_calc(obj);
	o->format.dirty = 0;
     }
   if (w) *w = o->format.w;
   if (h) *h = o->format.h;
}

void
evas_object_textblock_native_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (o->native.dirty)
     {
	/* FIXME: DO */
	evas_object_textblock_native_calc(obj);
	o->native.dirty = 0;
     }
   if (w) *w = o->native.w;
   if (h) *h = o->native.h;
}

/* all nice and private */
static void
evas_object_textblock_init(Evas_Object *obj)
{
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_textblock_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0.0;
   obj->cur.geometry.y = 0.0;
   obj->cur.geometry.w = 0.0;
   obj->cur.geometry.h = 0.0;
   obj->cur.layer = 0;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_textblock_new(void)
{
   Evas_Object_Textblock *o;
   
   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Textblock));
   o->magic = MAGIC_OBJ_TEXTBLOCK;
   return o;
}

static void
evas_object_textblock_free(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   evas_object_textblock_layout_clean(obj);
   evas_object_textblock_contents_clean(obj);
   o->magic = 0;
   free(o);
}

static void
evas_object_textblock_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Textblock *o;
   Evas_Object_List *l;
   int pbackx;
   
   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Textblock *)(obj->object_data);
   obj->layer->evas->engine.func->context_multiplier_unset(output,
							   context);
   if (o->changed)
     {
	evas_object_textblock_layout_fonts_hold(obj);
	evas_object_textblock_layout_clean(obj);
	evas_object_textblock_layout(obj);
	evas_object_textblock_layout_fonts_hold_clean(obj);
	o->changed = 0;
     }
#if 0   
    obj->layer->evas->engine.func->context_color_set(output,
                                                     context,
                                                     230, 160, 30, 100);
    obj->layer->evas->engine.func->rectangle_draw(output,
                                                  context,
                                                  surface,
                                                  obj->cur.cache.geometry.x + x,
                                                  obj->cur.cache.geometry.y + y,
                                                  obj->cur.cache.geometry.w,
                                                  obj->cur.cache.geometry.h);
#endif   
   for (l = (Evas_Object_List *)o->layout_nodes; l; l = l->next)
     {
	Layout_Node *lnode, *nlnode;
	
	lnode = (Layout_Node *)l;
	nlnode = (Layout_Node *)(l->next);
	if (lnode->line_start) pbackx = 0;
	if ((lnode->layout.font.font) && (lnode->text))
	  {
	     int lin;
	     int inset = 0;
	     
	     lin = 0;
	     if (lnode->layout.underline) lin++;
	     if (lnode->layout.second_underline) lin++;
	     if (lnode->layout.strikethrough) lin++;
	     if (lnode->layout.backing) lin++;
	     if (lin > 0)
	       inset = ENFN->font_inset_get(ENDT, 
					    lnode->layout.font.font,
					    lnode->text);
	     if (lnode->layout.backing)
	       {
		  int bx;
		  
		  ENFN->context_color_set(output,
					  context,
					  (obj->cur.cache.clip.r * lnode->layout.backing_color.r) / 255,
					  (obj->cur.cache.clip.g * lnode->layout.backing_color.g) / 255,
					  (obj->cur.cache.clip.b * lnode->layout.backing_color.b) / 255,
					  (obj->cur.cache.clip.a * lnode->layout.backing_color.a) / 255);
		  bx = lnode->layout.line.x + inset;
		  if (pbackx > 0) bx = pbackx;
		  pbackx = bx + lnode->layout.line.advance;
		  if (((nlnode) && ((nlnode->line_start) ||
				    (!nlnode->layout.backing))) ||
		      (!nlnode))
		    ENFN->rectangle_draw(output,
					 context,
					 surface,
					 obj->cur.cache.geometry.x + 
					 bx + x,
					 obj->cur.cache.geometry.y +
					 lnode->layout.line.y + y,
					 lnode->w,
					 lnode->layout.line.mascent + lnode->layout.line.mdescent);
		  else
		    ENFN->rectangle_draw(output,
					 context,
					 surface,
					 obj->cur.cache.geometry.x + 
					 bx + x,
					 obj->cur.cache.geometry.y +
					 lnode->layout.line.y + y,
					 lnode->layout.line.advance,
					 lnode->layout.line.mascent + lnode->layout.line.mdescent);
	       }
	     else
	       pbackx = 0;
	     ENFN->context_color_set(output,
				     context,
				     (obj->cur.cache.clip.r * lnode->layout.color.r) / 255,
				     (obj->cur.cache.clip.g * lnode->layout.color.g) / 255,
				     (obj->cur.cache.clip.b * lnode->layout.color.b) / 255,
				     (obj->cur.cache.clip.a * lnode->layout.color.a) / 255);
	     if (lnode->layout.valign < 0.0)
	       ENFN->font_draw(output,
			       context,
			       surface,
			       lnode->layout.font.font,
			       obj->cur.cache.geometry.x + 
			       lnode->layout.line.x + x,
			       obj->cur.cache.geometry.y + 
			       lnode->layout.line.y + y + 
			       lnode->layout.line.mascent,
			       lnode->w,
			       lnode->h,
			       lnode->w,
			       lnode->h,
			       lnode->text);
	     else
	       ENFN->font_draw(output,
			       context,
			       surface,
			       lnode->layout.font.font,
			       obj->cur.cache.geometry.x +
			       lnode->layout.line.x + x,
			       obj->cur.cache.geometry.y + 
			       lnode->layout.line.y + y + 
			       ((double)(((lnode->layout.line.mascent + 
					   lnode->layout.line.mdescent) -
					  (lnode->layout.line.ascent + 
					   lnode->layout.line.descent)) * 
					 lnode->layout.valign)) +
			       lnode->layout.line.ascent,
			       lnode->w,
			       lnode->h,
			       lnode->w,
			       lnode->h,
			       lnode->text);
	     if (lnode->layout.underline)
	       {
		  ENFN->context_color_set(output,
					  context,
					  (obj->cur.cache.clip.r * lnode->layout.underline_color.r) / 255,
					  (obj->cur.cache.clip.g * lnode->layout.underline_color.g) / 255,
					  (obj->cur.cache.clip.b * lnode->layout.underline_color.b) / 255,
					  (obj->cur.cache.clip.a * lnode->layout.underline_color.a) / 255);
		  ENFN->rectangle_draw(output,
				       context,
				       surface,
				       obj->cur.cache.geometry.x + 
				       lnode->layout.line.x + x + inset,
				       obj->cur.cache.geometry.y +
				       lnode->layout.line.y + y + 
				       lnode->layout.line.mascent + 1,
				       lnode->w,
				       1);
	       }
	     if (lnode->layout.second_underline)
	       {
		  ENFN->context_color_set(output,
					  context,
					  (obj->cur.cache.clip.r * lnode->layout.double_underline_color.r) / 255,
					  (obj->cur.cache.clip.g * lnode->layout.double_underline_color.g) / 255,
					  (obj->cur.cache.clip.b * lnode->layout.double_underline_color.b) / 255,
					  (obj->cur.cache.clip.a * lnode->layout.double_underline_color.a) / 255);
		  ENFN->rectangle_draw(output,
				       context,
				       surface,
				       obj->cur.cache.geometry.x + 
				       lnode->layout.line.x + x + inset,
				       obj->cur.cache.geometry.y +
				       lnode->layout.line.y + y + 
				       lnode->layout.line.mascent + 3,
				       lnode->w,
				       1);
	       }
	     if (lnode->layout.strikethrough)
	       {
		  ENFN->context_color_set(output,
					  context,
					  (obj->cur.cache.clip.r * lnode->layout.strikethrough_color.r) / 255,
					  (obj->cur.cache.clip.g * lnode->layout.strikethrough_color.g) / 255,
					  (obj->cur.cache.clip.b * lnode->layout.strikethrough_color.b) / 255,
					  (obj->cur.cache.clip.a * lnode->layout.strikethrough_color.a) / 255);
		  ENFN->rectangle_draw(output,
				       context,
				       surface,
				       obj->cur.cache.geometry.x + 
				       lnode->layout.line.x + x + inset,
				       obj->cur.cache.geometry.y +
				       lnode->layout.line.y + y + 
				       lnode->layout.line.mascent - ((lnode->layout.line.ascent - lnode->layout.line.descent) / 2),
				       lnode->w,
				       1);
	       }
	  }
     }
/*   
   if (o->engine_data)
     {
	
     }
 */
}

static void
evas_object_textblock_render_pre(Evas_Object *obj)
{
   Evas_List *updates = NULL;
   Evas_Object_Textblock *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw textblocks */
   o = (Evas_Object_Textblock *)(obj->object_data);
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	updates = evas_object_render_pre_visible_change(updates, obj, is_v, was_v);
	goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   updates = evas_object_render_pre_clipper_change(updates, obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if (obj->restack)
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   if (o->changed)
     {
/*	
	Evas_Rectangle *r;
	
	r = malloc(sizeof(Evas_Rectangle));
	r->x = 0; r->y = 0;
	r->w = obj->cur.geometry.w;
	r->h = obj->cur.geometry.h;
	updates = evas_list_append(updates, r);
*/
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	evas_object_textblock_layout_fonts_hold(obj);
	evas_object_textblock_layout_clean(obj);
	evas_object_textblock_layout(obj);
	evas_object_textblock_layout_fonts_hold_clean(obj);
	o->changed = 0;
     }
   done:
   evas_object_render_pre_effect_updates(updates, obj, is_v, was_v);
}

static void
evas_object_textblock_render_post(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Textblock *)(obj->object_data);
   /* remove those pesky changes */
   while (obj->clip.changes)
     {
	Evas_Rectangle *r;
	
	r = (Evas_Rectangle *)obj->clip.changes->data;
	obj->clip.changes = evas_list_remove(obj->clip.changes, r);
	free(r);
     }
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   o->prev = o->cur;
   o->changed = 0;
}

static int
evas_object_textblock_is_opaque(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   /* this returns 1 if the internal object data implies that the object is */
   /* currently fulyl opque over the entire gradient it occupies */
   o = (Evas_Object_Textblock *)(obj->object_data);
   return 0;
}

static int
evas_object_textblock_was_opaque(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   /* this returns 1 if the internal object data implies that the object was */
   /* currently fulyl opque over the entire gradient it occupies */
   o = (Evas_Object_Textblock *)(obj->object_data);
   return 0;
}

static void
evas_object_textblock_coords_recalc(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   if ((obj->cur.geometry.w != o->last_w) ||
       (obj->cur.geometry.h != o->last_h))
     {
	o->format.dirty = 1;
	o->changed = 1;
     }
}
