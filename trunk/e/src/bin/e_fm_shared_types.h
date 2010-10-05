#ifndef E_FM_SHARED_TYPES
#define E_FM_SHARED_TYPES

#include <Eina.h>
#include <Ecore.h>
#include <E_DBus.h>

# define E_DEVICE_TYPE_STORAGE 1
# define E_DEVICE_TYPE_VOLUME  2
typedef struct _E_Storage E_Storage;
typedef struct _E_Volume  E_Volume;
typedef struct _E_Fm2_Mount  E_Fm2_Mount;

#ifndef HAVE_EEZE_MOUNT
struct _E_Storage
{
   int type;
   const char *udi, *bus;
   const char *drive_type;

   const char *model, *vendor, *serial;

   Eina_Bool removable;
   Eina_Bool media_available;
   unsigned long long media_size;

   Eina_Bool requires_eject;
   Eina_Bool hotpluggable;
   Eina_Bool media_check_enabled;

   struct 
     {
        const char *drive, *volume;
     } icon;

   Eina_List *volumes;

   Eina_Bool validated : 1;
   Eina_Bool trackable : 1;
};

struct _E_Volume
{
   int type;
   const char *udi, *uuid;
   const char *label, *icon, *fstype;
   unsigned long long size;

   Eina_Bool partition;
   int partition_number;
   const char *partition_label;
   Eina_Bool mounted;
   const char *mount_point;

   const char *parent;
   E_Storage *storage;
   void *prop_handler;
   Eina_List *mounts;

   Eina_Bool validated : 1;

   Eina_Bool auto_unmount : 1;                  // unmount, when last associated fm window closed
   Eina_Bool first_time;                    // volume discovery in init sequence
   Ecore_Timer *guard;                 // operation guard timer
   DBusPendingCall *op;                // d-bus call handle
};

struct _E_Fm2_Mount
{
   const char *udi;
   const char *mount_point;

   Ecore_Cb mount_ok;
   Ecore_Cb mount_fail;
   Ecore_Cb unmount_ok;
   Ecore_Cb unmount_fail;
   void *data;

   E_Volume *volume;

   Eina_Bool mounted : 1;
};

#endif
#endif
