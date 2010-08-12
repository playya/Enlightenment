/* This file has been automatically generated by geneet.py */
/*                      DO NOT MODIFY                      */

#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "history.h"

struct _Hist_Item {
    const char * title;
    const char * url;
    unsigned int visit_count;
    double last_visit;
};

struct _Hist {
    int version;
    Eina_Hash * items;
    const char *__eet_filename;
};

static const char HIST_ITEM_ENTRY[] = "hist_item";
static const char HIST_ENTRY[] = "hist";

static Eet_Data_Descriptor *_hist_item_descriptor = NULL;
static Eet_Data_Descriptor *_hist_descriptor = NULL;

static inline void
_hist_item_init(void)
{
    Eet_Data_Descriptor_Class eddc;
    
    if (_hist_item_descriptor) return;
    
    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Hist_Item);
    _hist_item_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_hist_item_descriptor, Hist_Item, "title", title, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_hist_item_descriptor, Hist_Item, "url", url, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_hist_item_descriptor, Hist_Item, "visit_count", visit_count, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_hist_item_descriptor, Hist_Item, "last_visit", last_visit, EET_T_DOUBLE);
}

static inline void
_hist_item_shutdown(void)
{
    if (!_hist_item_descriptor) return;
    eet_data_descriptor_free(_hist_item_descriptor);
    _hist_item_descriptor = NULL;
}

Hist_Item *
hist_item_new(const char * title, const char * url, unsigned int visit_count, double last_visit)
{
    Hist_Item *hist_item = calloc(1, sizeof(Hist_Item));
    
    if (!hist_item)
       {
          fprintf(stderr, "ERROR: could not calloc Hist_Item\n");
          return NULL;
       }

    hist_item->title = eina_stringshare_add(title ? title : "Untitled");
    hist_item->url = eina_stringshare_add(url ? url : "about:blank");
    hist_item->visit_count = visit_count;
    hist_item->last_visit = last_visit;

    return hist_item;
}

void
hist_item_free(Hist_Item *hist_item)
{
    eina_stringshare_del(hist_item->title);
    eina_stringshare_del(hist_item->url);
    free(hist_item);
}

inline const char *
hist_item_title_get(const Hist_Item *hist_item)
{
    return hist_item->title;
}

inline void
hist_item_title_set(Hist_Item *hist_item, const char *title)
{
    EINA_SAFETY_ON_NULL_RETURN(hist_item);
    eina_stringshare_del(hist_item->title);
    hist_item->title = eina_stringshare_add(title);
}
  
inline const char *
hist_item_url_get(const Hist_Item *hist_item)
{
    return hist_item->url;
}

inline void
hist_item_url_set(Hist_Item *hist_item, const char *url)
{
    EINA_SAFETY_ON_NULL_RETURN(hist_item);
    eina_stringshare_del(hist_item->url);
    hist_item->url = eina_stringshare_add(url);
}
  
inline unsigned int
hist_item_visit_count_get(const Hist_Item *hist_item)
{
    return hist_item->visit_count;
}

inline void
hist_item_visit_count_set(Hist_Item *hist_item, unsigned int visit_count)
{
    EINA_SAFETY_ON_NULL_RETURN(hist_item);
    hist_item->visit_count = visit_count;
}
  
inline double
hist_item_last_visit_get(const Hist_Item *hist_item)
{
    return hist_item->last_visit;
}

inline void
hist_item_last_visit_set(Hist_Item *hist_item, double last_visit)
{
    EINA_SAFETY_ON_NULL_RETURN(hist_item);
    hist_item->last_visit = last_visit;
}
  

static inline void
_hist_init(void)
{
    Eet_Data_Descriptor_Class eddc;
    
    if (_hist_descriptor) return;
    
    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Hist);
    _hist_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_hist_descriptor, Hist, "version", version, EET_T_INT);
    EET_DATA_DESCRIPTOR_ADD_HASH(_hist_descriptor, Hist, "items", items, _hist_item_descriptor);
}

static inline void
_hist_shutdown(void)
{
    if (!_hist_descriptor) return;
    eet_data_descriptor_free(_hist_descriptor);
    _hist_descriptor = NULL;
}

Hist *
hist_new(int version)
{
    Hist *hist = calloc(1, sizeof(Hist));
    
    if (!hist)
       {
          fprintf(stderr, "ERROR: could not calloc Hist\n");
          return NULL;
       }

    hist->version = version;
    hist->items = eina_hash_stringshared_new(EINA_FREE_CB(hist_item_free));

    return hist;
}

void
hist_free(Hist *hist)
{
    eina_hash_free(hist->items);
    free(hist);
}

inline int
hist_version_get(const Hist *hist)
{
    return hist->version;
}

inline void
hist_version_set(Hist *hist, int version)
{
    EINA_SAFETY_ON_NULL_RETURN(hist);
    hist->version = version;
}
  
void
hist_items_add(Hist *hist, const char * url, Hist_Item *hist_item)
{
    EINA_SAFETY_ON_NULL_RETURN(hist);
    eina_hash_add(hist->items, url, hist_item);
}

void
hist_items_del(Hist *hist, const char * url)
{
    EINA_SAFETY_ON_NULL_RETURN(hist);
    eina_hash_del(hist->items, url, NULL);
}

inline Hist_Item *
hist_items_get(const Hist *hist, const char * url)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(hist, NULL);
    return eina_hash_find(hist->items, url);
}

inline Eina_Hash *
hist_items_hash_get(const Hist *hist)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(hist, NULL);
    return hist->items;
}

void
hist_items_modify(Hist *hist, const char * key, void *value)
{
    EINA_SAFETY_ON_NULL_RETURN(hist);
    eina_hash_modify(hist->items, key, value);
}

Hist *
hist_load(const char *filename)
{
    Hist *hist;
    Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
    if (!ef)
      {
        fprintf(stderr, "ERROR: could not open '%s' for read\n", filename);
        return NULL;
      }
    
    hist = eet_data_read(ef, _hist_descriptor, HIST_ENTRY);
    hist->__eet_filename = eina_stringshare_add(filename);

    if (!hist->items) hist->items = eina_hash_stringshared_new(EINA_FREE_CB(hist_item_free));

    eet_close(ef);
    return hist;
}

Eina_Bool
hist_save(Hist *hist, const char *filename)
{
    char tmp[PATH_MAX];
    Eet_File *ef;
    Eina_Bool ret;
    unsigned int i, len;
    struct stat st;
    
    if (filename) hist->__eet_filename = eina_stringshare_add(filename);
    else if (hist->__eet_filename) filename = hist->__eet_filename;
    else return EINA_FALSE;

    len = eina_strlcpy(tmp, filename, sizeof(tmp));
    if (len + 12 >= (int)sizeof(tmp))
      {
        fprintf(stderr, "ERROR: filename is too big: %s\n", filename);
        return EINA_FALSE;
      }
    
    i = 0;
    do
       {
          snprintf(tmp + len, 12, ".%u", i);
          i++;
       }
    while(stat(tmp, &st) == 0);
    
    ef = eet_open(tmp, EET_FILE_MODE_WRITE);
    if (!ef)
       {
          fprintf(stderr, "ERROR: could not open '%s' for write\n", tmp);
          return EINA_FALSE;
       }

    ret = !!eet_data_write(ef, _hist_descriptor, HIST_ENTRY, hist, EINA_TRUE);
    eet_close(ef);
    
    if (ret)
       {
          unlink(filename);
          rename(tmp, filename);
       }
    
    return ret;
}

void
history_init(void)
{
    _hist_item_init();
    _hist_init();
}

void
history_shutdown(void)
{
    _hist_item_shutdown();
    _hist_shutdown();
}

