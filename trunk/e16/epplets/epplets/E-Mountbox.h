#include "epplet.h"
#include <math.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>

#define TYPE_CD 0
#define TYPE_FD 1
#define TYPE_ZIP 2
#define TYPE_JAZZ 3
#define TYPE_HD 4
#define TYPE_BG 5
#define MAXTYPE 6

#define FSTAB      "/etc/fstab"
#define PROCMOUNTS "/proc/mounts"
#define ETCMTAB    "/etc/mtab"
#define MOUNT_CMD  "/bin/mount"
#define UMOUNT_CMD "/bin/umount"

#define __BG_IMAGE EROOT"/epplet_data/E-Mountbox/E-Mountbox-bg.png"
#define __DEFAULT  EROOT"/epplet_data/E-Mountbox/E-Mountbox-blockdev.png"

ConfigItem defaults[] = {
  {"BG_IMAGE", EROOT"/epplet_data/E-Mountbox/E-Mountbox-bg.png"},
  {"DEFAULT", EROOT"/epplet_data/E-Mountbox/E-Mountbox-blockdev.png"}
};

char *default_types[] = {
  "cd   "EROOT"/epplet_data/E-Mountbox/E-Mountbox-cd.png",
  "fd   "EROOT"/epplet_data/E-Mountbox/E-Mountbox-floppy.png",
  "zip  "EROOT"/epplet_data/E-Mountbox/E-Mountbox-zip.png",
  "jazz "EROOT"/epplet_data/E-Mountbox/E-Mountbox-jazz.png"
};

typedef struct _tile Tile;
typedef struct _mountpointtype MountPointType;

struct _mountpointtype
{
  int             config_index;
  char           *key;
  char           *imagefile;
  ImlibImage     *image;
  MountPointType *next;
  MountPointType *prev;
};

typedef struct _mountpoint
{
  char         *device;
  char         *path;
  char          mounted;
}
MountPoint;

struct _tile
{
  ImlibImage   *image;
  MountPoint   *mountpoint;  
  Tile         *prev;
  Tile         *next;
};

Tile           *tiles = NULL;
Tile           *current_tile = NULL;
int             current_tile_index = 0;
int             num_tiles = 0;
MountPointType *types = NULL;
int             num_types = 0;
ImlibData      *id = NULL;
ImlibImage     *bg_image = NULL;
ImlibImage     *default_image = NULL;
Epplet_gadget   action_area, button_close, button_config, button_help;

/* stuff for the config win */
Epplet_gadget   tbox_key, tbox_file, tbox_default, tbox_bg;
Window          config_win = 0;
MountPointType *current_type = NULL;

RGB_buf         window_buf;
RGB_buf         widescreen_buf;
RGB_buf         widescreen_canvas_buf;
char            anim_mount = 0;
int             is_shown = 0;

/* graphx handling */
int             IsTransparent(ImlibImage * im, int x, int y);
void            UpdateView(int dir, int fast);
void            FreeImages(void);

/* mount handling */
void            SetupMounts(void);
void            FreeMounts(void);
void            AddMountPoint(char *device, char *path);
void            FreeMountPointTypes(void);
void            AddMountPointType(int index, char *key, char *image);
void            Mount(MountPoint * mp);
void            Umount(MountPoint * mp);

/* miscellaneous nitty gritty */
int             ParseFstab(void);
int             ParseProcMounts(void);
int             ParseEtcMtab(void);
void            VisitMountPoints(void);
MountPoint     *FindMountPointByClick(int x, int y);

/* callbacks/ handlers */
/*
static void     CallbackEnter(void *data, Window w);
static void     CallbackLeave(void *data, Window w);
*/
static void     CallbackExpose(void *data, Window win, int x, int y, int w, int h);
static void     CallbackButtonUp(void *data, Window win, int x, int y, int b);
static void     CallbackExit(void *data);
static void     CallbackSlideLeft(void *data);
static void     CallbackSlideRight(void *data);
static void     CallbackAnimate(void *data);
static void     CallbackHelp(void *data);
static void     CallbackShowMore(void *data);
static void     Callback_ConfigOK(void *data);
static void     Callback_ConfigApply(void *data);
static void     Callback_ConfigCancel(void *data);
static void     Callback_DefaultChange(void *data);
static void     Callback_BGChange(void *data);
static void     Callback_KeyChange(void *data);
static void     Callback_ConfigLeft(void *data);
static void     Callback_ConfigRight(void *data);
static void     Callback_FileChange(void *data);

/* config stuff */
void            SetupDefaults(void);
void            SetupGraphx(void);
