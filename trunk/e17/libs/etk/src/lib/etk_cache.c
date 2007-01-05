/** @file etk_cache.c */
#include "etk_cache.h"
#include <stdlib.h>
#include <string.h>
#include "etk_utils.h"

/**
 * @addtogroup Etk_Cache
 * @{
 */

#if 0
 
static char *_etk_cache_hash_key_generate(const char *filename, const char *key);
static Evas_Bool _etk_cache_hash_list_free(Evas_Hash *hash, const char *key, void *data, void *fdata);

/**************************
 *
 * Implementation
 *
 **************************/
 
typedef struct Etk_Cache_Item
{
   char *filename;
   char *key;
   char *hash_key;
   Evas_Object *object;
} Etk_Cache_Item;

/**
 * @brief Creates a new cache system that you can use to cache image objects or Edje objects. You usually don't need
 * to use that, except if you are implementing your own widget or your own tree model which may need to load a lot
 * of images efficiently
 * @param size the max number of objects the cache system could store
 * @return Returns the new cache system
 * @note You will need to destroy it with etk_cache_destroy() when you no longer need it
 */
Etk_Cache *etk_cache_new(int size)
{
   Etk_Cache *cache;
   
   cache = malloc(sizeof(Etk_Cache));
   cache->cached_objects = NULL;
   cache->objects_hash = NULL;
   cache->size = ETK_MAX(0, size);
   
   return cache;
}

/**
 * @brief Destroys the cache system: it destroys all the cached objects, and frees the memory used by the cache system
 * @param cache the cache system to destroy
 */
void etk_cache_destroy(Etk_Cache *cache)
{
   if (!cache)
      return;
   
   etk_cache_clear(cache);
   free(cache);
}

/**
 * @brief Clears the cache system: it destroys all the cached objects. The cache system remains still usable
 * @param cache the cache system to clear
 */
void etk_cache_clear(Etk_Cache *cache)
{
   Etk_Cache_Item *item;
   
   if (!cache)
      return;
   
   while (cache->cached_objects)
   {
      item = cache->cached_objects->data;
      evas_object_del(item->object);
      free(item->filename);
      free(item->key);
      free(item->hash_key);
      free(item);
      
      cache->cached_objects = evas_list_remove_list(cache->cached_objects, cache->cached_objects);
   }
   
   evas_hash_foreach(cache->objects_hash, _etk_cache_hash_list_free, NULL);
   evas_hash_free(cache->objects_hash);
   cache->objects_hash = NULL;
}

/**
 * @brief Sets the max number of objects that the cache system can contain. If the new size is smaller than current
 * number of objects in the cache, the oldest objects that can't fit in the new cache size will be destroyed
 * @param cache the cache system to resize
 * @param size the new size (max number of objects) of the cache system
 */
void etk_cache_size_set(Etk_Cache *cache, int size)
{
   Etk_Cache_Item *item;
   Evas_List *items, *new_items;
   Evas_List *l, *l2;
   int num_objects;
   
   if (!cache)
      return;
   
   cache->size = ETK_MAX(0, size);
   num_objects = evas_list_count(cache->cached_objects);
   
   /* Remove the items that can't fit anymore */
   for (l = evas_list_last(cache->cached_objects); l && num_objects > cache->size; l = l2)
   {
      item = l->data;
      
      if ((items = evas_hash_find(cache->objects_hash, item->hash_key)))
      {
         new_items = evas_list_remove(items, item);
         
         if (!new_items)
            cache->objects_hash = evas_hash_del(cache->objects_hash, item->hash_key, NULL);
         else if (new_items != items)
            evas_hash_modify(cache->objects_hash, item->hash_key, new_items);
      }
      
      evas_object_del(item->object);
      free(item->filename);
      free(item->key);
      free(item->hash_key);
      free(item);
      
      l2 = l->prev;
      cache->cached_objects = evas_list_remove_list(cache->cached_objects, l);
      num_objects--;
   }
}

/**
 * @brief Gets the max number of objects that can be stored by the cache system
 * @param cache a cache system
 * @return Returns the max number of objects that can be stored by the cache system
 */
int etk_cache_size_get(Etk_Cache *cache)
{
   if (!cache)
      return 0;
   return cache->size;
}

/**
 * @brief Gets the current number of objects stored in the cache system
 * @param cache a cache system
 * @return Returns the current number of objects stored by the cache system
 */
int etk_cache_num_objects_get(Etk_Cache *cache)
{
   if (!cache)
      return 0;
   return evas_list_count(cache->cached_objects);
}

/**
 * @brief Adds an Evas image object or an Edje object in the cache system. If the cache is already full, the oldest
 * object will be removed. The object to cache will also be automatically hidden
 * @param cache a cache system
 * @param object the Evas image object or the Edje object to cache
 * @param filename the filename associated to the object
 * @param key the key associated to the object (the group for an Edje object, the key for an image from an Eet file,
 * or NULL otherwise)
 * @note Once the object is added to the cache, you should keep no reference to it. It may for example be deleted if
 * there is no more space in the cache system
 */
void etk_cache_add(Etk_Cache *cache, Evas_Object *object, const char *filename, const char *key)
{
   Etk_Cache_Item *item;
   Evas_List *items, *new_items;
   
   if (!cache || !object || cache->size <= 0 || !filename)
   {
      if (object)
         evas_object_del(object);
      return;
   }
   
   evas_object_hide(object);
   
   /* If no more space is available, we remove the oldest object of the cache */
   if (evas_list_count(cache->cached_objects) >= cache->size)
   {
      item = cache->cached_objects->data;
      
      if ((items = evas_hash_find(cache->objects_hash, item->hash_key)))
      {
         new_items = evas_list_remove(items, item);
         if (!new_items)
            cache->objects_hash = evas_hash_del(cache->objects_hash, item->hash_key, NULL);
         else if (new_items != items)
            evas_hash_modify(cache->objects_hash, item->hash_key, new_items);
      }
      cache->cached_objects = evas_list_remove_list(cache->cached_objects, cache->cached_objects);
      
      evas_object_del(item->object);
      free(item->filename);
      free(item->key);
      free(item->hash_key);
      free(item);
   }
   
   /* We create a new cache-item for the object and we add it to the cache */
   item = malloc(sizeof(Etk_Cache_Item));
   item->filename = strdup(filename);
   item->key = key ? strdup(key) : NULL;
   item->hash_key = _etk_cache_hash_key_generate(filename, key);
   item->object = object;
   
   if ((items = evas_hash_find(cache->objects_hash, item->hash_key)))
      evas_list_append(items, item);
   else
   {
      items = evas_list_append(NULL, item);
      cache->objects_hash = evas_hash_add(cache->objects_hash, item->hash_key, item);
   }
   cache->cached_objects = evas_list_append(cache->cached_objects, item);
}

/**
 * @brief Removes an object from the cache. The object won't be destroyed.
 * @param cache a cache system
 * @param object the object to remove from the cache system
 */
void etk_cache_remove(Etk_Cache *cache, Evas_Object *object)
{
   Etk_Cache_Item *item;
   Evas_List *items, *new_items;
   Evas_List *l;
   
   if (!cache || !object)
      return;
   
   for (l = cache->cached_objects; l; l = l->next)
   {
      item = l->data;
      if (item->object == object)
      {
         /* We have found our object. We can now remove it */
         
         if ((items = evas_hash_find(cache->objects_hash, item->hash_key)))
         {
            new_items = evas_list_remove(items, item);
            if (!new_items)
               cache->objects_hash = evas_hash_del(cache->objects_hash, item->hash_key, NULL);
            else if (new_items != items)
               evas_hash_modify(cache->objects_hash, item->hash_key, new_items);
         }
         cache->cached_objects = evas_list_remove_list(cache->cached_objects, l);
         
         free(item->filename);
         free(item->key);
         free(item->hash_key);
         free(item);
         
         return;
      }
   }
}

/**
 * @brief Finds an object in the cache according to its filename and its key. If the object is present in the cache,
 * it will be removed from the cache and returned. Otherwise NULL is returned
 * @param cache the cache system where to find the object
 * @param filename the filename of the object to find
 * @param key the key associated to the object to find (the group for an Edje object, the key for an image from an
 * Eet file, or NULL otherwise)
 * @return Returns an object corresponding to the given filename and key, or NULL if no such object is cached
 */
Evas_Object *etk_cache_find(Etk_Cache *cache, const char *filename, const char *key)
{
   Etk_Cache_Item *item;
   Evas_List *items, *new_items, *last;
   Evas_Object *object;
   char *hash_key;
   
   if (!cache || !filename)
      return NULL;
   
   hash_key = _etk_cache_hash_key_generate(filename, key);
   if ((items = evas_hash_find(cache->objects_hash, hash_key)))
   {
      /* An object has been found */
      last = evas_list_last(items);
      item = last->data;
      object = item->object;
      
      /* We remove it from the cache */
      new_items = evas_list_remove_list(items, last);
      if (!new_items)
         cache->objects_hash = evas_hash_del(cache->objects_hash, hash_key, NULL);
      else if (new_items != items)
         evas_hash_modify(cache->objects_hash, hash_key, new_items);
      cache->cached_objects = evas_list_remove(cache->cached_objects, item);
      
      free(item->filename);
      free(item->key);
      free(item->hash_key);
      free(item);
   }
   else
      object = NULL;
   
   free(hash_key);
   return object;
}

/**************************
 *
 * Private functions
 *
 **************************/

/* Generates a key to use with the hash table, from the filenam and the key
 * The returned key will have to be freed with free() */
static char *_etk_cache_hash_key_generate(const char *filename, const char *key)
{
   char *hash_key;
   int length;
   
   length = 3;
   if (filename)
      length += strlen(filename);
   if (key)
      length += strlen(key);
   
   hash_key = malloc(length * sizeof(char));
   sprintf(hash_key, "%s::%s", filename ? filename : "", key ? key : "");
   
   return hash_key;
}

/* Called on each member of the hash table of a cache system when it is destroyed */
static Evas_Bool _etk_cache_hash_list_free(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   evas_list_free(data);
   return 1;
}

#else

/**************************
 *
 * Implementation
 *
 **************************/
 
typedef struct Etk_Cache_Item
{
   char *filename;
   char *key;
   Evas_Object *object;
} Etk_Cache_Item;

/**
 * @brief Creates a new cache system that you can use to cache image objects or Edje objects. You usually don't need
 * to use that, except if you are implementing your own widget or your own tree model which may need to load a lot
 * of images efficiently
 * @param size the max number of objects the cache system could store
 * @return Returns the new cache system
 * @note You will need to destroy it with etk_cache_destroy() when you no longer need it
 */
Etk_Cache *etk_cache_new(int size)
{
   Etk_Cache *cache;
   
   cache = malloc(sizeof(Etk_Cache));
   cache->cached_objects = NULL;
   cache->objects_hash = NULL;
   cache->size = ETK_MAX(0, size);
   
   return cache;
}

/**
 * @brief Destroys the cache system: it destroys all the cached objects, and frees the memory used by the cache system
 * @param cache the cache system to destroy
 */
void etk_cache_destroy(Etk_Cache *cache)
{
   if (!cache)
      return;
   
   etk_cache_clear(cache);
   free(cache);
}

/**
 * @brief Clears the cache system: it destroys all the cached objects. The cache system remains still usable
 * @param cache the cache system to clear
 */
void etk_cache_clear(Etk_Cache *cache)
{
   Etk_Cache_Item *item;
   
   if (!cache)
      return;
   
   while (cache->cached_objects)
   {
      item = cache->cached_objects->data;
      evas_object_del(item->object);
      free(item->filename);
      free(item->key);
      free(item);
      
      cache->cached_objects = evas_list_remove_list(cache->cached_objects, cache->cached_objects);
   }
}

/**
 * @brief Sets the max number of objects that the cache system can contain. If the new size is smaller than current
 * number of objects in the cache, the oldest objects that can't fit in the new cache size will be destroyed
 * @param cache the cache system to resize
 * @param size the new size (max number of objects) of the cache system
 */
void etk_cache_size_set(Etk_Cache *cache, int size)
{
}

/**
 * @brief Gets the max number of objects that can be stored by the cache system
 * @param cache a cache system
 * @return Returns the max number of objects that can be stored by the cache system
 */
int etk_cache_size_get(Etk_Cache *cache)
{
   if (!cache)
      return 0;
   return cache->size;
}

/**
 * @brief Gets the current number of objects stored in the cache system
 * @param cache a cache system
 * @return Returns the current number of objects stored by the cache system
 */
int etk_cache_num_objects_get(Etk_Cache *cache)
{
   if (!cache)
      return 0;
   return evas_list_count(cache->cached_objects);
}

/**
 * @brief Adds an Evas image object or an Edje object in the cache system. If the cache is already full, the oldest
 * object will be removed. The object to cache will also be automatically hidden
 * @param cache a cache system
 * @param object the Evas image object or the Edje object to cache
 * @param filename the filename associated to the object
 * @param key the key associated to the object (the group for an Edje object, the key for an image from an Eet file,
 * or NULL otherwise)
 * @note Once the object is added to the cache, you should keep no reference to it. It may for example be deleted if
 * there is no more space in the cache system
 */
void etk_cache_add(Etk_Cache *cache, Evas_Object *object, const char *filename, const char *key)
{
   Etk_Cache_Item *item;
   
   if (!cache || !object || cache->size <= 0 || !filename)
   {
      if (object)
         evas_object_del(object);
      return;
   }
   
   evas_object_hide(object);
   
   /* If no more space is available, we remove the oldest object of the cache */
   if (evas_list_count(cache->cached_objects) >= cache->size)
   {
      item = cache->cached_objects->data;
      
      evas_object_del(item->object);
      free(item->filename);
      free(item->key);
      free(item);
      
      cache->cached_objects = evas_list_remove_list(cache->cached_objects, cache->cached_objects);
   }
   
   /* We create a new cache-item for the object and we add it to the cache */
   item = malloc(sizeof(Etk_Cache_Item));
   item->filename = strdup(filename);
   item->key = key ? strdup(key) : NULL;
   item->object = object;
   
   cache->cached_objects = evas_list_append(cache->cached_objects, item);
}

/**
 * @brief Removes an object from the cache. The object won't be destroyed.
 * @param cache a cache system
 * @param object the object to remove from the cache system
 */
void etk_cache_remove(Etk_Cache *cache, Evas_Object *object)
{
   Etk_Cache_Item *item;
   Evas_List *l;
   
   if (!cache || !object)
      return;
   
   for (l = cache->cached_objects; l; l = l->next)
   {
      item = l->data;
      if (item->object == object)
      {
         /* We have found our object. We can now remove it */
         free(item->filename);
         free(item->key);
         free(item);
         
         cache->cached_objects = evas_list_remove_list(cache->cached_objects, l);
         return;
      }
   }
}

/**
 * @brief Finds an object in the cache according to its filename and its key. If the object is present in the cache,
 * it will be removed from the cache and returned. Otherwise NULL is returned
 * @param cache the cache system where to find the object
 * @param filename the filename of the object to find
 * @param key the key associated to the object to find (the group for an Edje object, the key for an image from an
 * Eet file, or NULL otherwise)
 * @return Returns an object corresponding to the given filename and key, or NULL if no such object is cached
 */
Evas_Object *etk_cache_find(Etk_Cache *cache, const char *filename, const char *key)
{
   Etk_Cache_Item *item;
   Evas_List *l;
   Evas_Object *object;
   
   if (!cache || !filename)
      return NULL;
   
   for (l = evas_list_last(cache->cached_objects); l; l = l->prev)
   {
      item = l->data;
      if (strcmp(item->filename, filename) == 0
         && ((!item->key && !key) || (item->key && key && strcmp(item->key, key) == 0)))
      {
         object = item->object;
         free(item->filename);
         free(item->key);
         free(item);
         
         cache->cached_objects = evas_list_remove(cache->cached_objects, item);
         return object;
      }
   }
   
   return NULL;
}

#endif

/** @} */
