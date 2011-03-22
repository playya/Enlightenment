/**
 * Display the ensure object tree
 */



#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/signalfd.h>

#include <Eina.h>
#include <Elementary.h>

#include "ensure.h"
#include "enobj.h"
#include "entree.h"

static Eina_Bool tree_add_toplevel(const Eina_Hash *hash, const void *key, void *data,
		void *ensurev);

/* Tree window callbacks */
static char *tree_window_label_get(void *enwinv, Evas_Object *obj, const char *part);
static void tree_window_select(void *data, Evas_Object *obj ensure_unused, void *event ensure_unused);
static void tree_window_del(void *enwinv, Evas_Object *obj);


/* Item callbacks */
static char *tree_item_label_get(void *enwinv, Evas_Object *obj, const char *part);
static void tree_item_select(void *data, Evas_Object *obj ensure_unused, void *event ensure_unused);
static void tree_item_del(void *enwinv, Evas_Object *obj);

static const Elm_Genlist_Item_Class windowclass = {
	.item_style = "default",
	.func = {
		.label_get = tree_window_label_get,
		.del = tree_window_del,
	}
};

static const Elm_Genlist_Item_Class treeitemclass = {
	.item_style = "default",
	.func = {
		.label_get = tree_item_label_get,
		.del = tree_item_del,
	}

};


/**
 * Display the object tree.
 *
 */
void
view_set_tree(void *ensurev, Evas_Object *button, void *event_info){

        struct ensure *ensure = ensurev;
	struct result *cur;
	struct enwin *enwin;
	Eina_List *l;

	if (ensure->current_view == ENVIEW_OBJECT_TREE) return;
	ensure->current_view = ENVIEW_OBJECT_TREE;

	elm_hoversel_label_set(ensure->viewselect, "Object Tree");
	elm_genlist_clear(ensure->view);

	cur = ensure->cur;

	EINA_LIST_FOREACH(cur->windows, l, enwin){
		assert(enwin->genitem == NULL);
		enwin->genitem = elm_genlist_item_append(ensure->view,
				&windowclass,
				enwin,  NULL,
				ELM_GENLIST_ITEM_GROUP |
					ELM_GENLIST_ITEM_SUBITEMS,
				tree_window_select,
				enwin);
	}

	eina_hash_foreach(cur->objdb, tree_add_toplevel, ensure);

	return;
}



/* Expand an item */
void
tree_expand_item(struct ensure *ensure, Elm_Genlist_Item *item){
	struct enobj *enobj;
	struct enobj *child;
	Eina_List *l;

	enobj = elm_genlist_item_data_get(item);

	assert(enobj->magic == ENOBJMAGIC);
	printf("Tree expand item %p %p\n",enobj->children, enobj->clippees);
	EINA_LIST_FOREACH(enobj->children, l, child){
		printf("Add %p",child);
		child->genitem = elm_genlist_item_append(ensure->view,
				&treeitemclass, child,
				enobj->genitem, 0,
				tree_item_select, child);
	}

}


static Eina_Bool
tree_add_toplevel(const Eina_Hash *hash, const void *key, void *enobjv,
		void *ensurev){
	struct enobj *enobj;
	struct ensure *ensure = ensurev;
	Elm_Genlist_Item_Flags flags;

	enobj = enobjv;
	if (enobj->parent) return true;
	assert(enobj->genitem == NULL);
	assert(enobj->magic == ENOBJMAGIC);

	flags = ELM_GENLIST_ITEM_SUBITEMS;
	printf("Obj: %p Children: %p Clips: %p\n",enobj, enobj->children,
			enobj->clippees);

	enobj->genitem = elm_genlist_item_append(ensure->view, &treeitemclass,
			enobj, enobj->enwin->genitem, flags,
			tree_item_select, enobj);

	return true;
}

/**
 * Display the name of hte window
 */
static char *
tree_window_label_get(void *enwinv, Evas_Object *obj, const char *part){
	struct enwin *enwin = enwinv;
	char buf[1000];

	snprintf(buf, sizeof(buf), "%s [%" PRIxPTR " %dx%d]",
			(enwin->name && *enwin->name) ? enwin->name: "Untitled",
			enwin->id,
			enwin->w, enwin->h);
	return strdup(buf);
}

void
tree_window_select(void *data, Evas_Object *obj ensure_unused, void *event ensure_unused){


}

void
tree_window_del(void *enwinv, Evas_Object *obj){
	struct enwin *enwin = enwinv;

	enwin->genitem = NULL;
}




static char *
tree_item_label_get(void *enobjv, Evas_Object *obj, const char *part){
	char buf[200];
	struct enobj *enobj = enobjv;

	if (enobj->name){
		snprintf(buf,sizeof(buf), "%.*s (%llx - %s)",70,enobj->name,
				(long long int)enobj->id, enobj->type);
	} else {
		snprintf(buf,sizeof(buf), "%llx - %s",
				(long long int)enobj->id, enobj->type);
	}

	return strdup(buf);
}
static void tree_item_select(void *data, Evas_Object *obj ensure_unused, void *event ensure_unused){

}
static void
tree_item_del(void *enobjv, Evas_Object *obj){
	struct enobj *enobj;

	enobj = enobjv;
	enobj->genitem = NULL;
}



