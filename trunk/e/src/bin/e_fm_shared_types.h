#ifndef E_FM_SHARED_TYPES
#define E_FM_SHARED_TYPES

#include <Eina.h>
#include <Ecore.h>

#if 0
# include <Eeze.h>
# include <Eeze_Disk.h>
#endif
#if 1
# include <E_DBus.h>
#endif

# define E_DEVICE_TYPE_STORAGE 1
# define E_DEVICE_TYPE_VOLUME  2
typedef struct _E_Storage E_Storage;
typedef struct _E_Volume  E_Volume;
typedef struct _E_Fm2_Mount  E_Fm2_Mount;

typedef enum
{
   EFM_MODE_USING_RASTER_MOUNT,
   EFM_MODE_USING_HAL_MOUNT,
   EFM_MODE_USING_UDISKS_MOUNT,
   EFM_MODE_USING_EEZE_MOUNT
} Efm_Mode;

typedef enum
{
  E_VOLUME_OP_TYPE_NONE,
  E_VOLUME_OP_TYPE_MOUNT,
  E_VOLUME_OP_TYPE_UNMOUNT,
  E_VOLUME_OP_TYPE_EJECT
} E_Volume_Op_Type;

struct _E_Storage
{
   int type;
   const char *udi; /* with eeze, this is actually the syspath */
   const char *drive_type;

   const char *model, *vendor, *serial;

   Eina_Bool removable;
   Eina_Bool media_available;
   Eina_Bool system_internal;
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
#if 0
   Eeze_Disk *disk;
#endif
   const char *bus;
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
   Eina_List *mounts;

   Eina_Bool validated : 1;

   Eina_Bool auto_unmount : 1;                  // unmount, when last associated fm window closed
   Eina_Bool first_time;                    // volume discovery in init sequence
   Ecore_Timer *guard;                 // operation guard timer
   E_Volume_Op_Type optype;
   Efm_Mode efm_mode;

   Eina_Bool encrypted;
   Eina_Bool unlocked;
   
#if 1
   DBusPendingCall *op;                // d-bus call handle
   void *prop_handler;
#endif
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
