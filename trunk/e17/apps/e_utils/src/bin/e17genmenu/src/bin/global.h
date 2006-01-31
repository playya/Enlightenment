#ifndef GLOBAL_H
#define GLOBAL_H

#define EFAVDIR "/.e/e/applications/favorite"
#define EAPPDIR "/.e/e/applications/all"
#define DEFAULTICON PACKAGE_DATA_DIR"/icons/e.png"

typedef struct _G_Eap G_Eap;
struct _G_Eap
{
   char *eap_name;
   char *name;
   char *generic;
   char *comment;
   char *categories;
   char *exec;
   char *icon;
   char *startup;
   char *type;
   char *window_class;
};

/* Function Prototypes */
char *get_home(void);
char *get_desktop_dir(void);
int get_overwrite(void);
char *get_eap_name(char *file);
char *get_icon_size(void);
char *get_icon_theme(void);
int get_mapping(void);
void backup_eaps(void);
void write_mapping_file(G_Eap *eap);
char *get_icon_compression();

#endif
