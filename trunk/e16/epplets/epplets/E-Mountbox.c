/* E-Mountbox.c
 *
 * Copyright (C) 1999-2000 Christian Kreibich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "E-Mountbox.h"

static void
error_exit(void)
{
  Esync();
  Epplet_cleanup();
  exit(1);
}


void
UpdateGraphics(void)
{
  /* ok, this is cheap. */
  FreeMounts();
  FreeMountPointTypes();
  FreeImages();

  SetupDefaults();
  SetupMounts();
  SetupGraphx();
}


void
ConfigShowMore(void)
{
  Epplet_gadget_hide(button_add_long);
  Epplet_gadget_show(tbox_key);
  Epplet_gadget_show(tbox_file);
  Epplet_gadget_show(arrow_left);
  Epplet_gadget_show(button_add);
  Epplet_gadget_show(button_del);
  Epplet_gadget_show(arrow_right);
  Epplet_gadget_show(label_key);
  Epplet_gadget_show(label_file);
}


void
ConfigShowLess(void)
{
  Epplet_gadget_show(button_add_long);
  Epplet_gadget_hide(tbox_key);
  Epplet_gadget_hide(tbox_file);
  Epplet_gadget_hide(arrow_left);
  Epplet_gadget_hide(button_add);
  Epplet_gadget_hide(button_del);
  Epplet_gadget_hide(arrow_right);
  Epplet_gadget_hide(label_key);
  Epplet_gadget_hide(label_file);
}


static void
CallbackShowMore(void *data)
{
  if (!is_shown)
    {
      Epplet_gadget_show(button_help);
      Epplet_gadget_show(button_config);
      Epplet_gadget_show(button_close);
    }
  else
    {
      Epplet_gadget_hide(button_help);
      Epplet_gadget_hide(button_config);
      Epplet_gadget_hide(button_close);
    }
  is_shown = !(is_shown);
  return;
  data = NULL;
}

/* don't need that right now.

static void
CallbackEnter(void *data, Window w)
{
  Epplet_gadget_show(button_more);
  return;
  data = NULL;
  w = (Window) 0;
}

static void
CallbackLeave(void *data, Window w)
{
  Epplet_gadget_hide(button_more);
  return;
  data = NULL;
  w = (Window) 0;
}

*/

static void
CallbackKeyPress(void *data, Window win, char *key)
{
  if (key)
    {
      if (!strcmp(key, "Left"))
	{
	  CallbackSlideLeft(NULL);
	}
      else if (!strcmp(key, "Right"))
	{
	  CallbackSlideRight(NULL);
	}
    }
  return;
  data = NULL;
  win = 0;
}

static void
CallbackHelp(void *data)
{
  Epplet_show_about("E-Mountbox");
  CallbackShowMore(NULL);
  return;
  data = NULL;
}


static void
Callback_ConfigOK(void *data)
{
  Callback_DefaultChange(NULL);
  Callback_BGChange(NULL);
  Callback_TypeChange(NULL);
  SyncConfigs();
  Epplet_save_config();
  UpdateGraphics();
  
  Epplet_window_hide (config_win);
  config_win = 0;
  return;
  data = NULL;
}


static void
Callback_ConfigApply(void *data)
{
  Callback_DefaultChange(NULL);
  Callback_BGChange(NULL);
  Callback_TypeChange(NULL);
  SyncConfigs();
  UpdateGraphics();

  /* Oh man. Of course current_type is now invalid. Banging head ... */
  current_type = types;
  if (current_type)
    {
      Epplet_change_textbox(tbox_key, current_type->key);
      Epplet_change_textbox(tbox_file, current_type->imagefile);
    }
  else
    {
      Epplet_change_textbox(tbox_key, "");
      Epplet_change_textbox(tbox_file, "");
    }
  return;
  data = NULL;
}


static void
Callback_ConfigCancel(void *data)
{
  Epplet_load_config();
  UpdateGraphics();
  Epplet_window_hide (config_win);
  config_win = 0;
  return;
  data = NULL;
}


static void
Callback_DefaultChange(void *data)
{
  Epplet_modify_config("DEFAULT", Epplet_textbox_contents(tbox_default));
  return;
  data = NULL;
}


static void
Callback_BGChange(void *data)
{
  Epplet_modify_config("BG_IMAGE", Epplet_textbox_contents(tbox_bg));
  return;
  data = NULL;
}


static void
Callback_TypeChange(void *data)
{
  ModifyMountPointType(current_type, Epplet_textbox_contents(tbox_key),
		       Epplet_textbox_contents(tbox_file));
  return;
  data = NULL;
}


static void
Callback_ConfigLeft(void *data)
{
  if (current_type)
    {
      if (current_type->prev)
	{
	  ModifyMountPointType(current_type, Epplet_textbox_contents(tbox_key),
			       Epplet_textbox_contents(tbox_file));
	  current_type = current_type->prev;
	  Epplet_change_textbox(tbox_key, current_type->key);
	  Epplet_change_textbox(tbox_file, current_type->imagefile);
	}
    }
  return;
  data = NULL;
}


static void
Callback_ConfigRight(void *data)
{
  if (current_type)
    {
      if (current_type->next)
	{
	  ModifyMountPointType(current_type, Epplet_textbox_contents(tbox_key),
			       Epplet_textbox_contents(tbox_file));
	  current_type = current_type->next;
	  Epplet_change_textbox(tbox_key, current_type->key);
	  Epplet_change_textbox(tbox_file, current_type->imagefile);
	}
    }
  return;
  data = NULL;
}


static void
Callback_ConfigAdd(void *data)
{
  if (num_types == 0)
    {
      ConfigShowMore();
    }
  ModifyMountPointType(current_type, Epplet_textbox_contents(tbox_key),
		       Epplet_textbox_contents(tbox_file));
  AddMountPointType(NULL, NULL);
  current_type = types;
  Epplet_reset_textbox(tbox_key);
  Epplet_reset_textbox(tbox_file);
  
  return;
  data = NULL;
}


static void
Callback_ConfigDel(void *data)
{
  if (current_type)
    {
      if (current_type->next)
	{
	  current_type = current_type->next;
	  DeleteMountPointType(current_type->prev);
	}
      else if (current_type->prev)
	{
	  current_type = current_type->prev;
	  DeleteMountPointType(current_type->next);
	}
      else
	{
	  DeleteMountPointType(current_type);
	  current_type = NULL;
	}
    }

  if (current_type)
    {
      Epplet_change_textbox(tbox_key, current_type->key);
      Epplet_change_textbox(tbox_file, current_type->imagefile);
    }
  else
    {
      ConfigShowLess();
    }

  return;
  data = NULL;
}


static void
CallbackConfigure(void *data)
{
  current_type = types;

  if (!config_win)
    {
      config_win =
	Epplet_create_window_config (420, 190 , "E-Mountbox Configuration",
				     Callback_ConfigOK, &config_win,
				     Callback_ConfigApply, &config_win,
				     Callback_ConfigCancel, &config_win);
      
      Epplet_gadget_show (Epplet_create_label (12, 10,
					       "Default icon",
					       2));
      Epplet_gadget_show ((tbox_default = Epplet_create_textbox (NULL, Epplet_query_config("DEFAULT"),
								 10, 23, 400, 20,
								 2, Callback_DefaultChange, NULL)));
      
      Epplet_gadget_show (Epplet_create_label (12, 55,
					       "Background",
					       2));
      Epplet_gadget_show ((tbox_bg = Epplet_create_textbox (NULL, Epplet_query_config("BG_IMAGE"),
							    10, 68, 400, 20,
							    2, Callback_BGChange, NULL)));
      
      label_key = Epplet_create_label (12, 100, "Pattern", 2);
      label_file = Epplet_create_label (72, 100, "Image file", 2);
      if (current_type)
	{
	  tbox_key  = Epplet_create_textbox(NULL, current_type->key, 10, 113, 60, 20, 2, Callback_TypeChange, NULL);
	  tbox_file = Epplet_create_textbox(NULL, current_type->imagefile, 70, 113, 340, 20, 2, Callback_TypeChange, NULL);
	}
      else
	{
	  tbox_key  = Epplet_create_textbox(NULL, "", 10, 113, 60, 20, 2, Callback_TypeChange, NULL);
	  tbox_file = Epplet_create_textbox(NULL, "", 70, 113, 340, 20, 2, Callback_TypeChange, NULL);
	}
      arrow_left = Epplet_create_button(NULL, NULL, 170, 140, 0, 0, "ARROW_LEFT", 0, NULL, Callback_ConfigLeft, NULL);
      button_add = Epplet_create_button("Add", NULL, 187, 140, 24, 12, NULL, 0, NULL, Callback_ConfigAdd, NULL);
      button_add_long = Epplet_create_button("Add mountpoint type", NULL, 165, 120, 110, 16, NULL, 0, NULL, Callback_ConfigAdd, NULL);
      button_del = Epplet_create_button("Delete", NULL, 216, 140, 36, 12, NULL, 0, NULL, Callback_ConfigDel, NULL);
      arrow_right = Epplet_create_button(NULL, NULL, 257, 140, 0, 0, "ARROW_RIGHT", 0, NULL, Callback_ConfigRight, NULL);
      
      Epplet_window_pop_context ();
    }

  if (current_type)
    {
      ConfigShowMore();
    }
  else
    {
      ConfigShowLess();
    }

  Epplet_window_show (config_win);

  CallbackShowMore(NULL);
  return;
  data = NULL;
}


static void
CallbackAnimate(void *data)
{
  static double k = 0.0;
  static double step = M_PI / 10.0;

  int i, j, linear, linear_w;
  double ratio;

  if (k < M_PI)
    {
      if (anim_mount)
	ratio = ((cos(k) + 1.0)/ 4.0) * 1.3;
      else
	ratio = ((cos(k + M_PI) + 1.0)/ 4.0) * 1.3;

      for (i=0; i<32; i++)
	{
	  for (j=0; j<44; j++)
	    {
	      if (!IsTransparent(current_tile->image, j, i))
		{
		  linear = 3*(i * 44 + j);
		  linear_w = (i*44*3*num_tiles) + (current_tile_index*44*3) + 3*j;
		    window_buf->im->rgb_data[linear] =
		    widescreen_buf->im->rgb_data[linear_w] = 
		      ratio * (widescreen_canvas_buf->im->rgb_data[linear_w])
		      + (1.0-ratio) * (current_tile->image->rgb_data[linear]);
		    window_buf->im->rgb_data[linear+1] =
		    widescreen_buf->im->rgb_data[linear_w+1] = 
		      ratio * (widescreen_canvas_buf->im->rgb_data[linear_w+1])
		      + (1.0-ratio) * (current_tile->image->rgb_data[linear+1]);
		    window_buf->im->rgb_data[linear+2] =
		    widescreen_buf->im->rgb_data[linear_w+2] = 
		      ratio * (widescreen_canvas_buf->im->rgb_data[linear_w+2])
		      + (1.0-ratio) * (current_tile->image->rgb_data[linear+2]);
		}
	    } 
	}
      Epplet_paste_buf(window_buf, Epplet_get_drawingarea_window(action_area), -2, -2);
      k += step;
      Epplet_timer(CallbackAnimate, NULL, 0.05, "Anim");
    }
  else
    k = 0.0;

  return;
  data = NULL;
}


void
UpdateView(int dir, int fast)
{
  int i,j;
  double start_t, delta_t, wait;
  double step = M_PI / 44;

  if (dir == 0)
    {
      for (i=0; i<32; i++)
	{
	  memcpy(window_buf->im->rgb_data + i * 44 * 3,
		 widescreen_buf->im->rgb_data + (i*44*3*num_tiles) + (current_tile_index*44*3),
		 44*3*sizeof(unsigned char));
	}
      Epplet_paste_buf(window_buf, Epplet_get_drawingarea_window(action_area), -2, -2);
    }
  else
    {
      for (j=0; j<=44; j++)
	{
	  if (!fast)
	    {
	      start_t = Epplet_get_time();
	      wait = fabs(cos(j*step)) / 100.0;
	      while ((delta_t = Epplet_get_time() - start_t) < wait) ;
	    }
	  for (i=0; i<32; i++)
	    {
	      memcpy(window_buf->im->rgb_data + i * 44 * 3,
		     widescreen_buf->im->rgb_data + (i*44*3*num_tiles) + (current_tile_index*44*3) + (dir)*j*3,
		     44*3*sizeof(unsigned char));
	    }
	  Epplet_paste_buf(window_buf, Epplet_get_drawingarea_window(action_area), -2, -2);
	}
    }
}


int
IsTransparent(ImlibImage *im, int x, int y)
{
  int        linear;
  ImlibColor ic;

  if (!im || x < 0 || y < 0 || x >= im->rgb_width || y >= im->rgb_height)
    return 0;

  Imlib_get_image_shape(id, im, &ic);
  if ((ic.r == -1) && (ic.g == -1) && (ic.b == -1))
    return 0;

  linear = 3*(y * im->rgb_width + x);

  if ((im->rgb_data[linear] == ic.r)
      && (im->rgb_data[linear+1] == ic.g)
      && (im->rgb_data[linear+2] == ic.b))
    return 1;

  return 0;
}


/* mount handling */
void        
SetupMounts(void)
{

  /* first, parse /etc/fstab to see what user-mountable mountpoints we have */
  if (!(ParseFstab()))
    {
      /* Couldn't read /etc/fstab */
      Epplet_dialog_ok("Could not read mountpoint information.");
      error_exit();
    }
  
  /* do we have user-mountable fs's at all? */
  if (num_tiles == 0)
    {
      Epplet_dialog_ok("Could not find any usable mountpoints.");
      error_exit();
    }

  /* now, check if these are actually mounted already */

  /* check for /proc/mounts */
  if (!(ParseProcMounts()))
    /* no? ok, check for /etc/mtab */
    if (!(ParseEtcMtab()))
      /* damnit, look if files exist in the mountpoints. */
      VisitMountPoints();

  /* Man, this code has far too many comments :) */
}


void            
AddMountPoint(char *device, char *path)
{
  Tile           *newtile = NULL;
  char           *tmp_dev = NULL;
  char           *tmp_path = NULL;
  int             i;
  MountPointType *type = NULL;
  char           *s = NULL;
  ImlibImage     *tmp_image = NULL;

  if (!tiles)
    {
      tiles = (Tile*)malloc(sizeof(Tile));
      if (tiles)
	{
	  memset(tiles, 0, sizeof(Tile));
	  num_tiles = 1;
	  current_tile = tiles;
	}
    }
  else
    {
      newtile = (Tile*)malloc(sizeof(Tile));
      if (newtile)
	{
	  memset(newtile, 0, sizeof(Tile));
	  newtile->next = tiles;
	  tiles->prev = newtile;
	  current_tile = tiles = newtile;
	  num_tiles++;
	}
    }
  current_tile_index = 0;

  if (current_tile)
    {
      current_tile->mountpoint = (MountPoint*)malloc(sizeof(MountPoint));
      if (current_tile->mountpoint)
	{
	  memset(current_tile->mountpoint, 0, sizeof(MountPoint));
	  if (device)
	    current_tile->mountpoint->device = strdup(device);
	  if (path)
	    current_tile->mountpoint->path = strdup(path);
	}
      if (device && path)
	{
	  tmp_path = strdup(path);
	  tmp_dev = strdup(device);
	  for (i=0; i<(int)strlen(tmp_path); i++)
	    tmp_path[i] = (char)(tolower(tmp_path[i]));
	  for (i=0; i<(int)strlen(tmp_dev); i++)
	    tmp_dev[i] = (char)(tolower(tmp_dev[i]));
	  if (tmp_path && tmp_dev)
	    {
	      type = types;
	      while (type)
		{
		  if (strstr(tmp_dev, type->key))
		    {
		      current_tile->image = type->image;
		      break;
		    }
		  else if (strstr(tmp_path, type->key))
		    {
		      current_tile->image = type->image;
		      break;
		    }
		  type = type->next;
		}
	      
	      if (current_tile->image == NULL)
		{
		  s = Epplet_query_config("DEFAULT");

		  if (!default_image)
		    {
		      tmp_image = Imlib_load_image(id, s);  
		      if (!tmp_image)
			tmp_image = Imlib_load_image(id, __DEFAULT);  
		      if (!tmp_image)
			{
			  Epplet_dialog_ok("  E-Mountbox could not load a default icon\n  "
					   "  for the mountpoints. Check your installation.  ");
			  error_exit();
			}
		      default_image = Imlib_clone_scaled_image(id, tmp_image, 44, 32);
		      Imlib_destroy_image(id, tmp_image);
		    }
		  current_tile->image = default_image;
		}

	      free(tmp_path);
	      free(tmp_dev);
	    }
	} 
    }
}


void            
AddMountPointType(char *key, char *image)
{
  MountPointType *newtype = NULL;
  ImlibImage     *tmp_image = NULL;

  if (!types)
    {
      types = (MountPointType*)malloc(sizeof(MountPointType));
      if (types)
	{
	  memset(types, 0, sizeof(MountPointType));
	  num_types = 1;
	}
    }
  else
    {
      newtype = (MountPointType*)malloc(sizeof(MountPointType));
      if (newtype)
	{
	  memset(newtype, 0, sizeof(MountPointType));
	  newtype->next = types;
	  types->prev = newtype;
	  types = newtype;
	  num_types++;
	}
    }

  if (types)
    {
      if ((types->key == NULL) && (types->image == NULL))
	{
	  if (key)
	    types->key = strdup(key);
	  if (image)
	    types->imagefile = strdup(image);
	  tmp_image = Imlib_load_image(id, image);  
	  if (tmp_image)
	    {
	      types->image = Imlib_clone_scaled_image(id, tmp_image, 44, 32);
	      Imlib_destroy_image(id, tmp_image);
	    }
	}
    }
}


void
ModifyMountPointType(MountPointType *mpt, char *key, char *imagefile)
{

  if (mpt)
    {
      if (key)
	{
	  if (mpt->key)
	    free(mpt->key);
	  mpt->key = strdup(key);
	}
      if (imagefile)
	{
	  if (mpt->imagefile)
	    free(mpt->imagefile);
	  mpt->imagefile = strdup(imagefile);
	}
    }
}


void
DeleteMountPointType(MountPointType *mpt)
{
  if (mpt)
    {
      /* is it in the middle */
      if (mpt->next && mpt->prev)
	{
	  mpt->prev->next = mpt->next;
	  mpt->next->prev = mpt->prev;
	}
      /* or at the beginning */
      else if (mpt->next)
	{
	  mpt->next->prev = NULL;
	  types = mpt->next;
	}
      /* or at the end ... */
      else if (mpt->prev) 
	{
	  mpt->prev->next = NULL;
	}

      num_types--;
      if (num_types == 0)
	{
	  types = NULL;
	}

      /* free it */
      if (mpt->key)
	{
	  free(mpt->key);
	}
      if (mpt->imagefile)
	{
	  free(mpt->imagefile);
	}
      if (mpt->image)
	{
	  Imlib_destroy_image(id, mpt->image);
	  mpt->image = NULL;
	}
      free(mpt);
    }
}


void
FreeImages(void)
{
  if (bg_image)
    {
      Imlib_destroy_image(id, bg_image);
      bg_image = NULL;
    }
  if (default_image)
    {
      Imlib_destroy_image(id, default_image);
      default_image = NULL;
    }
}


void        
FreeMounts(void)
{
  Tile *current, *tmp;

  current = tiles;
  while (current)
    {
      if (current->mountpoint)
	{
	  if (current->mountpoint->device)
	    free(current->mountpoint->device);
	  if (current->mountpoint->path)
	    free(current->mountpoint->path);
	  free(current->mountpoint);
	}
      /* images need _not_ be freed here */
      tmp = current;
      current = current->next;
      free(tmp);
    }
  tiles = NULL;
  num_tiles = 0;
}


void        
FreeMountPointTypes(void)
{
  MountPointType *current, *tmp;

  current = types;
  while (current)
    {
      if (current->key)
	{
	  free(current->key);
	}
      if (current->imagefile)
	{
	  free(current->imagefile);
	}
      if (current->image)
	{
	  Imlib_destroy_image(id, current->image);
	  current->image = NULL;
	}
      tmp = current;
      current = current->next;
      free(tmp);
    }
  types = NULL;
  num_types = 0;
}


void        
Mount(MountPoint * mp)
{
  char s[1024];

  if (mp)
    {
      if (mp->mounted)
	return;
      if (mp->path)
	{
	  Esnprintf(s, sizeof(s), "%s %s", MOUNT_CMD, mp->path);
	  if (!Epplet_run_command(s))
	    {
	      mp->mounted = 1;
	      anim_mount = 1;
	      Epplet_timer(CallbackAnimate, NULL, 0, "Anim");
	    }
	  else
	    {
	      s[0] = 0;
	      Esnprintf(s, sizeof(s), "Could not mount %s.", mp->path);
	      Epplet_dialog_ok(s);
	    }
	}
    }
}


void        
Umount(MountPoint * mp)
{
  char s[1024];

  s[0] = 0;

  if (mp)
    {
      if (!(mp->mounted))
	return;
      if (mp->path)
	{
	  Esnprintf(s, sizeof(s), "%s %s", UMOUNT_CMD, mp->path);
	  if (!Epplet_run_command(s))
	    {
	      mp->mounted = 0;
	      anim_mount = 0;
	      Epplet_timer(CallbackAnimate, NULL, 0, "Anim");
	    }
	  else
	    {
	      s[0] = 0;
	      Esnprintf(s, sizeof(s), "Could not unmount %s.", mp->path);
	      Epplet_dialog_ok(s);
	    }
	}
    }
}


int        
ParseFstab(void)
{
  FILE *f;
  char  s[1024];
  char *token = NULL;
  char *info[4];
  int   i;

  if ((f = fopen(FSTAB, "r")) == NULL)
    return 0;
  *s = 0;
  for (; fgets(s, sizeof(s), f);)
    {
      /* skip comments and blank lines */
      if (!(*s) || (*s == '\n') || (*s == '#'))
        {
          continue;
        }

      /* parse out tokens we need */
      i = 0;
      token = strtok(s, " \t");
      if (token)
	{
	  info[i++] = strdup(token);
	}
      while((token = strtok(NULL, " \t")))
	{
	  info[i++] = strdup(token);
	}

     /* see if device is user-mountable */
      if (strstr(info[3], "user"))
	  {
	    AddMountPoint(info[0], info[1]);
	  }

      for (i=0; i<5; i++)
	if (info[i])
	  free(info[i]);
    }

  fclose(f);
  return 1;
}


int        
ParseProcMounts(void)
{
  FILE *f;
  char  s[1024];
  char *token = NULL, *device = NULL, *path = NULL;
  Tile *tile;

  if ((f = fopen(PROCMOUNTS, "r")) == NULL)
    return 0;
  *s = 0;
  for (; fgets(s, sizeof(s), f);)
    {
      /* skip comments and blank lines (shouldn't be there, actually ...)*/
      if (!(*s) || (*s == '\n') || (*s == '#'))
        {
          continue;
        }

      /* parse out tokens we need */
      token = strtok(s, " \t");
      if (token)
	{
	  device = strdup(token);
	}
      token = NULL;
      token = strtok(NULL, " \t");
      if (token)
	{
	  path = strdup(token);
	}

      /* set that device mounted in our list ... */
      tile = tiles;
      while (tile)
	{
	  if (tile->mountpoint)
	    {
	      if (!strcmp(tile->mountpoint->path, path))
		{
		  tile->mountpoint->mounted = 1;
		}
	    }
	  tile = tile->next;
	}
      
      if (device)
	free(device);
      if (path)
	free(path);
    }

  fclose(f);
  return 1;
}


int        
ParseEtcMtab(void)
{
  FILE *f;
  char  s[1024];
  char *token = NULL, *device = NULL, *path = NULL;
  Tile *tile;

  if ((f = fopen(ETCMTAB, "r")) == NULL)
    return 0;
  *s = 0;
  for (; fgets(s, sizeof(s), f);)
    {
      /* skip comments and blank lines (shouldn't be there, actually ...)*/
      if (!(*s) || (*s == '\n') || (*s == '#'))
        {
          continue;
        }

      /* parse out tokens we need */
      token = strtok(s, " \t");
      if (token)
	{
	  device = strdup(token);
	}
      token = NULL;
      token = strtok(NULL, " \t");
      if (token)
	{
	  path = strdup(token);
	}

      /* set that device mounted in our list ... */
      tile = tiles;
      while (tile)
	{
	  if (tile->mountpoint)
	    {
	      if (!strcmp(tile->mountpoint->path, path))
		{
		  tile->mountpoint->mounted = 1;
		}
	    }
	  tile = tile->next;
	}
      
      if (device)
	free(device);
      if (path)
	free(path);
    }

  fclose(f);
  return 1;
}


void
VisitMountPoints(void)
{
  DIR            *dir;
  int             num_entries;
  struct dirent  *dp;
  Tile           *tile = NULL;
  
  tile = tiles;
  while (tile)
    {
      if (tile->mountpoint)
	{
	  if (tile->mountpoint->path)
	    {
	      dir = NULL;
	      dir = opendir(tile->mountpoint->path);
	      if (dir)
		{
		  num_entries = 0;
		  for (num_entries = 0; (dp = readdir(dir)) != NULL; num_entries++);
		  if (num_entries > 2)
		    {
		      tile->mountpoint->mounted = 1;
		    }
		}
	    }
	}
      tile = tile->next;
    }
}


MountPoint *
FindMountPointByClick(int x, int y)
{
  if (!IsTransparent(current_tile->image, x, y))
    return current_tile->mountpoint;

  return NULL;
}


static void     
CallbackExit(void * data)
{
  data = NULL;
  FreeMounts();
  FreeMountPointTypes();
  FreeImages();
  Epplet_unremember();
  Esync();
  Epplet_cleanup();
  exit(0);
}


static void
CallbackButtonUp (void *data, Window win, int x, int y, int b)
{
  MountPoint *mountpoint = NULL;
  char        s[1024];

  if (win == Epplet_get_drawingarea_window(action_area))
    {
      mountpoint = FindMountPointByClick(x,y);
      if (mountpoint)
	{
	  if (b == 1)
	    {
	      if (mountpoint->mounted)
		Umount(mountpoint);
	      else
		Mount(mountpoint);
	    }
	  else
	    {
	      Epplet_gadget popup = Epplet_create_popup();

	      s[0] = 0;
	      Esnprintf(s, sizeof(s), "%s at %s.", mountpoint->device, mountpoint->path);
	      Epplet_add_popup_entry(popup, s, NULL, NULL, NULL);
	      Epplet_pop_popup(popup, 0);
	    }
	}
    }
  return;
  data = NULL;
}


static void
CallbackSlideLeft(void *data)
{
  if (current_tile->prev)
    {
      UpdateView(-1, 0);
      current_tile = current_tile->prev;
      current_tile_index--;
    }
  else
    {
      while (current_tile->next)
	{
	  UpdateView(+1, 1);
	  current_tile = current_tile->next;
	  current_tile_index++;
	}
    }
  return;
  data = NULL;
}


static void
CallbackSlideRight(void *data)
{
  if (current_tile->next)
    {
      UpdateView(1, 0);
      current_tile = current_tile->next;
      current_tile_index++;
    }
  else
    {
      while (current_tile->prev)
	{
	  UpdateView(-1, 1);
	  current_tile = current_tile->prev;
	  current_tile_index--;
	}
    }
  return;
  data = NULL;
}


static void     
CallbackExpose(void *data, Window win, int x, int y, int w, int h)
{
  UpdateView(0, 0);
  return;
  data = NULL;
  win = x = y = w = h = 0;
}

void
SetupDefaults(void)
{
  int    i, instance, num_results;
  char  *s, s2[256], *key = NULL, *image = NULL, *token;
  char **results = NULL;
  
  for (i=0; i<(int)(sizeof(defaults)/sizeof(ConfigItem)); i++)
    {
      if (!Epplet_query_config(defaults[i].key))
	Epplet_add_config(defaults[i].key, defaults[i].value);
    }
  
  instance = atoi(Epplet_query_config_def("INSTANCE", "0"));
  Esnprintf(s2, sizeof(s), "%i", ++instance);
  Epplet_modify_config("INSTANCE", s2);

  results = Epplet_query_multi_config("TYPEDEF", &num_results);
  if ((!results) && (instance == 1))
    {
      Epplet_modify_multi_config("TYPEDEF", default_types, (int)(sizeof(default_types)/sizeof(char*)));
      results = Epplet_query_multi_config("TYPEDEF", &num_results);
      if (!results)
	{
	  Epplet_dialog_ok("  Could not set up mountpoint types.  \n"
			   "  Check your installation.  \n");
	  error_exit();
	}
    }
  
  for (i = 0; i < num_results; i++)
    {
      if (results[i])
	{
	  s = strdup(results[i]);
	  token = strtok(s, " \t");
	  if (token)
	    key = strdup(token);
	  token = strtok(NULL, " \t");
	  if (token)
	    image = strdup(token);
	  
	  if (key && image)
	    {
	      AddMountPointType(key, image);
	    }
	  free(key);
	  free(image);
	  free(s);
	}
    }
  free(results);
}


void
SetupGraphx(void)
{
  static int  first_time = 1;
  int         i, j, k, linear, linear_w;
  ImlibImage *tmp = NULL;
  Tile       *tile;
  char       *s = NULL;

  s = Epplet_query_config("BG_IMAGE");

  tmp = Imlib_load_image(id, s);  
  if (!tmp)
    tmp = Imlib_load_image(id, __BG_IMAGE);  
  if (!tmp)
    {
      /* Even the fallbacks didn't work.  If we don't exit
         here, we'll seg fault.  -- mej */
      Epplet_dialog_ok("Could not load all images.");
      Esync();
      exit(-1);
    }
  /*
    sscanf(Epplet_query_config("BG_BORDER"), "%i %i %i %i",
    &(border.left), &(border.right), &(border.top), &(border.bottom));    
    Imlib_set_image_border(id, tmp, &border);
  */
  bg_image = Imlib_clone_scaled_image(id, tmp, 44 * num_tiles, 32);
  Imlib_destroy_image(id, tmp);

  /* setup widescreen according to current mounts */
  if (!window_buf)
    window_buf = Epplet_make_rgb_buf(44, 32);  
  if (widescreen_buf)
    Epplet_free_rgb_buf(widescreen_buf);
  widescreen_buf = Epplet_make_rgb_buf((44 * num_tiles), 32);  
  if (widescreen_canvas_buf)
    Epplet_free_rgb_buf(widescreen_canvas_buf);
  widescreen_canvas_buf = Epplet_make_rgb_buf((44 * num_tiles), 32);  

  memcpy(widescreen_buf->im->rgb_data, bg_image->rgb_data,
	 sizeof(unsigned char) * 44 * 3 * num_tiles * 32); 
  memcpy(widescreen_canvas_buf->im->rgb_data, bg_image->rgb_data,
	 sizeof(unsigned char) * 44 * 3 * num_tiles * 32); 

  tile = tiles;
  for (k=0; k<num_tiles; k++, tile = tile->next)
    {
      for (i=0; i<32; i++)
	{
	  for (j=0; j<44; j++)
	    {
	      if (!IsTransparent(tile->image, j, i))
		{
		  linear = 3*(i * 44 + j);
		  linear_w = (i*44*3*num_tiles) + (k*44*3) + 3*j;
		  if (tile->mountpoint->mounted)
		    {
		      widescreen_buf->im->rgb_data[linear_w] = tile->image->rgb_data[linear];
		      widescreen_buf->im->rgb_data[linear_w+1] = tile->image->rgb_data[linear+1];
		      widescreen_buf->im->rgb_data[linear_w+2] = tile->image->rgb_data[linear+2];
		    }
		  else
		    {
		      widescreen_buf->im->rgb_data[linear_w] =
			0.65 * widescreen_buf->im->rgb_data[linear_w] + 0.35 * tile->image->rgb_data[linear];
		      widescreen_buf->im->rgb_data[linear_w+1] =
			0.65 * widescreen_buf->im->rgb_data[linear_w+1] + 0.35 * tile->image->rgb_data[linear+1];
		      widescreen_buf->im->rgb_data[linear_w+2] =
			0.65 * widescreen_buf->im->rgb_data[linear_w+2] + 0.35 * tile->image->rgb_data[linear+2];
		    }
		} 
	    }
	}      
    }

  if (first_time)
    {
      first_time = 0;
      Epplet_gadget_show((Epplet_create_button(NULL, NULL, 
					       2, 34, 0, 0, "ARROW_LEFT", 0, NULL, 
					       CallbackSlideLeft, NULL)));
      Epplet_gadget_show((Epplet_create_button(NULL, NULL, 
					       33, 34, 0, 0, "ARROW_RIGHT", 0, NULL, 
					       CallbackSlideRight, NULL)));
      Epplet_gadget_show((action_area = Epplet_create_drawingarea(2, 2, 44, 32)));
      
      Epplet_gadget_show((Epplet_create_button("...", NULL, 14, 34, 20, 12, NULL, 0, NULL, CallbackShowMore, NULL)));
      button_help = Epplet_create_button(NULL, NULL, 3, 3, 0, 0, "HELP", 0, NULL, CallbackHelp, NULL);
      button_close = Epplet_create_button(NULL, NULL, 33, 3, 0, 0, "CLOSE", 0, NULL, CallbackExit, NULL);
      button_config = Epplet_create_button(NULL, NULL, 18, 3, 0, 0, "CONFIGURE", 0, NULL, CallbackConfigure, NULL);
      
      /*
	Epplet_register_focus_in_handler(CallbackEnter, NULL);
	Epplet_register_focus_out_handler(CallbackLeave, NULL);
      */
      Epplet_register_expose_handler(CallbackExpose, NULL);
      Epplet_register_button_release_handler(CallbackButtonUp, NULL);
      Epplet_register_key_press_handler(CallbackKeyPress, NULL);
      
      /* Setup the current view */
      Epplet_show();
    }

  UpdateView(0, 0);
}


void
SyncConfigs(void)
{
  char          **strings = NULL;
  char            s[1024];
  int             i;
  MountPointType *mpt = NULL;

  strings = (char**)malloc(sizeof(char*) * num_types);
  if (strings)
    {
      for (mpt=types, i=0; mpt; mpt=mpt->next, i++) 
	{
	  Esnprintf(s, sizeof(s), "%s  %s", mpt->key, mpt->imagefile);
	  strings[i] = strdup(s);
	}
      /*
      for (i=0; i<num_types; i++)
	{
	  printf("DEBUG: %s\n", strings[i]);
	  }*/

      Epplet_modify_multi_config("TYPEDEF", strings, num_types);

      for (i=0; i<num_types; i++)
	if (strings[i])
	  free(strings[i]);
      free(strings);
    }
}


int
main(int argc, char** argv)
 {
   atexit(Epplet_cleanup);
   Epplet_Init("E-Mountbox", "0.1", "Enlightenment Mount Epplet",
	       3, 3, argc, argv, 0);
   Epplet_load_config();
   id = Epplet_get_imlib_data();

   SetupDefaults();
   SetupMounts();
   SetupGraphx();

   Epplet_Loop();
   error_exit();
   return 0;
}
