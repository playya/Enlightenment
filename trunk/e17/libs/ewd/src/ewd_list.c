#include <../config.h>

#include <Ewd.h>
#include <ewd_list_private.h>

/* 
 * Description: Create and initialize a new list.
 * Parameters: None
 * Returns: Returns a new initialized list.
 */
Ewd_List *ewd_list_new()
{
	Ewd_List *list = NULL;

	list = (Ewd_List *)malloc(sizeof(Ewd_List));
	if (!list)
		return NULL;

	if (!ewd_list_init(list)) {
		IF_FREE(list);
		return NULL;
	}

	return list;
}

/* 
 * Description: Initialize a list to some sane starting values.
 * Parameters: 1. list - the list to initialize
 * Returns: FALSE if an error occurs, TRUE if successful
 */
int ewd_list_init(Ewd_List *list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	memset(list, 0, sizeof(Ewd_List));

	EWD_INIT_STRUCT_LOCK(list);

	return TRUE;
}

/* 
 * Description: Free a list and all of it's nodes.
 * Parameters: 1. list - the list to be free'd
 * Returns: None
 */
void ewd_list_destroy(Ewd_List * list)
{
	CHECK_PARAM_POINTER("list", list);

	EWD_LOCK_STRUCT(list);

	while (list->first)
		_ewd_list_remove_first(list);

	EWD_UNLOCK_STRUCT(list);

	IF_FREE(list);
}

/*
 * Description: Add a function that will be called at node destroy time, the
 * 		function will be passed the value of the node to be destroyed.
 * Parameters: 1. list - the list that will use this function when nodes are
 *                       destroyed.
 *             2. free_func - the function that will be passed the value of
 *                            the node being freed.
 * Returns: TRUE on successful set, FALSE otherwise.
 */
int ewd_list_set_free_cb(Ewd_List * list, Ewd_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);

	list->free_func = free_func;

	EWD_UNLOCK_STRUCT(list);

	return TRUE;
}

/*
 * Description: Checks the list for any nodes.
 * Parameters: 1. list - the list to check
 * Returns: TRUE if no nodes in list, FALSE if the list contains nodes
 */
int ewd_list_is_empty(Ewd_List * list)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_is_empty(list);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* Check to see if the list has any data in it */
int _ewd_list_is_empty(Ewd_List * list)
{
	if (!list)
		return FALSE;

	if (list->nodes)
		return FALSE;

	return TRUE;
}

/*
 * Description: Returns the number of the current node
 * Parameters: 1. list - the list to return the number of the current node
 * Returns: The number of the current node in the list.
 */
int ewd_list_index(Ewd_List * list)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_index(list);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* Find the current position in the list */
int _ewd_list_index(Ewd_List * list)
{
	int index;

	if (!list)
		return FALSE;

	index = list->index;

	return index;
}

/*
 * Description: Find the number of nodes in the list.
 * Parameters: 1. list - the list to find the number of nodes
 * Returns: The number of nodes in the list.
 */
int ewd_list_nodes(Ewd_List * list)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_nodes(list);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* Find the current position in the list */
int _ewd_list_nodes(Ewd_List * list)
{
	int index;

	if (!list)
		return FALSE;

	index = list->nodes;

	return index;
}

/*
 * Description: Append data to the list.
 * Parameters: 1. list - the list to append the data
 *             2. data - the data to append to the list.
 * Returns: FALSE if an error occurs, TRUE if the data is appended successfully
 */
int ewd_list_append(Ewd_List * list, void *data)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_append(list, data);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* For adding items to the end of the list */
int _ewd_list_append(Ewd_List * list, void *data)
{
	Ewd_List_Node *end = NULL;

	if (!list)
		return FALSE;

	end = ewd_list_node_new();
	if (!end)
		return FALSE;

	if (list->last) {
		EWD_LOCK_STRUCT(list->last);
		list->last->next = end;
		EWD_UNLOCK_STRUCT(list->last);
	}

	EWD_LOCK_STRUCT(end);
	end->data = data;
	EWD_UNLOCK_STRUCT(end);

	list->last = end;

	if (list->first == NULL) {

		list->first = list->current = end;

		list->index = 1;
	}

	list->nodes++;

	return TRUE;
}

/*
 * Description: Prepend data to the beginning of the list
 * Parameters: 1. list - the list to prepend the data
 *             2. data - the data to prepend to the list
 * Returns: FALSE if an error occurs, TRUE if data prepended successfully
 */
int ewd_list_prepend(Ewd_List * list, void *data)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_prepend(list, data);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* For adding items to the beginning of the list */
int _ewd_list_prepend(Ewd_List * list, void *data)
{
	Ewd_List_Node *start = NULL;

	if (!list)
		return FALSE;

	/* Create a new node to add to the list */
	start = ewd_list_node_new();
	if (!start)
		return FALSE;

	/* Put it at the beginning of the list */
	EWD_LOCK_STRUCT(start);
	start->next = list->first;
	start->data = data;
	EWD_UNLOCK_STRUCT(start);

	list->first = start;

	/* If there's no current item selected, select the first one */
	if (!list->current)
		list->current = list->first;

	/* If no last node, then the first node is the last node */
	if (list->last == NULL)
		list->last = list->first;

	list->nodes++;
	list->index++;

	return TRUE;
}

/*
 * Description: Insert data at the current point in the list
 * Parameters: 1. list - the list to hold the inserted data
 *             2. data - the data to insert into the list
 * Returns: FALSE on an error, TRUE on success
 */
int ewd_list_insert(Ewd_List * list, void *data)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_insert(list, data);
	EWD_LOCK_STRUCT(list);

	return ret;
}

/* For adding items in front of the current position in the list */
int _ewd_list_insert(Ewd_List * list, void *data)
{
	Ewd_List_Node *new = NULL;

	if (!list)
		return FALSE;

	/*
	 * If the current point is at the beginning of the list, then it's the
	 * same as prepending it to the list.
	 */
	if (list->current == list->first)
		return _ewd_list_prepend(list, data);
	else if (!list->current || list->current == list->last)
		return _ewd_list_append(list, data);

	/* Create the node to insert into the list */
	new = ewd_list_node_new();
	if (!new)
		return FALSE;

	/* Setup the fields of the new node */
	EWD_LOCK_STRUCT(new);
	new->data = data;
	new->next = list->current;
	EWD_UNLOCK_STRUCT(new);

	/* And hook the node into the list */
	_ewd_list_goto_index(list, _ewd_list_index(list) - 1);

	EWD_LOCK_STRUCT(list->current);
	list->current->next = new;
	EWD_UNLOCK_STRUCT(list->current);

	list->current = new;
	list->index++;
	list->nodes++;

	return TRUE;
}

/*
 * Description: Remove the current item from the list.
 * Parameters: 1. list - the list to remove the current item
 * Returns: TRUE on success, FALSE on error
 */
int ewd_list_remove(Ewd_List * list)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_remove(list);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* Remove the current item from the list */
int _ewd_list_remove(Ewd_List * list)
{
	Ewd_List_Node *old;

	if (!list)
		return FALSE;

	if (_ewd_list_is_empty(list))
		return FALSE;

	if (!list->current)
		return FALSE;

	if (list->current == list->first)
		return _ewd_list_remove_first(list);

	if (list->current == list->last)
		return _ewd_list_remove_last(list);

	old = list->current;

	_ewd_list_goto_index(list, ewd_list_index(list));

	EWD_LOCK_STRUCT(list->current);
	EWD_LOCK_STRUCT(old);

	list->current->next = old->next;
	old->next = NULL;

	EWD_UNLOCK_STRUCT(old);
	EWD_UNLOCK_STRUCT(list->current);

	ewd_list_node_destroy(old, list->free_func);
	list->nodes--;

	return TRUE;
}

/*
 * Description: Remove the first item from the list.
 * Parameters: 1. list - the list to remove the current item
 * Returns: TRUE on success, FALSE on error
 */
int ewd_list_remove_first(Ewd_List * list)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_remove_first(list);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* Remove the first item from the list */
int _ewd_list_remove_first(Ewd_List * list)
{
	Ewd_List_Node *old;

	if (!list)
		return FALSE;

	if (_ewd_list_is_empty(list))
		return FALSE;

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

	ewd_list_node_destroy(old, list->free_func);
	list->nodes--;

	return TRUE;
}

/*
 * Description: Remove the last item from the list.
 * Parameters: 1. list - the list to remove the last node from
 * Returns: TRUE on success, FALSE on error
 */
int ewd_list_remove_last(Ewd_List * list)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_remove_last(list);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* Remove the last item from the list */
int _ewd_list_remove_last(Ewd_List * list)
{
	int index;
	Ewd_List_Node *old;

	if (!list)
		return FALSE;

	if (_ewd_list_is_empty(list))
		return FALSE;

	if (!list->last)
		return FALSE;

	old = list->last;
	if (list->current == list->last)
		index = list->nodes - 1;
	else
		index = _ewd_list_index(list);

	_ewd_list_goto_index(list, list->nodes - 1);
	list->last = list->current;
	_ewd_list_goto_index(list, index);

	EWD_LOCK_STRUCT(old);
	old->next = NULL;
	EWD_UNLOCK_STRUCT(old);

	ewd_list_node_destroy(old, list->free_func);
	list->nodes--;

	return TRUE;
}

/* 
 * Description: Move the current item to the index number
 * Parameters: 1. list - the list to move the current item
 *             2. index - the position to move the current item
 * Returns: TRUE on success, FALSE on error
 */
int ewd_list_goto_index(Ewd_List * list, int index)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_goto_index(list, index);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* This is the non-threadsafe version, use this inside internal functions that
 * already lock the list */
int _ewd_list_goto_index(Ewd_List *list, int index)
{
	int i;

	if (!list)
		return FALSE;

	if (_ewd_list_is_empty(list))
		return FALSE;

	if (index > _ewd_list_nodes(list) || index < 0)
		return FALSE;

	_ewd_list_goto_first(list);

	for (i = 1; i < index && _ewd_list_next(list); i++);

	list->index = i;

	return TRUE;
}

/*
 * Description: Move the current item to the node that contains data
 * Parameters: 1. list - the list to move the current item in
 *             2. data - the data to find and set the current item to
 * Returns: TRUE on success, FALSE on error
 */
int ewd_list_goto(Ewd_List * list, void *data)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_goto(list, data);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* Set the current position to the node containing data */
int _ewd_list_goto(Ewd_List * list, void *data)
{
	int index;
	Ewd_List_Node *node;

	if (!list)
		return FALSE;

	index = 1;

	node = list->first;
	EWD_LOCK_STRUCT(node);
	while (node && node->data) {
		if (node->data == data)
			break;
		EWD_UNLOCK_STRUCT(node);

		node = node->next;

		EWD_LOCK_STRUCT(node);
		index++;
	}

	if (!node)
		return FALSE;
	EWD_UNLOCK_STRUCT(node);

	list->current = node;
	list->index = index;

	return TRUE;
}

/*
 * Description: :
 */
int ewd_list_goto_first(Ewd_List *list)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_goto_first(list);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* Set the current position to the start of the list */
int _ewd_list_goto_first(Ewd_List * list)
{

	if (!list)
		return FALSE;

	list->current = list->first;
	list->index = 1;

	return TRUE;
}

int ewd_list_goto_last(Ewd_List * list)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_goto_last(list);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* Set the current position to the end of the list */
int _ewd_list_goto_last(Ewd_List * list)
{
	if (!list)
		return FALSE;

	list->current = list->last;
	list->index = list->nodes;

	return TRUE;
}

void *ewd_list_current(Ewd_List * list)
{
	void *ret;

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_current(list);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

/* Return the data of the current node without incrementing */
void *_ewd_list_current(Ewd_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	if (!list->current)
		return NULL;

	EWD_LOCK_STRUCT(list->current);
	ret = list->current->data;
	EWD_UNLOCK_STRUCT(list->current);

	return ret;
}

void *ewd_list_next(Ewd_List * list)
{
	void *data;

	EWD_LOCK_STRUCT(list);
	data = _ewd_list_next(list);
	EWD_UNLOCK_STRUCT(list);

	return data;
}

/* Return the data contained in the current node and go to the next node */
void *_ewd_list_next(Ewd_List * list)
{
	void *data;
	Ewd_List_Node *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	if (!list->current)
		return NULL;

	EWD_LOCK_STRUCT(list->current);
	if (list->current == list->current->next)
		return NULL;

	ret = list->current;
	EWD_UNLOCK_STRUCT(list->current);

	list->current = list->current->next;
	list->index++;

	EWD_LOCK_STRUCT(ret);
	data = ret->data;
	EWD_UNLOCK_STRUCT(ret);

	return data;
}

/* Initialize a node to starting values */
int ewd_list_node_init(Ewd_List_Node * node)
{

	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	node->next = NULL;
	node->data = NULL;

	EWD_INIT_STRUCT_LOCK(node);

	return TRUE;
}

/* Remove all nodes from the list */
int ewd_list_clear(Ewd_List * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);

	if (_ewd_list_is_empty(list))
		return TRUE;

	_ewd_list_goto_first(list);

	if (list->current) {
		_ewd_list_remove(list);
	}

	EWD_UNLOCK_STRUCT(list);

	return TRUE;
}

/* Allocate and initialize a new list node */
Ewd_List_Node *ewd_list_node_new()
{
	Ewd_List_Node *new;

	new = malloc(sizeof(Ewd_List_Node));

	if (!new)
		return NULL;

	if (!ewd_list_node_init(new)) {
		IF_FREE(new);
		return NULL;
	}

	return new;
}

int ewd_list_node_destroy(Ewd_List_Node * node, Ewd_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("node", node,
			FALSE);

	EWD_LOCK_STRUCT(node);

	if (free_func)
		free_func(node->data);

	EWD_UNLOCK_STRUCT(node);

	FREE(node);

	return TRUE;
}

/*
 * Description: Execute function for each node in the list.
 * Parameters: 1. list - The list to retrieve nodes from.
 *             2. function - The function to pass each node from the list to.
 * Returns: FALSE if there is an error, TRUE otherwise.
 */
int ewd_list_for_each(Ewd_List *list, Ewd_For_Each function)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	EWD_LOCK_STRUCT(list);
	ret = _ewd_list_for_each(list, function);
	EWD_UNLOCK_STRUCT(list);

	return ret;
}

int _ewd_list_for_each(Ewd_List *list, Ewd_For_Each function)
{
	int i, index;
	int nodes;

	if (!list || !function)
		return FALSE;

	index = _ewd_list_index(list);
	nodes = _ewd_list_nodes(list);
	_ewd_list_goto_first(list);

	for (i = 0; i < nodes; i++) {
		function(ewd_list_next(list));
	}

	_ewd_list_goto_index(list, index);

	return TRUE;
}
