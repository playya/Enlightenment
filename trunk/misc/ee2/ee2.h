/* header file for Electric Eyes 2 */
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <Imlib2.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "config.h"


/* Macros and #defines */
#define CHECKS 16
#define SPLASHSCREEN DATA_DIR "/ee2-alpha.png"
#ifdef DEBUG
# define D(x)  do {printf(__FILE__ ", line %d, " __FUNCTION__ "():  ", __LINE__); printf x;} while (0)
#else
# define D(x)  ((void) 0)
#endif

/* Variables */
extern GtkWidget *MainWindow, *area, *RootMenu, *FileSel, *SaveSel;
extern Imlib_Image *im, *bimg, *bg;
extern int imgw, imgh, winw, winh;
extern char currentimage[];

/* functions */
void about_init(void);
void about_show(void);
void about_hide(void);

void browser_init(void);
void browser_show(void);
void browser_hide(void);
void AddList(char *foo);

void menus_init(void);

void ee2_init(int, char **);
void LoadImage(char *imagetoload);
void DrawChecks(void);
void Checks(int image_h, int image_w);
void DrawImage(Imlib_Image * im, int w, int h);
void CloseWindow(GtkWidget * widget, gpointer data);
void CloseFileSel(GtkWidget * widget, gpointer data);
void FileOpen(GtkWidget * widget, GtkFileSelection * fs);
void SaveImage(GtkWidget * widget, gpointer data);
void CloseSaveSel(GtkWidget * widget, gpointer data);
void SaveImageAs(GtkWidget * widget, GtkFileSelection * fs);
gint ButtonPressed(GtkWidget * widget, GdkEvent * event, gpointer data);
gboolean a_config(GtkWidget * widget, GdkEventConfigure * event, gpointer user_data);
