/*
 * Copyright (C) 1999, Michael Jennings
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

#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "epplet.h"

extern void Epplet_redraw(void);

#define MAIL_PATH       "/var/spool/mail"
#define MAIL_PROG       "Eterm -t mutt"
#define POLL_INTERVAL   "2.0"
#define NOMAIL_IMAGE    EROOT "/epplet_data/E-Biff/nomail.png"
#define NEWMAIL_IMAGE   EROOT "/epplet_data/E-Biff/newmail.png"
#define SEVEN_IMAGE     EROOT "/epplet_data/E-Biff/7of9.png"

#if 0
#  define D(x) do {printf("%10s | %7d:  [debug] ", __FILE__, __LINE__); printf x; fflush(stdout);} while (0)
#else
#  define D(x) ((void) 0)
#endif
#define BEGMATCH(a, b)  (!strncasecmp((a), (b), (sizeof(b) - 1)))
#define NONULL(x)       ((x) ? (x) : (""))

Epplet_gadget close_button, mp_button, help_button, nomail, newmail, seven, label;
unsigned long new_cnt, total_cnt;
size_t file_size;
time_t file_mtime;
char *folder_path = NULL, *mailprog = MAIL_PROG, *sound = NULL,
  *nomail_image = NOMAIL_IMAGE, *newmail_image = NEWMAIL_IMAGE,
  *seven_image = SEVEN_IMAGE;
int mp_pid = 0;
int beep = 1;
double interval = 2.0;

static void mailcheck_cb(void *data);
static void close_cb(void *data);
static void mailprog_cb(void *data);
static void help_cb(void *data);
static void in_cb(void *data, Window w);
static void out_cb(void *data, Window w);
static void process_conf(void);
extern int mbox_folder_count(char *, int);

static void
mailcheck_cb(void *data)
{
  char label_text[64];

  D(("mailcheck_cb() called.\n"));
  if ((mbox_folder_count(folder_path, 0)) != 0) {
    if (new_cnt != 0) {
      if (new_cnt == 7 && total_cnt == 9) {
        Epplet_gadget_hide(nomail);
        Epplet_gadget_hide(newmail);
        Epplet_gadget_show(seven);
      } else {
        Epplet_gadget_hide(nomail);
        Epplet_gadget_hide(seven);
        Epplet_gadget_show(newmail);
      }
      if (beep) {
        XBell(Epplet_get_display(), 0);
      } else if (sound != NULL) {
        Epplet_run_command(sound);
      }
    } else {
      Epplet_gadget_hide(newmail);
      Epplet_gadget_hide(seven);
      Epplet_gadget_show(nomail);
    }
    Esnprintf(label_text, sizeof(label_text), "%lu / %lu", new_cnt, total_cnt);
    Epplet_change_label(label, label_text);
  }
  Epplet_timer(mailcheck_cb, NULL, interval, "TIMER");
  return;
  data = NULL;
}

static void
close_cb(void *data)
{
  Epplet_unremember();
  Esync();
  exit(0);
  data = NULL;
}

static void
mailprog_cb(void *data)
{
  mp_pid = Epplet_spawn_command(mailprog);
  return;
  data = NULL;
}

static void
help_cb(void *data)
{
  Epplet_show_about("E-Biff");
  return;
  data = NULL;
}

static void
in_cb(void *data, Window w)
{
  if (w == Epplet_get_main_window()) {
    Epplet_gadget_show(close_button);
    Epplet_gadget_show(mp_button);
    Epplet_gadget_show(help_button);
  }
  return;
  data = NULL;
}

static void
out_cb(void *data, Window w)
{
  if (w == Epplet_get_main_window()) {
    Epplet_gadget_hide(close_button);
    Epplet_gadget_hide(mp_button);
    Epplet_gadget_hide(help_button);
  }
  return;
  data = NULL;
}

static void
process_conf(void) {

  char *s;

  s = Epplet_query_config("mailbox");
  folder_path = s;
  s = Epplet_query_config_def("mailprog", MAIL_PROG);
  mailprog = s;
  s = Epplet_query_config_def("interval", POLL_INTERVAL);
  interval = (double) atof(s);
  s = Epplet_query_config_def("beep", "1");
  beep = (!strcasecmp(s, "1"));
  s = Epplet_query_config_def("no_mail_image", NOMAIL_IMAGE);
  nomail_image = s;
  s = Epplet_query_config_def("new_mail_image", NEWMAIL_IMAGE);
  newmail_image = s;
  s = Epplet_query_config_def("seven_image", SEVEN_IMAGE);
  seven_image = s;
  s = Epplet_query_config("sound");
  sound = s;
}

int
main(int argc, char **argv)
{
  int prio;

  prio = getpriority(PRIO_PROCESS, getpid());
  setpriority(PRIO_PROCESS, getpid(), prio + 10);
  atexit(Epplet_cleanup);
  Epplet_Init("E-Biff", "0.5", "Enlightenment Mailbox Checker Epplet", 3, 3, argc, argv, 0);
  Epplet_load_config();
  process_conf();
  if (folder_path == NULL) {
    if ((folder_path = getenv("MAIL")) == NULL) {
      char *username = getenv("LOGNAME");

      if (!username) {
        username = getenv("USER");
        if (!username) {
          return -1;
        }
      }
      folder_path = (char *) malloc(sizeof(MAIL_PATH "/") + strlen(username) + 1);
      Esnprintf(folder_path, sizeof(folder_path), MAIL_PATH "/%s", username);
      D(("Generated folder path of \"%s\"\n", folder_path));
    }
    Epplet_modify_config("mailbox", folder_path);
  }
  close_button = Epplet_create_button(NULL, NULL, 2, 2, 0, 0, "CLOSE", 0, NULL, close_cb, NULL);
  help_button = Epplet_create_button(NULL, NULL, 18, 2, 0, 0, "HELP", 0, NULL, help_cb, NULL);
  mp_button = Epplet_create_button(NULL, NULL, 34, 2, 0, 0, "CONFIGURE", 0, NULL, mailprog_cb, NULL);

  nomail = Epplet_create_image(2, 3, 44, 30, nomail_image);
  newmail = Epplet_create_image(2, 3, 44, 30, newmail_image);
  seven = Epplet_create_image(2, 3, 44, 30, seven_image);
  Epplet_gadget_show(nomail);

  label = Epplet_create_label(6, 34, "- / -", 2);
  Epplet_gadget_show(label);
  Epplet_show();

  Epplet_register_focus_in_handler(in_cb, NULL);
  Epplet_register_focus_out_handler(out_cb, NULL);
  mailcheck_cb(NULL);  /* Set everything up */
  Epplet_Loop();

  return 0;
}
