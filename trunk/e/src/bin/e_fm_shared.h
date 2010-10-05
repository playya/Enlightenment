#ifdef E_FM_SHARED_DATATYPES

# define E_DEVICE_TYPE_STORAGE 1
# define E_DEVICE_TYPE_VOLUME  2
typedef struct _E_Storage E_Storage;
typedef struct _E_Volume  E_Volume;
typedef struct _E_Fm2_Mount  E_Fm2_Mount;

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

#ifdef E_FM_SHARED_CODEC
static Eet_Data_Descriptor *_e_volume_edd = NULL;
static Eet_Data_Descriptor *_e_storage_edd = NULL;

static void
_e_volume_free(E_Volume *v)
{
   if (v->storage)
     {
	v->storage->volumes = eina_list_remove(v->storage->volumes, v);
	v->storage = NULL;
     }
   if (v->udi) eina_stringshare_del(v->udi);
   if (v->uuid) eina_stringshare_del(v->uuid);
   if (v->label) eina_stringshare_del(v->label);
   if (v->icon) eina_stringshare_del(v->icon);
   if (v->fstype) eina_stringshare_del(v->fstype);
   if (v->partition_label) eina_stringshare_del(v->partition_label);
   if (v->mount_point) eina_stringshare_del(v->mount_point);
   if (v->parent) eina_stringshare_del(v->parent);
   free(v);
}

static void
_e_storage_free(E_Storage *s)
{
   E_Volume *v;
   EINA_LIST_FREE(s->volumes, v)
     {
        v->storage = NULL;
	_e_volume_free(v);
     }
   if (s->udi) eina_stringshare_del(s->udi);
   if (s->bus) eina_stringshare_del(s->bus);
   if (s->drive_type) eina_stringshare_del(s->drive_type);
   if (s->model) eina_stringshare_del(s->model);
   if (s->vendor) eina_stringshare_del(s->vendor);
   if (s->serial) eina_stringshare_del(s->serial);
   if (s->icon.drive) eina_stringshare_del(s->icon.drive);
   if (s->icon.volume) eina_stringshare_del(s->icon.volume);
   free(s);
}

static Eet_Data_Descriptor *
_e_volume_edd_new(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;

   if (!eet_eina_stream_data_descriptor_class_set(&eddc, sizeof(eddc), "e_volume", sizeof(E_Volume)))
     return NULL;

//   eddc.func.str_alloc = (char *(*)(const char *)) strdup;
//   eddc.func.str_free = (void (*)(const char *)) free;

   edd = eet_data_descriptor_stream_new(&eddc);
#define DAT(MEMBER, TYPE) EET_DATA_DESCRIPTOR_ADD_BASIC(edd, E_Volume, #MEMBER, MEMBER, EET_T_##TYPE)
   DAT(type, INT);
   DAT(udi, STRING);
   DAT(uuid, STRING);
   DAT(label, STRING);
   DAT(fstype, STRING);
   DAT(size, ULONG_LONG);
   DAT(partition, CHAR);
   DAT(partition_number, INT);
   DAT(partition_label, STRING);
   DAT(mounted, CHAR);
   DAT(mount_point, STRING);
   DAT(parent, STRING);
   DAT(first_time, CHAR);
#undef DAT
   return edd;
}

static Eet_Data_Descriptor *
_e_storage_edd_new(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;

   if (!eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc), "e_storage", sizeof (E_Storage)))
     return NULL;

//   eddc.func.str_alloc = (char *(*)(const char *)) strdup;
//   eddc.func.str_free = (void (*)(const char *)) free;

   edd = eet_data_descriptor_stream_new(&eddc);
#define DAT(MEMBER, TYPE) EET_DATA_DESCRIPTOR_ADD_BASIC(edd, E_Storage, #MEMBER, MEMBER, EET_T_##TYPE)
   DAT(type, INT);
   DAT(udi, STRING);
   DAT(bus, STRING);
   DAT(drive_type, STRING);
   DAT(model, STRING);
   DAT(vendor, STRING);
   DAT(serial, STRING);
   DAT(removable, CHAR);
   DAT(media_available, CHAR);
   DAT(media_size, ULONG_LONG);
   DAT(requires_eject, CHAR);
   DAT(hotpluggable, CHAR);
   DAT(media_check_enabled, CHAR);
   DAT(icon.drive, STRING);
   DAT(icon.volume, STRING);
#undef DAT
   return edd;
}

static void
_e_storage_volume_edd_init(void)
{
   _e_volume_edd = _e_volume_edd_new();
   _e_storage_edd = _e_storage_edd_new();
}

static void
_e_storage_volume_edd_shutdown(void)
{
   if (_e_volume_edd)
     {
	eet_data_descriptor_free(_e_volume_edd);
	_e_volume_edd = NULL;
     }
   if (_e_storage_edd)
     {
	eet_data_descriptor_free(_e_storage_edd);
	_e_storage_edd = NULL;
     }
}

#endif
