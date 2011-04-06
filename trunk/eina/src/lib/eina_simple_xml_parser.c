/* EINA - EFL data type library
 * Copyright (C) 2011 Gustavo Sverzut Barbieri
 *                    Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#include "eina_simple_xml_parser.h"

#include <strings.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_private.h"
#include "eina_log.h"
#include "eina_mempool.h"
#include "eina_stringshare.h"
#include "eina_strbuf.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

static Eina_Mempool *_eina_simple_xml_tag_mp = NULL;
static Eina_Mempool *_eina_simple_xml_attribute_mp = NULL;
static int _eina_simple_xml_log_dom = -1;

static const char EINA_MAGIC_SIMPLE_XML_TAG_STR[] = "Eina Simple XML Tag";
static const char EINA_MAGIC_SIMPLE_XML_DATA_STR[] = "Eina Simple XML Data";
static const char EINA_MAGIC_SIMPLE_XML_ATTRIBUTE_STR[] = "Eina Simple XML Attribute";

#define EINA_MAGIC_CHECK_TAG(d, ...)                            \
  do {                                                          \
     if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_SIMPLE_XML_TAG))       \
       {                                                        \
          EINA_MAGIC_FAIL(d, EINA_MAGIC_SIMPLE_XML_TAG);        \
          return __VA_ARGS__;                                   \
       }                                                        \
  } while(0)

#define EINA_MAGIC_CHECK_DATA(d, ...)                           \
  do {                                                          \
     if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_SIMPLE_XML_DATA))      \
       {                                                        \
          EINA_MAGIC_FAIL(d, EINA_MAGIC_SIMPLE_XML_DATA);       \
          return __VA_ARGS__;                                   \
       }                                                        \
  } while(0)

#define EINA_MAGIC_CHECK_ATTRIBUTE(d, ...)                      \
  do {                                                          \
     if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_SIMPLE_XML_ATTRIBUTE)) \
       {                                                        \
          EINA_MAGIC_FAIL(d, EINA_MAGIC_SIMPLE_XML_ATTRIBUTE);  \
          return __VA_ARGS__;                                   \
       }                                                        \
  } while(0)


#ifndef EINA_LOG_COLOR_DEFAULT
#define EINA_LOG_COLOR_DEFAULT EINA_COLOR_CYAN
#endif

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_simple_xml_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eina_simple_xml_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_simple_xml_log_dom, __VA_ARGS__)


static inline const char *
_eina_simple_xml_whitespace_find(const char *itr, const char *itr_end)
{
   for (; itr < itr_end; itr++)
     if (isspace(*itr)) break;
   return itr;
}

static inline const char *
_eina_simple_xml_whitespace_skip(const char *itr, const char *itr_end)
{
   for (; itr < itr_end; itr++)
     if (!isspace(*itr)) break;
   return itr;
}

static inline const char *
_eina_simple_xml_whitespace_unskip(const char *itr, const char *itr_start)
{
   for (itr--; itr > itr_start; itr--)
     if (!isspace(*itr)) break;
   return itr + 1;
}

static inline const char *
_eina_simple_xml_tag_start_find(const char *itr, const char *itr_end)
{
   return memchr(itr, '<', itr_end - itr);
}

static inline const char *
_eina_simple_xml_tag_end_find(const char *itr, const char *itr_end)
{
   for (; itr < itr_end; itr++)
     if ((*itr == '>') || (*itr == '<')) /* consider < also ends a tag */
       return itr;
   return NULL;
}

/**
 * @endcond
 */

/*============================================================================*
*                                 Global                                     *
*============================================================================*/


/**
 * @internal
 * @brief Initialize the simple xml parser module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the simple xml parser module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_simple_xml_init(void)
{
   const char *choice, *tmp;

   _eina_simple_xml_log_dom = eina_log_domain_register("eina_simple_xml",
                                                       EINA_LOG_COLOR_DEFAULT);
   if (_eina_simple_xml_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_simple_xml");
        return EINA_FALSE;
     }

#ifdef EINA_DEFAULT_MEMPOOL
   choice = "pass_through";
#else
   choice = "chained_mempool";
#endif
   tmp = getenv("EINA_MEMPOOL");
   if (tmp && tmp[0])
      choice = tmp;

   _eina_simple_xml_tag_mp = eina_mempool_add
         (choice, "simple_xml_tag", NULL,
          sizeof(Eina_Simple_XML_Node_Tag), 320);
   if (!_eina_simple_xml_tag_mp)
     {
        ERR("Mempool for simple_xml_tag cannot be allocated in init.");
        goto on_init_fail;
     }

   _eina_simple_xml_attribute_mp = eina_mempool_add
         (choice, "simple_xml_attribute", NULL,
          sizeof(Eina_Simple_XML_Attribute), 80);
   if (!_eina_simple_xml_attribute_mp)
     {
        ERR("Mempool for simple_xml_attribute cannot be allocated in init.");
        eina_mempool_del(_eina_simple_xml_tag_mp);
        goto on_init_fail;
     }

#define EMS(n) eina_magic_string_static_set(n, n ## _STR)
   EMS(EINA_MAGIC_SIMPLE_XML_TAG);
   EMS(EINA_MAGIC_SIMPLE_XML_DATA);
   EMS(EINA_MAGIC_SIMPLE_XML_ATTRIBUTE);
#undef EMS

   return EINA_TRUE;

on_init_fail:
   eina_log_domain_unregister(_eina_simple_xml_log_dom);
   _eina_simple_xml_log_dom = -1;
   return EINA_FALSE;
}

/**
 * @internal
 * @brief Shut down the simple xml parser module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the simple xml parser module set
 * up by eina_simple_xml_init(). It is called by
 * eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_simple_xml_shutdown(void)
{
   eina_mempool_del(_eina_simple_xml_attribute_mp);
   eina_mempool_del(_eina_simple_xml_tag_mp);

   eina_log_domain_unregister(_eina_simple_xml_log_dom);
   _eina_simple_xml_log_dom = -1;
   return EINA_TRUE;
}

/**
 * @defgroup Eina_Simple_XML_Group Simple_XML
 *
 * Simplistic relaxed SAX-like XML parser.
 *
 * This parser is far from being compliant with XML standard, but will
 * do for most XMLs out there. If you know that your format is simple
 * and will not vary in future with strange corner cases, then you can
 * use it safely.
 *
 * The parser is SAX like, that is, it will tokenize contents and call
 * you back so you can take some action. No contents are allocated
 * during this parser work and it's not recursive, so you can use it
 * with a very large document without worries.
 *
 * It will not validate the document anyhow, neither it will create a
 * tree hierarchy. That's up to you.
 *
 * Accordingly to XML, open tags may contain attributes. This parser
 * will not tokenize this. If you want you can use
 * eina_simple_xml_tag_attributes_find() and then
 * eina_simple_xml_attributes_parse().
 *
 * @{
 */

/*
 * @param buf the input string. May not contain \0 terminator.
 * @param buflen the input string size.
 * @param strip whenever this parser should strip leading and trailing
 *        whitespace. These whitespace will still be issued, but as type
 *        #EINA_SIMPLE_XML_IGNORED.
 * @param func what to call back while parse to do some action.  The
 *        first parameter is the given user @a data, the second is the
 *        token type, the third is the pointer to content start (it's
 *        not a NULL terminated string!), the forth is where this
 *        content is located inside @a buf (does not include tag
 *        start, for instance "<!DOCTYPE value>" the offset points at
 *        "value"), the fifth is the size of the content. Whenver this
 *        function return EINA_FALSE the parser will abort.  @param
 *        data what to give as context to @a func.
 *
 * @return EINA_TRUE on success or EINA_FALSE if it was aborted by user or
 *         parsing error.
 */
EAPI Eina_Bool
eina_simple_xml_parse(const char *buf, unsigned buflen, Eina_Bool strip, Eina_Simple_XML_Cb func, const void *data)
{
   const char *itr = buf, *itr_end = buf + buflen;

   if (!buf) return EINA_FALSE;
   if (!func) return EINA_FALSE;

#define CB(type, start, end)                                            \
   do                                                                   \
     {                                                                  \
        size_t _sz = end - start;                                       \
        Eina_Bool _ret;                                                 \
        _ret = func((void*)data, type, start, start - buf, _sz);        \
        if (!_ret) return EINA_FALSE;                                   \
     }                                                                  \
   while (0)

   while (itr < itr_end)
     {
        if (itr[0] == '<')
          {
             if (itr + 1 >= itr_end)
               {
                  CB(EINA_SIMPLE_XML_ERROR, itr, itr_end);
                  return EINA_FALSE;
               }
             else
               {
                  Eina_Simple_XML_Type type;
                  size_t toff;
                  const char *p;

                  if (itr[1] == '/')
                    {
                       type = EINA_SIMPLE_XML_CLOSE;
                       toff = 1;
                    }
                  else if (itr[1] == '?')
                    {
                       type = EINA_SIMPLE_XML_PROCESSING;
                       toff = 1;
                    }
                  else if (itr[1] == '!')
                    {
                       if ((itr + sizeof("<!DOCTYPE>") - 1 < itr_end) &&
                           (!memcmp(itr + 2, "DOCTYPE",
                                    sizeof("DOCTYPE") - 1)) &&
                           ((itr[2 + sizeof("DOCTYPE") - 1] == '>') ||
                            (isspace(itr[2 + sizeof("DOCTYPE") - 1]))))
                         {
                            type = EINA_SIMPLE_XML_DOCTYPE;
                            toff = sizeof("!DOCTYPE") - 1;
                         }
                       else if ((itr + sizeof("<!---->") - 1 < itr_end) &&
                                (!memcmp(itr + 2, "--", sizeof("--") - 1)))
                         {
                            type = EINA_SIMPLE_XML_COMMENT;
                            toff = sizeof("!--") - 1;
                         }
                       else if ((itr + sizeof("<![CDATA[]]>") - 1 < itr_end) &&
                                (!memcmp(itr + 2, "[CDATA[",
                                         sizeof("[CDATA[") - 1)))
                         {
                            type = EINA_SIMPLE_XML_CDATA;
                            toff = sizeof("![CDATA[") - 1;
                         }
                       else
                         {
                            type = EINA_SIMPLE_XML_OPEN;
                            toff = 0;
                         }
                    }
                  else
                    {
                       type = EINA_SIMPLE_XML_OPEN;
                       toff = 0;
                    }

                  p = _eina_simple_xml_tag_end_find(itr + 1 + toff, itr_end);
                  if (p)
                    {
                       if (type == EINA_SIMPLE_XML_CDATA)
                         {
                            /* must end with ]]> */
                            while ((p) && (memcmp(p - 2, "]]>", 3)))
                              p = _eina_simple_xml_tag_end_find(p + 1, itr_end);
                         }

                       if (*p == '<')
                         {
                            type = EINA_SIMPLE_XML_ERROR;
                            toff = 0;
                         }
                    }

                  if (p)
                    {
                       const char *start, *end;

                       start = itr + 1 + toff;
                       end = p;

                       switch (type)
                         {
                          case EINA_SIMPLE_XML_OPEN:
                             if (p[-1] == '/')
                               {
                                  type = EINA_SIMPLE_XML_OPEN_EMPTY;
                                  end--;
                               }
                             break;
                          case EINA_SIMPLE_XML_CDATA:
                             if (!memcmp(p - 2, "]]", 2)) end -= 2;
                             break;
                          case EINA_SIMPLE_XML_PROCESSING:
                             if (p[-1] == '?') end--;
                             break;
                          case EINA_SIMPLE_XML_COMMENT:
                             if (!memcmp(p - 2, "--", 2)) end -= 2;
                             break;
                          case EINA_SIMPLE_XML_OPEN_EMPTY:
                          case EINA_SIMPLE_XML_CLOSE:
                          case EINA_SIMPLE_XML_DATA:
                          case EINA_SIMPLE_XML_ERROR:
                          case EINA_SIMPLE_XML_DOCTYPE:
                          case EINA_SIMPLE_XML_IGNORED:
                             break;
                         }

                       if ((strip) && (type != EINA_SIMPLE_XML_ERROR))
                         {
                            start = _eina_simple_xml_whitespace_skip
                              (start, end);
                            end = _eina_simple_xml_whitespace_unskip
                              (end, start + 1);
                         }

                       CB(type, start, end);

                       if (type != EINA_SIMPLE_XML_ERROR)
                         itr = p + 1;
                       else
                         itr = p;
                    }
                  else
                    {
                       CB(EINA_SIMPLE_XML_ERROR, itr, itr_end);
                       return EINA_FALSE;
                    }
               }
          }
        else
          {
             const char *p, *end;

             if (strip)
               {
                  p = _eina_simple_xml_whitespace_skip(itr, itr_end);
                  if (p)
                    {
                       CB(EINA_SIMPLE_XML_IGNORED, itr, p);
                       itr = p;
                    }
               }

             p = _eina_simple_xml_tag_start_find(itr, itr_end);
             if (!p) p = itr_end;

             end = p;
             if (strip)
               end = _eina_simple_xml_whitespace_unskip(end, itr);

             if (itr != end)
               CB(EINA_SIMPLE_XML_DATA, itr, end);

             if ((strip) && (end < p))
               CB(EINA_SIMPLE_XML_IGNORED, end, p);

             itr = p;
          }
     }

#undef CB

   return EINA_TRUE;
}

/**
 * Given the contents of a tag, find where the attributes start.
 *
 * The tag contents is returned by eina_simple_xml_parse() when
 * type is #EINA_SIMPLE_XML_OPEN or #EINA_SIMPLE_XML_OPEN_EMPTY.
 *
 * @return pointer to the start of attributes, it can be used
 *         to feed eina_simple_xml_attributes_parse(). NULL is returned
 *         if no attributes were found.
 */
EAPI const char *
eina_simple_xml_tag_attributes_find(const char *buf, unsigned buflen)
{
   const char *itr = buf, *itr_end = buf + buflen;

   for (; itr < itr_end; itr++)
     {
        if (!isspace(*itr))
          {
             /* user skip tagname and already gave it the attributes */
             if (*itr == '=')
               return buf;
          }
        else
          {
             itr = _eina_simple_xml_whitespace_skip(itr + 1, itr_end);
             if (itr == itr_end)
               return NULL;
             return itr;
          }
     }

   return NULL;
}

/**
 * Given a buffer with xml attributes, parse them to key=value pairs.
 *
 * @param buf the input string. May not contain \0 terminator.
 * @param buflen the input string size.
 * @param func what to call back while parse to do some action. The
 *        first parameter is the given user @a data, the second is the
 *        key (null-terminated) and the last is the value (null
 *        terminated). These strings should not be modified and
 *        reference is just valid until the function return.
 *
 * @return EINA_TRUE on success or EINA_FALSE if it was aborted by user or
 *         parsing error.
 */
EAPI Eina_Bool
eina_simple_xml_attributes_parse(const char *buf, unsigned buflen, Eina_Simple_XML_Attribute_Cb func, const void *data)
{
   const char *itr = buf, *itr_end = buf + buflen;
   char *tmpbuf = alloca(buflen + 1);

   if (!buf) return EINA_FALSE;
   if (!func) return EINA_FALSE;

   while (itr < itr_end)
     {
        const char *p = _eina_simple_xml_whitespace_skip(itr, itr_end);
        const char *key, *key_end, *value, *value_end;
        char *tval;

        if (p == itr_end) return EINA_TRUE;

        key = p;
        for (key_end = key; key_end < itr_end; key_end++)
          if ((*key_end == '=') || (isspace(*key_end))) break;
        if (key_end == itr_end) return EINA_FALSE;
        if (key_end == key) continue;

        if (*key_end == '=') value = key_end + 1;
        else
          {
             value = memchr(key_end, '=', itr_end - key_end);
             if (!value) return EINA_FALSE;
             value++;
          }
        for (; value < itr_end; value++)
          if (!isspace(*value)) break;
        if (value == itr_end) return EINA_FALSE;

        if ((*value == '"') || (*value == '\''))
          {
             value_end = memchr(value + 1, *value, itr_end - value);
             if (!value_end) return EINA_FALSE;
             value++;
          }
        else
          {
             value_end = _eina_simple_xml_whitespace_find(value, itr_end);
          }

        memcpy(tmpbuf, key, key_end - key);
        tmpbuf[key_end - key] = '\0';

        tval = tmpbuf + (key_end - key) + 1;
        memcpy(tval, value, value_end - value);
        tval[value_end - value] = '\0';

        if (!func((void*)data, tmpbuf, tval))
          return EINA_FALSE;

        itr = value_end + 1;
     }
   return EINA_TRUE;
}

/* Node loader *************************************************************/

/**
 * Create (and append) new attribute to tag.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the attribute will be appended to attributes list.
 * @param key null-terminated string. Must not be NULL.
 * @param value null-terminated string. If NULL, the empty string will be used.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_attribute_free() or indirectly
 *         with eina_simple_xml_node_tag_free().
 */
EAPI Eina_Simple_XML_Attribute *
eina_simple_xml_attribute_new(Eina_Simple_XML_Node_Tag *parent, const char *key, const char *value)
{
   Eina_Simple_XML_Attribute *attr;

   if (!key) return NULL;

   attr = eina_mempool_malloc(_eina_simple_xml_attribute_mp, sizeof(*attr));
   if (!attr)
     {
        ERR("could not allocate memory for attribute from mempool");
        return NULL;
     }

   EINA_MAGIC_SET(attr, EINA_MAGIC_SIMPLE_XML_ATTRIBUTE);
   attr->parent = parent;
   attr->key = eina_stringshare_add(key);
   attr->value = eina_stringshare_add(value ? value : "");

   if (parent)
     parent->attributes = eina_inlist_append
       (parent->attributes, EINA_INLIST_GET(attr));

   return attr;
}

/**
 * Remove attribute from parent and delete it.
 *
 * @param attr attribute to release memory.
 */
EAPI void
eina_simple_xml_attribute_free(Eina_Simple_XML_Attribute *attr)
{
   EINA_MAGIC_CHECK_ATTRIBUTE(attr);

   if (attr->parent)
     attr->parent->attributes = eina_inlist_remove
          (attr->parent->attributes, EINA_INLIST_GET(attr));

   eina_stringshare_del(attr->key);
   eina_stringshare_del(attr->value);
   EINA_MAGIC_SET(attr, EINA_MAGIC_NONE);
   eina_mempool_free(_eina_simple_xml_attribute_mp, attr);
}

static void
_eina_simple_xml_node_data_free(Eina_Simple_XML_Node_Data *node)
{
   if (node->base.parent)
     node->base.parent->children = eina_inlist_remove
          (node->base.parent->children, EINA_INLIST_GET(&node->base));

   EINA_MAGIC_SET(&node->base, EINA_MAGIC_NONE);
   free(node);
}

/**
 * Create new tag. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the tag will be appended to children list.
 * @param name null-terminated string. Must not be NULL.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_tag_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_Tag *
eina_simple_xml_node_tag_new(Eina_Simple_XML_Node_Tag *parent, const char *name)
{
   Eina_Simple_XML_Node_Tag *n;

   if (!name) return NULL;

   n = eina_mempool_malloc(_eina_simple_xml_tag_mp, sizeof(*n));
   if (!n)
     {
        ERR("could not allocate memory for node from mempool");
        return NULL;
     }

   memset(n, 0, sizeof(*n));

   EINA_MAGIC_SET(&n->base, EINA_MAGIC_SIMPLE_XML_TAG);

   n->base.type = EINA_SIMPLE_XML_NODE_TAG;
   n->base.parent = parent;
   n->name = eina_stringshare_add(name);

   if (parent)
     parent->children = eina_inlist_append
       (parent->children, EINA_INLIST_GET(&n->base));

   return n;
}

void
_eina_simple_xml_node_tag_free(Eina_Simple_XML_Node_Tag *tag)
{
   while (tag->children)
     {
        Eina_Simple_XML_Node *n = EINA_INLIST_CONTAINER_GET
          (tag->children, Eina_Simple_XML_Node);
        if (n->type == EINA_SIMPLE_XML_NODE_TAG)
          _eina_simple_xml_node_tag_free((Eina_Simple_XML_Node_Tag *)n);
        else
          _eina_simple_xml_node_data_free((Eina_Simple_XML_Node_Data *)n);
     }

   while (tag->attributes)
     {
        Eina_Simple_XML_Attribute *a = EINA_INLIST_CONTAINER_GET
          (tag->attributes, Eina_Simple_XML_Attribute);
        eina_simple_xml_attribute_free(a);
     }

   if (tag->base.parent)
     tag->base.parent->children = eina_inlist_remove
          (tag->base.parent->children, EINA_INLIST_GET(&tag->base));

   eina_stringshare_del(tag->name);
   EINA_MAGIC_SET(&tag->base, EINA_MAGIC_NONE);
   eina_mempool_free(_eina_simple_xml_tag_mp, tag);
}

/**
 * Remove tag from parent and delete it.
 *
 * @param tag to release memory.
 */
EAPI void
eina_simple_xml_node_tag_free(Eina_Simple_XML_Node_Tag *tag)
{
   EINA_MAGIC_CHECK_TAG(&tag->base);
   if (tag->base.type != EINA_SIMPLE_XML_NODE_TAG)
     {
        ERR("expected tag node!");
        return;
     }
   _eina_simple_xml_node_tag_free(tag);
}

static Eina_Simple_XML_Node_Data *
_eina_simple_xml_node_data_new(Eina_Simple_XML_Node_Tag *parent, Eina_Simple_XML_Node_Type type, const char *content, unsigned length)
{
   Eina_Simple_XML_Node_Data *n = malloc(sizeof(*n) + length + 1);

   if (!content) return NULL;

   if (!n)
     {
        ERR("could not allocate memory for node");
        return NULL;
     }

   EINA_MAGIC_SET(&n->base, EINA_MAGIC_SIMPLE_XML_DATA);
   n->base.type = type;
   n->base.parent = parent;

   n->length = length;
   memcpy(n->data, content, length);
   n->data[length] = '\0';

   if (parent)
     parent->children = eina_inlist_append
       (parent->children, EINA_INLIST_GET(&n->base));

   return n;
}

/**
 * Create new data. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the data will be appended to children list.
 * @param content string to be used. Must not be NULL.
 * @param length size in bytes of @a content.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_data_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_Data *
eina_simple_xml_node_data_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length)
{
   return _eina_simple_xml_node_data_new
     (parent, EINA_SIMPLE_XML_NODE_DATA, contents, length);
}

/**
 * Remove data from parent and delete it.
 *
 * @param data to release memory.
 */
EAPI void
eina_simple_xml_node_data_free(Eina_Simple_XML_Node_Data *node)
{
   EINA_MAGIC_CHECK_DATA(&node->base);
   if (node->base.type != EINA_SIMPLE_XML_NODE_DATA)
     {
        ERR("expected node of type: data!");
        return;
     }
   _eina_simple_xml_node_data_free(node);
}

/**
 * Create new cdata. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the cdata will be appended to children list.
 * @param content string to be used. Must not be NULL.
 * @param length size in bytes of @a content.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_cdata_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_CData *
eina_simple_xml_node_cdata_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length)
{
   return _eina_simple_xml_node_data_new
     (parent, EINA_SIMPLE_XML_NODE_CDATA, contents, length);
}

/**
 * Remove cdata from parent and delete it.
 *
 * @param cdata to release memory.
 */
EAPI void
eina_simple_xml_node_cdata_free(Eina_Simple_XML_Node_Data *node)
{
   EINA_MAGIC_CHECK_DATA(&node->base);
   if (node->base.type != EINA_SIMPLE_XML_NODE_CDATA)
     {
        ERR("expected node of type: cdata!");
        return;
     }
   _eina_simple_xml_node_data_free(node);
}

/**
 * Create new processing. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the processing will be appended to children list.
 * @param content string to be used. Must not be NULL.
 * @param length size in bytes of @a content.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_processing_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_Processing *
eina_simple_xml_node_processing_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length)
{
   return _eina_simple_xml_node_data_new
     (parent, EINA_SIMPLE_XML_NODE_PROCESSING, contents, length);
}

/**
 * Remove processing from parent and delete it.
 *
 * @param processing to release memory.
 */
EAPI void
eina_simple_xml_node_processing_free(Eina_Simple_XML_Node_Data *node)
{
   EINA_MAGIC_CHECK_DATA(&node->base);
   if (node->base.type != EINA_SIMPLE_XML_NODE_PROCESSING)
     {
        ERR("expected node of type: processing!");
        return;
     }
   _eina_simple_xml_node_data_free(node);
}

/**
 * Create new doctype. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the doctype will be appended to children list.
 * @param content string to be used. Must not be NULL.
 * @param length size in bytes of @a content.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_doctype_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_Doctype *
eina_simple_xml_node_doctype_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length)
{
   return _eina_simple_xml_node_data_new
     (parent, EINA_SIMPLE_XML_NODE_DOCTYPE, contents, length);
}

/**
 * Remove doctype from parent and delete it.
 *
 * @param doctype to release memory.
 */
EAPI void
eina_simple_xml_node_doctype_free(Eina_Simple_XML_Node_Data *node)
{
   EINA_MAGIC_CHECK_DATA(&node->base);
   if (node->base.type != EINA_SIMPLE_XML_NODE_DOCTYPE)
     {
        ERR("expected node of type: doctype!");
        return;
     }
   _eina_simple_xml_node_data_free(node);
}

/**
 * Create new comment. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the comment will be appended to children list.
 * @param content string to be used. Must not be NULL.
 * @param length size in bytes of @a content.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_comment_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_Comment *
eina_simple_xml_node_comment_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length)
{
   return _eina_simple_xml_node_data_new
     (parent, EINA_SIMPLE_XML_NODE_COMMENT, contents, length);
}

/**
 * Remove comment from parent and delete it.
 *
 * @param comment to release memory.
 */
EAPI void
eina_simple_xml_node_comment_free(Eina_Simple_XML_Node_Data *node)
{
   EINA_MAGIC_CHECK_DATA(&node->base);
   if (node->base.type != EINA_SIMPLE_XML_NODE_COMMENT)
     {
        ERR("expected node of type: comment!");
        return;
     }
   _eina_simple_xml_node_data_free(node);
}

struct eina_simple_xml_node_load_ctxt
{
   Eina_Simple_XML_Node_Root *root;
   Eina_Simple_XML_Node_Tag *current;
};

static Eina_Bool
_eina_simple_xml_attrs_parse(void *data, const char *key, const char *value)
{
   Eina_Simple_XML_Node_Tag *n = data;
   Eina_Simple_XML_Attribute *attr;

   attr = eina_simple_xml_attribute_new(n, key, value);
   return !!attr;
}

static Eina_Bool
_eina_simple_xml_node_parse(void *data, Eina_Simple_XML_Type type, const char *content, unsigned offset, unsigned length)
{
   struct eina_simple_xml_node_load_ctxt *ctx = data;

   switch (type)
     {
      case EINA_SIMPLE_XML_OPEN:
      case EINA_SIMPLE_XML_OPEN_EMPTY:
        {
           Eina_Simple_XML_Node_Tag *n;
           const char *name, *name_end, *attrs;

           attrs = eina_simple_xml_tag_attributes_find(content, length);
           if (!attrs)
             name_end = content + length;
           else
             name_end = attrs;

           name_end = _eina_simple_xml_whitespace_unskip(name_end, content);

           name = eina_stringshare_add_length(content, name_end - content);
           n = eina_simple_xml_node_tag_new(ctx->current, name);
           eina_stringshare_del(name);
           if (!n) return EINA_FALSE;

           if (attrs)
             eina_simple_xml_attributes_parse
               (attrs, length - (attrs - content),
                _eina_simple_xml_attrs_parse, n);

           if (type == EINA_SIMPLE_XML_OPEN)
             ctx->current = n;
        }
        break;

      case EINA_SIMPLE_XML_CLOSE:
         if (ctx->current->base.parent)
           {
              const char *end = _eina_simple_xml_whitespace_unskip
                (content + length, content);
              int len;
              len = end - content;
              if ((len == 0) /* </> closes the tag for us. */ ||
                  ((eina_stringshare_strlen(ctx->current->name) == len) &&
                   (memcmp(ctx->current->name, content, len) == 0)))
                ctx->current = ctx->current->base.parent;
              else
                WRN("closed incorrect tag: '%.*s', '%s' was expected!",
                    len, content, ctx->current->name);
           }
         else
           WRN("closed tag '%.*s' but already at document root!",
               length, content);
         break;

      case EINA_SIMPLE_XML_DATA:
         return !!eina_simple_xml_node_data_new
           (ctx->current, content, length);
      case EINA_SIMPLE_XML_CDATA:
         return !!eina_simple_xml_node_cdata_new
           (ctx->current, content, length);
      case EINA_SIMPLE_XML_PROCESSING:
         return !!eina_simple_xml_node_processing_new
           (ctx->current, content, length);
      case EINA_SIMPLE_XML_DOCTYPE:
         return !!eina_simple_xml_node_doctype_new
           (ctx->current, content, length);
      case EINA_SIMPLE_XML_COMMENT:
         return !!eina_simple_xml_node_comment_new
           (ctx->current, content, length);

      case EINA_SIMPLE_XML_ERROR:
         ERR("parser error at offset %u-%u: %.*s",
             offset, length, length, content);
         break;
      case EINA_SIMPLE_XML_IGNORED:
         DBG("ignored contents at offset %u-%u: %.*s",
             offset, length, length, content);
         break;
     }

   return EINA_TRUE;
}

/**
 * Load a XML node tree based on the given string.
 *
 * @param buf the input string. May not contain \0 terminator.
 * @param buflen the input string size.
 * @param strip whenever this parser should strip leading and trailing
 *        whitespace.
 *
 * @return document root with children tags, or NULL on errors.
 *         Document with errors may return partial tree instead of NULL,
 *         we'll do our best to avoid returning nothing.
 */
EAPI Eina_Simple_XML_Node_Root *
eina_simple_xml_node_load(const char *buf, unsigned buflen, Eina_Bool strip)
{
   Eina_Simple_XML_Node_Root *root;
   struct eina_simple_xml_node_load_ctxt ctx;

   if (!buf) return NULL;

   root = eina_mempool_malloc(_eina_simple_xml_tag_mp, sizeof(*root));
   if (!root) return NULL;

   memset(root, 0, sizeof(*root));
   EINA_MAGIC_SET(&root->base, EINA_MAGIC_SIMPLE_XML_TAG);
   root->base.type = EINA_SIMPLE_XML_NODE_ROOT;

   ctx.root = root;
   ctx.current = root;
   eina_simple_xml_parse(buf, buflen, strip, _eina_simple_xml_node_parse, &ctx);

   return root;
}

/**
 * Free node tree build with eina_simple_xml_node_load()
 *
 * @param root memory returned by eina_simple_xml_node_load()
 */
EAPI void
eina_simple_xml_node_root_free(Eina_Simple_XML_Node_Root *root)
{
   if (!root) return;
   EINA_MAGIC_CHECK_TAG(&root->base);
   if (root->base.type != EINA_SIMPLE_XML_NODE_ROOT)
     {
        ERR("expected root node!");
        return;
     }
   _eina_simple_xml_node_tag_free(root);
}

static inline void
_eina_simple_xml_node_dump_indent(Eina_Strbuf *buf, const char *indent, unsigned level)
{
   unsigned i, indent_len = strlen(indent);
   for (i = 0; i < level; i++)
     eina_strbuf_append_length(buf, indent, indent_len);
}

static void
_eina_simple_xml_node_tag_attributes_append(Eina_Strbuf *buf, Eina_Simple_XML_Node_Tag *tag)
{
   Eina_Simple_XML_Attribute *a;

   EINA_INLIST_FOREACH(tag->attributes, a)
     eina_strbuf_append_printf(buf, " %s=\"%s\"", a->key, a->value);
}

static void _eina_simple_xml_node_dump(Eina_Strbuf *buf, Eina_Simple_XML_Node *node, const char *indent, unsigned level);

static void
_eina_simple_xml_node_children_dump(Eina_Strbuf *buf, Eina_Simple_XML_Node_Tag *tag, const char *indent, unsigned level)
{
   Eina_Simple_XML_Node *node;

   EINA_INLIST_FOREACH(tag->children, node)
     _eina_simple_xml_node_dump(buf, node, indent, level);
}

static void
_eina_simple_xml_node_dump(Eina_Strbuf *buf, Eina_Simple_XML_Node *node, const char *indent, unsigned level)
{
   switch (node->type)
     {
      case EINA_SIMPLE_XML_NODE_ROOT:
         _eina_simple_xml_node_children_dump
           (buf, (Eina_Simple_XML_Node_Tag *)node, indent, level);
         break;

      case EINA_SIMPLE_XML_NODE_TAG:
        {
           Eina_Simple_XML_Node_Tag *n = (Eina_Simple_XML_Node_Tag *)node;

           if (indent) _eina_simple_xml_node_dump_indent(buf, indent, level);

           eina_strbuf_append_char(buf, '<');
           eina_strbuf_append_length
               (buf, n->name, eina_stringshare_strlen(n->name));

           if (n->attributes)
             _eina_simple_xml_node_tag_attributes_append(buf, n);

           if (n->children)
             eina_strbuf_append_char(buf, '>');
           else
             eina_strbuf_append_length(buf, "/>", sizeof("/>") - 1);

           if (indent) eina_strbuf_append_char(buf, '\n');

           if (n->children)
             {
                _eina_simple_xml_node_children_dump(buf, n, indent, level + 1);

                if (indent)
                  _eina_simple_xml_node_dump_indent(buf, indent, level);

                eina_strbuf_append_length(buf, "</", sizeof("</") - 1);
                eina_strbuf_append_length
                    (buf, n->name, eina_stringshare_strlen(n->name));
                eina_strbuf_append_char(buf, '>');

                if (indent) eina_strbuf_append_char(buf, '\n');
             }
        }
        break;
      case EINA_SIMPLE_XML_NODE_DATA:
        {
           Eina_Simple_XML_Node_Data *n = (Eina_Simple_XML_Node_Data *)node;

           if (indent) _eina_simple_xml_node_dump_indent(buf, indent, level);
           eina_strbuf_append_length(buf, n->data, n->length);
           if (indent) eina_strbuf_append_char(buf, '\n');
        }
        break;

      case EINA_SIMPLE_XML_NODE_CDATA:
        {
           Eina_Simple_XML_Node_Data *n = (Eina_Simple_XML_Node_Data *)node;

           if (indent) _eina_simple_xml_node_dump_indent(buf, indent, level);
           eina_strbuf_append_length(buf, "<![CDATA[", sizeof("<![CDATA[") - 1);
           eina_strbuf_append_length(buf, n->data, n->length);
           eina_strbuf_append_length(buf, "]]>", sizeof("]]>") - 1);
           if (indent) eina_strbuf_append_char(buf, '\n');
        }
        break;

      case EINA_SIMPLE_XML_NODE_PROCESSING:
        {
           Eina_Simple_XML_Node_Data *n = (Eina_Simple_XML_Node_Data *)node;

           if (indent) _eina_simple_xml_node_dump_indent(buf, indent, level);
           eina_strbuf_append_length(buf, "<?", sizeof("<?") - 1);
           eina_strbuf_append_length(buf, n->data, n->length);
           eina_strbuf_append_length(buf, " ?>", sizeof(" ?>") - 1);
           if (indent) eina_strbuf_append_char(buf, '\n');
        }
        break;

      case EINA_SIMPLE_XML_NODE_DOCTYPE:
        {
           Eina_Simple_XML_Node_Data *n = (Eina_Simple_XML_Node_Data *)node;

           if (indent) _eina_simple_xml_node_dump_indent(buf, indent, level);
           eina_strbuf_append_length
             (buf, "<!DOCTYPE ", sizeof("<!DOCTYPE ") - 1);
           eina_strbuf_append_length(buf, n->data, n->length);
           eina_strbuf_append_char(buf, '>');
           if (indent) eina_strbuf_append_char(buf, '\n');
        }
        break;

      case EINA_SIMPLE_XML_NODE_COMMENT:
        {
           Eina_Simple_XML_Node_Data *n = (Eina_Simple_XML_Node_Data *)node;

           if (indent) _eina_simple_xml_node_dump_indent(buf, indent, level);
           eina_strbuf_append_length(buf, "<!-- ", sizeof("<!-- ") - 1);
           eina_strbuf_append_length(buf, n->data, n->length);
           eina_strbuf_append_length(buf, " -->", sizeof(" -->") - 1);
           if (indent) eina_strbuf_append_char(buf, '\n');
        }
        break;
     }
}

/**
 * Converts the node tree under the given element to a XML string.
 *
 * @param node the base node to convert.
 * @param indent indentation string, or NULL to disable it.
 *
 * @param NULL on errors or a newly allocated string on success.
 */
EAPI char *
eina_simple_xml_node_dump(Eina_Simple_XML_Node *node, const char *indent)
{
   Eina_Strbuf *buf;
   char *ret;

   if (!node) return NULL;

   buf = eina_strbuf_new();
   if (!buf) return NULL;

   _eina_simple_xml_node_dump(buf, node, indent, 0);

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

/**
 * @}
 */
