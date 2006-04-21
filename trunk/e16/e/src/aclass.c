/*
 * Copyright (C) 2000-2006 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2006 Kim Woelders
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
#include "aclass.h"
#include "conf.h"
#include "e16-ecore_list.h"
#include "emodule.h"
#include "ewins.h"
#include <ctype.h>

typedef struct _actiontype
{
   char               *params;
   struct _actiontype *next;
}
ActionType;

struct _action
{
   char                event;
   char                anymodifier;
   int                 modifiers;
   char                anybutton;
   int                 button;
   char                anykey;
   KeyCode             key;
   char               *key_str;
   char               *tooltipstring;
   ActionType         *action;
};

struct _actionclass
{
   char               *name;
   int                 num;
   Action            **list;
   char               *tooltipstring;
   unsigned int        ref_count;
   char global;
};

static void         UnGrabActionKey(Action * aa);
static void         GrabActionKey(Action * aa);

static Ecore_List  *aclass_list = NULL;
static Ecore_List  *aclass_list_global = NULL;

static char         mode_action_destroy = 0;

static void
RemoveActionType(ActionType * ActionTypeToRemove)
{
   ActionType         *ptr, *pp;

   ptr = ActionTypeToRemove;
   while (ptr)
     {
	if (ptr->params)
	   Efree(ptr->params);
	pp = ptr;
	ptr = ptr->next;
	Efree(pp);
     }
}

Action             *
ActionCreate(char event, char anymod, int mod, int anybut, int but,
	     char anykey, const char *key, const char *tooltipstring)
{
   Action             *aa;

   aa = Emalloc(sizeof(Action));
   aa->action = NULL;
   aa->event = event;
   aa->anymodifier = anymod;
   aa->modifiers = mod;
   aa->anybutton = anybut;
   aa->button = but;
   aa->anykey = anykey;
   if (!key || !key[0] || (event != EVENT_KEY_DOWN && event != EVENT_KEY_UP))
      aa->key = 0;
   else
      aa->key = XKeysymToKeycode(disp, XStringToKeysym(key));
   aa->key_str = (aa->key) ? Estrdup(key) : NULL;
   aa->tooltipstring =
      (tooltipstring) ? Estrdup((tooltipstring[0]) ? tooltipstring : "?!?") :
      NULL;

   return aa;
}

static void
ActionDestroy(Action * aa)
{
   if (!aa)
      return;

   if ((aa->event == EVENT_KEY_DOWN) || (aa->event == EVENT_KEY_UP))
      UnGrabActionKey(aa);
   if (aa->action)
      RemoveActionType(aa->action);
   if (aa->tooltipstring)
      Efree(aa->tooltipstring);
   if (aa->key_str)
      Efree(aa->key_str);
   Efree(aa);
}

void
ActionAddTo(Action * aa, const char *params)
{
   ActionType         *pptr, *ptr, *at;

   at = Emalloc(sizeof(ActionType));
   if (!at)
      return;
   at->next = NULL;
   at->params = Estrdup(params);
   if (!aa->action)
     {
	aa->action = at;
     }
   else
     {
	pptr = NULL;
	ptr = aa->action;
	while (ptr)
	  {
	     pptr = ptr;
	     ptr = ptr->next;
	  }
	if (pptr)
	   pptr->next = at;
     }
}

void
ActionclassAddAction(ActionClass * ac, Action * aa)
{
   ac->num++;
   if (!ac->list)
      ac->list = Emalloc(sizeof(Action *));
   else
      ac->list = Erealloc(ac->list, ac->num * sizeof(Action *));
   ac->list[ac->num - 1] = aa;
}

ActionClass        *
ActionclassCreate(const char *name, int global)
{
   ActionClass        *ac;

   ac = Ecalloc(1, sizeof(ActionClass));
   ac->name = Estrdup(name);

   if (global)
     {
	if (!aclass_list_global)
	   aclass_list_global = ecore_list_new();
	ecore_list_prepend(aclass_list_global, ac);
	ac->global = 1;
     }
   else
     {
	if (!aclass_list)
	   aclass_list = ecore_list_new();
	ecore_list_prepend(aclass_list, ac);
     }

   return ac;
}

void
ActionclassDestroy(ActionClass * ac)
{
   int                 i;

   if (!ac)
      return;

   if (ac->ref_count > 0)
     {
	DialogOK(_("ActionClass Error!"), _("%u references remain\n"),
		 ac->ref_count);
	return;
     }

   ecore_list_remove_node(aclass_list, ac);

   for (i = 0; i < ac->num; i++)
      ActionDestroy(ac->list[i]);
   if (ac->list)
      Efree(ac->list);
   if (ac->name)
      Efree(ac->name);
   if (ac->tooltipstring)
      Efree(ac->tooltipstring);
   Efree(ac);
   mode_action_destroy = 1;
}

static int
_ActionclassMatchName(const void *data, const void *match)
{
   return strcmp(((const ActionClass *)data)->name, match);
}

static ActionClass *
ActionclassFindGlobal(const char *name)
{
   return ecore_list_find(aclass_list_global, _ActionclassMatchName, name);
}

ActionClass        *
ActionclassFind(const char *name)
{
   return ecore_list_find(aclass_list, _ActionclassMatchName, name);
}

static ActionClass *
ActionclassFindAny(const char *name)
{
   ActionClass        *ac;

   ac = ecore_list_find(aclass_list_global, _ActionclassMatchName, name);
   if (ac)
      return ac;
   return ecore_list_find(aclass_list, _ActionclassMatchName, name);
}

int
AclassConfigLoad(FILE * fs)
{
   int                 err = 0;
   ActionClass        *ac = NULL;
   Action             *aa = NULL;
   char                s[FILEPATH_LEN_MAX];
   int                 i1;
   char                s2[FILEPATH_LEN_MAX];
   char                event = 0;
   char                anymod = 0;
   int                 mod = 0;
   int                 anybut = 0;
   int                 but = 0;
   int                 first = 1;
   char                anykey = 0;
   char               *key = NULL;
   char               *aclass_tooltipstring = NULL;
   char               *action_tooltipstring = NULL;
   char global = 0;
   int                 fields;

   while (GetLine(s, sizeof(s), fs))
     {
	s2[0] = 0;
	i1 = CONFIG_INVALID;
	fields = sscanf(s, "%i %4000s", &i1, s2);

	if (fields < 1)
	  {
	     i1 = CONFIG_INVALID;
	  }
	else if (i1 == CONFIG_CLOSE || i1 == CONFIG_NEXT)
	  {
	     if (fields != 1)
	       {
		  RecoverUserConfig();
		  Alert(_("CONFIG: ignoring extra data in \"%s\"\n"), s);
	       }
	  }
	else if (i1 != CONFIG_INVALID)
	  {
	     if (fields != 2)
	       {
		  RecoverUserConfig();
		  Alert(_("CONFIG: missing required data in \"%s\"\n"), s);
	       }
	  }

	switch (i1)
	  {
	  case CONFIG_VERSION:
	     break;
	  case CONFIG_ACTIONCLASS:
	     err = -1;
	     i1 = atoi(s2);
	     if (i1 != CONFIG_OPEN)
		goto done;
	     ac = NULL;
	     aa = NULL;
	     event = 0;
	     anymod = anybut = anykey = 0;
	     mod = 0;
	     but = 0;
	     first = 1;
	     _EFREE(key);
	     break;
	  case CONFIG_CLOSE:
	     ac->tooltipstring =
		(aclass_tooltipstring) ? Estrdup((aclass_tooltipstring[0]) ?
						 aclass_tooltipstring :
						 "?!?") : NULL;
	     _EFREE(aclass_tooltipstring);
	     _EFREE(action_tooltipstring);
	     err = 0;
	     goto done;

	  case CONFIG_CLASSNAME:
	  case ACLASS_NAME:
	     ac = ecore_list_remove_node(aclass_list, ActionclassFind(s2));
	     if (!ac)
		ac = ecore_list_remove_node(aclass_list_global,
					    ActionclassFindGlobal(s2));
	     if (ac)
	       {
		  if (!strcmp(s2, "KEYBINDINGS"))
		     Mode.keybinds_changed = 1;
		  ActionclassDestroy(ac);
	       }
	     ac = ActionclassCreate(s2, 0);
	     break;
	  case CONFIG_TYPE:
	  case ACLASS_TYPE:
	     if (atoi(s2) == ACLASS_TYPE_ACLASS)
		break;
	     ecore_list_remove_node(aclass_list, ActionclassFind(s2));
	     ecore_list_prepend(aclass_list_global, ac);
	     global = 1;

	     break;
	  case CONFIG_MODIFIER:
	  case ACLASS_MODIFIER:
	     /* These are the defines that I have listed...
	      * These, therefore, are the ones that I am 
	      * going to accept by default.
	      * REMINDER: add and'ing in future!!!!
	      * #define ShiftMask       (1<<0)
	      * #define LockMask        (1<<1)
	      * #define ControlMask     (1<<2)
	      * #define Mod1Mask        (1<<3)
	      * #define Mod2Mask        (1<<4)
	      * #define Mod3Mask        (1<<5)
	      * #define Mod4Mask        (1<<6)
	      * #define Mod5Mask        (1<<7)
	      */
	     switch (atoi(s2))
	       {
	       case MASK_NONE:
		  mod = 0;
		  break;
	       case MASK_SHIFT:
		  mod |= ShiftMask;
		  break;
	       case MASK_LOCK:
		  mod |= LockMask;
		  break;
	       case MASK_CTRL:
		  mod |= ControlMask;
		  break;
	       case MASK_MOD1:
		  mod |= Mod1Mask;
		  break;
	       case MASK_MOD2:
		  mod |= Mod2Mask;
		  break;
	       case MASK_MOD3:
		  mod |= Mod3Mask;
		  break;
	       case MASK_MOD4:
		  mod |= Mod4Mask;
		  break;
	       case MASK_MOD5:
		  mod |= Mod5Mask;
		  break;
	       case MASK_CTRL_ALT:
		  mod |= ControlMask | Mod1Mask;
		  break;
	       case MASK_SHIFT_ALT:
		  mod |= ShiftMask | Mod1Mask;
		  break;
	       case MASK_CTRL_SHIFT:
		  mod |= ShiftMask | ControlMask;
		  break;
	       case MASK_CTRL_SHIFT_ALT:
		  mod |= ShiftMask | ControlMask | Mod1Mask;
		  break;
	       case MASK_SHIFT_META4:
		  mod |= Mod4Mask | ShiftMask;
		  break;
	       case MASK_CTRL_META4:
		  mod |= Mod4Mask | ControlMask;
		  break;
	       case MASK_CTRL_META4_SHIFT:
		  mod |= Mod4Mask | ControlMask | ShiftMask;
		  break;
	       case MASK_SHIFT_META5:
		  mod |= Mod5Mask | ShiftMask;
		  break;
	       case MASK_CTRL_META5:
		  mod |= Mod5Mask | ControlMask;
		  break;
	       case MASK_CTRL_META5_SHIFT:
		  mod |= Mod5Mask | ControlMask | ShiftMask;
		  break;
	       case MASK_WINDOWS_SHIFT:
		  mod |= Mod2Mask | ShiftMask;
		  break;
	       case MASK_WINDOWS_CTRL:
		  mod |= Mod2Mask | ControlMask;
		  break;
	       case MASK_WINDOWS_ALT:
		  mod |= Mod2Mask | Mod1Mask;
		  break;
	       default:
		  break;
	       }
	     break;
	  case CONFIG_ANYMOD:
	  case ACLASS_ANYMOD:
	     anymod = atoi(s2);
	     break;
	  case CONFIG_ANYBUT:
	  case ACLASS_ANYBUT:
	     anybut = atoi(s2);
	     break;
	  case CONFIG_BUTTON:
	  case ACLASS_BUT:
	     but = atoi(s2);
	     break;
	  case CONFIG_ANYKEY:
	  case ACLASS_ANYKEY:
	     anykey = atoi(s2);
	     break;
	  case ACLASS_KEY:
	     if (key)
		Efree(key);
	     key = Estrdup(s2);
	     break;
	  case ACLASS_EVENT_TRIGGER:
	     event = atoi(s2);
	     break;
	  case CONFIG_NEXT:
	     mod = 0;
	     anymod = 0;
	     anybut = 0;
	     first = 1;
	     break;
	  case CONFIG_ACTION:
	     if (first)
	       {
		  aa = ActionCreate(event, anymod, mod, anybut, but, anykey,
				    key, action_tooltipstring);
		  /* the correct place to grab an action key */
		  _EFREE(action_tooltipstring);
		  _EFREE(key);
		  if (global)
		     GrabActionKey(aa);
		  ActionclassAddAction(ac, aa);
		  first = 0;
	       }
	     ActionAddTo(aa, atword(s, 2));
	     break;
	  case CONFIG_ACTION_TOOLTIP:
	     action_tooltipstring =
		Estrdupcat2(action_tooltipstring, "\n", atword(s, 2));
	     break;
	  case CONFIG_TOOLTIP:
	     aclass_tooltipstring =
		Estrdupcat2(aclass_tooltipstring, "\n", atword(s, 2));
	     break;
	  default:
	     RecoverUserConfig();
	     Alert(_("Warning: unable to determine what to do with\n"
		     "the following text in the middle of current "
		     "ActionClass definition:\n"
		     "%s\nWill ignore and continue...\n"), s);
	     break;
	  }
     }

   if (ac && err)
      ActionclassDestroy(ac);

 done:
   _EFREE(aclass_tooltipstring);
   _EFREE(action_tooltipstring);
   _EFREE(key);

   return err;
}

static Action      *
ActionDecode(const char *line)
{
   Action             *aa;
   char                ev[16], mod[16], key[128], *s;
   int                 len, event, modifiers, button;
   char                anymod, anybut, anykey;

   len = -1;
   sscanf(line, "%15s %15s %127s %n", ev, mod, key, &len);
   if (len <= 0)
      return NULL;

   event = -1;
   if (!strcmp(ev, "KeyDown"))
      event = EVENT_KEY_DOWN;
   else if (!strcmp(ev, "MouseDown"))
      event = EVENT_MOUSE_DOWN;
   else if (!strcmp(ev, "KeyUp"))
      event = EVENT_KEY_UP;
   else if (!strcmp(ev, "MouseUp"))
      event = EVENT_MOUSE_UP;
   else if (!strcmp(ev, "MouseDouble"))
      event = EVENT_DOUBLE_DOWN;
   else if (!strcmp(ev, "MouseIn"))
      event = EVENT_MOUSE_ENTER;
   else if (!strcmp(ev, "MouseOut"))
      event = EVENT_MOUSE_LEAVE;
   else if (!strcmp(ev, "FocusIn"))
      event = EVENT_FOCUS_IN;
   else if (!strcmp(ev, "FocusOut"))
      event = EVENT_FOCUS_OUT;

   anymod = anybut = anykey = 0;
   button = 0;

   modifiers = 0;
   for (s = mod; *s; s++)
     {
	switch (*s)
	  {
	  case '*':
	     anymod = 1;
	     break;
	  case 'C':
	     modifiers |= ControlMask;
	     break;
	  case 'S':
	     modifiers |= ShiftMask;
	     break;
	  case 'A':
	     modifiers |= Mod1Mask;
	     break;
	  case '1':
	     modifiers |= Mod1Mask;
	     break;
	  case '2':
	     modifiers |= Mod2Mask;
	     break;
	  case '3':
	     modifiers |= Mod3Mask;
	     break;
	  case '4':
	     modifiers |= Mod4Mask;
	     break;
	  case '5':
	     modifiers |= Mod5Mask;
	     break;
	  }
     }

   switch (event)
     {
     case EVENT_MOUSE_DOWN:
     case EVENT_MOUSE_UP:
     case EVENT_DOUBLE_DOWN:
     case EVENT_MOUSE_ENTER:
     case EVENT_MOUSE_LEAVE:
	if (key[0] == '*')
	   anybut = 1;
	else if (isdigit(key[0]))
	   button = atoi(key);
	if (!anybut && button == 0)
	   return NULL;		/* Invalid */
	key[0] = '\0';
	break;
     }

   aa =
      ActionCreate(event, anymod, modifiers, anybut, button, anykey, key, NULL);
   ActionAddTo(aa, line + len);

   return aa;
}

static int
ActionEncode(Action * aa, char *buf, int len)
{
   const char         *event;
   char               *p, mod[32], btn[32];

   if (!aa || !aa->action)
      return 0;

   p = mod;
   if (aa->anymodifier)
      *p++ = '*';
   if (aa->modifiers & ControlMask)
      *p++ = 'C';
   if (aa->modifiers & ShiftMask)
      *p++ = 'S';
   if (aa->modifiers & Mod1Mask)
      *p++ = 'A';
   if (aa->modifiers & Mod2Mask)
      *p++ = '2';
   if (aa->modifiers & Mod3Mask)
      *p++ = '3';
   if (aa->modifiers & Mod4Mask)
      *p++ = '4';
   if (aa->modifiers & Mod5Mask)
      *p++ = '5';
   if (p == mod)
      *p++ = '-';
   *p++ = '\0';

   switch (aa->event)
     {
     default:
	return 0;
     case EVENT_KEY_DOWN:
	event = "KeyDown";
	goto encode_kb;
     case EVENT_KEY_UP:
	event = "KeyUp";
	goto encode_kb;
      encode_kb:
	if (!aa->key_str)
	   return 0;
	len = Esnprintf(buf, len, "%-7s %4s %8s %s\n", event, mod, aa->key_str,
			(aa->action->params) ? aa->action->params : "");
	break;

     case EVENT_MOUSE_DOWN:
	event = "MouseDown";
	goto encode_mb;
     case EVENT_MOUSE_UP:
	event = "MouseUp";
	goto encode_mb;
     case EVENT_DOUBLE_DOWN:
	event = "MouseDouble";
	goto encode_mb;
     case EVENT_MOUSE_ENTER:
	event = "MouseIn";
	goto encode_mb;
     case EVENT_MOUSE_LEAVE:
	event = "MouseOut";
	goto encode_mb;
      encode_mb:
	if (aa->anybutton)
	   strcpy(btn, "*");
	else
	   sprintf(btn, "%u", aa->button);
	len = Esnprintf(buf, len, "%-11s %4s %s %s\n", event, mod, btn,
			(aa->action->params) ? aa->action->params : "");
	break;

     case EVENT_FOCUS_IN:
	event = "FocusIn";
	goto encode_fc;
     case EVENT_FOCUS_OUT:
	event = "FocusOut";
	goto encode_fc;
      encode_fc:
	break;
     }

   return len;
}

static int
AclassEncodeTT(const char *str, char *buf, int len)
{
   char              **lst;
   int                 i, num, l, nw;

   lst = StrlistFromString(str, '\n', &num);
   nw = 0;
   for (i = 0; i < num; i++)
     {
	l = Esnprintf(buf, len, "Tooltip %s\n", lst[i]);
	nw += l;
	len -= l;
	buf += l;
     }
   StrlistFree(lst, num);
   return nw;
}

static void
AclassConfigLoad2(FILE * fs)
{
   char                s[FILEPATH_LEN_MAX], *ss;
   char                prm1[128], prm2[128], prm3[128];
   ActionClass        *ac = NULL;
   Action             *aa = NULL;
   int                 len;

   for (;;)
     {
	ss = fgets(s, sizeof(s), fs);
	if (!ss)
	   break;

	len = strcspn(s, "#\r\n");
	if (len <= 0)
	   continue;
	s[len] = '\0';

	prm3[0] = '\0';
	len = sscanf(s, "%16s %128s %16s", prm1, prm2, prm3);
	if (len < 2)
	   continue;

	if (!strcmp(prm1, "Aclass"))
	  {
	     if (!strcmp(prm2, "KEYBINDINGS_UNCHANGABLE"))
	       {
		  /* No more "unchangable" keybindings. */
		  ac = ActionclassFindGlobal("KEYBINDINGS");
		  prm2[11] = '\0';
	       }
	     else
	       {
		  ac =
		     ecore_list_remove_node(aclass_list, ActionclassFind(prm2));
		  if (!ac)
		     ac = ecore_list_remove_node(aclass_list_global,
						 ActionclassFindGlobal(prm2));
		  if (ac)
		     ActionclassDestroy(ac);
		  ac = NULL;
	       }

	     if (!ac)
		ac = ActionclassCreate(prm2, prm3[0] == 'g');

	     aa = NULL;
	  }
	else if (!strncmp(prm1, "Key", 3) || !strncmp(prm1, "Mouse", 5))
	  {
	     if (!ac)
		continue;

	     aa = ActionDecode(s);
	     if (!aa)
		continue;

	     ActionclassAddAction(ac, aa);
	     GrabActionKey(aa);
	  }
	else if (!strcmp(prm1, "Tooltip"))
	  {
	     /* FIXME - Multiple line strings may break */
	     if (aa)
	       {
		  aa->tooltipstring =
		     Estrdupcat2(aa->tooltipstring, "\n", atword(s, 2));
	       }
	     else if (ac)
	       {
		  ac->tooltipstring =
		     Estrdupcat2(ac->tooltipstring, "\n", atword(s, 2));
	       }
	  }
     }
}

static void
AclassConfigLoadConfig(const char *name)
{
   char               *file;
   FILE               *fs;

   file = ConfigFileFind(name, NULL, 0);
   if (!file)
      return;

   fs = fopen(file, "r");
   Efree(file);
   if (!fs)
      return;

   AclassConfigLoad2(fs);

   fclose(fs);
}

/* This is now only for backward compatibility */
static void
AclassConfigLoadUser(void)
{
   char                s[FILEPATH_LEN_MAX], ss[FILEPATH_LEN_MAX];
   FILE               *fs;

   Esnprintf(s, sizeof(s), "%s.bindings", EGetSavePrefixCommon());
   fs = fopen(s, "r");
   if (!fs)
      return;

   AclassConfigLoad2(fs);

   fclose(fs);

   /* This file should no longer be used. Rename. */
   Esnprintf(ss, sizeof(ss), "%s.old", s);
   E_mv(s, ss);
}

static void
AclassConfigWrite(const ActionClass * ac, void (*prf) (const char *fmt, ...))
{
   char                s[FILEPATH_LEN_MAX];
   Action             *aa;
   int                 i, len;

   if (!ac || ac->num <= 0)
      return;

   prf("Aclass %s %s\n", ac->name, (ac->global)? "global" : "normal");
   if (ac->tooltipstring)
     {
	len = AclassEncodeTT(ac->tooltipstring, s, sizeof(s));
	prf(s);
     }
   for (i = 0; i < ac->num; i++)
     {
	aa = ac->list[i];
	len = ActionEncode(aa, s, sizeof(s));
	if (len <= 0)
	   continue;
	prf(s);
	if (aa->tooltipstring)
	  {
	     len = AclassEncodeTT(aa->tooltipstring, s, sizeof(s));
	     prf(s);
	  }
     }
}

static FILE        *_ac_fs = NULL;	/* Ugly! Yeah well... */

static void
_ac_prf(const char *fmt, ...)
{
   va_list             args;
   int                 len;

   va_start(args, fmt);
   len = vfprintf(_ac_fs, fmt, args);
   va_end(args);
}

static void
BindingsSave(void)
{
   char                s[FILEPATH_LEN_MAX], ss[FILEPATH_LEN_MAX];
   FILE               *fs;

   if (!Mode.keybinds_changed)
      return;

   Etmp(ss);
   fs = fopen(ss, "w");
   if (!fs)
      return;
   _ac_fs = fs;

   AclassConfigWrite(ActionclassFind("BUTTONBINDINGS"), _ac_prf);
   AclassConfigWrite(ActionclassFind("DESKBINDINGS"), _ac_prf);
   AclassConfigWrite(ActionclassFindGlobal("KEYBINDINGS"), _ac_prf);
   AclassConfigWrite(ActionclassFindGlobal("KEYBINDINGS_UNCHANGABLE"), _ac_prf);

   fclose(fs);
   _ac_fs = NULL;

   Esnprintf(s, sizeof(s), "%s/bindings.cfg", EDirUser());
   E_mv(ss, s);
}

void
ActionclassSetTooltipString(ActionClass * ac, const char *tts)
{
   _EFDUP(ac->tooltipstring, tts);
}

void
ActionclassIncRefcount(ActionClass * ac)
{
   if (ac)
      ac->ref_count++;
}

void
ActionclassDecRefcount(ActionClass * ac)
{
   if (ac)
      ac->ref_count--;
}

const char         *
ActionclassGetName(ActionClass * ac)
{
   return (ac) ? ac->name : NULL;
}

const char         *
ActionclassGetTooltipString(ActionClass * ac)
{
   return (ac) ? ac->tooltipstring : NULL;
}

int
ActionclassGetActionCount(ActionClass * ac)
{
   return (ac) ? ac->num : 0;
}

Action             *
ActionclassGetAction(ActionClass * ac, int ix)
{
   return (ac && ix < ac->num) ? ac->list[ix] : NULL;
}

const char         *
ActionGetTooltipString(Action * aa)
{
   return (aa) ? aa->tooltipstring : NULL;
}

int
ActionGetEvent(Action * aa)
{
   return (aa) ? aa->event : 0;
}

int
ActionGetAnybutton(Action * aa)
{
   return (aa) ? aa->anybutton : 0;
}

int
ActionGetButton(Action * aa)
{
   return (aa) ? aa->button : 0;
}

int
ActionGetModifiers(Action * aa)
{
   return (aa) ? aa->modifiers : 0;
}

static void
handleAction(EWin * ewin, ActionType * action)
{
   EFunc(ewin, action->params);

   /* Did we just hose ourselves? if so, we'd best not stick around here */
   if (mode_action_destroy)
      return;

   /* If there is another action in this series, (now that
    * we're sure we didn't already die) perform it
    */
   if (action->next)
      handleAction(ewin, action->next);
}

int
ActionclassEvent(ActionClass * ac, XEvent * ev, EWin * ewin)
{
   KeyCode             key;
   int                 i, type, button, modifiers, ok, mouse, mask, val = 0;
   Action             *aa;

   if (Mode.action_inhibit || (ewin && ewin->state.inhibit_actions))
      return 0;

   key = type = button = modifiers = mouse = 0;

   mask =
      (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask |
       Mod5Mask) & (~(Mode.masks.numlock | Mode.masks.scrollock | LockMask));

   switch (ev->type)
     {
     case KeyPress:
	type = EVENT_KEY_DOWN;
	key = ev->xkey.keycode;
	modifiers = ev->xbutton.state & mask;
	mouse = 0;
	break;
     case KeyRelease:
	type = EVENT_KEY_UP;
	key = ev->xkey.keycode;
	modifiers = ev->xbutton.state & mask;
	mouse = 0;
	break;
     case ButtonPress:
	if (Mode.events.double_click)
	   type = EVENT_DOUBLE_DOWN;
	else
	   type = EVENT_MOUSE_DOWN;
	button = ev->xbutton.button;
	modifiers = ev->xbutton.state & mask;
	mouse = 1;
	break;
     case ButtonRelease:
	type = EVENT_MOUSE_UP;
	button = ev->xbutton.button;
	modifiers = ev->xbutton.state & mask;
	mouse = 1;
	break;
     case EnterNotify:
	type = EVENT_MOUSE_ENTER;
	button = -1;
	modifiers = ev->xcrossing.state & mask;
	mouse = 1;
	break;
     case LeaveNotify:
	/* If frame window, quit if pointer is still inside */
	if (ewin && ev->xcrossing.window == EoGetXwin(ewin) &&
	    (ev->xcrossing.x >= 0 && ev->xcrossing.x < EoGetW(ewin) &&
	     ev->xcrossing.y >= 0 && ev->xcrossing.y < EoGetH(ewin)))
	   return 0;
	type = EVENT_MOUSE_LEAVE;
	button = -1;
	modifiers = ev->xcrossing.state & mask;
	mouse = 1;
	break;
     case FocusIn:
	type = EVENT_FOCUS_IN;
	button = -1;
	mouse = 1;
	break;
     case FocusOut:
	type = EVENT_FOCUS_OUT;
	button = -1;
	mouse = 1;
	break;
     default:
	break;
     }

   mode_action_destroy = 0;

   for (i = 0; i < ac->num; i++)
     {
	if (!mode_action_destroy)
	  {
	     aa = ac->list[i];
	     ok = 0;
	     if ((aa->event == type) && (aa->action))
	       {
		  if (mouse)
		    {
		       if (button < 0)
			 {
			    if (aa->anymodifier)
			       ok = 1;
			    else if (aa->modifiers == modifiers)
			       ok = 1;
			 }
		       else
			 {
			    if (aa->anymodifier)
			      {
				 if (aa->anybutton)
				    ok = 1;
				 else if (aa->button == button)
				    ok = 1;
			      }
			    else if (aa->modifiers == modifiers)
			      {
				 if (aa->anybutton)
				    ok = 1;
				 else if (aa->button == button)
				    ok = 1;
			      }
			 }
		    }
		  else
		    {
		       if (aa->anymodifier)
			 {
			    if (aa->anykey)
			       ok = 1;
			    else if (aa->key == key)
			       ok = 1;
			 }
		       else if (aa->modifiers == modifiers)
			 {
			    if (aa->anykey)
			       ok = 1;
			    else if (aa->key == key)
			       ok = 1;
			 }
		    }
		  if (ok)
		    {
		       handleAction(ewin, aa->action);
		       val = 1;
		    }
	       }
	  }
	if (mode_action_destroy)
	   break;
     }

   mode_action_destroy = 0;

   return val;
}

int
ActionclassesGlobalEvent(XEvent * ev)
{
   ActionClass        *ac;
   int                 match;

   match = 0;
   ECORE_LIST_FOR_EACH(aclass_list_global, ac)
      match |= ActionclassEvent(ac, ev, GetFocusEwin());

   return match;
}

static void
AclassSetupFallback(void)
{
   ActionClass        *ac;
   Action             *aa;

   /* Create a default fallback actionclass for the fallback border */
   ac = ActionclassCreate("__FALLBACK_ACTION", 0);

   aa = ActionCreate(EVENT_MOUSE_DOWN, 1, 0, 0, 1, 0, NULL, NULL);
   ActionclassAddAction(ac, aa);
   ActionAddTo(aa, "wop * mo ptr");

   aa = ActionCreate(EVENT_MOUSE_DOWN, 1, 0, 0, 2, 0, NULL, NULL);
   ActionclassAddAction(ac, aa);
   ActionAddTo(aa, "wop * close");

   aa = ActionCreate(EVENT_MOUSE_DOWN, 1, 0, 0, 3, 0, NULL, NULL);
   ActionclassAddAction(ac, aa);
   ActionAddTo(aa, "wop * sz ptr");
}

/*
 * Actions module
 */

static void
AclassSighan(int sig, void *prm __UNUSED__)
{
   switch (sig)
     {
     case ESIGNAL_INIT:
	AclassSetupFallback();
	AclassConfigLoadConfig("bindings.cfg");
	AclassConfigLoadUser();
	break;
     }
}

static void
AclassIpc(const char *params, Client * c __UNUSED__)
{
   const char         *p;
   char                cmd[128], prm[4096];
   int                 len;
   ActionClass        *ac;

   cmd[0] = prm[0] = '\0';
   p = params;
   if (p)
     {
	len = 0;
	sscanf(p, "%100s %4000s %n", cmd, prm, &len);
	p += len;
     }

   if (!p || cmd[0] == '\0' || cmd[0] == '?')
     {
     }
   else if (!strncmp(cmd, "kb", 2))
     {
	AclassConfigWrite(ActionclassFindGlobal("KEYBINDINGS"), IpcPrintf);
     }
   else if (!strncmp(cmd, "list", 2))
     {
	if (prm[0] == '\0')
	  {
	     IpcPrintf("Normal:\n");
	     ECORE_LIST_FOR_EACH(aclass_list, ac) IpcPrintf("%s\n", ac->name);
	     IpcPrintf("Global:\n");
	     ECORE_LIST_FOR_EACH(aclass_list_global, ac) IpcPrintf("%s\n",
								   ac->name);
	  }
	else if (!strcmp(prm, "all"))
	  {
	     ECORE_LIST_FOR_EACH(aclass_list, ac)
	     {
		IpcPrintf("\n");
		AclassConfigWrite(ac, IpcPrintf);
	     }
	     ECORE_LIST_FOR_EACH(aclass_list_global, ac)
	     {
		IpcPrintf("\n");
		AclassConfigWrite(ac, IpcPrintf);
	     }
	  }
	else
	  {
	     AclassConfigWrite(ActionclassFindAny(prm), IpcPrintf);
	  }
     }
   else if (!strcmp(cmd, "load"))
     {
	if (*prm == '\0')
	   AclassConfigLoadConfig("bindings.cfg");
	else
	   AclassConfigLoadConfig(prm);
     }
}

/* Should only be used via e16keyedit */
static void
IPC_KeybindingsGet(const char *params __UNUSED__, Client * c __UNUSED__)
{
   ActionClass        *ac;
   Action             *aa;
   int                 i, mod;

   ac = ActionclassFindGlobal("KEYBINDINGS");
   if (!ac)
      return;

   for (i = 0; i < ac->num; i++)
     {
	aa = ac->list[i];
	if ((aa) && (aa->action) && (aa->event == EVENT_KEY_DOWN))
	  {
	     char               *key;

	     key = XKeysymToString(XKeycodeToKeysym(disp, aa->key, 0));
	     if (!key)
		continue;

	     mod = 0;
	     if (aa->modifiers == (ControlMask))
		mod = 1;
	     else if (aa->modifiers == (Mod1Mask))
		mod = 2;
	     else if (aa->modifiers == (ShiftMask))
		mod = 3;
	     else if (aa->modifiers == (ControlMask | Mod1Mask))
		mod = 4;
	     else if (aa->modifiers == (ShiftMask | ControlMask))
		mod = 5;
	     else if (aa->modifiers == (ShiftMask | Mod1Mask))
		mod = 6;
	     else if (aa->modifiers == (ShiftMask | ControlMask | Mod1Mask))
		mod = 7;
	     else if (aa->modifiers == (Mod2Mask))
		mod = 8;
	     else if (aa->modifiers == (Mod3Mask))
		mod = 9;
	     else if (aa->modifiers == (Mod4Mask))
		mod = 10;
	     else if (aa->modifiers == (Mod5Mask))
		mod = 11;
	     else if (aa->modifiers == (Mod2Mask | ShiftMask))
		mod = 12;
	     else if (aa->modifiers == (Mod2Mask | ControlMask))
		mod = 13;
	     else if (aa->modifiers == (Mod2Mask | Mod1Mask))
		mod = 14;
	     else if (aa->modifiers == (Mod4Mask | ShiftMask))
		mod = 15;
	     else if (aa->modifiers == (Mod4Mask | ControlMask))
		mod = 16;
	     else if (aa->modifiers == (Mod4Mask | ControlMask | ShiftMask))
		mod = 17;
	     else if (aa->modifiers == (Mod5Mask | ShiftMask))
		mod = 18;
	     else if (aa->modifiers == (Mod5Mask | ControlMask))
		mod = 19;
	     else if (aa->modifiers == (Mod5Mask | ControlMask | ShiftMask))
		mod = 20;

	     if (aa->action->params)
		IpcPrintf("%s %i %i %s\n", key, mod, 0, aa->action->params);
	     else
		IpcPrintf("%s %i %i\n", key, mod, 0);
	  }
     }
}

/* Should only be used via e16keyedit */
static void
IPC_KeybindingsSet(const char *params, Client * c __UNUSED__)
{
   ActionClass        *ac;
   Action             *aa;
   int                 i, l;
   char                buf[FILEPATH_LEN_MAX];
   const char         *sp, *ss;

   Mode.keybinds_changed = 1;

   ac = ecore_list_remove_node(aclass_list_global,
			       ActionclassFindGlobal("KEYBINDINGS"));
   if (ac)
      ActionclassDestroy(ac);

   ac = ActionclassCreate("KEYBINDINGS", 1);

   ss = atword(params, 1);
   if (!ss)
      return;

   i = 0;
   l = strlen(ss);
   while (i < l)
     {
	char                key[256];
	int                 mod = 0;
	int                 act_id = 0;
	int                 j = 0;

	/* put line in buf */
	sp = &(ss[i]);
	while ((sp[j]) && (sp[j] != '\n'))
	  {
	     buf[j] = sp[j];
	     j++;
	  }
	buf[j] = 0;
	if (sp[j] == '\n')
	   j++;
	i += j;

	/* parse the line */
	sscanf(buf, "%250s %i %i", key, &mod, &act_id);
	if (mod == 0)
	   mod = 0;
	else if (mod == 1)
	   mod = ControlMask;
	else if (mod == 2)
	   mod = Mod1Mask;
	else if (mod == 3)
	   mod = ShiftMask;
	else if (mod == 4)
	   mod = ControlMask | Mod1Mask;
	else if (mod == 5)
	   mod = ShiftMask | ControlMask;
	else if (mod == 6)
	   mod = ShiftMask | Mod1Mask;
	else if (mod == 7)
	   mod = ShiftMask | ControlMask | Mod1Mask;
	else if (mod == 8)
	   mod = Mod2Mask;
	else if (mod == 9)
	   mod = Mod3Mask;
	else if (mod == 10)
	   mod = Mod4Mask;
	else if (mod == 11)
	   mod = Mod5Mask;
	else if (mod == 12)
	   mod = Mod2Mask | ShiftMask;
	else if (mod == 13)
	   mod = Mod2Mask | ControlMask;
	else if (mod == 14)
	   mod = Mod2Mask | Mod1Mask;
	else if (mod == 15)
	   mod = Mod4Mask | ShiftMask;
	else if (mod == 16)
	   mod = Mod4Mask | ControlMask;
	else if (mod == 17)
	   mod = Mod4Mask | ControlMask | ShiftMask;
	else if (mod == 18)
	   mod = Mod5Mask | ShiftMask;
	else if (mod == 19)
	   mod = Mod5Mask | ControlMask;
	else if (mod == 20)
	   mod = Mod5Mask | ControlMask | ShiftMask;

	aa = ActionCreate(4, 0, mod, 0, 0, 0, key, NULL);
	ActionclassAddAction(ac, aa);
	if (atword(buf, 4))
	   ActionAddTo(aa, atword(buf, 4));
	else
	   ActionAddTo(aa, NULL);
	GrabActionKey(aa);
     }

   BindingsSave();
}

static const IpcItem AclassIpcArray[] = {
   {
    AclassIpc,
    "aclass", "ac",
    "Action class functions",
    "  aclass kb                 List key bindings\n"
    "  aclass list [name/all]    List action class[es]\n"
    "  aclass load [name]        Reload action classes (default is bindings.cfg)\n"}
   ,
   {
    IPC_KeybindingsGet, "get_keybindings", NULL,
    "List keybindings (deprecated)", NULL}
   ,
   {
    IPC_KeybindingsSet, "set_keybindings", NULL, "Set keybindings (deprecated)",
    NULL}
   ,
};
#define N_IPC_FUNCS (sizeof(AclassIpcArray)/sizeof(IpcItem))

/*
 * Module descriptor
 */
const EModule       ModAclass = {
   "aclass", "ac",
   AclassSighan,
   {N_IPC_FUNCS, AclassIpcArray}
   ,
   {0, NULL}
};

void
GrabButtonGrabs(EWin * ewin)
{
   ActionClass        *ac;
   int                 j;
   Action             *aa;
   unsigned int        mod, button, mask;

   ac = ActionclassFind("BUTTONBINDINGS");
   if (!ac)
      return;

   ac->ref_count++;
   for (j = 0; j < ac->num; j++)
     {
	aa = ac->list[j];
	if ((!aa) || ((aa->event != EVENT_MOUSE_DOWN)
		      && (aa->event != EVENT_MOUSE_UP)))
	   continue;

	mod = 0;
	button = 0;

	if (aa->anymodifier)
	   mod = AnyModifier;
	else
	   mod = aa->modifiers;

	if (aa->anybutton)
	   button = AnyButton;
	else
	   button = aa->button;

	mask = ButtonPressMask | ButtonReleaseMask;

	if (mod == AnyModifier)
	  {
	     GrabButtonSet(button, mod, EoGetWin(ewin), mask, ECSR_PGRAB, 1);
	  }
	else
	  {
	     int                 i;

	     for (i = 0; i < 8; i++)
	       {
		  GrabButtonSet(button, mod | Mode.masks.mod_combos[i],
				EoGetWin(ewin), mask, ECSR_PGRAB, 1);
	       }
	  }
     }
}

void
UnGrabButtonGrabs(EWin * ewin)
{
   ActionClass        *ac;
   int                 j;
   Action             *aa;
   unsigned int        mod, button;

   ac = ActionclassFind("BUTTONBINDINGS");
   if (!ac)
      return;

   ac->ref_count--;
   for (j = 0; j < ac->num; j++)
     {
	aa = ac->list[j];
	if ((!aa) || ((aa->event != EVENT_MOUSE_DOWN)
		      && (aa->event != EVENT_MOUSE_UP)))
	   continue;

	mod = 0;
	button = 0;

	if (aa->anymodifier)
	   mod = AnyModifier;
	else
	   mod = aa->modifiers;

	if (aa->anybutton)
	   button = AnyButton;
	else
	   button = aa->button;

	if (mod == AnyModifier)
	  {
	     GrabButtonRelease(button, mod, EoGetWin(ewin));
	  }
	else
	  {
	     int                 i;

	     for (i = 0; i < 8; i++)
	       {
		  GrabButtonRelease(button, mod | Mode.masks.mod_combos[i],
				    EoGetWin(ewin));
	       }
	  }
     }
}

static void
GrabActionKey(Action * aa)
{
   int                 mod;

   if (!aa->key)
      return;

   mod = aa->modifiers;
   if (aa->anymodifier)
     {
	mod = AnyModifier;
	XGrabKey(disp, aa->key, mod, VRoot.win, False, GrabModeAsync,
		 GrabModeSync);
     }
   else
     {
	int                 i;

	/* grab the key even if locks are on or not */
	for (i = 0; i < 8; i++)
	   XGrabKey(disp, aa->key, mod | Mode.masks.mod_combos[i], VRoot.win,
		    False, GrabModeAsync, GrabModeSync);
     }
}

static void
UnGrabActionKey(Action * aa)
{
   int                 mod;

   if (!aa->key)
      return;

   mod = aa->modifiers;
   if (aa->anymodifier)
     {
	mod = AnyModifier;
	XUngrabKey(disp, aa->key, mod, VRoot.win);
     }
   else
     {
	int                 i;

	/* ungrab the key even if locks are on or not */
	for (i = 0; i < 8; i++)
	   XUngrabKey(disp, aa->key, mod | Mode.masks.mod_combos[i], VRoot.win);
     }
}
