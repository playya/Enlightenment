/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <assert.h>

#include "evas_common.h"
#include "evas_private.h"
#include "evas_image_private.h"

#define SCALECACHE 1

#define MAX_SCALEITEMS 16
#define MIN_SCALE_USES 3
#define MIN_SCALE_AGE_GAP 5000
#define MIN_SCALECACHE_SIZE 3200

typedef struct _Scaleitem Scaleitem;

struct _Scaleitem
{
   EINA_INLIST;
   unsigned long long usage;
   unsigned long long usage_count;
   RGBA_Image *im, *parent_im;
   int src_x, src_y, src_w, src_h;
   int dst_w, dst_h;
   int smooth, populate_me, flop;
};

#ifdef SCALECACHE
static unsigned long long use_counter = 0;

#ifdef BUILD_PTHREAD
static LK(cache_lock);
#endif
static Eina_Inlist *cache_list = NULL;
static int cache_size = 0;
static int max_cache_size = 4 * 1024 * 1024;
#endif

void
evas_common_scalecache_init(void)
{
#ifdef SCALECACHE
   use_counter = 0;
   LKI(cache_lock);
#endif
}

void
evas_common_scalecache_shutdown(void)
{
#ifdef SCALECACHE
   LKD(cache_lock);
#endif
}

void
evas_common_rgba_image_scalecache_init(Image_Entry *ie)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   LKI(im->cache.lock);
#endif
}

void
evas_common_rgba_image_scalecache_shutdown(Image_Entry *ie)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   evas_common_rgba_image_scalecache_dirty(ie);
   LKD(im->cache.lock);
#endif
}

void
evas_common_rgba_image_scalecache_dirty(Image_Entry *ie)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   LKL(im->cache.lock);
   while (im->cache.list)
     {
        Scaleitem *sci;
        sci = im->cache.list->data;
        im->cache.list = eina_list_remove(im->cache.list, sci);
        if (sci->im)
          {
//             printf(" 0- %i\n", sci->dst_w * sci->dst_h * 4);
             evas_common_rgba_image_free(&sci->im->cache_entry);
             cache_size -= sci->dst_w * sci->dst_h * 4;
             cache_list = eina_inlist_remove(cache_list, (Eina_Inlist *)sci);
          }
        free(sci);
     }
   LKU(im->cache.lock);
#endif
}

void
evas_common_rgba_image_scalecache_orig_use(Image_Entry *ie)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   LKL(im->cache.lock);
   use_counter++;
   // FIXME: if orig not loaded, reload
   // FIXME: mark orig with current used counter
   im->cache.orig_usage++;
   im->cache.usage_count = use_counter;
   LKU(im->cache.lock);
#endif
}

int
evas_common_rgba_image_scalecache_usage_get(Image_Entry *ie)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   int size = 0;
   Eina_List *l;
   Scaleitem *sci;
   LKL(im->cache.lock);
   EINA_LIST_FOREACH(im->cache.list, l, sci)
     {
        if (sci->im) size += sci->dst_w * sci->dst_h * 4;
     }
   LKU(im->cache.lock);
   return size;
#else
   return 0;
#endif
}

#ifdef SCALECACHE
static Scaleitem *
_sci_find(RGBA_Image *im,
          RGBA_Draw_Context *dc, int smooth,
          int src_region_x, int src_region_y,
          int src_region_w, int src_region_h,
          int dst_region_w, int dst_region_h)
{
   Eina_List *l;
   Scaleitem *sci;
   
   EINA_LIST_FOREACH(im->cache.list, l, sci)
     {
        if (
            (sci->src_w == src_region_w) &&
            (sci->src_h == src_region_h) &&
            (sci->dst_w == dst_region_w) &&
            (sci->dst_h == dst_region_h) &&
            (sci->src_x == src_region_x) &&
            (sci->src_y == src_region_y) &&
            (sci->smooth == smooth)
            )
          {
             if (im->cache.list != l)
               {
                  im->cache.list = eina_list_remove_list(im->cache.list, l);
                  im->cache.list = eina_list_prepend(im->cache.list, sci);
               }
             return sci;
          }
     }
   if (eina_list_count(im->cache.list) > MAX_SCALEITEMS)
     {
        l = eina_list_last(im->cache.list);
        sci = l->data;
        im->cache.list = eina_list_remove_list(im->cache.list, l);
        if (sci->im)
          {
             evas_common_rgba_image_free(&sci->im->cache_entry);
             cache_size -= sci->dst_w * sci->dst_h * 4;
//             printf(" 1- %i\n", sci->dst_w * sci->dst_h * 4);
             cache_list = eina_inlist_remove(cache_list, (Eina_Inlist *)sci);
          }
     }
   else
     {
        sci = malloc(sizeof(Scaleitem));
        memset(sci, 0, sizeof(Eina_Inlist));
        sci->parent_im = im;
     }
   sci->usage = 0;
   sci->usage_count = 0;
   sci->populate_me = 0;
   sci->flop = 0;
   sci->im = NULL;
   sci->src_x = src_region_x;
   sci->src_y = src_region_y;
   sci->src_w = src_region_w;
   sci->src_h = src_region_h;
   sci->dst_w = dst_region_w;
   sci->dst_h = dst_region_h;
   sci->smooth = smooth;
   im->cache.list = eina_list_prepend(im->cache.list, sci);
   return sci;
}

static void
_cache_prune(Scaleitem *notsci)
{
   RGBA_Image *im;
   Scaleitem *sci;
   while (cache_size > max_cache_size)
     {
        if (!cache_list) break;
        sci = (Scaleitem *)(cache_list);
        if (sci == notsci) return;
        im = sci->parent_im;
        if (sci->im)
          {
             evas_common_rgba_image_free(&sci->im->cache_entry);
             sci->im = NULL;
             sci->usage = 0;
             sci->flop += 4;
             cache_size -= sci->dst_w * sci->dst_h * 4;
//             printf(" 2- %i\n", sci->dst_w * sci->dst_h * 4);
             cache_list = eina_inlist_remove(cache_list, (Eina_Inlist *)sci);
             memset(sci, 0, sizeof(Eina_Inlist));
          }
//        printf("FLUSH %i > %i\n", cache_size, max_cache_size);
      }
}
#endif

EAPI void
evas_common_rgba_image_scalecache_prepare(Image_Entry *ie, RGBA_Image *dst,
                                          RGBA_Draw_Context *dc, int smooth,
                                          int src_region_x, int src_region_y,
                                          int src_region_w, int src_region_h,
                                          int dst_region_x, int dst_region_y,
                                          int dst_region_w, int dst_region_h)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   Scaleitem *sci;
   if ((dst_region_w == 0) || (dst_region_h == 0) ||
       (src_region_w == 0) || (src_region_h == 0)) return;
   LKL(im->cache.lock);
   use_counter++;
   if ((src_region_w == dst_region_w) && (src_region_h == dst_region_h))
     {
        im->cache.orig_usage++;
        im->cache.usage_count = use_counter;
        // 1:1 scale.
        LKU(im->cache.lock);
        return;
     }
   sci = _sci_find(im, dc, smooth, 
                   src_region_x, src_region_y, src_region_w, src_region_h, 
                   dst_region_w, dst_region_h);
//   printf("%10i | %4i %4i %4ix%4i -> %4i %4i %4ix%4i | %i\n",
//          (int)use_counter,
//          src_region_x, src_region_y, src_region_w, src_region_h,
//          dst_region_x, dst_region_y, dst_region_w, dst_region_h,
//          smooth);
   if ((sci->usage >= MIN_SCALE_USES) && 
       (sci->usage_count > (use_counter - MIN_SCALE_AGE_GAP)))
     {
        if (!sci->im)
          {
             if ((sci->dst_w < MIN_SCALECACHE_SIZE) && 
                 (sci->dst_h < MIN_SCALECACHE_SIZE))
               {
                  if (sci->flop <= 16)
                    {
                       sci->populate_me = 1;
                       im->cache.populate_count++;
                    }
               }
          }
     }
   sci->usage++;
   sci->usage_count = use_counter;
//   printf("  -------------- used %8i#, %8i@\n", (int)sci->usage, (int)sci->usage_count);
   LKU(im->cache.lock);
#endif
}

#ifdef SCALECACHE
static int pops = 0;
static int hits = 0;
static int misses = 0;
static int noscales = 0;
#endif

EAPI void
evas_common_rgba_image_scalecache_do(Image_Entry *ie, RGBA_Image *dst,
                                     RGBA_Draw_Context *dc, int smooth,
                                     int src_region_x, int src_region_y,
                                     int src_region_w, int src_region_h,
                                     int dst_region_x, int dst_region_y,
                                     int dst_region_w, int dst_region_h)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   Scaleitem *sci;
   int didpop = 0;
/*
   static int i = 0;

   i++;
   if (i > 2000)
     {
        printf("p: %6i, h: %6i, m: %6i, n: %6i\n",
               pops, hits, misses, noscales);
        i = 0;
     }
 */
   if ((dst_region_w == 0) || (dst_region_h == 0) ||
       (src_region_w == 0) || (src_region_h == 0)) return;
   if ((src_region_w == dst_region_w) && (src_region_h == dst_region_h))
     {
        noscales++;
        if (smooth)
          evas_common_scale_rgba_in_to_out_clip_smooth(im, dst, dc,
                                                       src_region_x, src_region_y, 
                                                       src_region_w, src_region_h,
                                                       dst_region_x, dst_region_y, 
                                                       dst_region_w, dst_region_h);
        else
          evas_common_scale_rgba_in_to_out_clip_sample(im, dst, dc,
                                                       src_region_x, src_region_y, 
                                                       src_region_w, src_region_h,
                                                       dst_region_x, dst_region_y, 
                                                       dst_region_w, dst_region_h);
        return;
     }
   LKL(im->cache.lock);
   sci = _sci_find(im, dc, smooth,
                   src_region_x, src_region_y, src_region_w, src_region_h,
                   dst_region_w, dst_region_h);
   if (!sci)
     {
        LKU(im->cache.lock);
        return;
     }
   if (sci->populate_me)
     {
//        printf("##! populate!\n");
        sci->im = evas_common_image_new
          (dst_region_w, dst_region_h, im->flags & RGBA_IMAGE_ALPHA_ONLY);
        if (sci->im)
          {
             static RGBA_Draw_Context *ct = NULL;
        
             im->cache.populate_count--;
             pops++;
             if (!ct)
               {
                  ct = evas_common_draw_context_new();
                  evas_common_draw_context_set_render_op(ct, _EVAS_RENDER_COPY);
               }
             if (smooth)
               evas_common_scale_rgba_in_to_out_clip_smooth
               (im, sci->im, ct,
                src_region_x, src_region_y, 
                src_region_w, src_region_h,
                0, 0,
                dst_region_w, dst_region_h);
             else
               evas_common_scale_rgba_in_to_out_clip_sample
               (im, sci->im, ct,
                src_region_x, src_region_y, 
                src_region_w, src_region_h,
                0, 0,
                dst_region_w, dst_region_h);
             sci->populate_me = 0;
             cache_size += sci->dst_w * sci->dst_h * 4;
//             printf(" + %i @ flop: %i (%ix%i)\n", 
//                    sci->dst_w * sci->dst_h * 4, sci->flop, 
//                    sci->dst_w, sci->dst_h);
             cache_list = eina_inlist_append(cache_list, (Eina_Inlist *)sci);
             _cache_prune(sci);
             didpop = 1;
          }
     }
   if (sci->im)
     {
        if (!didpop)
          {
             cache_list = eina_inlist_remove(cache_list, (Eina_Inlist *)sci);
             cache_list = eina_inlist_append(cache_list, (Eina_Inlist *)sci);
          }
        else
          {
             if (sci->flop > 0) sci->flop--;
          }
//        printf("use cached!\n");
        evas_common_scale_rgba_in_to_out_clip_sample
          (sci->im, dst, dc,
           0, 0,
           dst_region_w, dst_region_h,
           dst_region_x, dst_region_y, 
           dst_region_w, dst_region_h);
        hits++;
        LKU(im->cache.lock);
     }
   else
     {
        misses++;
        LKU(im->cache.lock);
        if (smooth)
          evas_common_scale_rgba_in_to_out_clip_smooth(im, dst, dc,
                                                       src_region_x, src_region_y, 
                                                       src_region_w, src_region_h,
                                                       dst_region_x, dst_region_y, 
                                                       dst_region_w, dst_region_h);
        else
          evas_common_scale_rgba_in_to_out_clip_sample(im, dst, dc,
                                                       src_region_x, src_region_y, 
                                                       src_region_w, src_region_h,
                                                       dst_region_x, dst_region_y, 
                                                       dst_region_w, dst_region_h);
     }
#else   
   RGBA_Image *im = (RGBA_Image *)ie;
   if (smooth)
     evas_common_scale_rgba_in_to_out_clip_smooth(im, dst, dc,
                                                  src_region_x, src_region_y, 
                                                  src_region_w, src_region_h,
                                                  dst_region_x, dst_region_y, 
                                                  dst_region_w, dst_region_h);
   else
     evas_common_scale_rgba_in_to_out_clip_sample(im, dst, dc,
                                                  src_region_x, src_region_y, 
                                                  src_region_w, src_region_h,
                                                  dst_region_x, dst_region_y, 
                                                  dst_region_w, dst_region_h);
#endif
}

// to be done
void
evas_common_rgba_image_scalecache_XXX(Image_Entry *ie)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   LKL(im->cache.lock);
   // FIXME: XXX
   LKU(im->cache.lock);
#endif
}

