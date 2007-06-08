/* vim: set sw=4 ts=4 sts=4 expandtab: */
#include <Eet.h>
#include <Ecore_Data.h>
#include <Evas.h>
#include <Ecore_File.h>
#include "points.h"
#include "eli_hiscore.h"
#include <stdio.h>
#include <string.h>

/* avoid warings */
typedef void * (*list_next)    (void *);
typedef void * (*list_append)  (void *, void *);
typedef void * (*list_data)    (void *);
typedef void * (*list_free)    (void *);
typedef void   (*hash_foreach) (void *,
                               int (*func) (void *, const char *, void *, void *), 
                               void *);
typedef void * (*hash_add)     (void *, const char *, void *);
typedef void   (*hash_free)    (void *);

typedef struct _Eli_Highscore {
    Evas_List * entries;
} Eli_Highscore;

/* globals */
static Eet_Data_Descriptor * edd_hiscore;
static Eet_Data_Descriptor * edd_entry;
static char * eet_file_name;

static Ecore_Tree * hiscore_tree = NULL;

/* internals declaration */
static void _eli_highscore_list_free(Evas_List * list);
static void _eli_highscore_write(const char * game);
static int _eli_highscore_list_sort_bad  (void * ent1, void * ent2);
static int _eli_highscore_list_sort_good (void * ent1, void * ent2);
static Eli_Highscore_Entry * _eli_entry_new(const char * username, float points, pointsType type);
static float _min(float v1, float v2);
static float _max(float v1, float v2);

/* ***************************************************************************
 *     Externals
 * ***************************************************************************/

void eli_highscore_init(const char * app)
{
    char buffer[1024];
    char * home;
    Eet_File * ef;
    eet_init();

    edd_entry = eet_data_descriptor_new("Eli_Highscore_Entry",
                                        sizeof(Eli_Highscore_Entry),
                                        (list_next) evas_list_next,
                                        (list_append) evas_list_append,
                                        (list_data) evas_list_data,
                                        (list_free) evas_list_free,
                                        (hash_foreach) evas_hash_foreach,
                                        (hash_add) evas_hash_add,
                                        (hash_free) evas_hash_free);

    EET_DATA_DESCRIPTOR_ADD_BASIC(edd_entry, Eli_Highscore_Entry,
                                  "username", username, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd_entry, Eli_Highscore_Entry,
                                  "points", points, EET_T_FLOAT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd_entry, Eli_Highscore_Entry,
                                  "type", type, EET_T_INT);

    edd_hiscore = eet_data_descriptor_new("Eli_Highscore",
                                          sizeof(Eli_Highscore),
                                          (list_next) evas_list_next,
                                          (list_append) evas_list_append,
                                          (list_data) evas_list_data,
                                          (list_free) evas_list_free,
                                          (hash_foreach) evas_hash_foreach,
                                          (hash_add) evas_hash_add,
                                          (hash_free) evas_hash_free);

    EET_DATA_DESCRIPTOR_ADD_LIST(edd_hiscore, Eli_Highscore,
                                 "entries", entries, edd_entry);

    /* this is just a temporally hack, the right path should be
     * some thing like /var/games/elitaire.score, but
     * for some reasons eet segv when the directory is not
     * writable, although the file is */
    home = getenv("HOME");
    if (!home)
	    home = "/tmp";

    snprintf(buffer, sizeof(buffer), 
		    "/%s/.e/apps/%s/score.eet", home, app);
    eet_file_name = strdup(buffer);

    /*
     * setup the hiscore hash
     */
    hiscore_tree = ecore_tree_new(ecore_str_compare);
    ecore_tree_set_free_key(hiscore_tree, free);

    /*
     * fill the hash
     */
    ef = eet_open(eet_file_name, EET_FILE_MODE_READ);

    if (ef) {
        char **list;
	int num, i;

	list = eet_list(ef, "*", &num);
	
	for(i = 0; i < num; i++) {
	    Eli_Highscore * hiscore;

	    hiscore = eet_data_read(ef, edd_hiscore, list[i]);
	    ecore_tree_set(hiscore_tree, strdup(list[i]), hiscore->entries);
	}
	free(list);
	eet_close(ef);
    }
}

void eli_highscore_shutdown(void)
{
    /* free the data */
    ecore_tree_set_free_value(hiscore_tree, 
		                      ECORE_FREE_CB(_eli_highscore_list_free));
    ecore_tree_destroy(hiscore_tree);
    eet_data_descriptor_free(edd_hiscore);
    eet_data_descriptor_free(edd_entry);
    free(eet_file_name);

    /* and the set the pointer to null */
    hiscore_tree = NULL;
    edd_hiscore = NULL;
    edd_entry = NULL;
    eet_file_name = NULL;

    eet_shutdown();
}

Evas_Bool eli_highscore_entry_add(const char * game, const char * username,
                                  float points, pointsType type)
{
    Evas_List * l = NULL;
    Eli_Highscore_Entry * entry;
    
    int (*list_sort) (void *, void *);
    int count = 0;

    if (!game || !username || !eet_file_name) return 0;

    printf("e_add: %s %s %f\n", game, username, points);
    
    entry = _eli_entry_new(username, points, type);

    l = eli_highscore_get(game);
    if (l) count = evas_list_count(l);
    else count = 0;

    /* select the right sorting function */
    switch (type) {
    case POINTS_TYPE_INTEGER_BAD:
    case POINTS_TYPE_FLOAT_BAD:
        list_sort = _eli_highscore_list_sort_bad;
        break;
    default:
        list_sort = _eli_highscore_list_sort_good;
        break;
    }

    /* 10 entries should be enough */
    if (count >= 10) {
        Evas_List * last_l;
        Eli_Highscore_Entry * last_e;

        last_l = evas_list_last(l);
        last_e = evas_list_data(last_l);

        if ( list_sort(last_e, entry) < 0) {
            free(entry->username);
            free(entry);
            return 0;
        }
    }

    l = evas_list_append(l, entry);
    if (count) l = evas_list_sort(l, (count + 1), list_sort);
    if (count >= 10) l = evas_list_remove_list(l, evas_list_last(l));

    ecore_tree_set(hiscore_tree, strdup(game), l);
  
    _eli_highscore_write(game);
    
    return 1;
}

Evas_Bool eli_highscore_accept(const char * game, float points,
                               pointsType type)
{
    Evas_List * l, * list;
    float m;
    float (*select) (float, float);

    m = 0.0;
    list = eli_highscore_get(game);

    if (!list) return (1 == 1);
    if (evas_list_count(list) < 10) return (1 == 1);

    /* select the right min/max function */
    switch (type) {
    case POINTS_TYPE_INTEGER_BAD:
    case POINTS_TYPE_FLOAT_BAD:
        select = _max;
        break;
    default:
        select = _min;
        break;
    }

    for (l = list; l; l = l->next) {
        Eli_Highscore_Entry * e;

        e = (Eli_Highscore_Entry *) evas_list_data(l);
        m = select(m, e->points);
    }

    return m == select(m, points);
}

Evas_List * eli_highscore_get(const char * game)
{
    if(!game || *game == '\0') return NULL;

    return ecore_tree_get(hiscore_tree, game);
}

/* ***************************************************************************
 *     Internals				             
 * ***************************************************************************/
static void _eli_highscore_list_free(Evas_List * list)
{
    while (list) {
        Eli_Highscore_Entry * entry;

        entry = (Eli_Highscore_Entry *) evas_list_data(list);

        if (entry) {
            if (entry->username) {
                free(entry->username);
                entry->username = NULL;
            }
	    free(entry);
	}

        list = evas_list_remove_list(list, list);
    }
}

static void _eli_highscore_write(const char * game)
{
    Eli_Highscore hiscore;
    Eet_File * file;
  
    hiscore.entries = eli_highscore_get(game);

    if (!hiscore.entries) return;
    if (ecore_file_exists(eet_file_name)) {
        file = eet_open(eet_file_name, EET_FILE_MODE_READ_WRITE);
	if (!file) fprintf(stderr, "Could not open file %s!\n", eet_file_name);
    }
    else {
        file = eet_open(eet_file_name, EET_FILE_MODE_WRITE);
        if (!file) fprintf(stderr, "Could not create file %s\n!", 
			eet_file_name);
    }

    if (file) {
	eet_data_write(file, edd_hiscore, game, &hiscore, 1);
	eet_close(file);
    }
}

static int _eli_highscore_list_sort_bad(void * ent1, void * ent2)
{
    Eli_Highscore_Entry * entry1, * entry2;

    entry1 = (Eli_Highscore_Entry *) ent1;
    entry2 = (Eli_Highscore_Entry *) ent2;

    return (entry1->points - entry2->points) * 1000.0;
}

static int _eli_highscore_list_sort_good(void * ent1, void * ent2)
{
    Eli_Highscore_Entry * entry1, * entry2;

    entry1 = (Eli_Highscore_Entry *) ent1;
    entry2 = (Eli_Highscore_Entry *) ent2;

    return (entry2->points - entry1->points) * 1000.0;
}

static Eli_Highscore_Entry * _eli_entry_new(const char * username, 
	                                float points, pointsType type)
{
    Eli_Highscore_Entry * entry;

    entry = (Eli_Highscore_Entry *) malloc(sizeof(Eli_Highscore_Entry));
    entry->points = points;
    entry->username = strdup(username);

    return entry;
}

static float _min(float v1, float v2)
{
    return (v1 < v2) ? v1 : v2;
}

static float _max(float v1, float v2)
{
    return (v1 > v2) ? v1 : v2;
}

