#ifndef EEZE_DISK_H
#define EEZE_DISK_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#else
# define EAPI
#endif

#include <Eina.h>
#include <Ecore.h>

/**
 * @file Eeze_Disk.h
 * @brief Disk manipulation
 *
 * Eeze disk functions allow you to quickly and efficiently manipulate disks
 * through simple function calls.
 *
 * @addtogroup disk Disk
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
   EEZE_DISK_TYPE_UNKNOWN = 0, /**< type could not be determined */
   EEZE_DISK_TYPE_INTERNAL = 1, /**< internal drive */
   EEZE_DISK_TYPE_CDROM = 2, /**< cdrom drive */
   EEZE_DISK_TYPE_USB = 4 /**< usb drive */
} Eeze_Disk_Type;

typedef enum
{
#define EEZE_DISK_MOUNTOPT_DEFAULTS (EEZE_DISK_MOUNTOPT_UTF8 | EEZE_DISK_MOUNTOPT_NOEXEC | EEZE_DISK_MOUNTOPT_NOSUID)
   EEZE_DISK_MOUNTOPT_LOOP = (1 << 1),
   EEZE_DISK_MOUNTOPT_UTF8 = (1 << 2),
   EEZE_DISK_MOUNTOPT_NOEXEC = (1 << 3),
   EEZE_DISK_MOUNTOPT_NOSUID = (1 << 4),
   EEZE_DISK_MOUNTOPT_REMOUNT = (1 << 5),
   EEZE_DISK_MOUNTOPT_UID = (1 << 6) /**< use current user's uid */
} Eeze_Mount_Opts;


EAPI extern int EEZE_EVENT_DISK_MOUNT;
EAPI extern int EEZE_EVENT_DISK_UNMOUNT;
EAPI extern int EEZE_EVENT_DISK_ERROR;

typedef struct _Eeze_Event_Disk_Mount Eeze_Event_Disk_Mount;
typedef struct _Eeze_Event_Disk_Unmount Eeze_Event_Disk_Unmount;
typedef struct _Eeze_Disk Eeze_Disk;

struct _Eeze_Event_Disk_Mount
{
   Eeze_Disk *disk;
};

struct _Eeze_Event_Disk_Unmount
{
   Eeze_Disk *disk;
};


typedef struct _Eeze_Event_Disk_Error Eeze_Event_Disk_Error;

struct _Eeze_Event_Disk_Error
{
   Eeze_Disk *disk;
   const char *message;
   struct
   { /* probably switching this to enum */
      Eina_Bool mount : 1;
      Eina_Bool unmount :1;
   } type;
};

/**
 * @brief Use this function to determine whether your eeze is disk-capable
 *
 * Since applications will die if they run/compile against a function that doesn't exist,
 * if your application successfully runs/compiles with this function then you have eeze_disk.
 */
EAPI void           eeze_disk_function(void);

/**
 * @brief Create a new disk object from a /sys/ path or /dev/ path
 * @param path The /sys/ or /dev path of the disk; CANNOT be #NULL
 * @return The new disk object
 *
 * This function creates a new #Eeze_Disk from @p path.  Note that this function
 * does the minimal amount of work in order to save memory, and udev info about the disk
 * is not retrieved in this call.
 */
EAPI Eeze_Disk     *eeze_disk_new(const char *path);

/**
 * @brief Create a new disk object from a mount point
 * @param mount_point The mount point of the disk; CANNOT be #NULL
 * @return The new disk object
 *
 * This function creates a new #Eeze_Disk from @p mount_point.  Note that this function
 * does the minimal amount of work in order to save memory, and udev info about the disk
 * is not retrieved in this call.  If the disk is not currently mounted, it must have an entry
 * in /etc/fstab.
 */
EAPI Eeze_Disk     *eeze_disk_new_from_mount(const char *mount_point);

/**
 * @brief Frees a disk object
 * @param disk The disk object to free
 *
 * This call frees an #Eeze_Disk.  Once freed, the disk can no longer be used.
 */
EAPI void           eeze_disk_free(Eeze_Disk *disk);

/**
 * @brief Retrieve all disk information
 * @param disk
 *
 * Use this function to retrieve all of a disk's information at once, then use
 * a "get" function to retrieve the value.  Data retrieved in this call is cached,
 * meaning that subsequent calls will return immediately without performing any work.
 */
EAPI void           eeze_disk_scan(Eeze_Disk *disk);

/**
 * @brief Associate data with a disk
 * @param disk The disk
 * @param data The data
 *
 * Data can be associated with @p disk with this function.
 * @see eeze_disk_data_get
 */
EAPI void           eeze_disk_data_set(Eeze_Disk *disk, void *data);

/**
 * @brief Retrieve data previously associated with a disk
 * @param disk The disk
 * @return The data
 *
 * Data that has been previously associated with @p disk
 * is returned with this function.
 * @see eeze_disk_data_set
 */
EAPI void          *eeze_disk_data_get(Eeze_Disk *disk);

/**
 * @brief Return the /sys/ path of a disk
 * @param disk The disk
 * @return The /sys/ path
 *
 * This retrieves the /sys/ path that udev associates with @p disk.
 */
EAPI const char    *eeze_disk_syspath_get(Eeze_Disk *disk);

/**
 * @brief Return the /dev/ path of a disk
 * @param disk The disk
 * @return The /dev/ path
 *
 * This retrieves the /dev/ path that udev has created a device node at for @p disk.
 */
EAPI const char    *eeze_disk_devpath_get(Eeze_Disk *disk);

/**
 * @brief Return the filesystem of the disk (if known)
 * @param disk The disk
 * @return The filesystem type
 *
 * This retrieves the filesystem that the disk is using, or #NULL if unknown.
 */
EAPI const char    *eeze_disk_fstype_get(Eeze_Disk *disk);

/**
 * @brief Return the manufacturing vendor of the disk
 * @param disk The disk
 * @return The vendor
 *
 * This retrieves the vendor which manufactured the disk, or #NULL if unknown.
 */
EAPI const char    *eeze_disk_vendor_get(Eeze_Disk *disk);

/**
 * @brief Return the model of the disk
 * @param disk The disk
 * @return The model
 *
 * This retrieves the model of the disk, or #NULL if unknown.
 */
EAPI const char    *eeze_disk_model_get(Eeze_Disk *disk);

/**
 * @brief Return the serial number of the disk
 * @param disk The disk
 * @return The serial number
 *
 * This retrieves the serial number the disk, or #NULL if unknown.
 */
EAPI const char    *eeze_disk_serial_get(Eeze_Disk *disk);

/**
 * @brief Return the UUID of the disk
 * @param disk The disk
 * @return The UUID
 *
 * This retrieves the UUID of the disk, or #NULL if unknown.
 * A UUID is a 36 character (hopefully) unique identifier which can
 * be used to store persistent information about a disk.
 */
EAPI const char    *eeze_disk_uuid_get(Eeze_Disk *disk);

/**
 * @brief Return the label of the disk
 * @param disk The disk
 * @return The label
 *
 * This retrieves the label (name) of the disk, or #NULL if unknown.
 */
EAPI const char    *eeze_disk_label_get(Eeze_Disk *disk);

/**
 * @brief Return the #Eeze_Disk_Type of the disk
 * @param disk The disk
 * @return The type
 *
 * This retrieves the #Eeze_Disk_Type of the disk.  This call is useful for determining
 * the bus that the disk is connected through.
 */
EAPI Eeze_Disk_Type eeze_disk_type_get(Eeze_Disk *disk);
EAPI Eina_Bool      eeze_disk_removable_get(Eeze_Disk *disk);


/**
 * @brief Return the mount state of a disk
 * @param disk The disk
 * @return The mount state
 *
 * This returns the mounted state of the disk.  #EINA_TRUE if mounted, else #EINA_FALSE.
 */
EAPI Eina_Bool      eeze_disk_mounted_get(Eeze_Disk *disk);

/**
 * @brief Get the previously set mount wrapper for a disk
 * @param disk The disk
 * @return The wrapper, or NULL on failure
 *
 * This returns the wrapper previously set with eeze_disk_mount_wrapper_set
 */
EAPI const char    *eeze_disk_mount_wrapper_get(Eeze_Disk *disk);

/**
 * @brief Set a wrapper to run mount commands with
 * @param disk The disk to wrap mount commands for
 * @param wrapper The wrapper executable
 * @return EINA_TRUE on success, else EINA_FALSE
 *
 * Use this function to set up a wrapper for running mount/umount commands. The wrapper must
 * NOT use any of the standard mount/umount error code return values, and it must return 0 on success.
 * Note that this function will call stat() on @p wrapper if not NULL to test for existence.
 */
EAPI Eina_Bool      eeze_disk_mount_wrapper_set(Eeze_Disk *disk, const char *wrapper);

/**
 * @brief Begin a mount operation on the disk
 * @param disk The disk
 * @return #EINA_TRUE if the operation was started, else #EINA_FALSE
 *
 * This call is used to begin a mount operation on @p disk.  The operation will
 * run asynchronously in a pipe, emitting an EEZE_EVENT_DISK_MOUNT event with the disk object
 * as its event on completion.  If any errors are encountered, they will automatically logged
 * to the eeze_disk domain and an EEZE_EVENT_DISK_ERROR event will be generated with an #Eeze_Event_Disk_Error
 * struct as its event.
 *
 * NOTE: The return value of this function does not in any way reflect the mount state of a disk.
 */
EAPI Eina_Bool      eeze_disk_mount(Eeze_Disk *disk);

/**
 * @brief Begin an unmount operation on the disk
 * @param disk The disk
 * @return #EINA_TRUE if the operation was started, else #EINA_FALSE
 *
 * This call is used to begin an unmount operation on @p disk.  The operation will
 * run asynchronously in a pipe, emitting an EEZE_EVENT_DISK_MOUNT event with the disk object
 * as its event on completion.  If any errors are encountered, they will automatically logged
 * to the eeze_disk domain and an EEZE_EVENT_DISK_ERROR event will be generated with
 * an #Eeze_Event_Disk_Error struct as its event.
 *
 * NOTE: The return value of this function does not in any way reflect the mount state of a disk.
 */
EAPI Eina_Bool      eeze_disk_unmount(Eeze_Disk *disk);

/**
 * @brief Cancel a pending operation on the disk
 * @param disk The disk
 *
 * This function cancels the current pending operation on @p disk which was previously
 * started with eeze_disk_mount or eeze_disk_unmount.
 */
EAPI void           eeze_disk_cancel(Eeze_Disk *disk);

/**
 * @brief Return the mount point of a disk
 * @param disk The disk
 * @return The mount point
 *
 * This function returns the mount point associated with @p disk.
 * Note that to determine whether the disk is actually mounted, eeze_disk_mounted_get should be used.
 */
EAPI const char    *eeze_disk_mount_point_get(Eeze_Disk *disk);

/**
 * @brief Set the mount point of a disk
 * @param disk The disk
 * @param mount_point The mount point
 * @return EINA_TRUE on success, else EINA_FALSE
 *
 * This function sets the mount point associated with @p disk.
 * Note that to determine whether the disk is actually mounted, eeze_disk_mounted_get should be used.
 * Also note that this function cannot be used while the disk is mounted to avoid losing the current mount point.
 */
EAPI Eina_Bool      eeze_disk_mount_point_set(Eeze_Disk *disk, const char *mount_point);

/**
 * @brief Set the mount options using flags
 * @param disk The disk
 * @param opts An ORed set of #Eeze_Mount_Opts
 * @return EINA_TRUE on success, else EINA_FALSE
 *
 * This function replaces the current mount opts of a disk with the ones in @p opts.
 */
EAPI Eina_Bool      eeze_disk_mountopts_set(Eeze_Disk *disk, unsigned long opts);

/**
 * @brief Get the flags of a disk's current mount options
 * @param disk The disk
 * @return An ORed set of #Eeze_Mount_Opts, 0 on failure
 *
 * This function returns the current mount opts of a disk.
 */
EAPI unsigned long  eeze_disk_mountopts_get(Eeze_Disk *disk);


/**
 * @brief Begin watching mtab and fstab
 * @return #EINA_TRUE if watching was started, else #EINA_FALSE
 *
 * This function creates inotify watches on /etc/mtab and /etc/fstab and watches
 * them for changes.  This function should be used when expecting a lot of disk
 * mounting/unmounting while you need disk data since it will automatically update
 * certain necessary data instead of waiting.
 * @see eeze_mount_mtab_scan, eeze_mount_fstab_scan
 */
EAPI Eina_Bool      eeze_mount_tabs_watch(void);

/**
 * @brief Stop watching /etc/fstab and /etc/mtab
 *
 * This function stops watching fstab and mtab.  Data obtained previously will be saved.
 */
EAPI void           eeze_mount_tabs_unwatch(void);

/**
 * @brief Scan /etc/mtab a single time
 * @return #EINA_TRUE if mtab could be scanned, else #EINA_FALSE
 *
 * This function is used to perform a single scan on /etc/mtab.  It is used to gather
 * information about mounted filesystems which can then be used with your #Eeze_Disk objects
 * where appropriate.  These files will automatically be scanned any time a mount point or mount state
 * is requested unless eeze_mount_tabs_watch has been called previously, in which case data is stored for
 * use.
 * If this function is called after eeze_mount_tabs_watch, #EINA_TRUE will be returned.
 * @see eeze_mount_tabs_watch, eeze_mount_fstab_scan
 */
EAPI Eina_Bool      eeze_mount_mtab_scan(void);

/**
 * @brief Scan /etc/fstab a single time
 * @return #EINA_TRUE if mtab could be scanned, else #EINA_FALSE
 *
 * This function is used to perform a single scan on /etc/fstab.  It is used to gather
 * information about mounted filesystems which can then be used with your #Eeze_Disk objects
 * where appropriate.  These files will automatically be scanned any time a mount point or mount state
 * is requested unless eeze_mount_tabs_watch has been called previously, in which case data is stored for
 * use.
 * If this function is called after eeze_mount_tabs_watch, #EINA_TRUE will be returned.
 * @see eeze_mount_tabs_watch, eeze_mount_mtab_scan
 */
EAPI Eina_Bool      eeze_mount_fstab_scan(void);

/**
 * @brief Get the property value of a disk
 *
 * @param disk The disk
 * @param property The property to get; full list of these is a FIXME
 * @return A stringshared char* with the property or NULL on failure
 */

EAPI const char    *eeze_disk_udev_get_property(Eeze_Disk *disk, const char *property);

/**
 * @brief Get the sysattr value of a disk.
 *
 * @param disk The disk
 * @param sysattr The sysattr to get; full list of these is a FIXME
 * @return A stringshared char* with the sysattr or NULL on failure
 */

EAPI const char    *eeze_disk_udev_get_sysattr(Eeze_Disk *disk, const char *sysattr);

/**
 * Find the root device of a disk.
 *
 * @param disk The disk
 * @return The syspath of the parent device
 *
 * Return a stringshared syspath (/sys/$syspath) for the parent device.
 */
EAPI const char    *eeze_disk_udev_get_parent(Eeze_Disk *disk);

/**
 * Walks up the device chain using the device from @p disk,
 * checking each device for @p sysattr with (optional) @p value.
 *
 * @param disk The disk to walk
 * @param sysattr The attribute to find
 * @param value OPTIONAL: The value that @p sysattr should have, or NULL
 *
 * @return If the sysattr (with value) is found, returns TRUE.  Else, false.
 */
EAPI Eina_Bool      eeze_disk_udev_walk_check_sysattr(Eeze_Disk *disk, const char *sysattr, const char *value);

/**
 * @brief Walks up the device chain of @p disk
 * checking each device for @p sysattr and returns the value if found.
 *
 * @param disk The disk
 * @param sysattr The attribute to find
 *
 * @return The stringshared value of @p sysattr if found, or NULL
 */
EAPI const char    *eeze_disk_udev_walk_get_sysattr(Eeze_Disk *disk, const char *sysattr);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */
#endif
