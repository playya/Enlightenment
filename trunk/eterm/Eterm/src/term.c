/*
 * Copyright (C) 1997-2001, Michael Jennings
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

static const char cvs_ident[] = "$Id$";

#include "config.h"
#include "feature.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#ifdef HAVE_X11_SUNKEYSYM_H
# include <X11/Sunkeysym.h>
#endif

#include "actions.h"
#include "buttons.h"
#include "command.h"
#include "e.h"
#include "events.h"
#include "font.h"
#include "misc.h"
#include "startup.h"
#include "options.h"
#include "pixmap.h"
#include "screen.h"
#include "scrollbar.h"
#include "term.h"
#include "windows.h"

#ifdef META8_OPTION
unsigned char meta_char = 033;	/* Alt-key prefix */
#endif
unsigned long PrivateModes = PrivMode_Default;
unsigned long SavedModes = PrivMode_Default;
char *def_colorName[] =
{
  "rgb:ff/ff/ff", "rgb:0/0/0",	/* fg/bg */
  "rgb:0/0/0",			/* 0: black             (#000000) */
#ifndef NO_BRIGHTCOLOR
    /* low-intensity colors */
  "rgb:cc/00/00",		/* 1: red     */
  "rgb:00/cc/00",		/* 2: green   */
  "rgb:cc/cc/00",		/* 3: yellow  */
  "rgb:00/00/cc",		/* 4: blue    */
  "rgb:cc/00/cc",		/* 5: magenta */
  "rgb:00/cc/cc",		/* 6: cyan    */
  "rgb:fa/eb/d7",		/* 7: white   */
    /* high-intensity colors */
  "rgb:33/33/33",		/* 8: bright black */
#endif				/* NO_BRIGHTCOLOR */
  "rgb:ff/00/00",		/* 1/9:  bright red     */
  "rgb:00/ff/00",		/* 2/10: bright green   */
  "rgb:ff/ff/00",		/* 3/11: bright yellow  */
  "rgb:00/00/ff",		/* 4/12: bright blue    */
  "rgb:ff/00/ff",		/* 5/13: bright magenta */
  "rgb:00/ff/ff",		/* 6/14: bright cyan    */
  "rgb:ff/ff/ff",		/* 7/15: bright white   */
#ifndef NO_CURSORCOLOR
  NULL, NULL,			/* cursorColor, cursorColor2 */
#endif				/* NO_CURSORCOLOR */
  NULL, NULL			/* pointerColor, borderColor */
#ifndef NO_BOLDUNDERLINE
  ,NULL, NULL			/* colorBD, colorUL */
#endif				/* NO_BOLDUNDERLINE */
};
char *rs_color[NRS_COLORS];
Pixel PixColors[NRS_COLORS + NSHADOWCOLORS];
unsigned int MetaMask = 0, AltMask = 0, NumLockMask = 0;
unsigned int modmasks[] = { Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask };

/* This function queries X to see which modifier keys (specifically Alt, Meta, and NumLock,
   since that's really all we care about) are bound to the 5 modifier masks.  It then sets
   the variables MetaMask, AltMask, and NumLockMask to the appropriate modifier mask (e.g.,
   Mod1Mask).  That way, we can use MetaMask in lookup_key() instead of using a specific
   ModMask.  This function also handles fallbacks, so that if there is no Meta key, the Alt
   key will be used as Meta, and vice versa.  get_modifiers() is called once on startup and
   after each MappingNotify event. */
void
get_modifiers(void)
{

  unsigned short i;
  XModifierKeymap *modmap;
  KeyCode *kc;

  modmap = XGetModifierMapping(Xdisplay);
  kc = modmap->modifiermap;

  /* For each of the 5 modifier masks... */
  for (i = Mod5MapIndex; i >= Mod1MapIndex; i--) {
    unsigned short j;
    register unsigned short k, l;

    k = i * modmap->max_keypermod;
    l = i - Mod1MapIndex;

    /* Find each key bound to it... */
    for (j = 0; j < modmap->max_keypermod; j++, k++) {
      unsigned char match = 0;

      if (kc[k] == 0) {
        break;
      }
      /* Check to see if it's one that we care about. */
      switch (XKeycodeToKeysym(Xdisplay, kc[k], 0)) {
        case XK_Meta_L:
        case XK_Meta_R:
          D_X11(("Found Meta key as mod %d\n", l + 1));
          match = MetaMask = modmasks[l];
          break;
        case XK_Alt_L:
        case XK_Alt_R:
          D_X11(("Found Alt key as mod %d\n", l + 1));
          match = AltMask = modmasks[l];
          break;
        case XK_Num_Lock:
          D_X11(("Found NumLock key as mod %d\n", l + 1));
          match = NumLockMask = modmasks[l];
          break;
        default:
          break;
      }
      if (match) {
        break;
      }
    }
  }
  XFreeModifiermap(modmap);
  /* Fallbacks. */
  if (MetaMask == 0) {
    if (AltMask != 0) {
      D_X11(("Defaulted Meta key to match Alt mask\n"));
      MetaMask = AltMask;
    } else {
      D_X11(("Defaulted Meta key to mod 1\n"));
      MetaMask = Mod1Mask;
    }
  }
  if (AltMask == 0) {
    D_X11(("Defaulted Alt key to match Meta mask\n"));
    AltMask = MetaMask;  /* MetaMask will always be defined at this point. */
  }

  /* See if the user wants to override any of those */
  if (rs_meta_mod) {
    MetaMask = modmasks[rs_meta_mod - 1];
  }
  if (rs_alt_mod) {
    AltMask = modmasks[rs_alt_mod - 1];
  }
  if (rs_numlock_mod) {
    NumLockMask = modmasks[rs_numlock_mod - 1];
  }
}

/* To handle buffer overflows properly, we must malloc a buffer.  Free it when done. */
#ifdef USE_XIM
#  define LK_RET()   do {if (kbuf_alloced) FREE(kbuf); return;} while (0)
#else
#  define LK_RET()   return
#endif

/* This function is called for every keypress event we receive.  Its job is to convert
   the keypress into its corresponding action.  It is responsible for calling the action
   bindings routine to see if there is an action binding for that keysym; if there is,
   this routine will exit.  If not, it continues.  It then uses the keysym to determine
   the action or escape sequence which should result from the keypress.  Actions are
   performed and the event discarded.  Escape sequences are generated and sent to the
   child process. */
void
lookup_key(XEvent * ev)
{

  static int numlock_state = 0;
  int ctrl, meta, shft, len;
  KeySym keysym;
#ifdef USE_XIM
  int valid_keysym = 0;
  static unsigned char short_buf[256];
  unsigned char *kbuf = short_buf;
  int kbuf_alloced = 0;
#else
  static unsigned char kbuf[KBUFSZ];
#endif
#ifdef GREEK_SUPPORT
  static short greek_mode = 0;
#endif

  /* Quick boolean variables tell us which modifier keys were pressed. */
  shft = (ev->xkey.state & ShiftMask);
  ctrl = (ev->xkey.state & ControlMask);
  meta = (ev->xkey.state & MetaMask);

  /* The num lock key toggles application keypad
     mode.  Num lock on, app. keypad mode off. */
  if (numlock_state || (ev->xkey.state & NumLockMask)) {
    numlock_state = (ev->xkey.state & NumLockMask);
    PrivMode((!numlock_state), PrivMode_aplKP);
  }
#ifdef USE_XIM
  if (xim_input_context != NULL) {
    Status status_return;

    kbuf[0] = '\0';
    /* Lookup the string equivalent in terms of the XIM input context. */
    len = XmbLookupString(xim_input_context, &ev->xkey, (char *) kbuf, sizeof(short_buf), &keysym, &status_return);
    D_TTY(("XmbLookupString() gave us len %d, keysym 0x%04x, and buffer \"%s\" based on the XIM input context %010p\n",
           len, keysym, safe_print_string(kbuf, len), xim_input_context));
    /* Whoops, it's too long.  Allocate a new buffer and repeat the call. */
    if (status_return == XBufferOverflow) {
      kbuf = (unsigned char *) MALLOC(len + 1);
      kbuf_alloced = 1;
      len = XmbLookupString(xim_input_context, &ev->xkey, (char *) kbuf, len, &keysym, &status_return);
      D_TTY(("XmbLookupString() gave us len %d, keysym 0x%04x, and buffer \"%s\" based on the XIM input context %010p\n",
             len, keysym, safe_print_string(kbuf, len), xim_input_context));
    }
    valid_keysym = (status_return == XLookupKeySym) || (status_return == XLookupBoth);
  } else {
    /* No XIM input context.  Do it the normal way. */
    len = XLookupString(&ev->xkey, (char *) kbuf, sizeof(short_buf), &keysym, NULL);
    D_TTY(("XLookupString() gave us len %d, keysym 0x%04x, and buffer \"%s\"\n", len, keysym, safe_print_string(kbuf, len)));
    valid_keysym = 1;
  }
#else /* USE_XIM */
  /* Translate the key event into its corresponding string according to X.  This also gets us a keysym. */
  len = XLookupString(&ev->xkey, (char *) kbuf, sizeof(kbuf), &keysym, NULL);
  D_TTY(("XLookupString() gave us len %d, keysym 0x%04x, and buffer \"%s\"\n", len, keysym, safe_print_string(kbuf, len)));

  /* If there is no string and it's a Latin2-4 character, replace it with the Latin1 character instead. */
  if (!len && (keysym >= 0x0100) && (keysym < 0x0400)) {
    len = 1;
    kbuf[0] = (keysym & 0xff);
  }
#endif /* USE_XIM */

#ifdef USE_XIM
  /* Don't do anything without a valid keysym. */
  if (valid_keysym) {
#endif

  /* Check for a corresponding action binding.  If there is one, we're done with this event. */
  if (action_dispatch(ev, keysym)) {
    LK_RET();
  }
  if (len) {
    /* Only home for keypresses with length. */
    if (Options & Opt_home_on_input) {
      TermWin.view_start = 0;
    }
  }

  /* This is a special mode that reports all extended keysyms (above 0xff00) to the application
     as escape sequences.  Very few applications use it, but it can be a handy thing to have. */
  if ((Options & Opt_report_as_keysyms) && (keysym >= 0xff00)) {
    len = sprintf((char *) kbuf, "\033[k%X;%X~", (unsigned int) (ev->xkey.state & 0xff), (unsigned int) (keysym & 0xff));
    tt_write(kbuf, len);
    LK_RET();
  }
#ifdef HOTKEY
  /* Ctrl-> and Ctrl-< should change font sizes.  (Meta if HOTKEY has been changed to Meta.) */
  if (HOTKEY) {
    if (keysym == ks_bigfont) {
      change_font(0, BIGGER_FONT);
      LK_RET();
    } else if (keysym == ks_smallfont) {
      change_font(0, SMALLER_FONT);
      LK_RET();
    }
  }
#endif

#if defined(HAVE_X11_SUNKEYSYM_H) && defined(HAVE_X11_XMU_ATOMS_H)
  switch (keysym) {
    case SunXK_Copy:
    case SunXK_Cut:
      selection_copy(XA_CLIPBOARD(Xdisplay));
      LK_RET();
      break;
    case SunXK_Paste:
      selection_paste(XA_CLIPBOARD(Xdisplay));
      LK_RET();
      break;
    case SunXK_Front:
      xterm_seq(XTerm_Takeover, "");
      LK_RET();
      break;
    default:
      break;
  }
#endif

  if (shft) {
    /* Shift + F1 - F10 generates F11 - F20 */
    if (keysym >= XK_F1 && keysym <= XK_F10) {
      keysym += (XK_F11 - XK_F1);
      shft = 0;
    } else if (!ctrl && !meta && (PrivateModes & PrivMode_ShiftKeys)) {
      switch (keysym) {
        case XK_Prior:  /* Shift-PgUp scrolls up a page */
	  if (TermWin.saveLines) {
	    scr_page(UP, (TermWin.nrow - CONTEXT_LINES));
	    LK_RET();
	  }
	  break;

	case XK_Next:  /* Shift-PgDn scrolls down a page */
	  if (TermWin.saveLines) {
	    scr_page(DN, (TermWin.nrow - CONTEXT_LINES));
	    LK_RET();
	  }
	  break;

        case XK_Insert:  /* Shift-Ins pastes the current selection. */
	  selection_paste(XA_PRIMARY);
	  LK_RET();
	  break;

        case XK_KP_Add:  /* Shift-Plus on the keypad increases the font size */
	  change_font(0, BIGGER_FONT);
	  LK_RET();
	  break;

	case XK_KP_Subtract:  /* Shift-Minus on the keypad decreases the font size */
	  change_font(0, SMALLER_FONT);
	  LK_RET();
	  break;
      }
    }
  }
#ifdef UNSHIFTED_SCROLLKEYS
  /* Allow PgUp/PgDn by themselves to scroll.  Not recommended. */
  else if (!ctrl && !meta) {
    switch (keysym) {
      case XK_Prior:
	if (TermWin.saveLines) {
	  scr_page(UP, TermWin.nrow - CONTEXT_LINES);
	  LK_RET();
	}
	break;

      case XK_Next:
	if (TermWin.saveLines) {
	  scr_page(DN, TermWin.nrow - CONTEXT_LINES);
	  LK_RET();
	}
	break;
    }
  }
#endif

  switch (keysym) {
    case XK_Print:  /* Print the screen contents out to the print pipe */
#if DEBUG >= DEBUG_SELECTION
      if (DEBUG_LEVEL >= DEBUG_SELECTION) {
        scr_dump_to_file("/tmp/Eterm_screen_dump.log");
      } else
#endif
#ifdef PRINTPIPE
        scr_printscreen(ctrl | shft);
#endif
      LK_RET();
      break;

    case XK_Mode_switch:
#ifdef GREEK_SUPPORT
      greek_mode = !greek_mode;
      if (greek_mode) {
	xterm_seq(XTerm_title, (greek_getmode() == GREEK_ELOT928 ? "[Greek: iso]" : "[Greek: ibm]"));
	greek_reset();
      } else
	xterm_seq(XTerm_title, APL_NAME "-" VERSION);
      LK_RET();
#endif
      break;
  }

  /* At this point, all the keystrokes that have special meaning to us have been handled.
     If we're in pause mode, this is a keystroke asking us to exit.  Otherwise, return here. */
  if (paused) {
    if (keysym && len) {
      exit(0);
    }
    LK_RET();
  }

  /* Process extended keysyms.  This is where the conversion to escape sequences happens. */
  if (keysym >= 0xff00 && keysym <= 0xffff) {
#ifdef KEYSYM_ATTRIBUTE
    /* The "keysym" attribute in the config file gets handled here. */
    if (!(shft | ctrl) && KeySym_map[keysym - 0xff00] != NULL) {

      const unsigned char *tmpbuf;
      unsigned int len;

      tmpbuf = (KeySym_map[keysym - 0xff00]);
      len = *tmpbuf++;

      /* escape prefix */
      if (meta
# ifdef META8_OPTION
	  && (meta_char == 033)
# endif
	  ) {
	const unsigned char ch = '\033';

	tt_write(&ch, 1);
      }
      tt_write(tmpbuf, len);
      LK_RET();
    } else
#endif
      /* This is a big-ass switch statement that handles all the special keysyms */
      switch (keysym) {
	case XK_BackSpace:
	  len = 1;
#ifdef FORCE_BACKSPACE
	  kbuf[0] = (!(shft | ctrl) ? '\b' : '\177');
#elif defined(FORCE_DELETE)
	  kbuf[0] = ((shft | ctrl) ? '\b' : '\177');
#else
	  kbuf[0] = (((PrivateModes & PrivMode_BackSpace) ? !(shft | ctrl) : (shft | ctrl)) ? '\b' : '\177');
#endif
#ifdef MULTI_CHARSET
	  if ((Options & Opt_mbyte_cursor) && scr_multi2()) {
	    memmove(kbuf + len, kbuf, len);
	    len *= 2;
	  }
#endif /* MULTI_CHARSET */
	  break;

        /* Tab key is normal unless it's shift-tab. */
	case XK_Tab:
	  if (shft) {
	    len = 3;
	    strcpy(kbuf, "\033[Z");
	  }
	  break;

#ifdef XK_KP_Home
	case XK_KP_Home:
	  /* allow shift to override */
	  if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
	    len = 3;
	    strcpy(kbuf, "\033Ow");
	    break;
	  }
	  /* -> else FALL THROUGH */
#endif

	case XK_Home:
          len = strlen(strcpy(kbuf, KS_HOME));
	  break;

#ifdef XK_KP_Left
	case XK_KP_Left:	/* \033Ot or standard cursor key */
	case XK_KP_Up:		/* \033Ox or standard cursor key */
	case XK_KP_Right:	/* \033Ov or standard cursor key */
	case XK_KP_Down:	/* \033Or or standard cursor key */
	  if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
	    len = 3;
	    strcpy(kbuf, "\033OZ");  /* The Z is replaced by t, x, v, or r */
	    kbuf[2] = ("txvr"[keysym - XK_KP_Left]);
	    break;
	  } else {
	    keysym = XK_Left + (keysym - XK_KP_Left);
	  }
	  /* Continue on with the normal cursor keys... */
#endif
	case XK_Left:		/* "\033[D" */
	case XK_Up:		/* "\033[A" */
	case XK_Right:		/* "\033[C" */
	case XK_Down:		/* "\033[B" */
	  len = 3;
	  strcpy(kbuf, "\033[@");
	  kbuf[2] = ("DACB"[keysym - XK_Left]);
	  if (PrivateModes & PrivMode_aplCUR) {
	    kbuf[1] = 'O';
	  } else if (shft) {	/* do Shift first */
	    kbuf[2] = ("dacb"[keysym - XK_Left]);
	  } else if (ctrl) {
	    kbuf[1] = 'O';
	    kbuf[2] = ("dacb"[keysym - XK_Left]);
	  }
#ifdef MULTI_CHARSET
	  if ((Options & Opt_mbyte_cursor)
	      && ((keysym == XK_Left && scr_multi2())
		  || (keysym == XK_Right && scr_multi1()))) {
	    memmove(kbuf + len, kbuf, len);
	    len *= 2;
	  }
#endif /* MULTI_CHARSET */
	  break;

          /* Keypad and normal PgUp/PgDn */
#ifndef UNSHIFTED_SCROLLKEYS
# ifdef XK_KP_Prior
	case XK_KP_Prior:
	  /* allow shift to override */
	  if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
	    len = 3;
	    strcpy(kbuf, "\033Oy");
	    break;
	  }
	  /* -> else FALL THROUGH */
# endif				/* XK_KP_Prior */
	case XK_Prior:
	  len = 4;
	  strcpy(kbuf, "\033[5~");
	  break;
# ifdef XK_KP_Next
	case XK_KP_Next:
	  /* allow shift to override */
	  if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
	    len = 3;
	    strcpy(kbuf, "\033Os");
	    break;
	  }
	  /* -> else FALL THROUGH */
# endif				/* XK_KP_Next */
	case XK_Next:
	  len = 4;
	  strcpy(kbuf, "\033[6~");
	  break;
#endif /* UNSHIFTED_SCROLLKEYS */

          /* End key */
#ifdef XK_KP_End
	case XK_KP_End:
	  /* allow shift to override */
	  if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
	    len = 3;
	    strcpy(kbuf, "\033Oq");
	    break;
	  }
	  /* -> else FALL THROUGH */
#endif /* XK_KP_End */
	case XK_End:
	  len = strlen(strcpy(kbuf, KS_END));
	  break;

	case XK_Select:
	  len = 4;
	  strcpy(kbuf, "\033[4~");
	  break;

#ifdef DXK_Remove
	case DXK_Remove:
#endif
	case XK_Execute:
	  len = 4;
	  strcpy(kbuf, "\033[3~");
	  break;

#ifdef XK_KP_Insert
	case XK_KP_Insert:
	  if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
            len = 3;
            strcpy(kbuf, "\033Op");
            break;
          }
#endif
	case XK_Insert:
	  len = 4;
	  strcpy(kbuf, "\033[2~");
	  break;

#ifdef XK_KP_Delete
	case XK_KP_Delete:
	  if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
            len = 3;
            strcpy(kbuf, "\033On");
            break;
          }
#endif
	case XK_Delete:
#ifdef KS_DELETE
	  len = strlen(strcpy(kbuf, KS_DELETE));
#ifdef MULTI_CHARSET
	  if ((Options & Opt_mbyte_cursor) && scr_multi1())
	  {
	    memmove(kbuf + len, kbuf, len);
	    len *= 2;
	  }
#endif /* MULTI_CHARSET */
#endif
	  break;

	case XK_Menu:
	  len = 5;
	  strcpy(kbuf, "\033[29~");
	  break;
	case XK_Find:
	  len = 4;
	  strcpy(kbuf, "\033[1~");
	  break;
	case XK_Help:
	  len = 5;
	  strcpy(kbuf, "\033[28~");
	  break;

	case XK_KP_Enter:
	  /* allow shift to override */
	  if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
	    len = 3;
	    strcpy(kbuf, "\033OM");
	  } else {
	    len = 1;
	    kbuf[0] = '\r';
	  }
	  break;

#ifdef XK_KP_Begin
	case XK_KP_Begin:
	  len = 3;
	  strcpy(kbuf, "\033Ou");
	  break;
#endif /* XK_KP_Begin */

	case XK_KP_F1:		/* "\033OP" */
	case XK_KP_F2:		/* "\033OQ" */
	case XK_KP_F3:		/* "\033OR" */
	case XK_KP_F4:		/* "\033OS" */
	  len = 3;
	  strcpy(kbuf, "\033OP");
	  kbuf[2] += (keysym - XK_KP_F1);
	  break;

        case XK_KP_Multiply:    /* "\033Oj" : "*" */
        case XK_KP_Add:         /* "\033Ok" : "+" */
        case XK_KP_Separator:   /* "\033Ol" : "," */
        case XK_KP_Subtract:    /* "\033Om" : "-" */
        case XK_KP_Decimal:     /* "\033On" : "." */
        case XK_KP_Divide:      /* "\033Oo" : "/" */
        case XK_KP_0:           /* "\033Op" : "0" */
        case XK_KP_1:           /* "\033Oq" : "1" */
        case XK_KP_2:           /* "\033Or" : "2" */
        case XK_KP_3:           /* "\033Os" : "3" */
        case XK_KP_4:           /* "\033Ot" : "4" */
        case XK_KP_5:           /* "\033Ou" : "5" */
        case XK_KP_6:           /* "\033Ov" : "6" */
        case XK_KP_7:           /* "\033Ow" : "7" */
        case XK_KP_8:           /* "\033Ox" : "8" */
        case XK_KP_9:           /* "\033Oy" : "9" */
	  /* allow shift to override */
	  if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
	    len = 3;
	    strcpy(kbuf, "\033Oj");
	    kbuf[2] += (keysym - XK_KP_Multiply);
	  } else {
	    len = 1;
	    kbuf[0] = ('*' + (keysym - XK_KP_Multiply));
	  }
	  break;

#define FKEY(n,fkey) do { \
len = 5; \
sprintf((char *) kbuf,"\033[%02d~", (int)((n) + (keysym - fkey))); \
} while (0);

        case XK_F1:             /* "\033[11~" */
        case XK_F2:             /* "\033[12~" */
        case XK_F3:             /* "\033[13~" */
        case XK_F4:             /* "\033[14~" */
        case XK_F5:             /* "\033[15~" */
          FKEY(11, XK_F1);
          break;

        case XK_F6:             /* "\033[17~" */
        case XK_F7:             /* "\033[18~" */
        case XK_F8:             /* "\033[19~" */
        case XK_F9:             /* "\033[20~" */
        case XK_F10:            /* "\033[21~" */
          FKEY(17, XK_F6);
          break;

        case XK_F11:            /* "\033[23~" */
        case XK_F12:            /* "\033[24~" */
        case XK_F13:            /* "\033[25~" */
        case XK_F14:            /* "\033[26~" */
          FKEY(23, XK_F11);
          break;

        case XK_F15:            /* "\033[28~" */
        case XK_F16:            /* "\033[29~" */
          FKEY(28, XK_F15);
          break;

        case XK_F17:            /* "\033[31~" */
        case XK_F18:            /* "\033[32~" */
        case XK_F19:            /* "\033[33~" */
        case XK_F20:            /* "\033[34~" */
        case XK_F21:            /* "\033[35~" */
        case XK_F22:            /* "\033[36~" */
        case XK_F23:            /* "\033[37~" */
        case XK_F24:            /* "\033[38~" */
        case XK_F25:            /* "\033[39~" */
        case XK_F26:            /* "\033[40~" */
        case XK_F27:            /* "\033[41~" */
        case XK_F28:            /* "\033[42~" */
        case XK_F29:            /* "\033[43~" */
        case XK_F30:            /* "\033[44~" */
        case XK_F31:            /* "\033[45~" */
        case XK_F32:            /* "\033[46~" */
        case XK_F33:            /* "\033[47~" */
        case XK_F34:            /* "\033[48~" */
        case XK_F35:            /* "\033[49~" */
          FKEY(31, XK_F17);
          break;
#undef FKEY
      }

#ifdef META8_OPTION
    if (meta && (meta_char == 0x80) && len > 0) {
      kbuf[len - 1] |= 0x80;
    }
#endif
  } else if (ctrl && keysym == XK_minus) {
    len = 1;
    kbuf[0] = '\037';		/* Ctrl-Minus generates ^_ (31) */
  } else {
#ifdef META8_OPTION
    /* set 8-bit on */
    if (meta && (meta_char == 0x80)) {

      unsigned char *ch;

      for (ch = kbuf; ch < kbuf + len; ch++)
	*ch |= 0x80;
      meta = 0;
    }
#endif
#ifdef GREEK_SUPPORT
    if (greek_mode)
      len = greek_xlat(kbuf, len);
#endif
  }

#ifdef USE_XIM
  }
#endif

  /* All processed.  If there's still no string, we're done. */
  if (len <= 0) {
    LK_RET();
  }

  /*
   * these modifications only affect the static keybuffer
   * pass Shift/Control indicators for function keys ending with `~'
   *
   * eg,
   *  Prior = "ESC[5~"
   *  Shift+Prior = "ESC[5$"
   *  Ctrl+Prior = "ESC[5^"
   *  Ctrl+Shift+Prior = "ESC[5@"
   */
  if (kbuf[0] == '\033' && kbuf[1] == '[' && kbuf[len - 1] == '~') {
    kbuf[len - 1] = (shft ? (ctrl ? '@' : '$') : (ctrl ? '^' : '~'));
  }

  /* escape prefix */
  if (meta
#ifdef META8_OPTION
      && (meta_char == 033)
#endif
      ) {

    const unsigned char ch = '\033';

    tt_write(&ch, 1);
  }
  D_TTY(("After handling:  len %d, keysym 0x%04x, and buffer \"%s\"\n", len, keysym, safe_print_string(kbuf, len)));
  tt_write(kbuf, len);  /* Send the resulting string to the child process */

  LK_RET();
}

#ifdef PRINTPIPE
/* Open the print pipe. */
FILE *
popen_printer(void)
{
  FILE *stream;

  if (((my_ruid != my_euid) || (my_rgid != my_egid)) && (strcmp(rs_print_pipe, PRINTPIPE))) {
    print_warning("Running setuid/setgid.  Refusing to use custom printpipe.\n");
    RESET_AND_ASSIGN(rs_print_pipe, STRDUP(PRINTPIPE));
  }
  if ((stream = (FILE *) popen(rs_print_pipe, "w")) == NULL) {
    print_error("Can't open printer pipe \"%s\" -- %s\n", rs_print_pipe, strerror(errno));
  }
  return stream;
}

/* Close the print pipe. */
int
pclose_printer(FILE * stream)
{
  fflush(stream);
  return pclose(stream);
}

/* Print everything until we hit a \e[4i sequence. */
void
process_print_pipe(void)
{
  const char *const escape_seq = "\033[4i";
  const char *const rev_escape_seq = "i4[\033";
  int index;
  FILE *fd;

  if ((fd = popen_printer()) != NULL) {
    for (index = 0; index < 4; /* nil */ ) {
      unsigned char ch = cmd_getc();

      if (ch == escape_seq[index])
	index++;
      else if (index)
	for ( /*nil */ ; index > 0; index--)
	  fputc(rev_escape_seq[index - 1], fd);

      if (index == 0)
	fputc(ch, fd);
    }
    pclose_printer(fd);
  }
}
#endif /* PRINTPIPE */

/* This routine processes escape sequences; i.e., when a \033 character is encountered in the
   input stream, this function is called to process it.  First, we get the next character off
   the input stream (the one after the ESC) and store it in ch.  Then we proceed based on what
   ch is.  Some escape sequences are only ESC followed by a single character, so the
   processing of those ends here.  Others are longer and get passed on to other functions from
   this one. */
void
process_escape_seq(void)
{
  unsigned char ch = cmd_getc();

  switch (ch) {
    case '#':
      if (cmd_getc() == '8')
	scr_E();
      break;
    case '(':
      scr_charset_set(0, cmd_getc());
      break;
    case ')':
      scr_charset_set(1, cmd_getc());
      break;
    case '*':
      scr_charset_set(2, cmd_getc());
      break;
    case '+':
      scr_charset_set(3, cmd_getc());
      break;
#ifdef MULTI_CHARSET
    case '$':
      scr_charset_set(-2, cmd_getc());
      break;
#endif
    case '7':
      scr_cursor(SAVE);
      break;
    case '8':
      scr_cursor(RESTORE);
      break;
    case '=':
    case '>':
      PrivMode((ch == '='), PrivMode_aplKP);
      break;
    case '@':
      (void) cmd_getc();
      break;
    case 'D':
      scr_index(UP);
      break;
    case 'E':
      scr_add_lines((unsigned char *) "\n\r", 1, 2);
      break;
    case 'G':
      if ((ch = cmd_getc()) == 'Q') {  /* query graphics */
        tt_printf((unsigned char *) "\033G0\n");  /* no graphics */
      } else {
        do {
          ch = cmd_getc();
        } while (ch != ':');
      }
      break;
    case 'H':
      scr_set_tab(1);
      break;
    case 'M':
      scr_index(DN);
      break;
    case 'Z':
      tt_printf((unsigned char *) ESCZ_ANSWER);
      break;
    case '[':
      process_csi_seq();
      break;
    case ']':
      process_xterm_seq();
      break;
    case 'c':
      scr_poweron();
      break;
    case 'n':
      scr_charset_choose(2);
      break;
    case 'o':
      scr_charset_choose(3);
      break;
  }
}

/* This function handles Code Sequence Introducer (CSI) escape sequences.  At this point,
   we've read "\e[" from the input stream.  CSI sequences take an arbitrary number of
   parameters and are used almost exclusively for terminal window navigation and
   manipulation. */
void
process_csi_seq(void)
{

  unsigned char ch, priv;
  unsigned int nargs;
  int arg[ESC_ARGS];

  nargs = 0;
  arg[0] = 0;
  arg[1] = 0;

  priv = 0;
  ch = cmd_getc();  /* Get the next character */
  if (ch >= '<' && ch <= '?') {
    priv = ch;  /* DEC private mode sequence.  Get next character. */
    ch = cmd_getc();
  }
  /* Read semicolon-delimited numerical arguments, if any. */
  do {
    int n;

    for (n = 0; isdigit(ch); ch = cmd_getc())
      n = n * 10 + (ch - '0');

    if (nargs < ESC_ARGS)
      arg[nargs++] = n;
    if (ch == '\b') {
      scr_backspace();
    } else if (ch == 033) {
      cmd_ungetc();  /* New escape sequence starting in the middle of one.  Punt. */
      return;
    } else if (ch < ' ') {
      scr_add_lines(&ch, 0, 1);  /* Insert verbatim non-printable character (NPC) */
      return;
    }
    if (ch < '@')
      ch = cmd_getc();  /* Separator.  Go to next digit or operation. */
  } while (ch >= ' ' && ch < '@');
  if (ch == 033) {
    cmd_ungetc();
    return;
  } else if (ch < ' ')
    return;  /* An NPC.  Punt. */

  switch (ch) {
    case '@':
      scr_insdel_chars((arg[0] ? arg[0] : 1), INSERT);
      break;
    case 'A':
    case 'e':  /* Cursor up n lines "\e[<n>A" */
      scr_gotorc((arg[0] ? -arg[0] : -1), 0, RELATIVE);
      break;
    case 'B':  /* Cursor down n lines "\e[<n>B" */
      scr_gotorc((arg[0] ? +arg[0] : +1), 0, RELATIVE);
      break;
    case 'C':
    case 'a':  /* Cursor right n columns "\e[<n>C" */
      scr_gotorc(0, (arg[0] ? +arg[0] : +1), RELATIVE);
      break;
    case 'D':  /* Cursor left n columns "\e[<n>D" */
      scr_gotorc(0, (arg[0] ? -arg[0] : -1), RELATIVE);
      break;
    case 'E':  /* Cursor down n lines and to first column "\e[<n>E" */
      scr_gotorc((arg[0] ? +arg[0] : +1), 0, R_RELATIVE);
      break;
    case 'F':  /* Cursor up n lines and to first column "\e[<n>F" */
      scr_gotorc((arg[0] ? -arg[0] : -1), 0, R_RELATIVE);
      break;
    case 'G':
    case '`':  /* Cursor to column n "\e[<n>G" */
      scr_gotorc(0, (arg[0] ? arg[0] - 1 : +1), R_RELATIVE);
      break;
    case 'H':
    case 'f':  /* Cursor to row r, column c "\e[<r>;<c>H" */
      switch (nargs) {
	case 0:
	  scr_gotorc(0, 0, 0);
	  break;
	case 1:
	  scr_gotorc((arg[0] ? arg[0] - 1 : 0), 0, 0);
	  break;
	default:
	  scr_gotorc(arg[0] - 1, arg[1] - 1, 0);
	  break;
      }
      break;
    case 'I':  /* Tab right n tab stops "\e[<n>I" */
      scr_tab(arg[0] ? +arg[0] : +1);
      break;
    case 'J':  /* Clear part or all of screen, depending on n "\e[<n>J" */
      scr_erase_screen(arg[0]);
      break;
    case 'K':  /* Clear part or all of line, depending on n "\e[<n>K" */
      scr_erase_line(arg[0]);
      break;
    case 'L':
      scr_insdel_lines((arg[0] ? arg[0] : 1), INSERT);
      break;
    case 'M':
      scr_insdel_lines((arg[0] ? arg[0] : 1), DELETE);
      break;
    case 'P':
      scr_insdel_chars((arg[0] ? arg[0] : 1), DELETE);
      break;
    case 'W':
      switch (arg[0]) {
	case 0:
	  scr_set_tab(1);
	  break;		/* = ESC H */
	case 2:
	  scr_set_tab(0);
	  break;		/* = ESC [ 0 g */
	case 5:
	  scr_set_tab(-1);
	  break;		/* = ESC [ 3 g */
      }
      break;
    case 'X':
      scr_insdel_chars((arg[0] ? arg[0] : 1), ERASE);
      break;
    case 'Z':  /* Tab left n tab stops "\e[<n>Z" */
      scr_tab(arg[0] ? -arg[0] : -1);
      break;

    case 'c':
#ifndef NO_VT100_ANS
      tt_printf(VT100_ANS);
#endif
      break;
    case 'd':  /* Cursor to row n "\e[<n>d" */
      scr_gotorc((arg[0] ? arg[0] - 1 : +1), 0, C_RELATIVE);
      break;
    case 'g':
      switch (arg[0]) {
	case 0:
	  scr_set_tab(0);
	  break;		/* delete tab */
	case 3:
	  scr_set_tab(-1);
	  break;		/* clear all tabs */
      }
      break;
#ifdef PRINTPIPE
    case 'i':
      switch (arg[0]) {
	case 0:
	  scr_printscreen(0);  /* Print screen "\e[0i" */
	  break;
	case 5:
	  process_print_pipe();  /* Start printing to print pipe "\e[5i" */
	  break;
      }
      break;
#endif
    case 'm':
      process_sgr_mode(nargs, arg);
      break;
    case 'n':			/* request for information */
      switch (arg[0]) {
	case 5:
	  tt_printf((unsigned char *) "\033[0n");
	  break;		/* ready */
	case 6:
	  scr_report_position();
	  break;
#if defined (ENABLE_DISPLAY_ANSWER)
	case 7:
	  tt_write((unsigned char *) display_name, strlen(display_name));
          tt_write("\n", 1);
	  break;
#endif
	case 8:
	  xterm_seq(XTerm_title, APL_NAME "-" VERSION);
	  break;
	case 9:
#ifdef PIXMAP_OFFSET
	  if (image_mode_is(image_bg, MODE_TRANS)) {
	    char tbuff[70];
	    char shading = 0;
	    unsigned long tint = 0xffffff;

	    if (images[image_bg].current->iml->mod) {
	      shading = images[image_bg].current->iml->mod->brightness / 0xff * 100;
	    }
	    if (images[image_bg].current->iml->rmod) {
	      tint = (tint & 0x00ffff) | ((images[image_bg].current->iml->rmod->brightness & 0xff) << 16);
	    }
	    if (images[image_bg].current->iml->gmod) {
	      tint = (tint & 0xff00ff) | ((images[image_bg].current->iml->gmod->brightness & 0xff) << 8);
	    }
	    if (images[image_bg].current->iml->bmod) {
	      tint = (tint & 0xffff00) | (images[image_bg].current->iml->bmod->brightness & 0xff);
	    }
	    snprintf(tbuff, sizeof(tbuff), APL_NAME "-" VERSION ":  Transparent - %d%% shading - 0x%06lx tint mask",
		     shading, tint);
	    xterm_seq(XTerm_title, tbuff);
	  } else
#endif
#ifdef PIXMAP_SUPPORT
	  {
	    char *tbuff;
	    unsigned short len;

	    if (background_is_pixmap()) {
	      const char *fname;

              imlib_context_set_image(images[image_bg].current->iml->im);
              fname = imlib_image_get_filename();
	      len = strlen(fname) + sizeof(APL_NAME) + sizeof(VERSION) + 5;
	      tbuff = MALLOC(len);
	      snprintf(tbuff, len, APL_NAME "-" VERSION ":  %s", fname);
	      xterm_seq(XTerm_title, tbuff);
	      FREE(tbuff);
	    } else {
	      xterm_seq(XTerm_title, APL_NAME "-" VERSION ":  No Pixmap");
	    }
	  }
#endif /* PIXMAP_SUPPORT */
	  break;
      }
      break;
    case 'r':			/* set top and bottom margins */
      if (priv != '?') {
	if (nargs < 2 || arg[0] >= arg[1])
	  scr_scroll_region(0, 10000);
	else
	  scr_scroll_region(arg[0] - 1, arg[1] - 1);
	break;
      }
      /* drop */
    case 't':
      if (priv != '?') {
	process_window_mode(nargs, arg);
	break;
      }
      /* drop */
    case 's':
      if (ch == 's' && !nargs) {
        scr_cursor(SAVE);
        break;
      }
      /* drop */
    case 'h':
    case 'l':
      process_terminal_mode(ch, priv, nargs, arg);
      break;
    case 'u':
      if (!nargs) {
        scr_cursor(RESTORE);
      }
      break;
  }
}

/* process xterm text parameters sequences `ESC ] Ps ; Pt BEL' */
void
process_xterm_seq(void)
{
  unsigned char ch, string[STRING_MAX];
  int arg;

  ch = cmd_getc();
  if (isdigit(ch)) {
    for (arg = 0; isdigit(ch); ch = cmd_getc()) {
      arg = arg * 10 + (ch - '0');
    }
  } else if (ch == ';') {
    arg = 0;
  } else {
    arg = ch;
    ch = cmd_getc();
  }
  if (arg == 'R') {
    stored_palette(RESTORE);
    redraw_image(image_bg);
    set_colorfgbg();
    scr_touch();
    scr_refresh(DEFAULT_REFRESH);
    return;
  } else if (arg == 'P') {
    unsigned char i, idx;

    idx = ((ch <= '9') ? (ch - '0') : (tolower(ch) - 'a' + 10)) + minColor;
    string[0] = '#';
    string[7] = 0;
    for (i = 1; i < 7; i++) {
      string[i] = cmd_getc();
    }
    set_window_color(idx, string);
    return;
  } else if (ch == ';') {
    unsigned long n = 0;

    while ((ch = cmd_getc()) != 007) {
      if (ch) {
	if (ch == '\t')
	  ch = ' ';		/* translate '\t' to space */
	else if (ch < ' ')
	  return;		/* control character - exit */

	if (n < sizeof(string) - 1)
	  string[n++] = ch;
      }
    }
    string[n] = '\0';
    xterm_seq(arg, (char *) string);

  } else {
    unsigned long n = 0;

    for (; ch != '\033'; ch = cmd_getc()) {
      if (ch) {
	if (ch == '\t')
	  ch = ' ';		/* translate '\t' to space */
	else if (ch < ' ')
	  return;		/* control character - exit */

	if (n < sizeof(string) - 1)
	  string[n++] = ch;
      }
    }
    string[n] = '\0';

    if ((ch = cmd_getc()) != '\\') {
      return;
    }
    switch (arg) {
      case 'l':
	xterm_seq(XTerm_title, (char *) string);
	break;
      case 'L':
	xterm_seq(XTerm_iconName, (char *) string);
	break;
      case 'I':
	set_icon_pixmap((char *) string, NULL);
	break;
      default:
	break;
    }
  }
}

/* Process window manipulations */
void
process_window_mode(unsigned int nargs, int args[])
{

  register unsigned int i;
  int x, y;
  Screen *scr;
  Window dummy_child;
  int dummy_x, dummy_y;
  unsigned int dummy_border, dummy_depth;
  char buff[1024], *name;

  if (!nargs)
    return;
  scr = ScreenOfDisplay(Xdisplay, Xscreen);
  if (!scr)
    return;

  for (i = 0; i < nargs; i++) {
    switch (args[i]) {
      case 1:
	XMapRaised(Xdisplay, TermWin.parent);
	break;
      case 2:
	XIconifyWindow(Xdisplay, TermWin.parent, Xscreen);
	break;
      case 3:
	if (i + 2 >= nargs)
	  return;		/* Make sure there are 2 args left */
	x = args[++i];
	y = args[++i];
	if (((unsigned int) x > (unsigned int) scr->width) || ((unsigned int) y > (unsigned int) scr->height))
	  return;		/* Don't move off-screen */
	XMoveWindow(Xdisplay, TermWin.parent, x, y);
	break;
      case 4:
	if (i + 2 >= nargs)
	  return;		/* Make sure there are 2 args left */
	y = args[++i];
	x = args[++i];
        BOUND(y, szHint.min_height, scr->height);
        BOUND(x, szHint.min_width, scr->width);
	XResizeWindow(Xdisplay, TermWin.parent, x, y);
#ifdef USE_XIM
	xim_set_status_position();
#endif
	break;
      case 5:
	XRaiseWindow(Xdisplay, TermWin.parent);
	break;
      case 6:
	XLowerWindow(Xdisplay, TermWin.parent);
	break;
      case 7:
	XClearWindow(Xdisplay, TermWin.vt);
	XSync(Xdisplay, False);
	scr_touch();
	scr_refresh(DEFAULT_REFRESH);
	break;
      case 8:
	if (i + 2 >= nargs)
	  return;		/* Make sure there are 2 args left */
	y = args[++i];
	x = args[++i];
        BOUND(y, 1, scr->height / TermWin.fheight);
        BOUND(x, 1, scr->width / TermWin.fwidth);
	XResizeWindow(Xdisplay, TermWin.parent,
		      Width2Pixel(x) + 2 * TermWin.internalBorder + (scrollbar_is_visible()? scrollbar_trough_width() : 0),
		      Height2Pixel(y) + 2 * TermWin.internalBorder);
	break;
      case 11:
	break;
      case 13:
	XTranslateCoordinates(Xdisplay, TermWin.parent, Xroot, 0, 0, &x, &y, &dummy_child);
	snprintf(buff, sizeof(buff), "\033[3;%d;%dt", x, y);
	tt_write((unsigned char *) buff, strlen(buff));
	break;
      case 14:
        /* Store current width and height in x and y */
        XGetGeometry(Xdisplay, TermWin.parent, &dummy_child, &dummy_x, &dummy_y, (unsigned int *) (&x), (unsigned int *) (&y), &dummy_border, &dummy_depth);
	snprintf(buff, sizeof(buff), "\033[4;%d;%dt", y, x);
	tt_write((unsigned char *) buff, strlen(buff));
	break;
      case 18:
	snprintf(buff, sizeof(buff), "\033[8;%d;%dt", TermWin.nrow, TermWin.ncol);
	tt_write((unsigned char *) buff, strlen(buff));
	break;
      case 20:
	XGetIconName(Xdisplay, TermWin.parent, &name);
	snprintf(buff, sizeof(buff), "\033]L%s\033\\", name);
	tt_write((unsigned char *) buff, strlen(buff));
	XFree(name);
	break;
      case 21:
	XFetchName(Xdisplay, TermWin.parent, &name);
	snprintf(buff, sizeof(buff), "\033]l%s\033\\", name);
	tt_write((unsigned char *) buff, strlen(buff));
	XFree(name);
	break;
      default:
	break;
    }
  }
}

/* process DEC private mode sequences `ESC [ ? Ps mode' */
/*
 * mode can only have the following values:
 *      'l' = low
 *      'h' = high
 *      's' = save
 *      'r' = restore
 *      't' = toggle
 */
void
process_terminal_mode(int mode, int priv, unsigned int nargs, int arg[])
{
  unsigned int i;
  int state;

  if (nargs == 0)
    return;

  /* make lo/hi boolean */
  switch (mode) {
    case 'l':
      mode = 0;
      break;
    case 'h':
      mode = 1;
      break;
  }

  switch (priv) {
    case 0:
      if (mode && mode != 1)
	return;			/* only do high/low */
      for (i = 0; i < nargs; i++)
	switch (arg[i]) {
	  case 4:
	    scr_insert_mode(mode);
	    break;
	    /* case 38:  TEK mode */
	}
      break;

    case '?':
      for (i = 0; i < nargs; i++)
	switch (arg[i]) {
	  case 1:		/* application cursor keys */
	    PrivCases(PrivMode_aplCUR);
	    break;

	    /* case 2:   - reset charsets to USASCII */

	  case 3:		/* 80/132 */
	    PrivCases(PrivMode_132);
	    if (PrivateModes & PrivMode_132OK)
	      set_width(state ? 132 : 80);
	    break;

	    /* case 4:   - smooth scrolling */

	  case 5:		/* reverse video */
	    PrivCases(PrivMode_rVideo);
	    scr_rvideo_mode(state);
	    break;

	  case 6:		/* relative/absolute origins  */
	    PrivCases(PrivMode_relOrigin);
	    scr_relative_origin(state);
	    break;

	  case 7:		/* autowrap */
	    PrivCases(PrivMode_Autowrap);
	    scr_autowrap(state);
	    break;

	    /* case 8:   - auto repeat, can't do on a per window basis */

	  case 9:		/* X10 mouse reporting */
	    PrivCases(PrivMode_MouseX10);
	    /* orthogonal */
	    if (PrivateModes & PrivMode_MouseX10)
	      PrivateModes &= ~(PrivMode_MouseX11);
	    break;

	  case 25:		/* visible/invisible cursor */
	    PrivCases(PrivMode_VisibleCursor);
	    scr_cursor_visible(state);
	    break;

	  case 30:
	    PrivCases(PrivMode_scrollbar);
	    map_scrollbar(state);
	    break;

	  case 35:
	    PrivCases(PrivMode_ShiftKeys);
	    break;

	  case 40:		/* 80 <--> 132 mode */
	    PrivCases(PrivMode_132OK);
	    break;

	  case 47:		/* secondary screen */
	    PrivCases(PrivMode_Screen);
	    scr_change_screen(state);
	    break;

	  case 66:		/* application key pad */
	    PrivCases(PrivMode_aplKP);
	    break;

	  case 67:
	    PrivCases(PrivMode_BackSpace);
	    break;

	  case 1000:		/* X11 mouse reporting */
	    PrivCases(PrivMode_MouseX11);
	    /* orthogonal */
	    if (PrivateModes & PrivMode_MouseX11)
	      PrivateModes &= ~(PrivMode_MouseX10);
	    break;

#if 0
	  case 1001:
	    break;		/* X11 mouse highlighting */
#endif
	  case 1010:		/* Scroll to bottom on TTY output */
	    if (Options & Opt_home_on_output)
	      Options &= ~Opt_home_on_output;
	    else
	      Options |= Opt_home_on_output;
	    break;
	  case 1012:		/* Scroll to bottom on TTY input */
	    if (Options & Opt_home_on_input)
	      Options &= ~Opt_home_on_input;
	    else
	      Options |= Opt_home_on_input;
	    break;
          case 1047:            /* Alternate screen & clear */
            PrivCases(PrivMode_Screen);
            scr_change_screen(state);
            scr_erase_screen(0);
            break;
          case 1048:            /* Save/restore cursor pos */
            PrivCases(PrivMode_Screen);
            scr_cursor(state ? SAVE : RESTORE);
            break;
          case 1049:            /* Alternate screen & cursor */
            PrivCases(PrivMode_Screen);
            scr_cursor(state ? SAVE : RESTORE);
            scr_change_screen(state);
            break;
	}
      break;
  }
}

/* process sgr sequences */
void
process_sgr_mode(unsigned int nargs, int arg[])
{
  unsigned int i;

  if (nargs == 0) {
    scr_rendition(0, ~RS_None);
    return;
  }
  for (i = 0; i < nargs; i++)
    switch (arg[i]) {
      case 0:
	scr_rendition(0, ~RS_None);
	break;
      case 1:
	scr_rendition(1, RS_Bold);
	break;
      case 4:
	scr_rendition(1, RS_Uline);
	break;
      case 5:
	scr_rendition(1, RS_Blink);
	break;
      case 7:
	scr_rendition(1, RS_RVid);
	break;
      case 22:
	scr_rendition(0, RS_Bold);
	break;
      case 24:
	scr_rendition(0, RS_Uline);
	break;
      case 25:
	scr_rendition(0, RS_Blink);
	break;
      case 27:
	scr_rendition(0, RS_RVid);
	break;

      case 30:
      case 31:			/* set fg color */
      case 32:
      case 33:
      case 34:
      case 35:
      case 36:
      case 37:
	scr_color(minColor + (arg[i] - 30), RS_Bold);
	break;
      case 39:			/* default fg */
	scr_color(restoreFG, RS_Bold);
	break;

      case 40:
      case 41:			/* set bg color */
      case 42:
      case 43:
      case 44:
      case 45:
      case 46:
      case 47:
	scr_color(minColor + (arg[i] - 40), RS_Blink);
	break;
      case 49:			/* default bg */
	scr_color(restoreBG, RS_Blink);
	break;
    }
}

/* find if fg/bg matches any of the normal (low-intensity) colors */
#ifndef NO_BRIGHTCOLOR
void
set_colorfgbg(void)
{
  unsigned int i;
  static char *colorfgbg_env = NULL;
  char *p;
  int fg = -1, bg = -1;

  if (!colorfgbg_env) {
    colorfgbg_env = (char *) MALLOC(30);
    strcpy(colorfgbg_env, "COLORFGBG=default;default;bg");
  }
  for (i = BlackColor; i <= WhiteColor; i++) {
    if (PixColors[fgColor] == PixColors[i]) {
      fg = (i - BlackColor);
      break;
    }
  }
  for (i = BlackColor; i <= WhiteColor; i++) {
    if (PixColors[bgColor] == PixColors[i]) {
      bg = (i - BlackColor);
      break;
    }
  }

  p = strchr(colorfgbg_env, '=');
  p++;
  if (fg >= 0)
    sprintf(p, "%d;", fg);
  else
    strcpy(p, "default;");
  p = strchr(p, '\0');
  if (bg >= 0)
    sprintf(p,
# ifdef PIXMAP_SUPPORT
	    "default;"
# endif
	    "%d", bg);
  else
    strcpy(p, "default");
  putenv(colorfgbg_env);

  colorfgbg = DEFAULT_RSTYLE;
  for (i = minColor; i <= maxColor; i++) {
    if (PixColors[fgColor] == PixColors[i]
# ifndef NO_BOLDUNDERLINE
	&& PixColors[fgColor] == PixColors[colorBD]
# endif				/* NO_BOLDUNDERLINE */
    /* if we wanted boldFont to have precedence */
# if 0				/* ifndef NO_BOLDFONT */
	&& TermWin.boldFont == NULL
# endif				/* NO_BOLDFONT */
	)
      colorfgbg = SET_FGCOLOR(colorfgbg, i);
    if (PixColors[bgColor] == PixColors[i])
      colorfgbg = SET_BGCOLOR(colorfgbg, i);
  }
}
#endif /* NO_BRIGHTCOLOR */

void
set_title(const char *str)
{
  static char *name = NULL;

  if (!str) {
    str = APL_NAME "-" VERSION;
  }
  if (name == NULL || strcmp(name, str)) {
    if (name != NULL) {
      FREE(name);
    }
    D_X11(("Setting window title to \"%s\"\n", str));
    XStoreName(Xdisplay, TermWin.parent, str);
    name = STRDUP(str);
  }
}

void
set_icon_name(const char *str)
{
  static char *name = NULL;

  if (!str) str = APL_NAME "-" VERSION;
  if (name == NULL || strcmp(name, str)) {
    if (name != NULL) {
      FREE(name);
    }
    D_X11(("Setting window icon name to \"%s\"\n", str));
    XSetIconName(Xdisplay, TermWin.parent, str);
    name = STRDUP(str);
  }
}

void
append_to_title(const char *str)
{
  char *name, *buff;

  REQUIRE(str != NULL);

  XFetchName(Xdisplay, TermWin.parent, &name);
  if (name != NULL) {
    buff = (char *) MALLOC(strlen(name) + strlen(str) + 1);
    strcpy(buff, name);
    strcat(buff, str);
    set_title(buff);
    FREE(buff);
  }
}

void
append_to_icon_name(const char *str)
{
  char *name, *buff;

  REQUIRE(str != NULL);

  XGetIconName(Xdisplay, TermWin.parent, &name);
  if (name != NULL) {
    buff = (char *) MALLOC(strlen(name) + strlen(str) + 1);
    strcpy(buff, name);
    strcat(buff, str);
    set_icon_name(buff);
    FREE(buff);
  }
}

/*
 * XTerm escape sequences: ESC ] Ps;Pt BEL
 *       0 = change icon name and window title
 *       1 = change icon name
 *       2 = change title
 *       3 = set text property on window
 *      46 = change logfile (not implemented)
 *      50 = change font
 *
 * rxvt/Eterm extensions:
 *       5 = Hostile takeover (grab focus and raise)
 *       6 = Transparency mode stuff
 *      10 = menu
 *      20 = bg pixmap
 *      39 = change default fg color
 *      49 = change default bg color
 */
void
xterm_seq(int op, const char *str)
{

  XColor xcol;
  char *nstr, *tnstr, *orig_tnstr, *valptr;
  unsigned char eterm_seq_op, which = 0;
  XWMHints *wm_hints;
#ifdef PIXMAP_SUPPORT
  unsigned char changed = 0, scaled = 0;
  char *color, *mod;
#endif

  if (!str)
    return;

#ifdef PIXMAP_SUPPORT
  orig_tnstr = tnstr = STRDUP(str);
#endif

  switch (op) {
    case XTerm_title:
      set_title(str);
      break;
    case XTerm_prop:
      if ((nstr = (char *) strsep(&tnstr, ";")) == NULL) {
        break;
      }
      if ((valptr = strchr(nstr, '=')) != NULL) {
        *(valptr++) = 0;
      }
      set_text_property(TermWin.parent, nstr, valptr);
      break;
    case XTerm_name:
      set_title(str);		/* drop */
    case XTerm_iconName:
      set_icon_name(str);
      break;
    case XTerm_Takeover:
      XSetInputFocus(Xdisplay, TermWin.parent, RevertToParent, CurrentTime);
      XRaiseWindow(Xdisplay, TermWin.parent);
      break;

    case XTerm_EtermSeq:

      /* Eterm proprietary escape sequences.  See technical reference for details. */
      D_CMD(("Got XTerm_EtermSeq sequence\n"));
      nstr = (char *) strsep(&tnstr, ";");
      eterm_seq_op = (unsigned char) strtol(nstr, (char **) NULL, 10);
      D_CMD(("    XTerm_EtermSeq operation is %d\n", eterm_seq_op));
      /* Yes, there is order to the numbers for this stuff.  And here it is:
         0-9      Image Class/Mode Configuration
         10-19    Scrollbar/Buttonbar/Menu Configuration
         20-39    Miscellaneous Toggles
         40-49    Foreground/Background Color Configuration
         50-69    Window/Window Manager Configuration/Interaction
         70+      Internal Eterm Operations
       */
      switch (eterm_seq_op) {
#ifdef PIXMAP_SUPPORT
	case 0:
	  nstr = (char *) strsep(&tnstr, ";");
          if (nstr) {
            if (BOOL_OPT_ISTRUE(nstr)) {
              D_CMD(("   Request to enable transparency.\n"));
              FOREACH_IMAGE(if (!image_mode_is(idx, MODE_TRANS) && image_mode_is(idx, ALLOW_TRANS)) { \
                              image_set_mode(idx, MODE_TRANS); \
                              if (images[idx].current->pmap->pixmap != None) { \
                                imlib_free_pixmap_and_mask(images[idx].current->pmap->pixmap);  \
                              } \
                              images[idx].current->pmap->pixmap = None; \
                            });
            } else if (BOOL_OPT_ISFALSE(nstr)) {
              D_CMD(("   Request to disable transparency.\n"));
              FOREACH_IMAGE(if (image_mode_is(idx, MODE_TRANS)) {if (image_mode_is(idx, ALLOW_IMAGE)) {image_set_mode(idx, MODE_IMAGE);} \
                                                                                                 else {image_set_mode(idx, MODE_SOLID);}});
            } else {
              D_CMD(("   Bad boolean value in transparency request.\n"));
              break;
            }
          } else {
            D_CMD(("   Request to toggle transparency.\n"));
            FOREACH_IMAGE(if (!image_mode_is(idx, MODE_TRANS) && image_mode_is(idx, ALLOW_TRANS)) { \
                            image_set_mode(idx, MODE_TRANS); \
                            if (images[idx].current->pmap->pixmap != None) { \
                              imlib_free_pixmap_and_mask(images[idx].current->pmap->pixmap);  \
                            } \
                            images[idx].current->pmap->pixmap = None; \
                          } else if (image_mode_is(idx, MODE_TRANS)) {if (image_mode_is(idx, ALLOW_IMAGE)) {image_set_mode(idx, MODE_IMAGE);} \
                                                                                                      else {image_set_mode(idx, MODE_SOLID);}});
          }
          redraw_all_images();
	  break;
	case 1:
          changed = 0;
          for (; 1;) {
            if ((color = (char *) strsep(&tnstr, ";")) == NULL) {
              break;
            }
            which = image_max;
            FOREACH_IMAGE(if (!strcasecmp(color, (get_image_type(idx) + 6))) {which = idx; break;});
            if (which != image_max) {
              if ((color = (char *) strsep(&tnstr, ";")) == NULL) {
                break;
              }
            } else {
              which = image_bg;
            }
            if ((mod = (char *) strsep(&tnstr, ";")) == NULL) {
              break;
            }
            if (!strcasecmp(mod, "clear")) {
              imlib_t *iml = images[which].current->iml;

              D_CMD(("Clearing the %s color modifier of the %s image\n", color, get_image_type(which)));
              if (!strcasecmp(color, "image")) {
                FREE(iml->mod);
              } else if (!strcasecmp(color, "red")) {
                FREE(iml->rmod);
              } else if (!strcasecmp(color, "green")) {
                FREE(iml->gmod);
              } else if (!strcasecmp(color, "blue")) {
                FREE(iml->bmod);
              }
# ifdef PIXMAP_OFFSET
              if (image_mode_is(which, MODE_TRANS) && (desktop_pixmap != None)) {
                free_desktop_pixmap();
              } else if (image_mode_is(which, MODE_VIEWPORT) && (viewport_pixmap != None)) {
                LIBAST_X_FREE_PIXMAP(viewport_pixmap);
                viewport_pixmap = None;	/* Force the re-read */
              }
# endif
              changed = 1;
              continue;
            }
            if ((valptr = (char *) strsep(&tnstr, ";")) == NULL) {
              break;
            }
            D_CMD(("Modifying the %s attribute of the %s color modifier of the %s image to be %s\n", mod, color, get_image_type(which), valptr));
            changed = 1;
# ifdef PIXMAP_OFFSET
            if (image_mode_is(which, MODE_TRANS) && (desktop_pixmap != None)) {
              free_desktop_pixmap();
            } else if (image_mode_is(which, MODE_VIEWPORT) && (viewport_pixmap != None)) {
              LIBAST_X_FREE_PIXMAP(viewport_pixmap);
              viewport_pixmap = None;	/* Force the re-read */
            }
# endif
            if (!strcasecmp(color, "image")) {
              imlib_t *iml = images[which].current->iml;

              if (iml->mod == NULL) {
                iml->mod = create_colormod();
              }
              if (!BEG_STRCASECMP("brightness", mod)) {
                iml->mod->brightness = (int) strtol(valptr, (char **) NULL, 0);
              } else if (!BEG_STRCASECMP("contrast", mod)) {
                iml->mod->contrast = (int) strtol(valptr, (char **) NULL, 0);
              } else if (!BEG_STRCASECMP("gamma", mod)) {
                iml->mod->gamma = (int) strtol(valptr, (char **) NULL, 0);
              }
              update_cmod(iml->mod);
              reload_image(iml);
              update_cmod_tables(iml);

            } else if (!strcasecmp(color, "red")) {
              imlib_t *iml = images[which].current->iml;

              if (iml->rmod == NULL) {
                iml->rmod = create_colormod();
              }
              if (!BEG_STRCASECMP("brightness", mod)) {
                iml->rmod->brightness = (int) strtol(valptr, (char **) NULL, 0);
              } else if (!BEG_STRCASECMP("contrast", mod)) {
                iml->rmod->contrast = (int) strtol(valptr, (char **) NULL, 0);
              } else if (!BEG_STRCASECMP("gamma", mod)) {
                iml->rmod->gamma = (int) strtol(valptr, (char **) NULL, 0);
              }
              update_cmod(iml->rmod);
              reload_image(iml);
              update_cmod_tables(iml);

            } else if (!strcasecmp(color, "green")) {
              imlib_t *iml = images[which].current->iml;

              if (iml->gmod == NULL) {
                iml->gmod = create_colormod();
              }
              if (!BEG_STRCASECMP("brightness", mod)) {
                iml->gmod->brightness = (int) strtol(valptr, (char **) NULL, 0);
              } else if (!BEG_STRCASECMP("contrast", mod)) {
                iml->gmod->contrast = (int) strtol(valptr, (char **) NULL, 0);
              } else if (!BEG_STRCASECMP("gamma", mod)) {
                iml->gmod->gamma = (int) strtol(valptr, (char **) NULL, 0);
              }
              update_cmod(iml->gmod);
              reload_image(iml);
              update_cmod_tables(iml);

            } else if (!strcasecmp(color, "blue")) {
              imlib_t *iml = images[which].current->iml;

              if (iml->bmod == NULL) {
                iml->bmod = create_colormod();
              }
              if (!BEG_STRCASECMP("brightness", mod)) {
                iml->bmod->brightness = (int) strtol(valptr, (char **) NULL, 0);
              } else if (!BEG_STRCASECMP("contrast", mod)) {
                iml->bmod->contrast = (int) strtol(valptr, (char **) NULL, 0);
              } else if (!BEG_STRCASECMP("gamma", mod)) {
                iml->bmod->gamma = (int) strtol(valptr, (char **) NULL, 0);
              }
              update_cmod(iml->bmod);
              reload_image(iml);
              update_cmod_tables(iml);
            }
          }
          if (changed) {
            redraw_all_images();
          }
	  break;
        case 2:
          changed = 0;
          which = image_max;
          if ((nstr = (char *) strsep(&tnstr, ";")) == NULL || (valptr = (char *) strsep(&tnstr, ";")) == NULL) {
            break;
          }
          FOREACH_IMAGE(if (!strcasecmp(valptr, (get_image_type(idx) + 6))) {which = idx; break;});
          if (which != image_max) {
            if ((valptr = (char *) strsep(&tnstr, ";")) == NULL) {
              break;
            }
          } else {
            which = image_bg;
          }
          D_PIXMAP(("Operation == \"%s\", which == %d, value == \"%s\"\n", nstr, (int) which, valptr));
          if (!strcasecmp(nstr, "shade")) {
            imlib_t *iml = images[which].current->iml;
            int s;

            s = (int) strtol(valptr, (char **) NULL, 0);
            s = ((100 - s) << 8) / 100;
            if (s == 0x100) {
              if (iml->mod != NULL) {
                if (iml->mod->brightness != 0x100) {
                  iml->mod->brightness = 0x100;
                  changed = 1;
                }
                if (iml->mod->contrast == 0x100 && iml->mod->gamma == 0x100) {
                  FREE(iml->mod);
                }
              }
            } else {
              if (iml->mod == NULL) {
                iml->mod = create_colormod();
              }
              if (iml->mod->brightness != s) {
                iml->mod->brightness = s;
                changed = 1;
              }
            }
          } else if (!strcasecmp(nstr, "tint")) {
            imlib_t *iml = images[which].current->iml;
            unsigned long t, r, g, b;

            if (!isdigit(*valptr)) {
              t = get_tint_by_color_name(valptr);
            } else {
              t = (unsigned long) strtoul(valptr, (char **) NULL, 0);
              D_PIXMAP(("Got numerical tint 0x%06x\n", t));
            }
            r = (t & 0xff0000) >> 16;
            if (r == 0xff) {
              if (iml->rmod != NULL) {
                if (iml->rmod->brightness != 0x100) {
                  iml->rmod->brightness = 0x100;
                  changed = 1;
                  if (iml->rmod->contrast == 0x100 && iml->rmod->gamma == 0x100) {
                    FREE(iml->rmod);
                  }
                }
              }
            } else {
              if (iml->rmod == NULL) {
                iml->rmod = create_colormod();
              }
              if (iml->rmod->brightness != (int) r) {
                iml->rmod->brightness = r;
                changed = 1;
              }
            }
            g = (t & 0xff00) >> 8;
            if (g == 0xff) {
              if (iml->gmod != NULL) {
                if (iml->gmod->brightness != 0x100) {
                  iml->gmod->brightness = 0x100;
                  changed = 1;
                  if (iml->gmod->contrast == 0x100 && iml->gmod->gamma == 0x100) {
                    FREE(iml->gmod);
                  }
                }
              }
            } else {
              if (iml->gmod == NULL) {
                iml->gmod = create_colormod();
              }
              if (iml->gmod->brightness != (int) g) {
                iml->gmod->brightness = g;
                changed = 1;
              }
            }
            b = t & 0xff;
            if (b == 0xff) {
              if (iml->bmod != NULL) {
                if (iml->bmod->brightness != 0x100) {
                  iml->bmod->brightness = 0x100;
                  changed = 1;
                  if (iml->bmod->contrast == 0x100 && iml->bmod->gamma == 0x100) {
                    FREE(iml->bmod);
                  }
                }
              }
            } else {
              if (iml->bmod == NULL) {
                iml->bmod = create_colormod();
                iml->bmod->contrast = iml->bmod->gamma = 0x100;
              }
              if (iml->bmod->brightness != (int) b) {
                iml->bmod->brightness = b;
                changed = 1;
              }
            }
          }
          if (changed) {
            if (image_mode_is(which, MODE_TRANS)) {
              free_desktop_pixmap();
            }
            redraw_image(which);
          }
	  break;
	case 3:
# ifdef PIXMAP_OFFSET
          get_desktop_window();
          if (desktop_window == None) {
            FOREACH_IMAGE(if (image_mode_is(idx, MODE_TRANS)) {image_set_mode(idx, MODE_IMAGE); image_allow_mode(idx, ALLOW_IMAGE);});
            break;
          }
          get_desktop_pixmap();
          redraw_images_by_mode(MODE_TRANS | MODE_VIEWPORT);
# endif
	  break;
#endif
	case 10:
	  nstr = (char *) strsep(&tnstr, ";");
	  if (nstr && *nstr) {
	    if (!strcasecmp(nstr, "xterm")) {
#ifdef XTERM_SCROLLBAR
	      scrollbar_change_type(SCROLLBAR_XTERM);
#else
	      print_error("Support for xterm scrollbars was not compiled in.  Sorry.\n");
#endif
	    } else if (!strcasecmp(nstr, "next")) {
#ifdef NEXT_SCROLLBAR
	      scrollbar_change_type(SCROLLBAR_NEXT);
#else
	      print_error("Support for NeXT scrollbars was not compiled in.  Sorry.\n");
#endif
	    } else if (!strcasecmp(nstr, "motif")) {
#ifdef MOTIF_SCROLLBAR
	      scrollbar_change_type(SCROLLBAR_MOTIF);
#else
	      print_error("Support for motif scrollbars was not compiled in.  Sorry.\n");
#endif
	    } else {
	      print_error("Unrecognized scrollbar type \"%s\".\n", nstr);
	    }
	  }
	  nstr = (char *) strsep(&tnstr, ";");
	  if (nstr && *nstr) {
            scrollbar_change_width((unsigned short) strtoul(nstr, (char **) NULL, 0));
	  }
	  break;
	case 11:
	  nstr = (char *) strsep(&tnstr, ";");
	  OPT_SET_OR_TOGGLE(nstr, Options, Opt_scrollbar_right);
          scr_touch();
          parent_resize();
	  break;
	case 12:
	  nstr = (char *) strsep(&tnstr, ";");
	  OPT_SET_OR_TOGGLE(nstr, Options, Opt_scrollbar_floating);
	  scrollbar_reposition_and_always_draw();
	  break;
	case 13:
	  nstr = (char *) strsep(&tnstr, ";");
	  OPT_SET_OR_TOGGLE(nstr, Options, Opt_scrollbar_popup);
	  break;
	case 14:
	  nstr = (char *) strsep(&tnstr, ";");
          if (!(nstr) || !(*(nstr))) {
            bbar_show_all(-1);
            parent_resize();
          } else if (BOOL_OPT_ISTRUE(nstr)) {
            bbar_show_all(1);
            parent_resize();
          } else if (BOOL_OPT_ISFALSE(nstr)) {
            bbar_show_all(0);
            parent_resize();
          }
	  break;
	case 20:
	  nstr = (char *) strsep(&tnstr, ";");
	  OPT_SET_OR_TOGGLE(nstr, Options, Opt_visual_bell);
	  break;
#ifdef MAPALERT_OPTION
	case 21:
	  nstr = (char *) strsep(&tnstr, ";");
	  OPT_SET_OR_TOGGLE(nstr, Options, Opt_map_alert);
	  break;
#endif
	case 22:
	  nstr = (char *) strsep(&tnstr, ";");
	  OPT_SET_OR_TOGGLE(nstr, Options, Opt_xterm_select);
	  break;
	case 23:
	  nstr = (char *) strsep(&tnstr, ";");
	  OPT_SET_OR_TOGGLE(nstr, Options, Opt_select_whole_line);
	  break;
	case 24:
	  nstr = (char *) strsep(&tnstr, ";");
          FOREACH_IMAGE(if (!image_mode_is(idx, MODE_VIEWPORT) && image_mode_is(idx, ALLOW_VIEWPORT)) {image_set_mode(idx, MODE_VIEWPORT);});
          redraw_images_by_mode(MODE_VIEWPORT);
	  break;
	case 25:
	  nstr = (char *) strsep(&tnstr, ";");
	  OPT_SET_OR_TOGGLE(nstr, Options, Opt_select_trailing_spaces);
	  break;
	case 26:
	  nstr = (char *) strsep(&tnstr, ";");
	  OPT_SET_OR_TOGGLE(nstr, Options, Opt_report_as_keysyms);
	  break;
	case 27:
	  nstr = (char *) strsep(&tnstr, ";");
	  OPT_SET_OR_TOGGLE(nstr, Options, Opt_no_input);
          wm_hints = XGetWMHints(Xdisplay, TermWin.parent);
          wm_hints->flags |= InputHint;
          wm_hints->input = ((Options & Opt_no_input) ? False : True);
          XSetWMHints(Xdisplay, TermWin.parent, wm_hints);
          XFree(wm_hints);
	  break;
	case 40:
	  nstr = (char *) strsep(&tnstr, ";");
	  if (nstr) {
	    if (XParseColor(Xdisplay, cmap, nstr, &xcol) && XAllocColor(Xdisplay, cmap, &xcol)) {
	      PixColors[fgColor] = xcol.pixel;
	      scr_refresh(DEFAULT_REFRESH);
	    }
	  }
	  break;
	case 41:
	  nstr = (char *) strsep(&tnstr, ";");
	  if (nstr) {
	    if (XParseColor(Xdisplay, cmap, nstr, &xcol) && XAllocColor(Xdisplay, cmap, &xcol)) {
	      PixColors[bgColor] = xcol.pixel;
	      scr_refresh(DEFAULT_REFRESH);
	    }
	  }
	  break;
	case 50:
	  /* Change desktops */
	  nstr = (char *) strsep(&tnstr, ";");
	  if (nstr && *nstr) {
	    XClientMessageEvent xev;

	    rs_desktop = (int) strtol(nstr, (char **) NULL, 0);
	    xev.type = ClientMessage;
	    xev.window = TermWin.parent;
	    xev.message_type = props[PROP_DESKTOP];
	    xev.format = 32;
	    xev.data.l[0] = rs_desktop;
	    XChangeProperty(Xdisplay, TermWin.parent, xev.message_type, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &rs_desktop, 1);
	    XSendEvent(Xdisplay, Xroot, False, SubstructureNotifyMask, (XEvent *) & xev);
	  }
	  break;
#if 0
	case 70:
	  /* Exit Eterm */
	  exit(0);
	  break;
	case 71:
	  /* Save current config */
	  nstr = (char *) strsep(&tnstr, ";");
	  if (nstr && *nstr) {
            valptr = (char *) strsep(&tnstr, ";");
            if (!strcasecmp(nstr, "theme")) {
              save_config(valptr, SAVE_THEME_CONFIG);
            } else {
              save_config(valptr, SAVE_USER_CONFIG);
            }
	  } else {
	    save_config(NULL, SAVE_USER_CONFIG);
	  }
	  break;
#endif
	case 72:
	  /* Search scrollback buffer for a string.  NULL to clear. */
	  nstr = (char *) strsep(&tnstr, ";");
	  if (nstr && *nstr) {
	    scr_search_scrollback(nstr);
	  } else {
	    scr_search_scrollback(NULL);
	  }
	  break;
#if 0
	case 73:
	  /* Spawn a subprogram */
	  nstr = (char *) strsep(&tnstr, ";");
	  if (nstr && *nstr) {
	    system_no_wait(nstr);
	  }
	  break;
#endif
	case 80:
	  /* Set debugging level */
	  nstr = (char *) strsep(&tnstr, ";");
	  if (nstr && *nstr) {
	    DEBUG_LEVEL = (unsigned int) strtoul(nstr, (char **) NULL, 0);
	  }
	  break;

	default:
	  break;
      }
      break;

    case XTerm_Pixmap:
#ifdef PIXMAP_SUPPORT
      FOREACH_IMAGE(if (!image_mode_is(idx, MODE_IMAGE) && image_mode_is(idx, ALLOW_IMAGE)) {image_set_mode(idx, MODE_IMAGE);});
      if (!strcmp(str, ";")) {
        image_set_mode(image_bg, MODE_SOLID);
	bg_needs_update = 1;
      } else {
	nstr = (char *) strsep(&tnstr, ";");
	if (nstr) {
	  if (*nstr) {
	    set_pixmap_scale("", images[image_bg].current->pmap);
	    bg_needs_update = 1;
	    load_image(nstr, images[image_bg].current);
	  }
	  while ((nstr = (char *) strsep(&tnstr, ";")) && *nstr) {
	    changed += set_pixmap_scale(nstr, images[image_bg].current->pmap);
	    scaled = 1;
	  }
	} else {
          image_set_mode(image_bg, MODE_SOLID);
	  bg_needs_update = 1;
	}
      }
      if ((changed) || (bg_needs_update)) {
        redraw_image(image_bg);
      }
#endif /* PIXMAP_SUPPORT */
      break;

    case XTerm_restoreFG:
#ifdef XTERM_COLOR_CHANGE
      set_window_color(fgColor, str);
#endif
      break;
    case XTerm_restoreBG:
#ifdef XTERM_COLOR_CHANGE
      set_window_color(bgColor, str);
#endif
      break;
    case XTerm_DumpScreen:
      nstr = (char *) strsep(&tnstr, ";");
      if (nstr && *nstr) {
        scr_dump_to_file(nstr);
      }
      break;
    case XTerm_logfile:
      nstr = (char *) strsep(&tnstr, ";");
      if (nstr && *nstr && BOOL_OPT_ISTRUE(nstr)) {
        /* Logging on */
      } else {
        /* Logging off */
      }
      break;
    case XTerm_font:
      change_font(0, str);
      break;
    default:
      D_CMD(("Unsupported xterm escape sequence operator:  0x%02x\n", op));
      break;
  }
#ifdef PIXMAP_SUPPORT
  FREE(orig_tnstr);
#endif
}
