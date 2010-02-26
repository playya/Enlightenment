#ifndef EINA_STRBUF_H
#define EINA_STRBUF_H

#include <stddef.h>

#include "eina_types.h"

typedef struct _Eina_Strbuf Eina_Strbuf;

EAPI Eina_Strbuf *eina_strbuf_new(void);
EAPI void eina_strbuf_free(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);
EAPI void eina_strbuf_reset(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

EAPI Eina_Bool eina_strbuf_append(Eina_Strbuf *buf, const char *str) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_append_escaped(Eina_Strbuf *buf, const char *str) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_append_n(Eina_Strbuf *buf, const char *str, size_t maxlen) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_append_length(Eina_Strbuf *buf, const char *str, size_t length) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_append_char(Eina_Strbuf *buf, char c) EINA_ARG_NONNULL(1);

EAPI Eina_Bool eina_strbuf_insert(Eina_Strbuf *buf, const char *str, size_t pos) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_insert_escaped(Eina_Strbuf *buf, const char *str, size_t pos) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_insert_n(Eina_Strbuf *buf, const char *str, size_t maxlen, size_t pos) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_insert_length(Eina_Strbuf *buf, const char *str, size_t length, size_t pos) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_insert_char(Eina_Strbuf *buf, char c, size_t pos) EINA_ARG_NONNULL(1);

#define eina_strbuf_prepend(buf, str) eina_strbuf_insert(buf, str, 0)
#define eina_strbuf_prepend_escaped(buf, str) eina_strbuf_insert_escaped(buf, str, 0)
#define eina_strbuf_prepend_n(buf, str, maxlen) eina_strbuf_insert_n(buf, str, maxlen, 0)
#define eina_strbuf_prepend_length(buf, str, length) eina_strbuf_insert_length(buf, str, length, 0)
#define eina_strbuf_prepend_char(buf, c) eina_strbuf_insert_char(buf, c, 0)


EAPI Eina_Bool eina_strbuf_remove(Eina_Strbuf *buf, size_t start, size_t end) EINA_ARG_NONNULL(1);
EAPI const char *eina_strbuf_string_get(const Eina_Strbuf *buf) EINA_ARG_NONNULL(1);
EAPI char *eina_strbuf_string_steal(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);
EAPI size_t eina_strbuf_length_get(const Eina_Strbuf *buf) EINA_ARG_NONNULL(1);
EAPI Eina_Bool eina_strbuf_replace(Eina_Strbuf *buf, const char *str, const char *with, unsigned int n) EINA_ARG_NONNULL(1, 2, 3);
#define eina_strbuf_replace_first(buf, str, with) \
	eina_strbuf_replace(buf, str, with, 1)
EAPI int eina_strbuf_replace_all(Eina_Strbuf *buf, const char *str, const char *with) EINA_ARG_NONNULL(1, 2, 3);

#endif /* EINA_STRBUF_H */
