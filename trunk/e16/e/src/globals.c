
#include "E.h"

pid_t               master_pid;
int                 master_screen;
int                 display_screens;
int                 single_screen_mode;
Display            *disp;
ImlibData          *id;
ImlibData          *ird;
FnlibData          *fd;
List               *lists;
int                 event_base_shape;
Window              comms_win;
Root                root;
int                 (*(ActionFunctions[ACTION_NUMBEROF])) (void *);
EMode               mode;
Desktops            desks;
Window              grab_window;
Window              init_win1 = 0;
Window              init_win2 = 0;
Window              init_win_ext = 0;
Window              bpress_win = 0;
int                 deskorder[ENLIGHTENMENT_CONF_NUM_DESKTOPS];
int                 sound_fd = -1;
char                themepath[FILEPATH_LEN_MAX];
char                themename[FILEPATH_LEN_MAX];
char               *command;
char                mustdel;
char                queue_up;
char                just_flipped = 0;
Menu               *all_task_menu = NULL;
Menu               *task_menu[ENLIGHTENMENT_CONF_NUM_DESKTOPS];
Menu               *desk_menu = NULL;
Menu               *group_menu = NULL;
char                no_overwrite = 0;
Window              external_pager_window = 0;
char                clickmenu = 0;
Window              last_bpress = 0;
int                 child_count = 0;
pid_t              *e_children = NULL;
int                 numlock_mask = 0;
int                 scrollock_mask = 0;
int                 mask_mod_combos[8];
Group              *current_group;
