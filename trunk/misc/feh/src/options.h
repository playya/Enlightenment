/* options.h
 *
 * Copyright (C) 2000 Tom Gilbert
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

#ifndef OPTIONS_H
#define OPTIONS_H

struct __fehoptions
{
   unsigned char multiwindow;
   unsigned char montage;
   unsigned char collage;
   unsigned char index;
   unsigned char index_show_name;
   unsigned char index_show_dim;
   unsigned char index_show_size;
   unsigned char thumbs;
   unsigned char slideshow;
   unsigned char recursive;
   unsigned char output;
   unsigned char verbose;
   unsigned char display;
   unsigned char bg;
   unsigned char alpha;
   unsigned char alpha_level;
   unsigned char aspect;
   unsigned char stretch;
   unsigned char progressive;
   unsigned char keep_http;
   unsigned char borderless;
   unsigned char randomize;
   unsigned char full_screen;
   unsigned char auto_zoom;
   unsigned char draw_filename;
   unsigned char list;
   unsigned char quiet;
   unsigned char preload;
   unsigned char loadables;
   unsigned char unloadables;
   unsigned char reverse;
   unsigned char no_menus;
   unsigned char progress_gran;

   char *output_file;
   char *bg_file;
   char *font;
   char *title_font;
   char *action;
   char *fontpath;
   char *filelistfile;
   char *menu_font;
   char *customlist;
   char *menu_bg;

   unsigned char next_button;
   unsigned char zoom_button;
   unsigned char pan_button;

   int thumb_w;
   int thumb_h;
   int limit_w;
   int limit_h;
   int slideshow_delay;
   int reload;
   int sort;
   unsigned char adjust_reload;

   unsigned char mode;
};

void init_parse_options(int argc, char **argv);
char *feh_string_normalize(char *str);

extern fehoptions opt;

#endif
