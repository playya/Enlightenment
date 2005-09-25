/*
 * Copyright (C) 2000-2005 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2005 Kim Woelders
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
#include "buttons.h"
#include "conf.h"
#include "menus.h"
#include "tooltips.h"
#include <ctype.h>

#define SKIP_If_EXISTS(name, type) \
   if (ConfigSkipIfExists(ConfigFile, name, type)) return

static void
SkipTillEnd(FILE * ConfigFile)
{
   char                s[FILEPATH_LEN_MAX];
   int                 i1, i2, fields;

   while (GetLine(s, sizeof(s), ConfigFile))
     {
	i1 = i2 = 0;
	fields = sscanf(s, "%i %i", &i1, &i2);
	if (i1 == CONFIG_CLOSE)
	   return;
	if (i2 == CONFIG_OPEN)
	   SkipTillEnd(ConfigFile);
     }
}

int
ConfigSkipIfExists(FILE * ConfigFile, const char *name, int type)
{
   if (FindItem(name, 0, LIST_FINDBY_NAME, type))
     {
	SkipTillEnd(ConfigFile);
	return 1;
     }
   return 0;
}

static int
IsWhitespace(const char *s)
{
   int                 i = 0;

   while (s[i])
     {
	if ((s[i] != ' ') && (s[i] != '\n') && (s[i] != '\t'))
	   return 0;
	i++;
     }
   return 1;
}

#define LINE_BUFFER_SIZE 1024
/*
 * This function will get a single line from the file
 * The string will be null terminated.
 * Size must be >= 2.
 */
char               *
GetLine(char *s, int size, FILE * f)
{
   char               *si, *so, ch, quote, escape;
   static char        *buffer = NULL;
   static char        *bufptr = NULL;

   si = bufptr;
   so = s;
   quote = '\0';
   escape = '\0';
   for (;;)
     {
	if (buffer == NULL)
	  {
	     buffer = Emalloc(LINE_BUFFER_SIZE);
	     if (buffer == NULL)
		return NULL;
	  }

	/* Get a line from the input file */
	if (si == NULL)
	  {
	     si = fgets(buffer, LINE_BUFFER_SIZE - 1, f);
	     buffer[LINE_BUFFER_SIZE - 1] = '\0';
	     if (si == NULL)
	       {
		  /* EOF */
		  Efree(buffer);
		  buffer = bufptr = NULL;
		  return NULL;
	       }
	  }

	/* Split on ';' or '\n', handle quoting */
	for (; si;)
	  {
	     ch = *si++;
	     switch (ch)
	       {
	       case '\0':
		  si = NULL;
		  if (so == s)	/* Skip empty lines */
		     break;
		  goto case_eol;
	       case ';':	/* Line separator */
		  if (escape || quote)
		     goto case_char;
	       case '\n':
		  if (so == s)	/* Skip empty lines */
		     break;
		case_eol:
		  *so = '\0';	/* Terminate and return */
		  goto done;
	       case '\r':	/* Ignore */
		  break;
	       case '\\':	/* Escape */
		  if (escape)
		     goto case_char;
		  escape = ch;
		  break;
	       case '"':	/* Quoting */
/*	       case '\'': */
		  if (escape)
		     goto case_char;
		  if (quote == '\0')
		     quote = ch;
		  else if (quote == ch)
		     quote = '\0';
		  else
		     goto case_char;
		  break;
	       case ' ':	/* Whitespace */
	       case '\t':
		  if (so == s)	/* Skip leading whitespace */
		     break;
		case_char:	/* Normal character */
	       default:
		  *so++ = ch;
		  escape = '\0';
		  if (--size > 1)
		     break;
		  *so = '\0';
		  goto done;
	       }
	  }
     }

 done:
   bufptr = si;

   /* Strip trailing whitespace */
   si = so;
   for (; so > s; so--)
     {
	ch = so[-1];
	if (ch != ' ' && ch != '\t')
	   break;
     }
   if (so != si)
      *so = '\0';

   if (EventDebug(EDBUG_TYPE_CONFIG))
      Eprintf("GetLine %s\n", s);

   return s;
}

void
ConfigAlertLoad(const char *txt)
{
   RecoverUserConfig();
   Alert(_("Warning:  Configuration error in %s block.\n"
	   "Outcome is likely not good.\n"), txt);
}

static int
ConfigFilePreparse(const char *path, const char *dest)
{
   char                execline[FILEPATH_LEN_MAX];
   const char         *epp_path = ENLIGHTENMENT_BIN "/epp";
   char               *def_home, *def_user, *def_shell;

   if (EventDebug(EDBUG_TYPE_CONFIG))
      Eprintf("ConfigFilePreparse %s->%s\n", path, dest);

   def_home = homedir(getuid());
   def_user = username(getuid());
   def_shell = usershell(getuid());

   Esnprintf(execline, sizeof(execline), "%s " "-P " "-nostdinc " "-undef "
	     "-include %s/config/definitions " "-I%s " "-I%s/config "
	     "-D ENLIGHTENMENT_VERSION=%s " "-D ENLIGHTENMENT_ROOT=%s "
	     "-D ENLIGHTENMENT_BIN=%s "
	     "-D ENLIGHTENMENT_THEME=%s " "-D ECONFDIR=%s "
	     "-D ECACHEDIR=%s " "-D SCREEN_RESOLUTION_%ix%i=1 "
	     "-D SCREEN_WIDTH_%i=1 " "-D SCREEN_HEIGHT_%i=1 "
	     "-D SCREEN_DEPTH_%i=1 " "-D USER_NAME=%s " "-D HOME_DIR=%s "
	     "-D USER_SHELL=%s " "-D ENLIGHTENMENT_VERSION_015=1 "
	     "%s %s",
	     epp_path, EDirRoot(), Mode.theme.path, EDirRoot(),
	     ENLIGHTENMENT_VERSION, EDirRoot(), EDirBin(),
	     Mode.theme.path, EDirUser(), EDirUserCache(), VRoot.w,
	     VRoot.h, VRoot.w, VRoot.h, VRoot.depth, def_user, def_home,
	     def_shell, path, dest);
   system(execline);

   if (def_user)
      Efree(def_user);
   if (def_shell)
      Efree(def_shell);
   if (def_home)
      Efree(def_home);

   return exists(dest) ? 0 : 1;
}

/* Split the process of finding the file from the process of loading it */
int
ConfigFileRead(FILE * fs)
{
   int                 err;
   int                 i1, i2, fields;
   char                s[FILEPATH_LEN_MAX];
   int                 e_cfg_ver = 0;
   int                 max_e_cfg_ver = 1;

   while (GetLine(s, sizeof(s), fs))
     {
	if (IsWhitespace(s))
	   continue;

	i1 = i2 = CONFIG_INVALID;
	fields = sscanf(s, "%i %i", &i1, &i2);

	if (fields < 1)
	   i1 = CONFIG_INVALID;
	else if (i1 == CONFIG_VERSION)
	  {
	     if (fields == 2)
		e_cfg_ver = i2;
	  }
	else if (i1 == CONFIG_CLOSE)
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
	if (i2 == CONFIG_OPEN)
	  {
	     if (e_cfg_ver > max_e_cfg_ver)
	       {
		  AlertX(_("Theme versioning ERROR"),
			 _("Restart with Defaults"), " ",
			 _("Abort and Exit"),
			 _("ERROR:\n" "\n"
			   "The configuration for the theme you are running is\n"
			   "incompatible. It's config revision is %i.\n"
			   "It needs to be marked as being revision <= %i\n"
			   "\n"
			   "Please contact the theme author or maintainer and\n"
			   "inform them that in order for their theme to function\n"
			   "with this version of Enlightenment, they have to\n"
			   "update it to the current settings, and then match\n"
			   "the revision number.\n" "\n"
			   "If the theme revision is higher than Enlightenment's\n"
			   "it may be that you haven't upgraded Enlightenment for\n"
			   "a while and this theme takes advantages of new\n"
			   "features in Enlightenment in new versions.\n"),
			 e_cfg_ver, max_e_cfg_ver);
		  SessionExit(EEXIT_THEME, "DEFAULT");
	       }
	     else
	       {
		  switch (i1)
		    {
		    case CONFIG_CLOSE:
		       goto done;

		    case CONFIG_IMAGECLASS:
		       err = ImageclassConfigLoad(fs);
		       if (err)
			  ConfigAlertLoad(_("Image class"));
		       break;
		    case CONFIG_TOOLTIP:
		       err = TooltipConfigLoad(fs);
		       if (err)
			  ConfigAlertLoad(_("Tooltip"));
		       break;
		    case CONFIG_FONTS:
		       err = FontConfigLoad(fs);
		       if (err)
			  ConfigAlertLoad(_("Font"));
		       break;
		    case CONFIG_TEXT:
		       err = TextclassConfigLoad(fs);
		       if (err)
			  ConfigAlertLoad(_("Text class"));
		       break;
		    case MENU_STYLE:
		       err = MenuStyleConfigLoad(fs);
		       if (err)
			  ConfigAlertLoad(_("Menu style"));
		       break;
		    case CONFIG_BORDER:
		       err = BorderConfigLoad(fs);
		       if (err)
			  ConfigAlertLoad(_("Border"));
		       break;
		    case CONFIG_BUTTON:
		       err = ButtonsConfigLoad(fs);
		       if (err)
			  ConfigAlertLoad(_("Button"));
		       break;
		    case CONFIG_DESKTOP:
		       err = BackgroundsConfigLoad(fs);
		       if (err)
			  ConfigAlertLoad(_("Background"));
		       break;
		    case CONFIG_WINDOWMATCH:
		       err = WindowMatchConfigLoad(fs);
		       if (err)
			  ConfigAlertLoad(_("Window match"));
		       break;
		    case CONFIG_COLORMOD:
#if ENABLE_COLOR_MODIFIERS
		       Config_ColorModifier(fs);
#endif
		       break;
		    case CONFIG_ACTIONCLASS:
		       err = AclassConfigLoad(fs);
		       if (err)
			  ConfigAlertLoad(_("Action class"));
		       break;
		    case CONFIG_SLIDEOUT:
		       err = SlideoutsConfigLoad(fs);
		       if (err)
			  ConfigAlertLoad(_("Slideout"));
		       break;
		    default:
		       break;
		    }
	       }
	  }
     }

 done:
   return 0;
}

static char        *
FindFileLocalized(const char *name, const char *path, int localized)
{
   char                s[FILEPATH_LEN_MAX];
   const char         *lang;
   char               *p[4];
   int                 i, len;

   if (path)
      len = Esnprintf(s, sizeof(s), "%s/%s", path, name);
   else
      len = Esnprintf(s, sizeof(s), "%s", name);
   if (len <= 0)
      return NULL;

   lang = Mode.locale.lang;
   if (!localized || !lang)
     {
	if (isfile(s) && canread(s))	/* FIXME - Only one stat */
	   return Estrdup(s);
	else
	   return NULL;
     }

   if (len + 1 + strlen(lang) >= sizeof(s))
      return NULL;

   s[len] = '.';
   strcpy(s + len + 1, lang);

   p[0] = s + len + 1 + strlen(lang);	/* .da_DK.UTF-8 */
   p[1] = strchr(s + len + 1, '.');	/* .da_DK       */
   p[2] = strchr(s + len + 1, '_');	/* .da          */
   p[3] = s + len;

   for (i = 0; i < 4; i++)
     {
	if (p[i] == NULL)
	   continue;

	*p[i] = '\0';
	if (isfile(s) && canread(s))	/* FIXME - Only one stat */
	   return Estrdup(s);
     }

   return NULL;
}

char               *
FindFile(const char *file, const char *themepath, int localized)
{
   char                s[FILEPATH_LEN_MAX];
   char               *p;

   /* if absolute path - and file exists - return it */
   if (isabspath(file))
     {
	p = FindFileLocalized(file, NULL, localized);
	if (p)
	   return p;
     }

   /* look in ~/.e16 first */
   p = FindFileLocalized(file, EDirUser(), localized);
   if (p)
      return p;

   if (themepath)
     {
	/* look in theme dir */
	p = FindFileLocalized(file, themepath, localized);
	if (p)
	   return p;
     }

   /* look in system config dir */
   Esnprintf(s, sizeof(s), "%s/config", EDirRoot());
   p = FindFileLocalized(file, s, localized);
   if (p)
      return p;

   /* not found.... NULL */
   return NULL;
}

char               *
ThemeFileFind(const char *file, int localized)
{
   return FindFile(file, Mode.theme.path, localized);
}

char               *
ConfigFileFind(const char *name, const char *themepath, int pp)
{
   char                s[FILEPATH_LEN_MAX];
   char               *fullname, *file, *ppfile;
   int                 i, err;

   fullname = FindFile(name, themepath, 1);
   if (!fullname)
      return NULL;

   /* Quit if not preparsing */
   if (!pp)
      return fullname;

   /* The file exists. Now check the preparsed one. */
   file = Estrdup(fullname);
   for (i = 0; file[i]; i++)
      if (file[i] == '/')
	 file[i] = '.';

   Esnprintf(s, sizeof(s), "%s/cached/cfg/%s.preparsed", EDirUserCache(), file);
   ppfile = Estrdup(s);
   if (exists(s) && moddate(s) > moddate(fullname))
      goto done;

   /* No preparesd file or source is newer. Do preparsing. */
   err = ConfigFilePreparse(fullname, ppfile);
   if (err)
     {
	Efree(ppfile);
	ppfile = NULL;
     }

 done:
   Efree(fullname);
   Efree(file);
   return ppfile;
}

int
ConfigFileLoad(const char *name, const char *themepath,
	       int (*parse) (FILE * fs), int preparse)
{
   int                 err = -1;
   char               *file;
   FILE               *fs;

   if (EventDebug(EDBUG_TYPE_CONFIG))
      Eprintf("ConfigFileLoad %s\n", name);

   file = ConfigFileFind(name, themepath, preparse);
   if (!file)
      goto done;

   fs = fopen(file, "r");
   Efree(file);
   if (!fs)
      goto done;

   err = parse(fs);

   fclose(fs);

 done:
   return err;
}

int
ThemeConfigLoad(void)
{
   static const char  *const config_files[] = {
      "fonts.cfg",
      "init.cfg",
      "textclasses.cfg",
      "backup-textclasses.cfg",
#if ENABLE_COLOR_MODIFIERS
      "colormodifiers.cfg",
      "backup-colormodifiers.cfg",
#endif
      "imageclasses.cfg",
      "backup-imageclasses.cfg",
      "desktops.cfg",
      "actionclasses.cfg",
      "backup-cursors.cfg",
      "buttons.cfg",
      "slideouts.cfg",
      "borders.cfg",
      "backup-borders.cfg",
      "windowmatches.cfg",
      "tooltips.cfg",
      "backup-tooltips.cfg",
      "menustyles.cfg",
   };
   char                s[FILEPATH_LEN_MAX];
   Progressbar        *p = NULL;
   int                 i;

   Esnprintf(s, sizeof(s), "%s/", Mode.theme.path);

   for (i = 0; i < (int)(sizeof(config_files) / sizeof(char *)); i++)

     {
	if (!Mode.wm.restart)
	  {
	     if (i == 2)
		StartupWindowsCreate();

	     if ((i > 1) && (!p))
	       {
		  p = ProgressbarCreate(_("Enlightenment Starting..."), 400,
					16);
		  if (p)
		     ProgressbarShow(p);
	       }
	  }

	ConfigFileLoad(config_files[i], Mode.theme.path, ConfigFileRead, 1);

	if (p)
	   ProgressbarSet(p, (i * 100) /
			  (int)(sizeof(config_files) / sizeof(char *)));
     }

   if (p)
      ProgressbarDestroy(p);

   /* Font mappings no longer needed */
   FontConfigUnload();

   /* Loose ends... */
   BordersSetupFallback();

   return 0;
}

void
RecoverUserConfig(void)
{
   int                 save;

   /* Don't save settings if we restart or quit */
   save = Conf.autosave;
   Conf.autosave = 0;

   AlertX(_("Recover system config?"), _("Yes, Attempt recovery"),
	  _("Restart and try again"), _("Quit and give up"),
	  _
	  ("Enlightenment has encountered parsing errors in your autosaved\n"
	   "configuration.\n" "\n"
	   "This may be due to filing system errors, Minor bugs or "
	   "unforeseen\n" "system shutdowns.\n" "\n"
	   "Do you wish Enlightenment to recover its original system\n"
	   "configuration and try again?\n"));

   /* Allow settings to be saved if we continue */
   Conf.autosave = save;
}
