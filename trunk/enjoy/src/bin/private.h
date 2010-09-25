#ifndef ENJOY_PRIVATE_H
#define ENJOY_PRIVATE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Elementary.h>
#include <limits.h>

#define stringify(X) #X

typedef struct _App                     App;
typedef struct _DB                      DB;
typedef struct _Song                    Song;
typedef struct _NameID                  NameID;
typedef struct _Album_Cover             Album_Cover;
typedef struct _Album                   Album;
typedef struct _NameID                  Artist;
typedef struct _NameID                  Genre;

extern int _log_domain;

#define CRITICAL(...) EINA_LOG_DOM_CRIT(_log_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_log_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_log_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_log_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_log_domain, __VA_ARGS__)

struct _App
{
   Eina_List   *add_dirs;
   Eina_List   *del_dirs;
   char         configdir[PATH_MAX];
   Evas_Object *win;
};

Evas_Object *win_new(App *app);

Evas_Object *list_add(Evas_Object *parent);
Eina_Bool    list_populate(Evas_Object *list, DB *db);
Eina_Bool    list_songs_show(Evas_Object *obj);
Eina_Bool    list_songs_exists(const Evas_Object *obj);
Song        *list_selected_get(const Evas_Object *list);
Eina_Bool    list_song_updated(Evas_Object *obj);
Eina_Bool    list_next_exists(const Evas_Object *list);
Song        *list_next_go(Evas_Object *list);
Eina_Bool    list_prev_exists(const Evas_Object *list);
Song        *list_prev_go(Evas_Object *list);
DB          *list_db_get(const Evas_Object *obj);

const char  *page_title_get(const Evas_Object *obj);
void         page_songs_exists_changed(Evas_Object *obj, Eina_Bool exists);


Evas_Object *page_root_add(Evas_Object *parent);

Song        *page_songs_selected_get(const Evas_Object *obj);
Eina_Bool    page_songs_song_updated(Evas_Object *obj);
Eina_Bool    page_songs_next_exists(const Evas_Object *obj);
Song        *page_songs_next_go(Evas_Object *obj);
Eina_Bool    page_songs_prev_exists(const Evas_Object *obj);
Song        *page_songs_prev_go(Evas_Object *obj);

Evas_Object *cover_album_fetch(Evas_Object *parent, DB *db, Album *album, unsigned short size);


DB        *db_open(const char *path);
Eina_Bool  db_close(DB *db);

struct _Song
{
   const char *path;
   const char *title;
   const char *album;
   const char *artist;
   const char *genre;
   int64_t id;
   int64_t album_id;
   int64_t artist_id;
   int64_t genre_id;
   int size; /* file size in bytes */
   int trackno;
   int rating;
   int playcnt;
   int length;
   struct {
      unsigned int path;
      unsigned int title;
      unsigned int album;
      unsigned int artist;
      unsigned int genre;
   } len; /* strlen of string fields */
   struct { /* not from db, for runtime use */
      Eina_Bool fetched_album:1;
      Eina_Bool fetched_artist:1;
      Eina_Bool fetched_genre:1;
   } flags;
};

struct _Album_Cover
{
   EINA_INLIST;
   unsigned short w;
   unsigned short h;
   unsigned short path_len;
   char path[];
};

struct _Album
{
   int64_t id;
   int64_t artist_id;
   const char *name;
   const char *artist;
   Eina_Inlist *covers;
   struct {
      unsigned int name;
      unsigned int artist;
   } len; /* strlen of string fields */
   struct { /* not from db, for runtime use */
      Eina_Bool fetched_artist:1;
      Eina_Bool fetched_covers:1;
   } flags;
};

struct _NameID
{
   int64_t id;
   const char *name;
   unsigned int len;
};

Eina_Iterator *db_songs_get(DB *db); /* walks over 'const Song*'  */
Song          *db_song_copy(const Song *orig);
void           db_song_free(Song *song);
Eina_Bool      db_song_rating_set(DB *db, Song *song, int rating);
Eina_Bool      db_song_length_set(DB *db, Song *song, int length);
Eina_Bool      db_song_album_fetch(DB *db, Song *song);
Eina_Bool      db_song_artist_fetch(DB *db, Song *song);
Eina_Bool      db_song_genre_fetch(DB *db, Song *song);

/* walks over 'const Song*'  */
Eina_Iterator *db_album_songs_get(DB *db, int64_t album_id);
Eina_Iterator *db_artist_songs_get(DB *db, int64_t artist_id);
Eina_Iterator *db_genre_songs_get(DB *db, int64_t genre_id);

/* walks over 'const Album*' */
Eina_Iterator *db_artist_albums_get(DB *db, int64_t artist_id);
Eina_Iterator *db_genre_albums_get(DB *db, int64_t genre_id);


Eina_Iterator *db_albums_get(DB *db); /* walks over 'const Album*' */
Eina_Iterator *db_artists_get(DB *db); /* walks over 'const Artist*' */
Eina_Iterator *db_genres_get(DB *db); /* walks over 'const Genre*' */


Album         *db_album_copy(const Album *album);
Eina_Bool      db_album_artist_fetch(DB *db, Album *album);
Eina_Bool      db_album_covers_fetch(DB *db, Album *album);
Eina_Bool      db_album_covers_update(DB *db, const Album *album);
void           db_album_free(Album *album);

NameID        *db_nameid_copy(const NameID *nameid);
#define        db_artist_copy(v) db_nameid_copy(v)
#define        db_genre_copy(v)  db_nameid_copy(v)

void           db_nameid_free(NameID *nameid);
#define        db_artist_free(v) db_nameid_free(v)
#define        db_genre_free(v)  db_nameid_free(v)

#endif
