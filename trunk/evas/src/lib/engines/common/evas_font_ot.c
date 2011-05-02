#include "evas_font_ot.h"

#ifdef USE_HARFBUZZ
# include <hb.h>
# include <hb-ft.h>
#endif

#include "evas_common.h"

#include <Eina.h>
#include "evas_font_private.h"

#ifdef OT_SUPPORT
/* FIXME: doc. returns #items */
EAPI int
evas_common_font_ot_cluster_size_get(const Evas_Text_Props *props, size_t char_index)
{
   int i;
   int items;
   int left_bound, right_bound;
   size_t base_cluster;
   char_index += props->start;
   base_cluster = EVAS_FONT_OT_POS_GET(props->info->ot[char_index]);
   for (i = (int) char_index ;
         (i >= (int) props->start) &&
         (EVAS_FONT_OT_POS_GET(props->info->ot[i]) == base_cluster) ;
         i--)
     ;
   left_bound = i;
   for (i = (int) char_index + 1;
         (i < (int) (props->start + props->len)) &&
         (EVAS_FONT_OT_POS_GET(props->info->ot[i]) == base_cluster) ;
         i++)
     ;
   right_bound = i;

   if (right_bound == left_bound)
     {
        items = 1;
     }
   else if (props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        if (left_bound < 0)
          {
             items = props->text_offset + props->text_len - base_cluster;
          }
        else
          {
             items = props->info->ot[left_bound].source_cluster - base_cluster;
          }
     }
   else
     {
        if (right_bound >= (int) (props->text_offset + props->text_len))
          {
             items = props->text_offset + props->text_len - base_cluster;
          }
        else
          {
             items = props->info->ot[right_bound].source_cluster - base_cluster;
          }
     }
   return (items > 0) ? items : 1;
}

EAPI void
evas_common_font_ot_load_face(void *_font)
{
   RGBA_Font_Source *font = (RGBA_Font_Source *) _font;
   /* Unload the face if by any chance it's already loaded */
   evas_common_font_ot_unload_face(font);
   font->hb.face = hb_ft_face_create(font->ft.face, NULL);
}

EAPI void
evas_common_font_ot_unload_face(void *_font)
{
   RGBA_Font_Source *font = (RGBA_Font_Source *) _font;
   if (!font->hb.face) return;
   hb_face_destroy(font->hb.face);
   font->hb.face = NULL;
}

/* Harfbuzz font functions */
static hb_font_funcs_t *_ft_font_funcs = NULL;

static hb_codepoint_t
_evas_common_font_ot_hb_get_glyph(hb_font_t *font, hb_face_t *face,
    const void *user_data, hb_codepoint_t unicode,
    hb_codepoint_t variation_selector)
{
   RGBA_Font_Int *fi = (RGBA_Font_Int *) user_data;
   return hb_font_funcs_get_glyph_func(_ft_font_funcs)(font, face,
      fi->src->ft.face, unicode, variation_selector);
}

static void
_evas_common_font_ot_hb_get_glyph_advance(hb_font_t *font, hb_face_t *face,
   const void *user_data, hb_codepoint_t glyph,
   hb_position_t *x_advance, hb_position_t *y_advance)
{
   /* Use our cache*/
   RGBA_Font_Int *fi = (RGBA_Font_Int *) user_data;
   RGBA_Font_Glyph *fg;
   (void) font;
   (void) face;
   fg = evas_common_font_int_cache_glyph_get(fi, glyph);
   if (fg)
     {
        *x_advance = fg->glyph->advance.x >> 10;
        *y_advance = fg->glyph->advance.y >> 10;
     }
}

static void
_evas_common_font_ot_hb_get_glyph_extents(hb_font_t *font, hb_face_t *face,
   const void *user_data, hb_codepoint_t glyph, hb_glyph_extents_t *extents)
{
   RGBA_Font_Int *fi = (RGBA_Font_Int *) user_data;
   hb_font_funcs_get_glyph_extents_func(_ft_font_funcs)(font, face,
      fi->src->ft.face, glyph, extents);
}

static hb_bool_t
_evas_common_font_ot_hb_get_contour_point(hb_font_t *font, hb_face_t *face,
   const void *user_data, unsigned int point_index, hb_codepoint_t glyph,
   hb_position_t *x, hb_position_t *y)
{
   RGBA_Font_Int *fi = (RGBA_Font_Int *) user_data;
   return hb_font_funcs_get_contour_point_func(_ft_font_funcs)(font, face,
      fi->src->ft.face, point_index, glyph, x, y);
}

static hb_position_t
_evas_common_font_ot_hb_get_kerning(hb_font_t *font, hb_face_t *face,
   const void *user_data, hb_codepoint_t first_glyph,
   hb_codepoint_t second_glyph)
{
   RGBA_Font_Int *fi = (RGBA_Font_Int *) user_data;
   int kern;
   (void) font;
   (void) face;
   if (evas_common_font_query_kerning(fi, first_glyph, second_glyph, &kern))
      return kern;
   else
      return 0;
}

/* End of harfbuzz font funcs */

static hb_font_funcs_t *
_evas_common_font_ot_font_funcs_get(void)
{
   static hb_font_funcs_t *font_funcs = NULL;
   if (!font_funcs)
     {
        _ft_font_funcs = hb_ft_get_font_funcs();
        font_funcs = hb_font_funcs_create();
        hb_font_funcs_set_glyph_func(font_funcs,
            _evas_common_font_ot_hb_get_glyph);
        hb_font_funcs_set_glyph_advance_func(font_funcs,
            _evas_common_font_ot_hb_get_glyph_advance);
        hb_font_funcs_set_glyph_extents_func(font_funcs,
            _evas_common_font_ot_hb_get_glyph_extents);
        hb_font_funcs_set_contour_point_func(font_funcs,
            _evas_common_font_ot_hb_get_contour_point);
        hb_font_funcs_set_kerning_func(font_funcs,
            _evas_common_font_ot_hb_get_kerning);
     }

   return font_funcs;
}

static void
_evas_common_font_ot_shape(hb_buffer_t *buffer, RGBA_Font_Int *fi)
{
   hb_font_t   *hb_font;

   hb_font = hb_ft_font_create(fi->src->ft.face, NULL);
   hb_font_set_funcs(hb_font, _evas_common_font_ot_font_funcs_get(), fi, NULL);

   hb_shape(hb_font, fi->src->hb.face, buffer, NULL, 0);
   hb_font_destroy(hb_font);
}

EAPI Eina_Bool
evas_common_font_ot_populate_text_props(void *_fn, const Eina_Unicode *text,
      Evas_Text_Props *props, int len)
{
   RGBA_Font *fn = (RGBA_Font *) _fn;
   RGBA_Font_Int *fi;
   hb_buffer_t *buffer;
   hb_glyph_position_t *positions;
   hb_glyph_info_t *infos;
   int slen;
   unsigned int i;
   Evas_Font_Glyph_Info *gl_itr;
   Evas_Font_OT_Info *ot_itr;
   Evas_Coord pen_x = 0;

   fi = fn->fonts->data;
   /* Load the font needed for this script */
     {
        /* Skip common chars */
        const Eina_Unicode *tmp;
        for (tmp = text ;
              *tmp &&
              evas_common_language_char_script_get(*tmp) == EVAS_SCRIPT_COMMON ;
              tmp++)
          ;
        if (!*tmp && (tmp > text)) tmp--;
        evas_common_font_glyph_search(fn, &fi, *tmp);
     }
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }

   if (len < 0)
     {
        slen = eina_unicode_strlen(text);
     }
   else
     {
        slen = len;
     }

   buffer = hb_buffer_create(slen);
   hb_buffer_set_unicode_funcs(buffer, evas_common_language_unicode_funcs_get());
   hb_buffer_set_language(buffer, hb_language_from_string(
            evas_common_language_from_locale_get()));
   hb_buffer_set_script(buffer, props->script);
   hb_buffer_set_direction(buffer,
         (props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) ?
         HB_DIRECTION_RTL : HB_DIRECTION_LTR);
   /* FIXME: add run-time conversions if needed, which is very unlikely */
   hb_buffer_add_utf32(buffer, (const uint32_t *) text, slen, 0, slen);

   _evas_common_font_ot_shape(buffer, fi);

   props->len = hb_buffer_get_length(buffer);
   props->info->ot = calloc(props->len,
         sizeof(Evas_Font_OT_Info));
   props->info->glyph = calloc(props->len,
              sizeof(Evas_Font_Glyph_Info));
   positions = hb_buffer_get_glyph_positions(buffer, NULL);
   infos = hb_buffer_get_glyph_infos(buffer, NULL);
   gl_itr = props->info->glyph;
   ot_itr = props->info->ot;
   for (i = 0 ; i < props->len ; i++)
     {
        Evas_Coord adv;
        ot_itr->source_cluster = infos->cluster;
        ot_itr->x_offset = positions->x_offset;
        ot_itr->y_offset = positions->y_offset;
        gl_itr->index = infos->codepoint;
        adv = positions->x_advance;

        pen_x += adv;
        gl_itr->pen_after = EVAS_FONT_ROUND_26_6_TO_INT(pen_x);

        ot_itr++;
        gl_itr++;
        infos++;
        positions++;
     }

   hb_buffer_destroy(buffer);
   evas_common_font_int_use_trim();

   return EINA_FALSE;
}

#endif

