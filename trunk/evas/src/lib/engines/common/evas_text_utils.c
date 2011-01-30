#include "evas_text_utils.h"
#include "language/evas_bidi_utils.h"
#include "language/evas_language_utils.h"
#include "evas_font_ot.h"

void
evas_common_text_props_bidi_set(Evas_Text_Props *props,
      Evas_BiDi_Paragraph_Props *bidi_par_props, size_t start)
{
#ifdef BIDI_SUPPORT
   props->bidi.dir = (evas_bidi_is_rtl_char(
            bidi_par_props,
            0,
            start)) ? EVAS_BIDI_DIRECTION_RTL : EVAS_BIDI_DIRECTION_LTR;
#else
   (void) start;
   (void) bidi_par_props;
   props->bidi.dir = EVAS_BIDI_DIRECTION_LTR;
#endif
}

void
evas_common_text_props_script_set(Evas_Text_Props *props,
      const Eina_Unicode *str)
{
   props->script = evas_common_language_script_type_get(str);
}

void
evas_common_text_props_content_copy_and_ref(Evas_Text_Props *dst,
      const Evas_Text_Props *src)
{
   memcpy(dst, src, sizeof(Evas_Text_Props));
#ifdef OT_SUPPORT
   if (dst->ot_data)
     {
        evas_common_font_ot_props_ref(dst->ot_data);
     }
#endif
}

void
evas_common_text_props_content_unref(Evas_Text_Props *props)
{
#ifdef OT_SUPPORT
   if (props->ot_data)
     {
        evas_common_font_ot_props_unref(props->ot_data);
     }
#else
   (void) props;
#endif
#ifdef BIDI_SUPPORT
   evas_bidi_props_clean(&props->bidi);
#else
   (void) props;
#endif
}

/* Won't work in the middle of ligatures */
EAPI void
evas_common_text_props_cutoff(Evas_Text_Props *props, int cutoff)
{
#ifdef OT_SUPPORT
   evas_common_font_ot_cutoff_text_props(props, cutoff);
#endif
}

/* Won't work in the middle of ligatures */
EAPI void
evas_common_text_props_split(Evas_Text_Props *base,
      Evas_Text_Props *ext, int cutoff)
{
   /* FIXME: move to their own functions */
   memcpy(&ext->bidi, &base->bidi, sizeof(Evas_BiDi_Props));
   memcpy(&ext->script, &base->script, sizeof(Evas_Script_Type));
#ifdef OT_SUPPORT
   evas_common_font_ot_split_text_props(base, ext, cutoff);
#endif
}

/* Won't work in the middle of ligatures */
EAPI void
evas_common_text_props_merge(Evas_Text_Props *item1,
      const Evas_Text_Props *item2)
{
#ifdef OT_SUPPORT
   evas_common_font_ot_merge_text_props(item1, item2);
#endif
}


