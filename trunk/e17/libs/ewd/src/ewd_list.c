#include <Ewd.h>

/* Return information about the list */
static void *_ewd_list_current(Ewd_List * list);

/* Adding functions */
static int _ewd_list_insert(Ewd_List * list, Ewd_List_Node *node);
static int _ewd_list_append(Ewd_List * list, Ewd_List_Node *node);
static int _ewd_list_prepend(Ewd_List * list, Ewd_List_Node *node);

/* Remove functions */
static void *_ewd_list_remove(Ewd_List * list);
static void *_ewd_list_remove_first(Ewd_List * list);
static void *_ewd_list_remove_last(Ewd_List * list);

/* Basic traversal functions */
static void *_ewd_list_next(Ewd_List * list);
static void *_ewd_list_goto_last(Ewd_List * list);
static void *_ewd_list_goto_first(Ewd_List * list);
static void *_ewd_list_goto(Ewd_List * list, void *data);
static void *_ewd_list_goto_index(Ewd_List *list, int index);

/* Iterative function */
static int _ewd_list_for_each(Ewd_List *list, Ewd_For_Each function);

/* Private double linked list functions */
static void *_ewd_dlist_previous(Ewd_DList * list);
static void *_ewd_dlist_remove_first(Ewd_DList *list);
static void *_ewd_dlist_goto_index(Ewd_DList *list, int index);

/**
 * ewd_list_new - create and initialize a new list.
 *
 * Returns a new initialized list on success, NULL on failure.
 */
Ewd_List *ewd_list_new()
{
	Ewd_List *list;

	list = (Ewd_List *)malloc(sizeof(Ewd_List));
	if (!list)
		return NULL;

	if (!ewd_list_init(list)) {
		FREE(list);
		return NULL;
	}

	return list;
}

/**
 * ewd_list_init - initialize a list to some sane starting values.
 * @list: the list to initialize
 *
 * Returns FALSE if an error occurs, TRUE if successful
 */
int ewd_list_init(Ewd_List *list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	memset(list, 0, sizeof(Ewd_List));

	EWD_INIT_LOCKS(list);

	return TRUE;
}

/**
 * ewd_list_destroy - Free a list and all of it's nodes.
 * @list: the list to be freed
 *
 * Returns no value
 */
void ewd_list_destroy(Ewd_List * list)
{
	void *data;

	CHECK_PARAM_POINTER("list", list);

	EWD_WRITE_LOCK(list);

	while (list->first) {
		data = _ewd_list_remove_first(list);
		if (list->free_func)
			list->free_func(data);
	}

	EWD_WRITE_UNLOCK(list);
	EWD_DESTROY_LOCKS(list);

	FREE(list);
}

/**
 * ewd_list_set_free_cb - set the function for freeing data
 * @list: the list that will use this function when nodes are destroyed.
 * @free_func: the function that will free the key data
 *
 * Returns TRUE on successful set, FALSE otherwise.
 */
int ewd_list_set_free_cb(Ewd_List * list, Ewd_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);

	list->free_func = free_func;

	EWD_WRITE_UNLOCK(list);

	return TRUE;
}

/**
 * ewd_list_is_empty - checks the list for any nodes.
 * @list: the list to check for nodes
 *
 * Returns TRUE if no nodes in list, FALSE if the list contains nodes
 */
int ewd_list_is_empty(Ewd_List * list)
{
	int ret = TRUE;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_READ_LOCK(list);

	if (list->nodes)
		ret = FALSE;

	EWD_READ_UNLOCK(list);

	return ret;
}

/**
 * ewd_list_index - returns the number of the current node
 * @list: the list to return the number of the current node
 *
 * Returns the number of the current node in the list.
 */
int ewd_list_index(Ewd_List * list)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_READ_LOCK(list);

	ret = list->index;

	EWD_READ_UNLOCK(list);

	return ret;
}

/**
 * ewd_list_nodes - find the number of nodes in the list.
 * @list: the list to find the number of nodes
 *
 * Returns the number of nodes in the list.
 */
int ewd_list_nodes(Ewd_List * list)
{
	int ret = 0;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_READ_LOCK(list);

	ret = list->nodes;

	EWD_READ_UNLOCK(list);

	return ret;
}

/**
 * ewd_list_append - append data to the list.
 * @list: the list to append @data
 * @data: the data to append to @list.
 *
 * Returns FALSE if an error occurs, TRUE if @data is appended successfully
 */
inline int ewd_list_append(Ewd_List * list, void *data)
{
	int ret;
	Ewd_List_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	node = ewd_list_node_new();
	node->data = data;

	EWD_WRITE_LOCK(list);

	ret = _ewd_list_append(list, node);

	EWD_WRITE_UNLOCK(list);

	return ret;
}

/* For adding items to the end of the list */
static int _ewd_list_append(Ewd_List * list, Ewd_List_Node *end)
{
	if (list->last) {
		EWD_WRITE_LOCK(list->last);
		list->last->next = end;
		EWD_WRITE_UNLOCK(list->last);
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
 * ewd_list_prepend - prepend data to the beginning of the list
 * @list: the list to prepend @data
 * @data: the data to prepend to @list
 *
 * Returns FALSE if an error occurs, TRUE if data prepended successfully
 */
inline int ewd_list_prepend(Ewd_List * list, void *data)
{
	int ret;
	Ewd_List_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	node = ewd_list_node_new();
	node->data = data;

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_prepend(list, node);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/* For adding items to the beginning of the list */
static int _ewd_list_prepend(Ewd_List * list, Ewd_List_Node *start)
{
	/* Put it at the beginning of the list */
	EWD_WRITE_LOCK(start);
	start->next = list->first;
	EWD_WRITE_UNLOCK(start);

	list->first = start;

	/* If no last node, then the first node is the last node */
	if (list->last == NULL)
		list->last = list->first;

	list->nodes++;
	list->index++;

	return TRUE;
}

/**
 * ewd_list_insert - insert data at the current point in the list
 * @list: the list to hold the inserted @data
 * @data: the data to insert into @list
 *
 * Returns FALSE on an error, TRUE on success
 */
inline int ewd_list_insert(Ewd_List * list, void *data)
{
	int ret;
	Ewd_List_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	node = ewd_list_node_new();
	node->data = data;

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_insert(list, node);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/* For adding items in front of the current position in the list */
static int _ewd_list_insert(Ewd_List * list, Ewd_List_Node *new_node)
{
	/*
	 * If the current point is at the beginning of the list, then it's the
	 * same as prepending it to the list.
	 */
	if (list->current == list->first)
		return _ewd_list_prepend(list, new_node);

	if (list->current == NULL) {
		int ret_value;

		ret_value = _ewd_list_append(list, new_node);
		list->current = list->last;

		return ret_value;
	}

	/* Setup the fields of the new node */
	EWD_WRITE_LOCK(new_node);
	new_node->next = list->current;
	EWD_WRITE_UNLOCK(new_node);

	/* And hook the node into the list */
	_ewd_list_goto_index(list, ewd_list_index(list) - 1);

	EWD_WRITE_LOCK(list->current);
	list->current->next = new_node;
	EWD_WRITE_UNLOCK(list->current);

	/* Now move the current item to the inserted item */
	list->current = new_node;
	list->index++;
	list->nodes++;

	return TRUE;
}

/**
 * ewd_list_remove - remove the current item from the list.
 * @list: the list to remove the current item
 *
 * Returns a pointer to the removed data on success, NULL on failure.
 */
inline void *ewd_list_remove(Ewd_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_remove(list);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/* Remove the current item from the list */
static void *_ewd_list_remove(Ewd_List * list)
{
	void *ret = NULL;
	Ewd_List_Node *old;

	if (!list)
		return FALSE;

	if (ewd_list_is_empty(list))
		return FALSE;

	if (!list->current)
		return FALSE;

	if (list->current == list->first)
		return _ewd_list_remove_first(list);

	if (list->current == list->last)
		return _ewd_list_remove_last(list);

	old = list->current;

	_ewd_list_goto_index(list, list->index - 1);

	EWD_WRITE_LOCK(list->current);
	EWD_WRITE_LOCK(old);

	list->current->next = old->next;
	old->next = NULL;
	ret = old->data;
	old->data = NULL;

	_ewd_list_next(list);

	EWD_WRITE_UNLOCK(old);
	EWD_WRITE_UNLOCK(list->current);

	ewd_list_node_destroy(old, NULL);
	list->nodes--;

	return ret;
}

/**
 * ewd_list_remove_destroy - remove and free the data in lists current position
 * @list: the list to remove and free the current item
 *
 * Returns TRUE on success, FALSE on error
 */
int ewd_list_remove_destroy(Ewd_List *list)
{
	void *data;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	data = _ewd_list_remove(list);
	if (list->free_func)
		list->free_func(data);

	EWD_WRITE_UNLOCK(list);

	return TRUE;
}

/**
 * ewd_list_remove_first - remove the first item from the list.
 * @list: the list to remove the current item
 *
 * Returns a pointer to the removed data on success, NULL on failure.
 */
inline void *ewd_list_remove_first(Ewd_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_remove_first(list);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/* Remove the first item from the list */
static void *_ewd_list_remove_first(Ewd_List * list)
{
	void *ret = NULL;
	Ewd_List_Node *old;

	if (!list)
		return FALSE;

	EWD_WRITE_UNLOCK(list);
	if (ewd_list_is_empty(list))
		return FALSE;
	EWD_WRITE_LOCK(list);

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

	EWD_WRITE_LOCK(old);
	ret = old->data;
	old->data = NULL;
	EWD_WRITE_UNLOCK(old);

	ewd_list_node_destroy(old, NULL);
	list->nodes--;

	return ret;
}

/**
 * ewd_list_remove_first - remove the last item from the list.
 * @list: the list to remove the last node from
 *
 * Returns a pointer to the removed data on success, NULL on failure.
 */
inline void *ewd_list_remove_last(Ewd_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_remove_last(list);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/* Remove the last item from the list */
static void *_ewd_list_remove_last(Ewd_List * list)
{
	void *ret = NULL;
	Ewd_List_Node *old, *prev;

	if (!list)
		return FALSE;

	if (ewd_list_is_empty(list))
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


	EWD_WRITE_LOCK(old);
	if (old) {
		old->next = NULL;
		ret = old->data;
		old->data = NULL;
	}
	EWD_WRITE_UNLOCK(old);

	ewd_list_node_destroy(old, NULL);
	list->nodes--;

	return ret;
}

/**
 * ewd_list_goto_index - move the current item to the index number
 * @list: the list to move the current item
 * @index: the position to move the current item
 *
 * Returns a pointer to the new current item on success, NULL on failure.
 */
inline void *ewd_list_goto_index(Ewd_List * list, int index)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_goto_index(list, index);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/* This is the non-threadsafe version, use this inside internal functions that
 * already lock the list */
static void *_ewd_list_goto_index(Ewd_List *list, int index)
{
	int i;

	if (!list)
		return FALSE;

	if (ewd_list_is_empty(list))
		return FALSE;

	if (index > ewd_list_nodes(list) || index < 0)
		return FALSE;

	_ewd_list_goto_first(list);

	for (i = 1; i < index && _ewd_list_next(list); i++);

	list->index = i;

	return list->current->data;
}

/**
 * ewd_list_goto - move the current item to the node that contains data
 * @list: the list to move the current item in
 * @data: the data to find and set the current item to
 *
 * Returns a pointer to @data on success, NULL on failure.
 */
inline void *ewd_list_goto(Ewd_List * list, void *data)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_goto(list, data);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/* Set the current position to the node containing data */
static void *_ewd_list_goto(Ewd_List * list, void *data)
{
	int index;
	Ewd_List_Node *node;

	if (!list)
		return NULL;

	index = 1;

	node = list->first;
	EWD_READ_LOCK(node);
	while (node && node->data) {
		Ewd_List_Node *next;

		if (node->data == data)
			break;

		next = node->next;
		EWD_READ_UNLOCK(node);

		node = next;

		EWD_READ_LOCK(node);
		index++;
	}

	EWD_READ_UNLOCK(node);
	if (!node)
		return NULL;

	list->current = node;
	list->index = index;

	return list->current->data;
}

/**
 * ewd_list_goto_first - move the current pointer to the first item in the list
 * @list: the list to move the current pointer in
 *
 * Returns a pointer to the first item on success, NULL on failure
 */
inline void *ewd_list_goto_first(Ewd_List *list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);

	ret = _ewd_list_goto_first(list);

	EWD_WRITE_UNLOCK(list);

	return ret;
}

/* Set the current position to the start of the list */
static void *_ewd_list_goto_first(Ewd_List * list)
{
	if (!list || !list->first)
		return NULL;

	list->current = list->first;
	list->index = 1;

	return list->current->data;
}

/**
 * ewd_list_goto_last - move the pointer to current to the last item in the list
 * @list: the list to move the current pointer in
 *
 * Returns a pointer to the last item on success, NULL on failure.
 */
inline void *ewd_list_goto_last(Ewd_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_goto_last(list);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/* Set the current position to the end of the list */
static void *_ewd_list_goto_last(Ewd_List * list)
{
	if (!list || !list->last)
		return NULL;

	list->current = list->last;
	list->index = list->nodes;

	return list->current->data;
}

/**
 * ewd_list_current - retrieve the data in the current node
 * @list: the list to retrieve the current data from
 *
 * Returns the data at the current position on success, NULL on failure or if
 * no current item set.
 */
inline void *ewd_list_current(Ewd_List * list)
{
	void *ret;

	EWD_READ_LOCK(list);
	ret = _ewd_list_current(list);
	EWD_READ_UNLOCK(list);

	return ret;
}

/* Return the data of the current node without incrementing */
static void *_ewd_list_current(Ewd_List * list)
{
	void *ret;

	if (!list->current)
		return NULL;

	EWD_READ_LOCK(list->current);
	ret = list->current->data;
	EWD_READ_UNLOCK(list->current);

	return ret;
}

/**
 * ewd_list_next - retrieve the data at the current node and move to the next
 * @list: the list to move to the next item
 *
 * Returns the current item in the list on success, NULL on failure.
 */
inline void *ewd_list_next(Ewd_List * list)
{
	void *data;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	data = _ewd_list_next(list);
	EWD_WRITE_UNLOCK(list);

	return data;
}

/* Return the data contained in the current node and go to the next node */
static void *_ewd_list_next(Ewd_List * list)
{
	void *data;
	Ewd_List_Node *ret;
	Ewd_List_Node *next;

	if (!list->current)
		return NULL;

	EWD_READ_LOCK(list->current);
	ret = list->current;
	next = list->current->next;
	EWD_READ_UNLOCK(list->current);

	list->current = next;
	list->index++;

	EWD_READ_LOCK(ret);
	data = ret->data;
	EWD_READ_UNLOCK(ret);

	return data;
}

/**
 * ewd_list_clear - remove all nodes from the list
 * @list: the list that will have it's nodes removed
 *
 * Returns TRUE on success, FALSE on error. The data for each item on the list
 * is not freed by ewd_list_clear.
 */
int ewd_list_clear(Ewd_List * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);

	while (!ewd_list_is_empty(list))
		_ewd_list_remove_first(list);

	EWD_WRITE_UNLOCK(list);

	return TRUE;
}

/**
 * ewd_list_for_each - execute function for each node in the list.
 * @list: the list to retrieve nodes from.
 * @function: The function to pass each node from the list to.
 *
 * Returns TRUE on success, FALSE on failure.
 */
int ewd_list_for_each(Ewd_List *list, Ewd_For_Each function)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_READ_LOCK(list);
	ret = _ewd_list_for_each(list, function);
	EWD_READ_UNLOCK(list);

	return ret;
}

/* The real meat of executing the function for each data node */
static int _ewd_list_for_each(Ewd_List *list, Ewd_For_Each function)
{
	void *value;

	if (!list || !function)
		return FALSE;

	_ewd_list_goto_first(list);
	while ((value = _ewd_list_next(list)) != NULL)
		function(value);

	return TRUE;
}

/* Initialize a node to starting values */
int ewd_list_node_init(Ewd_List_Node * node)
{

	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	node->next = NULL;
	node->data = NULL;

	EWD_INIT_LOCKS(node);

	return TRUE;
}

/* Allocate and initialize a new list node */
Ewd_List_Node *ewd_list_node_new()
{
	Ewd_List_Node *new_node;

	new_node = malloc(sizeof(Ewd_List_Node));

	if (!ewd_list_node_init(new_node)) {
		FREE(new_node);
		return NULL;
	}

	return new_node;
}

/* Here we actually call the function to free the data and free the node */
int ewd_list_node_destroy(Ewd_List_Node * node, Ewd_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	EWD_WRITE_LOCK(node);

	if (free_func && node->data)
		free_func(node->data);

	EWD_WRITE_UNLOCK(node);
	EWD_DESTROY_LOCKS(node);

	FREE(node);

	return TRUE;
}

/**
 * ewd_dlist_new - create and initialize a new list.
 *
 * Returns a new initialized list on success, NULL on failure.
 */
Ewd_DList *ewd_dlist_new()
{
	Ewd_DList *list = NULL;

	list = (Ewd_DList *)malloc(sizeof(Ewd_DList));
	if (!list)
		return NULL;

	if (!ewd_dlist_init(list)) {
		IF_FREE(list);
		return NULL;
	}

	return list;
}

/**
 * ewd_dlist_init - initialize a list to some sane starting values.
 * @list: the list to initialize
 *
 * Returns FALSE if an error occurs, TRUE if successful
 */
int ewd_dlist_init(Ewd_DList *list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	memset(list, 0, sizeof(Ewd_DList));

	EWD_INIT_LOCKS(list);

	return TRUE;
}

/**
 * ewd_dlist_destroy - free a list and all of it's nodes.
 * @list: the list to be freed
 *
 * Returns no value
 */
void ewd_dlist_destroy(Ewd_DList * list)
{
	void *data;
	CHECK_PARAM_POINTER("list", list);

	EWD_WRITE_LOCK(list);

	while (list->first) {
		data = _ewd_dlist_remove_first(list);
		if (list->free_func)
			list->free_func(data);
	}

	EWD_WRITE_UNLOCK(list);
	EWD_DESTROY_LOCKS(list);

	FREE(list);
}

/**
 * ewd_dlist_set_free_cb - set the function for freeing data
 * @list: the list that will use this function when nodes are destroyed.
 * @free_func: the function that will free the key data
 *
 * Returns TRUE on success, FALSE on failure.
 */
int ewd_dlist_set_free_cb(Ewd_DList * list, Ewd_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ewd_list_set_free_cb(EWD_LIST(list), free_func);
}

/**
 * ewd_dlist_is_empty - checks the list for any nodes.
 * @list: the list to check for nodes
 *
 * Returns TRUE if no nodes in list, FALSE if the list contains nodes
 */
int ewd_dlist_is_empty(Ewd_DList * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ewd_list_is_empty(EWD_LIST(list));
}

/**
 * ewd_dlist_index - returns the number of the current node
 * @list: the list to return the number of the current node
 *
 * Returns the number of the current node in the list.
 */
inline int ewd_dlist_index(Ewd_DList * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ewd_list_index(EWD_LIST(list));
}

/**
 * ewd_dlist_append - append data to the list.
 * @list: the list to append @data
 * @data: the data to append to @list
 *
 * Returns FALSE if an error occurs, TRUE if the data is appended successfully
 */
int ewd_dlist_append(Ewd_DList * list, void *data)
{
	int ret;
	Ewd_DList_Node *prev;
	Ewd_DList_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);

	node = ewd_dlist_node_new();
	EWD_LIST_NODE(node)->data = data;

	prev = EWD_DLIST_NODE(EWD_LIST(list)->last);
	ret = _ewd_list_append(EWD_LIST(list), EWD_LIST_NODE(node));
	if (ret) {
		node->previous = prev;
	}

	EWD_WRITE_UNLOCK(list);

	return ret;
}

/**
 * ewd_dlist_prepend - prepend data to the beginning of the list
 * @list: the list to prepend @data
 * @data: the data to prepend to @list
 *
 * Returns FALSE if an error occurs, TRUE if data prepended successfully
 */
int ewd_dlist_prepend(Ewd_DList * list, void *data)
{
	int ret;
	Ewd_DList_Node *prev;
	Ewd_DList_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);

	node = ewd_dlist_node_new();
	EWD_LIST_NODE(node)->data = data;

	prev = EWD_DLIST_NODE(EWD_LIST(list)->first);
	ret = _ewd_list_prepend(EWD_LIST(list), EWD_LIST_NODE(node));
	if (ret && prev)
		prev->previous = node;

	EWD_WRITE_UNLOCK(list);

	return ret;
}

/**
 * ewd_dlist_insert - insert data at the current point in the list
 * @list: the list to hold the inserted @data
 * @data: the data to insert into @list
 *
 * Returns FALSE on an error, TRUE on success
 */
int ewd_dlist_insert(Ewd_DList * list, void *data)
{
	int ret;
	Ewd_DList_Node *prev;
	Ewd_DList_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);

	prev = EWD_DLIST_NODE(EWD_LIST(list)->current);
	if (!prev)
		prev = EWD_DLIST_NODE(EWD_LIST(list)->last);

	if (prev)
		prev = prev->previous;

	node = ewd_dlist_node_new();
	EWD_LIST_NODE(node)->data = data;

	ret = _ewd_list_insert(list, EWD_LIST_NODE(node));
	if (!ret) {
		EWD_WRITE_UNLOCK(list);
		return ret;
	}

	if (EWD_LIST_NODE(node)->next)
		EWD_DLIST_NODE(EWD_LIST_NODE(node)->next)->previous = node;

	if (prev)
		node->previous = prev;

	EWD_WRITE_UNLOCK(list);

	return ret;
}

/**
 * ewd_dlist_remove - remove the current item from the list.
 * @list: the list to remove the current item
 *
 * Returns a pointer to the removed data on success, NULL on failure.
 */
void *ewd_dlist_remove(Ewd_DList * list)
{
	void *ret;
	Ewd_List_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);

	if (list->current) {
		node = list->current->next;
		EWD_DLIST_NODE(node)->previous =
			EWD_DLIST_NODE(EWD_LIST(list)->current)->previous;
	}
	ret = _ewd_list_remove(list);

	EWD_WRITE_UNLOCK(list);

	return ret;
}

/**
 * ewd_dlist_remove_first - remove the first item from the list.
 * @list: the list to remove the current item
 *
 * Returns a pointer to the removed data on success, NULL on failure.
 */
void *ewd_dlist_remove_first(Ewd_DList * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_dlist_remove_first(list);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/**
 * ewd_dlist_remove_destroy - remove and free the data at the current position
 * @list: the list to remove the data from
 *
 * Returns TRUE on success, FALSE on error
 */
int ewd_dlist_remove_destroy(Ewd_DList *list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ewd_list_remove_destroy(list);
}

static void *_ewd_dlist_remove_first(Ewd_DList *list)
{
	void *ret;

	if (!list)
		return FALSE;

	ret = _ewd_list_remove_first(list);
	if (ret && EWD_LIST(list)->first)
		EWD_DLIST_NODE(EWD_LIST(list)->first)->previous = NULL;

	return ret;
}

/**
 * ewd_dlist_remove_last - remove the last item from the list
 * @list: the list to remove the last node from
 *
 * Returns a pointer to the removed data on success, NULL on failure.
 */
void *ewd_dlist_remove_last(Ewd_DList * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_remove_last(list);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/**
 * ewd_dlist_goto_index - move the current item to the index number
 * @list: the list to move the current item
 * @index: the position to move the current item
 *
 * Returns node at specified index on success, NULL on error
 */
void *ewd_dlist_goto_index(Ewd_DList * list, int index)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_dlist_goto_index(list, index);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/* This is the non-threadsafe version, use this inside internal functions that
 * already lock the list */
static void *_ewd_dlist_goto_index(Ewd_DList *list, int index)
{
	int i, increment;

	if (!list)
		return FALSE;

	if (ewd_list_is_empty(EWD_LIST(list)))
		return FALSE;

	if (index > ewd_list_nodes(EWD_LIST(list)) || index < 1)
		return FALSE;

	if (EWD_LIST(list)->index > EWD_LIST(list)->nodes)
		_ewd_list_goto_last(EWD_LIST(list));

	if (index < EWD_LIST(list)->index)
		increment = -1;
	else
		increment = 1;

	for (i = EWD_LIST(list)->index; i != index; i += increment) {
		if (increment > 0)
			_ewd_list_next(list);
		else
			_ewd_dlist_previous(list);
	}

	return _ewd_list_current(list);
}

/**
 * ewd_dlist_goto - move the current item to the node that contains data
 * @list: the list to move the current item in
 * @data: the data to find and set the current item to
 *
 * Returns specified data on success, NULL on error
 */
void *ewd_dlist_goto(Ewd_DList * list, void *data)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_goto(EWD_LIST(list), data);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/**
 * ewd_dlist_goto_first - move the current pointer to the first item in the list
 * @list: the list to change the current to the first item
 *
 * Returns a pointer to the first item on success, NULL on failure.
 */
void *ewd_dlist_goto_first(Ewd_DList *list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_goto_first(list);
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/**
 * ewd_dlist_goto_last - move the pointer to the current item to the last item
 * @list: the list to move the current item pointer to the last
 *
 * Returns a pointer to the last item in the list on success, NULL on failure.
 */
void *ewd_dlist_goto_last(Ewd_DList * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_WRITE_LOCK(list);
	ret = _ewd_list_goto_last(EWD_LIST(list));
	EWD_WRITE_UNLOCK(list);

	return ret;
}

/**
 * ewd_dlist_current - return the data in the current list item
 * @list: the list to the return the current data
 *
 * Returns value of the current data item, NULL if no current item
 */
void *ewd_dlist_current(Ewd_DList * list)
{
	void *ret;

	EWD_READ_LOCK(list);
	ret = _ewd_list_current(EWD_LIST(list));
	EWD_READ_UNLOCK(list);

	return ret;
}

/**
 * ewd_dlist_next - move to the next item in the list and return current item
 * @list: the list to move to the next item in.
 *
 * Returns data in the current list node, or NULL on error
 */
void *ewd_dlist_next(Ewd_DList * list)
{
	void *data;

	EWD_WRITE_LOCK(list);
	data = _ewd_list_next(list);
	EWD_WRITE_UNLOCK(list);

	return data;
}

/**
 * ewd_dlist_previous - move to the previous item and return current item
 * @list: the list to move to the previous item in.
 *
 * Returns data in the current list node, or NULL on error
 */
void *ewd_dlist_previous(Ewd_DList * list)
{
	void *data;

	EWD_WRITE_LOCK(list);
	data = _ewd_dlist_previous(list);
	EWD_WRITE_UNLOCK(list);

	return data;
}

static void *_ewd_dlist_previous(Ewd_DList * list)
{
	void *data = NULL;

	if (!list)
		return NULL;

	if (EWD_LIST(list)->current) {
		data = EWD_LIST(list)->current->data;
		EWD_LIST(list)->current = EWD_LIST_NODE(EWD_DLIST_NODE(
				EWD_LIST(list)->current)->previous);
		EWD_LIST(list)->index--;
	}
	else
		_ewd_list_goto_last(EWD_LIST(list));

	return data;
}

/**
 * ewd_dlist_clear - remove all nodes from the list.
 * @list: the list to remove all nodes from
 *
 * Returns TRUE on success, FALSE on errors
 */
int ewd_dlist_clear(Ewd_DList * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ewd_list_clear(EWD_LIST(list));

	return TRUE;
}

/*
 * Description: Initialize a node to sane starting values
 * Parameters: 1. node - the node to initialize
 * Returns: TRUE on success, FALSE on errors
 */
int ewd_dlist_node_init(Ewd_DList_Node * node)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	ret = ewd_list_node_init(EWD_LIST_NODE(node));
	if (ret)
		node->previous = NULL;

	return ret;
}

/*
 * Description: Allocate and initialize a new list node
 * Parameters: None
 * Returns: NULL on error, new list node on success
 */
Ewd_DList_Node *ewd_dlist_node_new()
{
	Ewd_DList_Node *new_node;

	new_node = malloc(sizeof(Ewd_DList_Node));

	if (!new_node)
		return NULL;

	if (!ewd_dlist_node_init(new_node)) {
		FREE(new_node);
		return NULL;
	}

	return new_node;
}

/*
 * Description: Call the data's free callback function, then free the node
 * Parameters: 1. node - the node to be freed
 *             2. free_func - the callback function to execute on the data
 * Returns: TRUE on success, FALSE on error
 */
int ewd_dlist_node_destroy(Ewd_DList_Node * node, Ewd_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("node", node,
			FALSE);

	return ewd_list_node_destroy(EWD_LIST_NODE(node), free_func);
}
