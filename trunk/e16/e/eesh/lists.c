
#include "E.h"

void               *
FindItem(char *name, int id, int find_by, int type)
{
   List               *ptr;

   EDBUG(6, "FindItem");
   ptr = lists.next;
   if (find_by == LIST_FINDBY_NAME)
     {
	while (ptr)
	  {
	     if ((ptr->type == type) && (!strcmp(name, ptr->name)))
		EDBUG_RETURN(ptr->item);
	     ptr = ptr->next;
	  }
     }
   else if (find_by == LIST_FINDBY_ID)
     {
	while (ptr)
	  {
	     if ((ptr->type == type) && (ptr->id == id))
		EDBUG_RETURN(ptr->item);
	     ptr = ptr->next;
	  }
     }
   else if (find_by == LIST_FINDBY_BOTH)
     {
	while (ptr)
	  {
	     if ((ptr->type == type) && (!strcmp(name, ptr->name)) && (ptr->id == id))
		EDBUG_RETURN(ptr->item);
	     ptr = ptr->next;
	  }
     }
   else if (find_by == LIST_FINDBY_NONE)
     {
	while (ptr)
	  {
	     if ((ptr->type == type))
		EDBUG_RETURN(ptr->item);
	     ptr = ptr->next;
	  }
     }
   EDBUG_RETURN(NULL);
}

void
AddItem(void *item, char *name, int id, int type)
{
   List               *ptr;

   EDBUG(6, "AddItem");
   ptr = Emalloc(sizeof(List));
   if (!ptr)
      EDBUG_RETURN_;
   ptr->item = item;
   ptr->name = duplicate(name);
   ptr->id = id;
   ptr->type = type;
   ptr->next = lists.next;
   lists.next = ptr;
   EDBUG_RETURN_;
}

void               *
RemoveItem(char *name, int id, int find_by, int type)
{
   List               *ptr, *pptr;
   void               *p;

   EDBUG(6, "RemoveItem");
   pptr = NULL;
   ptr = lists.next;
   if (find_by == LIST_FINDBY_NAME)
     {
	while (ptr)
	  {
	     if ((ptr->type == type) && (!strcmp(name, ptr->name)))
	       {
		  if (pptr)
		     pptr->next = ptr->next;
		  else
		     lists.next = ptr->next;
		  p = ptr->item;
		  if (ptr->name)
		     Efree(ptr->name);
		  Efree(ptr);
		  EDBUG_RETURN(p);
	       }
	     pptr = ptr;
	     ptr = ptr->next;
	  }
     }
   else if (find_by == LIST_FINDBY_ID)
     {
	while (ptr)
	  {
	     if ((ptr->type == type) && (ptr->id == id))
	       {
		  if (pptr)
		     pptr->next = ptr->next;
		  else
		     lists.next = ptr->next;
		  p = ptr->item;
		  if (ptr->name)
		     Efree(ptr->name);
		  Efree(ptr);
		  EDBUG_RETURN(p);
	       }
	     pptr = ptr;
	     ptr = ptr->next;
	  }
     }
   else if (find_by == LIST_FINDBY_BOTH)
     {
	while (ptr)
	  {
	     if ((ptr->type == type) && (!strcmp(name, ptr->name)) && (ptr->id == id))
	       {
		  if (pptr)
		     pptr->next = ptr->next;
		  else
		     lists.next = ptr->next;
		  p = ptr->item;
		  if (ptr->name)
		     Efree(ptr->name);
		  Efree(ptr);
		  EDBUG_RETURN(p);
	       }
	     pptr = ptr;
	     ptr = ptr->next;
	  }
     }
   else if (find_by == LIST_FINDBY_NONE)
     {
	while (ptr)
	  {
	     if ((ptr->type == type))
	       {
		  if (pptr)
		     pptr->next = ptr->next;
		  else
		     lists.next = ptr->next;
		  p = ptr->item;
		  if (ptr->name)
		     Efree(ptr->name);
		  Efree(ptr);
		  EDBUG_RETURN(p);
	       }
	     pptr = ptr;
	     ptr = ptr->next;
	  }
     }
   EDBUG_RETURN(NULL);
}

void              **
ListItemType(int *num, int type)
{
   List               *ptr;
   int                 i, len;
   void              **lst;

   EDBUG(6, "ListItemType");
   *num = 0;
   len = 0;
   if (type == LIST_TYPE_ANY)
      EDBUG_RETURN(NULL);
   ptr = lists.next;
   while (ptr)
     {
	if (ptr->type == type)
	   len++;
	ptr = ptr->next;
     }
   if (!len)
      EDBUG_RETURN(NULL);
   lst = Emalloc(len * sizeof(void *));

   i = 0;
   ptr = lists.next;
   while (ptr)
     {
	if (ptr->type == type)
	   lst[i++] = ptr->item;
	ptr = ptr->next;
     }
   *num = i;
   EDBUG_RETURN(lst);
}

char              **
ListItems(int *num, int type)
{
   List               *ptr;
   int                 i, len;
   char              **list;

   EDBUG(7, "ListItems");
   i = 0;
   len = 0;
   list = NULL;
   ptr = lists.next;
   if (type != LIST_TYPE_ANY)
     {
	while (ptr)
	  {
	     if (ptr->type == type)
		len++;
	     ptr = ptr->next;
	  }
     }
   else
     {
	while (ptr)
	  {
	     len++;
	     ptr = ptr->next;
	  }
     }
   list = Emalloc(len * sizeof(char *));

   if (!list)
     {
	*num = 0;
	EDBUG_RETURN(NULL);
     }
   ptr = lists.next;
   if (type != LIST_TYPE_ANY)
     {
	while (ptr)
	  {
	     if (ptr->type == type)
	       {
		  list[i] = duplicate(ptr->name);
		  i++;
	       }
	     ptr = ptr->next;
	  }
     }
   else
     {
	while (ptr)
	  {
	     list[i] = duplicate(ptr->name);
	     i++;
	     ptr = ptr->next;
	  }
     }
   *num = len;
   EDBUG_RETURN(list);
}

void              **
ListItemTypeID(int *num, int type, int id)
{
   List               *ptr;
   int                 i, len;
   void              **lst;

   EDBUG(6, "ListItemType");
   *num = 0;
   len = 0;
   if (type == LIST_TYPE_ANY)
      EDBUG_RETURN(NULL);
   ptr = lists.next;
   while (ptr)
     {
	if ((ptr->type == type) && (ptr->id == id))
	   len++;
	ptr = ptr->next;
     }
   if (!len)
      EDBUG_RETURN(NULL);
   lst = Emalloc(len * sizeof(void *));

   i = 0;
   ptr = lists.next;
   while (ptr)
     {
	if ((ptr->type == type) && (ptr->id == id))
	   lst[i++] = ptr->item;
	ptr = ptr->next;
     }
   *num = i;
   EDBUG_RETURN(lst);
}
