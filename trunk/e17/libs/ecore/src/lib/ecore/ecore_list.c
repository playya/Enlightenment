#include "ecore_private.h"
#include "Ecore.h"

/* Return information about the list */
static void *_ecore_list_current(Ecore_List * list);

/* Adding functions */
static int _ecore_list_insert(Ecore_List * list, Ecore_List_Node *node);
static int _ecore_list_append_0(Ecore_List * list, Ecore_List_Node *node);
static int _ecore_list_prepend_0(Ecore_List * list, Ecore_List_Node *node);

/* Remove functions */
static void *_ecore_list_remove_0(Ecore_List * list);
static void *_ecore_list_remove_first(Ecore_List * list);
static void *_ecore_list_remove_last(Ecore_List * list);

/* Basic traversal functions */
static void *_ecore_list_next(Ecore_List * list);
static void *_ecore_list_goto_last(Ecore_List * list);
static void *_ecore_list_goto_first(Ecore_List * list);
static void *_ecore_list_goto(Ecore_List * list, void *data);
static void *_ecore_list_goto_index(Ecore_List *list, int index);

/* Iterative function */
static int _ecore_list_for_each(Ecore_List *list, Ecore_For_Each function);

/* Private double linked list functions */
static void *_ecore_dlist_previous(Ecore_DList * list);
static void *_ecore_dlist_remove_first(Ecore_DList *list);
static void *_ecore_dlist_goto_index(Ecore_DList *list, int index);

/* XXX: Begin deprecated code */
void *
_ecore_list_append(void *in_list, void *in_item)
{
   Ecore_Oldlist *l, *new_l;
   Ecore_Oldlist *list, *item;
   
   list = in_list;
   item = in_item;
   new_l = item;
   new_l->next = NULL;
   if (!list) 
     {
	new_l->prev = NULL;
	new_l->last = new_l;
	return new_l;
     }
   if (list->last) l = list->last;
   else for (l = list; l; l = l->next);
   l->next = new_l;
   new_l->prev = l;
   list->last = new_l;
   return list;
}

void *
_ecore_list_prepend(void *in_list, void *in_item)
{
   Ecore_Oldlist *new_l;
   Ecore_Oldlist *list, *item;
   
   list = in_list;
   item = in_item;   
   new_l = item;
   new_l->prev = NULL;
   if (!list) 
     {
	new_l->next = NULL;
	new_l->last = new_l;	
	return new_l;
     }
   new_l->next = list;
   list->prev = new_l;
   new_l->last = list->last;
   list->last = NULL;
   return new_l;
}

void *
_ecore_list_append_relative(void *in_list, void *in_item, void *in_relative)
{
   Ecore_Oldlist *l;
   Ecore_Oldlist *list, *item, *relative;
   
   list = in_list;
   item = in_item;
   relative = in_relative;
   for (l = list; l; l = l->next)
     {
	if (l == relative)
	  {
	     Ecore_Oldlist *new_l;
	     
	     new_l = item;
	     if (l->next)
	       {
		  new_l->next = l->next;
		  l->next->prev = new_l;
	       }

	     else new_l->next = NULL;
	     l->next = new_l;
	     new_l->prev = l;
	     if (!new_l->next)
	       list->last = new_l;
	     return list;
	  }
     }
   return _ecore_list_append(list, item);
}

void *
_ecore_list_prepend_relative(void *in_list, void *in_item, void *in_relative)
{
   Ecore_Oldlist *l;
   Ecore_Oldlist *list, *item, *relative;
   
   list = in_list;
   item = in_item;
   relative = in_relative;
   for (l = list; l; l = l->next)
     {
	if (l == relative)
	  {
	     Ecore_Oldlist *new_l;
	     
	     new_l = item;
	     new_l->prev = l->prev;
	     new_l->next = l;
	     l->prev = new_l;
	     if (new_l->prev)
	       {
		  new_l->prev->next = new_l;
		  if (!new_l->next)
		    list->last = new_l;
		  return list;
	       }
	     else
	       {
		  if (!new_l->next)
		    new_l->last = new_l;
		  else
		    {
		       new_l->last = list->last;
		       list->last = NULL;
		    }
		  return new_l;
	       }
	  }
     }
   return _ecore_list_prepend(list, item);
}

void *
_ecore_list_remove(void *in_list, void *in_item)
{
   Ecore_Oldlist *return_l;
   Ecore_Oldlist *list, *item;

   /* checkme */
   if(!in_list)
     return in_list;

   list = in_list;
   item = in_item;
   if (!item) return list;
   if (item->next)
     item->next->prev = item->prev;
   if (item->prev)
     {
	item->prev->next = item->next;
	return_l = list;
     }
   else
     {
	return_l = item->next;
	if (return_l)
	  return_l->last = list->last;
     }
   if (item == list->last)
     list->last = item->prev;
   item->next = NULL;
   item->prev = NULL;
   return return_l;
}

void *
_ecore_list_find(void *in_list, void *in_item)
{
   Ecore_Oldlist *l;
   Ecore_Oldlist *list, *item;
   
   list = in_list;
   item = in_item;   
   for (l = list; l; l = l->next)
     {
	if (l == item) return item;
     }
   return NULL;
}
/* XXX: End deprecated code */

/**
@defgroup Ecore_Data_List_Creation_Group List Creation/Destruction Functions

Functions that create, initialize and destroy Ecore_Lists.
*/

/**
 * Create and initialize a new list.
 * @return  A new initialized list on success, @c NULL on failure.
 * @ingroup Ecore_Data_List_Creation_Group
 */
Ecore_List *ecore_list_new()
{
	Ecore_List *list;

	list = (Ecore_List *)malloc(sizeof(Ecore_List));
	if (!list)
		return NULL;

	if (!ecore_list_init(list)) {
		FREE(list);
		return NULL;
	}

	return list;
}

/**
 * Initialize a list to some sane starting values.
 * @param   list The list to initialize.
 * @return  @c TRUE if successful, @c FALSE if an error occurs.
 * @ingroup Ecore_Data_List_Creation_Group
 */
int ecore_list_init(Ecore_List *list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	memset(list, 0, sizeof(Ecore_List));

	ECORE_INIT_LOCKS(list);

	return TRUE;
}

/**
 * Free a list and all of it's nodes.
 * @param   list The list to be freed.
 * @ingroup Ecore_Data_List_Creation_Group
 */
void ecore_list_destroy(Ecore_List * list)
{
	void *data;

	CHECK_PARAM_POINTER("list", list);

	ECORE_WRITE_LOCK(list);

	while (list->first) {
		data = _ecore_list_remove_first(list);
		if (list->free_func)
			list->free_func(data);
	}

	ECORE_WRITE_UNLOCK(list);
	ECORE_DESTROY_LOCKS(list);

	FREE(list);
}

/**
 * Set the function for freeing data.
 * @param  list      The list that will use this function when nodes are
 *                   destroyed.
 * @param  free_func The function that will free the key data.
 * @return @c TRUE on successful set, @c FALSE otherwise.
 */
int ecore_list_set_free_cb(Ecore_List * list, Ecore_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	list->free_func = free_func;

	ECORE_WRITE_UNLOCK(list);

	return TRUE;
}

/**
 * Checks the list for any nodes.
 * @param  list  The list to check for nodes
 * @return @c TRUE if no nodes in list, @c FALSE if the list contains nodes
 */
int ecore_list_is_empty(Ecore_List * list)
{
	int ret = TRUE;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_READ_LOCK(list);

	if (list->nodes)
		ret = FALSE;

	ECORE_READ_UNLOCK(list);

	return ret;
}

/**
 * Returns the number of the current node.
 * @param  list The list to return the number of the current node.
 * @return The number of the current node in the list.
 */
int ecore_list_index(Ecore_List * list)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_READ_LOCK(list);

	ret = list->index;

	ECORE_READ_UNLOCK(list);

	return ret;
}

/**
 * Find the number of nodes in the list.
 * @param  list The list to find the number of nodes
 * @return The number of nodes in the list.
 */
int ecore_list_nodes(Ecore_List * list)
{
	int ret = 0;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_READ_LOCK(list);

	ret = list->nodes;

	ECORE_READ_UNLOCK(list);

	return ret;
}

/**
@defgroup Ecore_Data_List_Add_Item_Group List Item Adding Functions

Functions that are used to add nodes to an Ecore_List.
*/

/**
 * Append data to the list.
 * @param   list The list.
 * @param   data The data to append.
 * @return  @c FALSE if an error occurs, @c TRUE if appended successfully
 * @ingroup Ecore_Data_List_Add_Item_Group
 */
inline int ecore_list_append(Ecore_List * list, void *data)
{
	int ret;
	Ecore_List_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	node = ecore_list_node_new();
	node->data = data;

	ECORE_WRITE_LOCK(list);

	ret = _ecore_list_append_0(list, node);

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* For adding items to the end of the list */
static int _ecore_list_append_0(Ecore_List * list, Ecore_List_Node *end)
{
	if (list->last) {
		ECORE_WRITE_LOCK(list->last);
		list->last->next = end;
		ECORE_WRITE_UNLOCK(list->last);
	}

	list->last = end;

	if (list->first == NULL) {
		list->first = end;
		list->index = 1;
	}

	list->nodes++;

	return TRUE;
}

/**
 * Prepend data to the beginning of the list.
 * @param  list The list.
 * @param  data The data to prepend.
 * @return @c FALSE if an error occurs, @c TRUE if prepended successfully.
 * @ingroup Ecore_Data_List_Add_Item_Group
 */
inline int ecore_list_prepend(Ecore_List * list, void *data)
{
	int ret;
	Ecore_List_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	node = ecore_list_node_new();
	node->data = data;

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_prepend_0(list, node);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* For adding items to the beginning of the list */
static int _ecore_list_prepend_0(Ecore_List * list, Ecore_List_Node *start)
{
	/* Put it at the beginning of the list */
	ECORE_WRITE_LOCK(start);
	start->next = list->first;
	ECORE_WRITE_UNLOCK(start);

	list->first = start;

	/* If no last node, then the first node is the last node */
	if (list->last == NULL)
		list->last = list->first;

	list->nodes++;
	list->index++;

	return TRUE;
}

/**
 * Insert data in front of the current point in the list.
 * @param   list The list to hold the inserted @p data.
 * @param   data The data to insert into @p list.
 * @return  @c FALSE if there is an error, @c TRUE on success
 * @ingroup Ecore_Data_List_Add_Item_Group
 */
inline int ecore_list_insert(Ecore_List * list, void *data)
{
	int ret;
	Ecore_List_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	node = ecore_list_node_new();
	node->data = data;

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_insert(list, node);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* For adding items in front of the current position in the list */
static int _ecore_list_insert(Ecore_List * list, Ecore_List_Node *new_node)
{
	/*
	 * If the current point is at the beginning of the list, then it's the
	 * same as prepending it to the list.
	 */
	if (list->current == list->first)
		return _ecore_list_prepend_0(list, new_node);

	if (list->current == NULL) {
		int ret_value;

		ret_value = _ecore_list_append_0(list, new_node);
		list->current = list->last;

		return ret_value;
	}

	/* Setup the fields of the new node */
	ECORE_WRITE_LOCK(new_node);
	new_node->next = list->current;
	ECORE_WRITE_UNLOCK(new_node);

	/* And hook the node into the list */
	_ecore_list_goto_index(list, ecore_list_index(list) - 1);

	ECORE_WRITE_LOCK(list->current);
	list->current->next = new_node;
	ECORE_WRITE_UNLOCK(list->current);

	/* Now move the current item to the inserted item */
	list->current = new_node;
	list->index++;
	list->nodes++;

	return TRUE;
}

/**
@defgroup Ecore_Data_List_Remove_Item_Group List Item Removing Functions

Functions that remove nodes from an Ecore_List.
*/

/**
 * Remove the current item from the list.
 * @param   list The list to remove the current item
 * @return  A pointer to the removed data on success, @c NULL on failure.
 * @ingroup Ecore_Data_List_Remove_Item_Group
 */
inline void *ecore_list_remove(Ecore_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_remove_0(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Remove the current item from the list */
static void *_ecore_list_remove_0(Ecore_List * list)
{
	void *ret = NULL;
	Ecore_List_Node *old;

	if (!list)
		return FALSE;

	if (ecore_list_is_empty(list))
		return FALSE;

	if (!list->current)
		return FALSE;

	if (list->current == list->first)
		return _ecore_list_remove_first(list);

	if (list->current == list->last)
		return _ecore_list_remove_last(list);

	old = list->current;

	_ecore_list_goto_index(list, list->index - 1);

	ECORE_WRITE_LOCK(list->current);
	ECORE_WRITE_LOCK(old);

	list->current->next = old->next;
	old->next = NULL;
	ret = old->data;
	old->data = NULL;

	_ecore_list_next(list);

	ECORE_WRITE_UNLOCK(old);
	ECORE_WRITE_UNLOCK(list->current);

	ecore_list_node_destroy(old, NULL);
	list->nodes--;

	return ret;
}

/**
 * Remove and free the data in lists current position.
 * @param   list The list to remove and free the current item.
 * @return  @c TRUE on success, @c FALSE on error
 * @ingroup Ecore_Data_List_Remove_Item_Group
 */
int ecore_list_remove_destroy(Ecore_List *list)
{
	void *data;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	data = _ecore_list_remove_0(list);
	if (list->free_func)
		list->free_func(data);

	ECORE_WRITE_UNLOCK(list);

	return TRUE;
}

/**
 * Remove the first item from the list.
 * @param   list The list to remove the current item
 * @return  Returns a pointer to the removed data on success, @c NULL on
 *          failure.
 * @ingroup Ecore_Data_List_Remove_Item_Group
 */
inline void *ecore_list_remove_first(Ecore_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_remove_first(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Remove the first item from the list */
static void *_ecore_list_remove_first(Ecore_List * list)
{
	void *ret = NULL;
	Ecore_List_Node *old;

	if (!list)
		return FALSE;

	ECORE_WRITE_UNLOCK(list);
	if (ecore_list_is_empty(list))
		return FALSE;
	ECORE_WRITE_LOCK(list);

	if (!list->first)
		return FALSE;

	old = list->first;

	list->first = list->first->next;

	if (list->current == old)
		list->current = list->first;
	else
		list->index--;

	if (list->last == old)
		list->last = list->first;

	ECORE_WRITE_LOCK(old);
	ret = old->data;
	old->data = NULL;
	ECORE_WRITE_UNLOCK(old);

	ecore_list_node_destroy(old, NULL);
	list->nodes--;

	return ret;
}

/**
 * Remove the last item from the list.
 * @param   list The list to remove the last node from
 * @return  A pointer to the removed data on success, @c NULL on failure.
 * @ingroup Ecore_Data_List_Remove_Item_Group
 */
inline void *ecore_list_remove_last(Ecore_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_remove_last(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Remove the last item from the list */
static void *_ecore_list_remove_last(Ecore_List * list)
{
	void *ret = NULL;
	Ecore_List_Node *old, *prev;

	if (!list)
		return FALSE;

	if (ecore_list_is_empty(list))
		return FALSE;

	if (!list->last)
		return FALSE;

	old = list->last;
	if (list->current == old)
		list->current = NULL;

	if (list->first == old)
		list->first = NULL;
	for (prev = list->first; prev && prev->next != old; prev = prev->next);
	if (prev) {
		prev->next = NULL;
		list->last = prev;
		if (list->current == old) {
			list->current = NULL;
		}
	}


	ECORE_WRITE_LOCK(old);
	if (old) {
		old->next = NULL;
		ret = old->data;
		old->data = NULL;
	}
	ECORE_WRITE_UNLOCK(old);

	ecore_list_node_destroy(old, NULL);
	list->nodes--;

	return ret;
}

/**
@defgroup Ecore_Data_List_Traverse_Group List Traversal Functions

Functions that can be used to traverse an Ecore_List.
*/

/**
 * Make the current item the item with the given index number.
 * @param   list  The list.
 * @param   index The position to move the current item.
 * @return  A pointer to new current item on success, @c NULL on failure.
 * @ingroup Ecore_Data_List_Traverse_Group
 */
inline void *ecore_list_goto_index(Ecore_List * list, int index)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto_index(list, index);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* This is the non-threadsafe version, use this inside internal functions that
 * already lock the list */
static void *_ecore_list_goto_index(Ecore_List *list, int index)
{
	int i;

	if (!list)
		return FALSE;

	if (ecore_list_is_empty(list))
		return FALSE;

	if (index > ecore_list_nodes(list) || index < 0)
		return FALSE;

	_ecore_list_goto_first(list);

	for (i = 1; i < index && _ecore_list_next(list); i++);

	list->index = i;

	return list->current->data;
}

/**
 * Make the current item the node that contains @p data.
 * @param   list The list.
 * @param   data The data to find.
 * @return  A pointer to @p data on success, @c NULL on failure.
 * @ingroup Ecore_Data_List_Traverse_Group
 */
inline void *ecore_list_goto(Ecore_List * list, void *data)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto(list, data);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Set the current position to the node containing data */
static void *_ecore_list_goto(Ecore_List * list, void *data)
{
	int index;
	Ecore_List_Node *node;

	if (!list)
		return NULL;

	index = 1;

	node = list->first;
	ECORE_READ_LOCK(node);
	while (node && node->data) {
		Ecore_List_Node *next;

		if (node->data == data)
			break;

		next = node->next;
		ECORE_READ_UNLOCK(node);

		node = next;

		ECORE_READ_LOCK(node);
		index++;
	}

	ECORE_READ_UNLOCK(node);
	if (!node)
		return NULL;

	list->current = node;
	list->index = index;

	return list->current->data;
}

/**
 * Make the current item the first item in the list
 * @param   list The list.
 * @return  A pointer to the first item on success, @c NULL on failure
 * @ingroup Ecore_Data_List_Traverse_Group
 */
inline void *ecore_list_goto_first(Ecore_List *list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	ret = _ecore_list_goto_first(list);

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Set the current position to the start of the list */
static void *_ecore_list_goto_first(Ecore_List * list)
{
	if (!list || !list->first)
		return NULL;

	list->current = list->first;
	list->index = 1;

	return list->current->data;
}

/**
 * Make the current item the last item in the list.
 * @param   list The list.
 * @return  A pointer to the last item on success, @c NULL on failure.
 * @ingroup Ecore_Data_List_Traverse_Group
 */
inline void *ecore_list_goto_last(Ecore_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto_last(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Set the current position to the end of the list */
static void *_ecore_list_goto_last(Ecore_List * list)
{
	if (!list || !list->last)
		return NULL;

	list->current = list->last;
	list->index = list->nodes;

	return list->current->data;
}

/**
 * Retrieve the data pointed to by the current item in @p list.
 * @param  list The list.
 * @return Returns the data at current position, can be @c NULL.
 */
inline void *ecore_list_current(Ecore_List * list)
{
	void *ret;

	ECORE_READ_LOCK(list);
	ret = _ecore_list_current(list);
	ECORE_READ_UNLOCK(list);

	return ret;
}

/* Return the data of the current node without incrementing */
static void *_ecore_list_current(Ecore_List * list)
{
	void *ret;

	if (!list->current)
		return NULL;

	ECORE_READ_LOCK(list->current);
	ret = list->current->data;
	ECORE_READ_UNLOCK(list->current);

	return ret;
}

/**
 * Retrieve the data pointed to by the current item, and make the next item
 * the current item.
 * @param   list The list to retrieve data from.
 * @return  The current item in the list on success, @c NULL on failure.
 */
inline void *ecore_list_next(Ecore_List * list)
{
	void *data;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	data = _ecore_list_next(list);
	ECORE_WRITE_UNLOCK(list);

	return data;
}

/* Return the data contained in the current node and go to the next node */
static void *_ecore_list_next(Ecore_List * list)
{
	void *data;
	Ecore_List_Node *ret;
	Ecore_List_Node *next;

	if (!list->current)
		return NULL;

	ECORE_READ_LOCK(list->current);
	ret = list->current;
	next = list->current->next;
	ECORE_READ_UNLOCK(list->current);

	list->current = next;
	list->index++;

	ECORE_READ_LOCK(ret);
	data = ret->data;
	ECORE_READ_UNLOCK(ret);

	return data;
}

/**
 * Remove all nodes from @p list.
 * @param  list The list.
 * @return Returns @c TRUE on success, @c FALSE on error.
 * @note The data for each item on the list is not freed by
 *       @c ecore_list_clear().
 */
int ecore_list_clear(Ecore_List * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	while (!ecore_list_is_empty(list))
		_ecore_list_remove_first(list);

	ECORE_WRITE_UNLOCK(list);

	return TRUE;
}

/**
 * Execute function for each node in @p list.
 * @param   list     The list.
 * @param   function The function to pass each node from @p list to.
 * @return  Returns @c TRUE on success, @c FALSE on failure.
 * @ingroup Ecore_Data_List_Traverse_Group
 */
int ecore_list_for_each(Ecore_List *list, Ecore_For_Each function)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_READ_LOCK(list);
	ret = _ecore_list_for_each(list, function);
	ECORE_READ_UNLOCK(list);

	return ret;
}

/* The real meat of executing the function for each data node */
static int _ecore_list_for_each(Ecore_List *list, Ecore_For_Each function)
{
	void *value;

	if (!list || !function)
		return FALSE;

	_ecore_list_goto_first(list);
	while ((value = _ecore_list_next(list)) != NULL)
		function(value);

	return TRUE;
}

/* Initialize a node to starting values */
int ecore_list_node_init(Ecore_List_Node * node)
{

	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	node->next = NULL;
	node->data = NULL;

	ECORE_INIT_LOCKS(node);

	return TRUE;
}

/**
@defgroup Ecore_Data_List_Node_Group List Node Functions

Functions that are used in the creation, maintenance and destruction of
Ecore_List nodes.
*/

/**
 * Allocates and initializes a new list node.
 * @return  A new Ecore_List_Node on success, @c NULL otherwise.
 * @ingroup Ecore_Data_List_Node_Group
 */
Ecore_List_Node *ecore_list_node_new()
{
	Ecore_List_Node *new_node;

	new_node = malloc(sizeof(Ecore_List_Node));

	if (!ecore_list_node_init(new_node)) {
		FREE(new_node);
		return NULL;
	}

	return new_node;
}

/**
 * Calls the function to free the data and the node.
 * @param   node      Node to destroy.
 * @param   free_func Function to call if @p node points to data to free.
 * @return  @c TRUE.
 * @ingroup Ecore_Data_List_Node_Group
 */
int ecore_list_node_destroy(Ecore_List_Node * node, Ecore_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	ECORE_WRITE_LOCK(node);

	if (free_func && node->data)
		free_func(node->data);

	ECORE_WRITE_UNLOCK(node);
	ECORE_DESTROY_LOCKS(node);

	FREE(node);

	return TRUE;
}

/**
 * @defgroup Ecore_Data_DList_Creation_Group Doubly Linked List Creation/Destruction Functions
 *
 * Functions used to create, initialize and destroy @c Ecore_DLists.
 */

/**
 * Creates and initialises a new doubly linked list.
 * @return  A new initialised doubly linked list on success, @c NULL
 *          on failure.
 * @ingroup Ecore_Data_DList_Creation_Group
 */
Ecore_DList *ecore_dlist_new()
{
	Ecore_DList *list = NULL;

	list = (Ecore_DList *)malloc(sizeof(Ecore_DList));
	if (!list)
		return NULL;

	if (!ecore_dlist_init(list)) {
		IF_FREE(list);
		return NULL;
	}

	return list;
}

/**
 * Initialises a list to some sane starting values.
 * @param   list The doubly linked list to initialise.
 * @return  @c TRUE if successful, @c FALSE if an error occurs.
 * @ingroup Ecore_Data_DList_Creation_Group
 */
int ecore_dlist_init(Ecore_DList *list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	memset(list, 0, sizeof(Ecore_DList));

	ECORE_INIT_LOCKS(list);

	return TRUE;
}

/**
 * Frees a doubly linked list and all of its nodes.
 * @param   list The doubly linked list to be freed.
 * @ingroup Ecore_Data_DList_Creation_Group
 */
void ecore_dlist_destroy(Ecore_DList * list)
{
	void *data;
	CHECK_PARAM_POINTER("list", list);

	ECORE_WRITE_LOCK(list);

	while (list->first) {
		data = _ecore_dlist_remove_first(list);
		if (list->free_func)
			list->free_func(data);
	}

	ECORE_WRITE_UNLOCK(list);
	ECORE_DESTROY_LOCKS(list);

	FREE(list);
}

/**
 * Sets the function used for freeing data stored in a doubly linked list.
 * @param   list      The doubly linked list that will use this function when
 *                    nodes are destroyed.
 * @param   free_func The function that will free the key data
 * @return  @c TRUE on success, @c FALSE on failure.
 * @ingroup Ecore_Data_DList_Creation_Group
 */
int ecore_dlist_set_free_cb(Ecore_DList * list, Ecore_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ecore_list_set_free_cb(ECORE_LIST(list), free_func);
}

/**
 * Returns whether there is anything in the given doubly linked list.
 * @param  list The given doubly linked list.
 * @return @c TRUE if there are nodes, @c FALSE otherwise.
 */
int ecore_dlist_is_empty(Ecore_DList * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ecore_list_is_empty(ECORE_LIST(list));
}

/**
 * Retrieves the index of the current node of the given doubly linked list.
 * @param  list The given doubly linked list.
 * @return The index of the current node.
 */
inline int ecore_dlist_index(Ecore_DList * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ecore_list_index(ECORE_LIST(list));
}

/**
 * @defgroup Ecore_Data_DList_Add_Item_Group Doubly Linked List Adding Functions
 *
 * Functions that are used to add nodes to an Ecore_DList.
 */

/**
 * Appends data to the given doubly linked list.
 * @param   list The given doubly linked list.
 * @param   data The data to append.
 * @return  @c TRUE if the data is successfully appended, @c FALSE otherwise.
 * @ingroup Ecore_Data_DList_Add_Item_Group
 */
int ecore_dlist_append(Ecore_DList * list, void *data)
{
	int ret;
	Ecore_DList_Node *prev;
	Ecore_DList_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	node = ecore_dlist_node_new();
	ECORE_LIST_NODE(node)->data = data;

	prev = ECORE_DLIST_NODE(ECORE_LIST(list)->last);
	ret = _ecore_list_append_0(ECORE_LIST(list), ECORE_LIST_NODE(node));
	if (ret) {
		node->previous = prev;
	}

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * Adds data to the very beginning of the given doubly linked list.
 * @param   list The given doubly linked list.
 * @param   data The data to prepend.
 * @return  @c TRUE if the data is successfully prepended, @c FALSE otherwise.
 * @ingroup Ecore_Data_DList_Add_Item_Group
 */
int ecore_dlist_prepend(Ecore_DList * list, void *data)
{
	int ret;
	Ecore_DList_Node *prev;
	Ecore_DList_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	node = ecore_dlist_node_new();
	ECORE_LIST_NODE(node)->data = data;

	prev = ECORE_DLIST_NODE(ECORE_LIST(list)->first);
	ret = _ecore_list_prepend_0(ECORE_LIST(list), ECORE_LIST_NODE(node));
	if (ret && prev)
		prev->previous = node;

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * Inserts data at the current point in the given doubly linked list.
 * @param   list The given doubly linked list.
 * @param   data The data to be inserted.
 * @return  @c TRUE on success, @c FALSE otherwise.
 * @ingroup Ecore_Data_DList_Add_Item_Group
 */
int ecore_dlist_insert(Ecore_DList * list, void *data)
{
	int ret;
	Ecore_DList_Node *prev;
	Ecore_DList_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	prev = ECORE_DLIST_NODE(ECORE_LIST(list)->current);
	if (!prev)
		prev = ECORE_DLIST_NODE(ECORE_LIST(list)->last);

	if (prev)
		prev = prev->previous;

	node = ecore_dlist_node_new();
	ECORE_LIST_NODE(node)->data = data;

	ret = _ecore_list_insert(list, ECORE_LIST_NODE(node));
	if (!ret) {
		ECORE_WRITE_UNLOCK(list);
		return ret;
	}

	if (ECORE_LIST_NODE(node)->next)
		ECORE_DLIST_NODE(ECORE_LIST_NODE(node)->next)->previous = node;

	if (prev)
		node->previous = prev;

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @defgroup Ecore_Data_DList_Remove_Item_Group Doubly Linked List Removing Functions
 * 
 * Functions that remove nodes from an @c Ecore_DList.
 */

/**
 * Removes the current item from the given doubly linked list.
 * @param   list The given doubly linked list.
 * @return  A pointer to the removed data on success, @c NULL otherwise.
 * @ingroup Ecore_Data_DList_Remove_Item_Group
 */
void *ecore_dlist_remove(Ecore_DList * list)
{
	void *ret;
	Ecore_List *l2 = ECORE_LIST(list);
	Ecore_DList_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	if (l2->current) {
		node = ECORE_DLIST_NODE(list->current->next);
		if (node)
			node->previous = ECORE_DLIST_NODE(l2->current)->previous;
	}
	ret = _ecore_list_remove_0(list);

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * Removes the first item from the given doubly linked list.
 * @param   list The given doubly linked list.
 * @return  A pointer to the removed data on success, @c NULL on failure.
 * @ingroup Ecore_Data_DList_Remove_Item_Group
 */
void *ecore_dlist_remove_first(Ecore_DList * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_dlist_remove_first(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * Removes and frees the data at the current position in the given doubly
 * linked list.
 * @param   list The given doubly linked list.
 * @return  @c TRUE on success, @c FALSE otherwise.
 * @ingroup Ecore_Data_DList_Remove_Item_Group
 */
int ecore_dlist_remove_destroy(Ecore_DList *list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ecore_list_remove_destroy(list);
}

static void *_ecore_dlist_remove_first(Ecore_DList *list)
{
	void *ret;

	if (!list)
		return FALSE;

	ret = _ecore_list_remove_first(list);
	if (ret && ECORE_LIST(list)->first)
		ECORE_DLIST_NODE(ECORE_LIST(list)->first)->previous = NULL;

	return ret;
}

/**
 * Removes the last item from the given doubly linked list.
 * @param   list The given doubly linked list.
 * @return  A pointer to the removed data on success, @c NULL otherwise.
 * @ingroup Ecore_Data_DList_Remove_Item_Group
 */
void *ecore_dlist_remove_last(Ecore_DList * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_remove_last(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * Moves the current item to the index number in the given doubly linked list.
 * @param  list  The given doubly linked list.
 * @param  index The position to move the current item
 * @return The node at specified index on success, @c NULL on error.
 */
void *ecore_dlist_goto_index(Ecore_DList * list, int index)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_dlist_goto_index(list, index);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* This is the non-threadsafe version, use this inside internal functions that
 * already lock the list */
static void *_ecore_dlist_goto_index(Ecore_DList *list, int index)
{
	int i, increment;

	if (!list)
		return FALSE;

	if (ecore_list_is_empty(ECORE_LIST(list)))
		return FALSE;

	if (index > ecore_list_nodes(ECORE_LIST(list)) || index < 1)
		return FALSE;

	if (ECORE_LIST(list)->index > ECORE_LIST(list)->nodes)
		_ecore_list_goto_last(ECORE_LIST(list));

	if (index < ECORE_LIST(list)->index)
		increment = -1;
	else
		increment = 1;

	for (i = ECORE_LIST(list)->index; i != index; i += increment) {
		if (increment > 0)
			_ecore_list_next(list);
		else
			_ecore_dlist_previous(list);
	}

	return _ecore_list_current(list);
}

/**
 * @brief Move the current item to the node that contains data
 * @param list: the list to move the current item in
 * @param data: the data to find and set the current item to
 *
 * @return Returns specified data on success, NULL on error
 */
void *ecore_dlist_goto(Ecore_DList * list, void *data)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto(ECORE_LIST(list), data);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Move the current pointer to the first item in the list
 * @param list: the list to change the current to the first item
 *
 * @return Returns a pointer to the first item on success, NULL on failure.
 */
void *ecore_dlist_goto_first(Ecore_DList *list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto_first(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Move the pointer to the current item to the last item
 * @param list: the list to move the current item pointer to the last
 * @return Returns a pointer to the last item in the list , NULL if empty.
 */
void *ecore_dlist_goto_last(Ecore_DList * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto_last(ECORE_LIST(list));
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Return the data in the current list item
 * @param list: the list to the return the current data
 * @return Returns value of the current data item, NULL if no current item
 */
void *ecore_dlist_current(Ecore_DList * list)
{
	void *ret;

	ECORE_READ_LOCK(list);
	ret = _ecore_list_current(ECORE_LIST(list));
	ECORE_READ_UNLOCK(list);

	return ret;
}

/**
 * @brief Move to the next item in the list and return current item
 * @param list: the list to move to the next item in.
 * @return Returns data in the current list node, or NULL on error
 */
void *ecore_dlist_next(Ecore_DList * list)
{
	void *data;

	ECORE_WRITE_LOCK(list);
	data = _ecore_list_next(list);
	ECORE_WRITE_UNLOCK(list);

	return data;
}

/**
 * @brief Move to the previous item and return current item
 * @param list: the list to move to the previous item in.
 * @return Returns data in the current list node, or NULL on error
 */
void *ecore_dlist_previous(Ecore_DList * list)
{
	void *data;

	ECORE_WRITE_LOCK(list);
	data = _ecore_dlist_previous(list);
	ECORE_WRITE_UNLOCK(list);

	return data;
}

static void *_ecore_dlist_previous(Ecore_DList * list)
{
	void *data = NULL;

	if (!list)
		return NULL;

	if (ECORE_LIST(list)->current) {
		data = ECORE_LIST(list)->current->data;
		ECORE_LIST(list)->current = ECORE_LIST_NODE(ECORE_DLIST_NODE(
				ECORE_LIST(list)->current)->previous);
		ECORE_LIST(list)->index--;
	}
	else
		_ecore_list_goto_last(ECORE_LIST(list));

	return data;
}

/**
 * @brief Remove all nodes from the list.
 * @param list: the list to remove all nodes from
 *
 * @return Returns TRUE on success, FALSE on errors
 */
int ecore_dlist_clear(Ecore_DList * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ecore_list_clear(ECORE_LIST(list));

	return TRUE;
}

/*
 * @brief Initialize a node to sane starting values
 * @param node: the node to initialize
 * @return Returns TRUE on success, FALSE on errors
 */
int ecore_dlist_node_init(Ecore_DList_Node * node)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	ret = ecore_list_node_init(ECORE_LIST_NODE(node));
	if (ret)
		node->previous = NULL;

	return ret;
}

/*
 * @brief Allocate and initialize a new list node
 * @return Returns NULL on error, new list node on success
 */
Ecore_DList_Node *ecore_dlist_node_new()
{
	Ecore_DList_Node *new_node;

	new_node = malloc(sizeof(Ecore_DList_Node));

	if (!new_node)
		return NULL;

	if (!ecore_dlist_node_init(new_node)) {
		FREE(new_node);
		return NULL;
	}

	return new_node;
}

/*
 * @brief Call the data's free callback function, then free the node
 * @param node: the node to be freed
 * @param free_func: the callback function to execute on the data
 * @return Returns TRUE on success, FALSE on error
 */
int ecore_dlist_node_destroy(Ecore_DList_Node * node, Ecore_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("node", node,
			FALSE);

	return ecore_list_node_destroy(ECORE_LIST_NODE(node), free_func);
}
