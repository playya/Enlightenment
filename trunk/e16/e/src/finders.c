/*
 * Copyright (C) 2000-2005 Carsten Haitzler, Geoff Harrison and various contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "E.h"
#include "borders.h"
#include "ewins.h"
#include "groups.h"

EWin               *
EwinFindByPtr(const EWin * ewin)
{
   EWin               *const *ewins;
   int                 i, num;

   ewins = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	if (ewin == ewins[i])
	   return ewins[i];
     }
   return NULL;
}

EWin               *
EwinFindByFrame(Window win)
{
   EWin               *const *ewins;
   int                 i, num;

   ewins = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	if (win == EoGetWin(ewins[i]))
	   return ewins[i];
     }
   return NULL;
}

EWin               *
EwinFindByClient(Window win)
{
   EWin               *const *ewins;
   int                 i, num;

   ewins = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	if (win == _EwinGetClientXwin(ewins[i]))
	   return ewins[i];
     }
   return NULL;
}

EWin               *
EwinFindByChildren(Window win)
{
   EWin               *const *ewins;
   int                 i, j, num;

   ewins = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	if ((win == _EwinGetClientXwin(ewins[i])) ||
	    (win == _EwinGetContainerXwin(ewins[i])))
	  {
	     return ewins[i];
	  }
	else
	  {
	     for (j = 0; j < ewins[i]->border->num_winparts; j++)
		if (win == ewins[i]->bits[j].win)
		  {
		     return ewins[i];
		  }
	  }
     }
   return NULL;
}

EWin               *
EwinFindByString(const char *match, int type)
{
   EWin               *ewin = NULL;
   EWin               *const *ewins;
   int                 i, num, len;
   char                ewinid[FILEPATH_LEN_MAX];
   const char         *name;

   len = strlen(match);
   if (len <= 0)
      goto done;

   ewins = EwinListGetAll(&num);
   if (ewins == NULL)
      goto done;

   for (i = 0; i < num; i++)
     {
	if (type == '+')
	  {
	     /* Match start of window ID */
	     sprintf(ewinid, "%x", (unsigned)_EwinGetClientXwin(ewins[i]));
	     if (strncmp(ewinid, match, len))
		continue;
	  }
	else if (type == '=')
	  {
	     /* Match name (substring) */
	     name = ewins[i]->icccm.wm_name;
	     if (!name)
		continue;
	     if (!strstr(name, match))
		continue;
	  }
	else
	   goto done;

	ewin = ewins[i];
	break;
     }

 done:
   return ewin;
}

Group             **
ListWinGroups(const EWin * ewin, char group_select, int *num)
{
   Group             **groups = NULL;
   Group             **groups2 = NULL;
   int                 i, j, killed = 0;

   switch (group_select)
     {
     case GROUP_SELECT_EWIN_ONLY:
	groups = (Group **) Emalloc(sizeof(Group *) * ewin->num_groups);
	if (!groups)
	   break;
	memcpy(groups, ewin->groups, sizeof(Group *) * ewin->num_groups);
	*num = ewin->num_groups;
	break;
     case GROUP_SELECT_ALL_EXCEPT_EWIN:
	groups2 = (Group **) ListItemType(num, LIST_TYPE_GROUP);
	if (groups2)
	  {
	     for (i = 0; i < (*num); i++)
	       {
		  for (j = 0; j < ewin->num_groups; j++)
		    {
		       if (ewin->groups[j] == groups2[i])
			 {
			    groups2[i] = NULL;
			    killed++;
			 }
		    }
	       }
	     groups = (Group **) Emalloc(sizeof(Group *) * (*num - killed));
	     if (groups)
	       {
		  j = 0;
		  for (i = 0; i < (*num); i++)
		     if (groups2[i])
			groups[j++] = groups2[i];
		  (*num) -= killed;
	       }
	     Efree(groups2);
	  }
	break;
     case GROUP_SELECT_ALL:
     default:
	groups = (Group **) ListItemType(num, LIST_TYPE_GROUP);
	break;
     }

   return groups;
}

EWin              **
ListWinGroupMembersForEwin(const EWin * ewin, int action, char nogroup,
			   int *pnum)
{

   EWin              **gwins, *ew;
   EWin               *const *ewins;
   Group              *grp;
   int                 i, num, gwcnt;

   if (!ewin)
     {
	*pnum = 0;
	return NULL;
     }

   gwcnt = 0;
   gwins = NULL;

   if (nogroup || ewin->num_groups <= 0)
      goto done;

   ewins = EwinListGetAll(&num);
   if (ewins == NULL)		/* Should not be possible */
      goto done;

   /* Loop through window stack, bottom up */
   for (i = num - 1; i >= 0; i--)
     {
	ew = ewins[i];

	if (ew == ewin)
	   goto do_add;

	/* To get consistent behaviour, limit groups to a single desktop for now: */
	if (EoGetDesk(ew) != EoGetDesk(ewin))
	   continue;

	grp = EwinsInGroup(ewin, ew);
	if (!grp)
	   continue;

	switch (action)
	  {
	  case GROUP_ACTION_SET_WINDOW_BORDER:
	     if (!grp->cfg.set_border)
		continue;
	     break;
	  case GROUP_ACTION_ICONIFY:
	     if (!grp->cfg.iconify)
		continue;
	     break;
	  case GROUP_ACTION_MOVE:
	     if (!grp->cfg.move)
		continue;
	     break;
	  case GROUP_ACTION_RAISE:
	  case GROUP_ACTION_LOWER:
	  case GROUP_ACTION_RAISE_LOWER:
	     if (!grp->cfg.raise)
		continue;
	     break;
	  case GROUP_ACTION_STICK:
	     if (!grp->cfg.stick)
		continue;
	     break;
	  case GROUP_ACTION_SHADE:
	     if (!grp->cfg.shade)
		continue;
	     break;
	  case GROUP_ACTION_KILL:
	     if (!grp->cfg.kill)
		continue;
	     break;
	  default:
	     break;
	  }

      do_add:
	gwins = Erealloc(gwins, (gwcnt + 1) * sizeof(EWin *));
	gwins[gwcnt] = ew;
	gwcnt++;
     }

 done:
   if (gwins == NULL)
     {
	gwins = Emalloc(sizeof(EWin *));
	gwins[0] = (EWin *) ewin;
	gwcnt = 1;
     }
   *pnum = gwcnt;
   return gwins;
}

EWin              **
EwinListTransients(const EWin * ewin, int *num, int group)
{
   EWin               *const *ewins, **lst, *ew;
   int                 i, j, n;

   j = 0;
   lst = NULL;

   if (EwinGetTransientCount(ewin) <= 0)
      goto done;

   ewins = EwinListGetAll(&n);

   /* Find regular transients */
   for (i = 0; i < n; i++)
     {
	ew = ewins[i];

	/* Skip self-reference */
	if (ew == ewin)
	   continue;

	if (EwinGetTransientFor(ew) == _EwinGetClientXwin(ewin))
	  {
	     lst = Erealloc(lst, (j + 1) * sizeof(EWin *));
	     lst[j++] = ew;
	  }
     }

   if (!group)
      goto done;

   /* Group transients (if ewin is not a transient) */
   if (EwinIsTransient(ewin))
      goto done;

   for (i = 0; i < n; i++)
     {
	ew = ewins[i];

	/* Skip self-reference */
	if (ew == ewin)
	   continue;

	if (EwinGetTransientFor(ew) == VRoot.win &&
	    EwinGetWindowGroup(ew) == EwinGetWindowGroup(ewin))
	  {
	     lst = Erealloc(lst, (j + 1) * sizeof(EWin *));
	     lst[j++] = ew;
	  }
     }

 done:
   *num = j;
   return lst;
}

EWin              **
EwinListTransientFor(const EWin * ewin, int *num)
{
   EWin               *const *ewins, **lst, *ew;
   int                 i, j, n;

   j = 0;
   lst = NULL;

   if (!EwinIsTransient(ewin))
      goto done;

   ewins = EwinListGetAll(&n);
   for (i = 0; i < n; i++)
     {
	ew = ewins[i];

	/* Skip self-reference */
	if (ew == ewin)
	   continue;

	/* Regular parent or if root trans, top level group members */
	if ((EwinGetTransientFor(ewin) == _EwinGetClientXwin(ew)) ||
	    (!EwinIsTransient(ew) &&
	     EwinGetTransientFor(ewin) == VRoot.win &&
	     EwinGetWindowGroup(ew) == EwinGetWindowGroup(ewin)))
	  {
	     lst = Erealloc(lst, (j + 1) * sizeof(EWin *));
	     lst[j++] = ew;
	  }
     }

 done:
   *num = j;
   return lst;
}
