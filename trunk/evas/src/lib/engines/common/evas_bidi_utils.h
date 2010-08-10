#ifndef _EVAS_BIDI_UTILS
#define _EVAS_BIDI_UTILS

/**
 * @internal
 * @addtogroup Evas_Utils
 *
 * @{
 */
/**
 * @internal
 * @defgroup Evas_BiDi Evas BiDi utility functions
 *
 * This set of functions and types helps evas handle BiDi strings correctly.
 * @todo Document types, structures and macros.
 *
 * @{
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_FRIBIDI
# define USE_FRIBIDI
# define BIDI_SUPPORT
#endif

#include "evas_common.h"

#ifdef USE_FRIBIDI
# include <fribidi/fribidi.h>
#endif

/* abstract fribidi - we statically define sizes here because otherwise we would
 * have to ifdef everywhere (because function decorations may change with/without
 * bidi support)
 * These types should only be passed as pointers! i.e do not directely use any of
 * these types in function declarations. Defining as void should help ensuring that.
 */

#ifdef USE_FRIBIDI
# define _EVAS_BIDI_TYPEDEF(type) \
   typedef FriBidi ## type EvasBiDi ## type
#else
# define _EVAS_BIDI_TYPEDEF(type) \
   typedef void EvasBiDi ## type
#endif

#if 0 /* We are using Eina_Unicode instead */
_EVAS_BIDI_TYPEDEF(Char);
#endif
_EVAS_BIDI_TYPEDEF(CharType);
_EVAS_BIDI_TYPEDEF(ParType);
_EVAS_BIDI_TYPEDEF(StrIndex);
_EVAS_BIDI_TYPEDEF(Level);
_EVAS_BIDI_TYPEDEF(JoiningType);

typedef struct _Evas_BiDi_Paragraph_Props Evas_BiDi_Paragraph_Props;
typedef struct _Evas_BiDi_Props Evas_BiDi_Props;

/* This structure defines a set of properties of a BiDi string. In case of a
 * non-bidi string, all values should be NULL.
 * To check if a structure describes a bidi string or not, use the macro
 * EVAS_BIDI_IS_BIDI_PROP. RTL-only strings are also treated as bidi ATM.
 */
struct _Evas_BiDi_Paragraph_Props {
   EvasBiDiCharType  *char_types; /* BiDi char types */
   EvasBiDiLevel     *embedding_levels; /* BiDi embedding levels */
#ifdef USE_FRIBIDI
   EvasBiDiParType    direction;
#endif
};

struct _Evas_BiDi_Props {
   Evas_BiDi_Paragraph_Props *props;
   size_t                     start;
};



#ifdef USE_FRIBIDI

# define EVAS_BIDI_IS_BIDI_PROP(intl_props) ((intl_props) && (intl_props)->char_types)
# define evas_bidi_position_visual_to_logical(list, position) \
                (list) ? list[position] : position;

EvasBiDiStrIndex
evas_bidi_position_logical_to_visual(EvasBiDiStrIndex *v_to_l, int len, EvasBiDiStrIndex position);

Eina_Bool
evas_bidi_is_rtl_str(const Eina_Unicode *str);

Eina_Bool
evas_bidi_is_rtl_char(EvasBiDiLevel *embedded_level_list, EvasBiDiStrIndex index);

Eina_Bool
evas_bidi_props_reorder_line(Eina_Unicode *text, const Evas_BiDi_Props *intl_props, EvasBiDiStrIndex **_v_to_l);

int
evas_bidi_update_props(const Eina_Unicode *text, Evas_BiDi_Paragraph_Props *intl_props) EINA_ARG_NONNULL(1, 2);

Eina_Bool
evas_bidi_shape_string(Eina_Unicode *ustr, const Evas_BiDi_Props *intl_props, size_t len);

void
evas_bidi_props_clean(Evas_BiDi_Props *intl_props) EINA_ARG_NONNULL(1);

void
evas_bidi_paragraph_props_clean(Evas_BiDi_Paragraph_Props *bidi_props) EINA_ARG_NONNULL(1);

#endif
/**
 * @}
 */
/**
 * @}
 */

#endif

