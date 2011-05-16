/**
 @brief Eeze Device Library
 *
 @mainpage Eeze
 @image html  eeze.png
 @version 1.0.0
 @author Mike Blumenkrantz (zmike/discomfitor) <mike@@zentific.com>
 @date 2010-2011

 @section intro What is Eeze?

 Eeze is a library for manipulating devices through udev with a simple and fast
 api. It interfaces directly with libudev, avoiding such middleman daemons as
 udisks/upower or hal, to immediately gather device information the instant it
 becomes known to the system.  This can be used to determine such things as:
 @li If a cdrom has a disk inserted
 @li The temperature of a cpu core
 @li The remaining power left in a battery
 @li The current power consumption of various parts
 @li Monitor in realtime the status of peripheral devices

 Each of the above examples can be performed by using only a single eeze
 function, as one of the primary focuses of the library is to reduce the
 complexity of managing devices.

 @li @link Eeze.h Eeze functions @endlink
 @li @ref udev UDEV functions
         @li @ref watch Functions that watch for events
         @li @ref syspath Functions that accept a device /sys/ path
         @li @ref find Functions which find types of devices

 @verbatim
 Pants
 @endverbatim
 */
#ifndef EEZE_UDEV_H
#define EEZE_UDEV_H

#include <Eina.h>

#define LIBUDEV_I_KNOW_THE_API_IS_SUBJECT_TO_CHANGE 1
#include <libudev.h>

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

/**
 * @file Eeze.h
 * @brief Easy device manipulation.
 *
 * Eeze is a library for manipulating devices through udev with a simple and fast
 * api. It interfaces directly with libudev, avoiding such middleman daemons as
 * udisks/upower or hal, to immediately gather device information the instant it
 * becomes known to the system.  This can be used to determine such things as:
 * @li If a cdrom has a disk inserted
 * @li The temperature of a cpu core
 * @li The remaining power left in a battery
 * @li The current power consumption of various parts
 * @li Monitor in realtime the status of peripheral devices
 * Each of the above examples can be performed by using only a single eeze
 * function, as one of the primary focuses of the library is to reduce the
 * complexity of managing devices.
 *
 *
 * For udev functions, see @ref udev.
 */

/**
 * @defgroup main main
 *
 * These are general eeze functions which include init and shutdown.
 */

/**
 * @defgroup udev udev
 *
 * These are functions which interact directly with udev.
 */

/**
 * @addtogroup udev
 *
 * These are the device subsystems of udev:
 * @li ac97
 * @li acpi
 * @li bdi
 * @li block
 * @li bsg
 * @li dmi
 * @li graphics
 * @li hid
 * @li hwmon
 * @li i2c
 * @li input
 * @li mem
 * @li misc
 * @li net
 * @li pci
 * @li pci_bus
 * @li pci_express
 * @li platform
 * @li pnp
 * @li rtc
 * @li scsi
 * @li scsi_device
 * @li scsi_disk
 * @li scsi_generic
 * @li scsi_host
 * @li serio
 * @li sound
 * @li thermal
 * @li tty
 * @li usb
 * @li usb_device
 * @li vc
 * @li vtconsole
 *
 * These are the devtypes of udev.
 * @li atapi
 * @li audio
 * @li block
 * @li cd
 * @li char
 * @li disk
 * @li floppy
 * @li generic
 * @li hid
 * @li hub
 * @li media
 * @li optical
 * @li printer
 * @li rbc
 * @li scsi
 * @li storage
 * @li tape
 * @li video
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup udev
 * @typedef Eeze_Udev_Event
 * @enum Eeze_Udev_Event
 * @brief Flags for watch events
 *
 * These events are used to specify the events to watch in a
 * #Eeze_Udev_Watch.  They can be ORed together.
 *@{
 */
typedef enum
{
    /** - No event specified */
    EEZE_UDEV_EVENT_NONE = 0xf0,
    /** - Device added */
    EEZE_UDEV_EVENT_ADD = (1 << 1),
    /** - Device removed */
    EEZE_UDEV_EVENT_REMOVE = (1 << 2),
    /** - Device changed */
    EEZE_UDEV_EVENT_CHANGE = (1 << 3),
    /** - Device has come online */
    EEZE_UDEV_EVENT_ONLINE = (1 << 4),
    /** - Device has gone offline */
    EEZE_UDEV_EVENT_OFFLINE = (1 << 5)
} Eeze_Udev_Event;
/** @} */

/**
 * @addtogroup udev udev
 * @typedef Eeze_Udev_Type Eeze_Udev_Type
 * @enum Eeze_Udev_Type
 * @brief Convenience types to simplify udev access.
 *
 * These types allow easy access to certain udev device types.  They
 * may only be used in specified functions.
 *
 * @{
 */
/*FIXME: these probably need to be bitmasks with categories*/
typedef enum
{
   /** - No type */
   EEZE_UDEV_TYPE_NONE,
   /** - Keyboard device */
   EEZE_UDEV_TYPE_KEYBOARD,
   /** - Mouse device */
   EEZE_UDEV_TYPE_MOUSE,
   /** - Touchpad device */
   EEZE_UDEV_TYPE_TOUCHPAD,
   /** - Mountable drive */
   EEZE_UDEV_TYPE_DRIVE_MOUNTABLE,
   /** - Internal drive */
   EEZE_UDEV_TYPE_DRIVE_INTERNAL,
   /** - Removable drive */
   EEZE_UDEV_TYPE_DRIVE_REMOVABLE,
   /** - cd drive */
   EEZE_UDEV_TYPE_DRIVE_CDROM,
   /** - AC adapter */
   EEZE_UDEV_TYPE_POWER_AC,
   /** - Battery */
   EEZE_UDEV_TYPE_POWER_BAT,
   /** - Temperature sensor */
   EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR,
   /** - Network devices */
   EEZE_UDEV_TYPE_NET
} Eeze_Udev_Type;
/**@}*/

struct Eeze_Udev_Watch;
typedef struct Eeze_Udev_Watch Eeze_Udev_Watch;

#define EEZE_VERSION_MAJOR 1
#define EEZE_VERSION_MINOR 1

   typedef struct _Eeze_Version
     {
        int major;
        int minor;
        int micro;
        int revision;
     } Eeze_Version;

   EAPI extern Eeze_Version *eeze_version;

/**
 * @addtogroup watch
 * @brief Callback type for use with #Eeze_Udev_Watch
 */
typedef void(*Eeze_Udev_Watch_Cb)(const char *, Eeze_Udev_Event, void *, Eeze_Udev_Watch *);


/**
 * Initialize the eeze library.
 * @return The number of times the function has been called, or -1 on failure.
 *
 * This function should be called prior to using any eeze functions, and MUST
 * be called prior to using any udev functions to avoid a segv.
 *
 * @ingroup main
 */
EAPI int             eeze_init(void);

/**
 * Shut down the eeze library.
 * @return The number of times the eeze_init has been called, or -1 when
 * all occurrences of eeze have been shut down.
 *
 * This function should be called when no further eeze functions will be called.
 *
 * @ingroup main
 */
EAPI int             eeze_shutdown(void);

   /**
    * @addtogroup find Find
    *
    * These are functions which find/supplement lists of devices.
    *
    * @ingroup udev
    *
    * @{
    */

/**
 * Returns a stringshared list of all syspaths that are (or should be) the same
 * device as the device pointed at by @p syspath.
 *
 * @param syspath The syspath of the device to find matches for
 * @return All devices which are the same as the one passed
 */
EAPI Eina_List       *eeze_udev_find_similar_from_syspath(const char *syspath);

/**
 * Updates a list of all syspaths that are (or should be) the same
 * device.
 *
 * @param list The list of devices to update
 * @return The updated list
 *
 * This function will update @p list to include all devices matching
 * devices with syspaths currently stored in @p list.  All strings are
 * stringshared.
 */
EAPI Eina_List       *eeze_udev_find_unlisted_similar(Eina_List *list);

/**
 * Find a list of devices by a sysattr (and, optionally, a value of that sysattr).
 *
 * @param sysattr The attribute to find
 * @param value Optional: the value that the attribute should have
 *
 * @return A stringshared list of the devices found with the attribute
 *
 * @ingroup find
 */
EAPI Eina_List       *eeze_udev_find_by_sysattr(const char *sysattr, const char *value);

/**
 * Find devices using an #Eeze_Udev_Type and/or a name.
 *
 * @param etype An #Eeze_Udev_Type or 0
 * @param name A filter for the device name or #NULL
 * @return A stringshared Eina_List of matched devices or #NULL on failure
 *
 * Return a list of syspaths (/sys/$syspath) for matching udev devices.
 */
EAPI Eina_List       *eeze_udev_find_by_type(Eeze_Udev_Type type, const char *name);

/**
 * A more advanced find, allows finds using udev properties.
 *
 * @param subsystem The udev subsystem to filter by, or NULL
 * @param type "ID_INPUT_KEY", "ID_INPUT_MOUSE", "ID_INPUT_TOUCHPAD", NULL, etc
 * @param name A filter for the device name, or NULL
 * @return A stringshared Eina_List* of matched devices or NULL on failure
 *
 * Return a list of syspaths (/sys/$syspath) for matching udev devices.
 * Requires at least one filter.
 */
EAPI Eina_List       *eeze_udev_find_by_filter(const char *subsystem, const char *type, const char *name);
   /**
    * @}
    */

   /**
    * @addtogroup syspath Syspath
    *
    * These are functions which interact with the syspath (/sys/$PATH) of
    * a device.
    *
    * @ingroup udev
    *
    * @{
    */

/**
 * Get the syspath of a device from the /dev/ path.
 *
 * @param devpath The /dev/ path of the device
 * @return A stringshared char* which corresponds to the /sys/ path of the device or NULL on failure
 *
 * Takes "/dev/path" and returns the corresponding /sys/ path (without the "/sys/")
 */
EAPI const char      *eeze_udev_devpath_get_syspath(const char *devpath);

/**
 * Find the root device of a device from its syspath.
 *
 * @param syspath The syspath of a device, with or without "/sys/"
 * @return The syspath of the parent device
 *
 * Return a stringshared syspath (/sys/$syspath) for the parent device.
 */
EAPI const char      *eeze_udev_syspath_get_parent(const char *syspath);

/**
 * Returns a list of all parent device syspaths for @p syspath.
 *
 * @param syspath The device to find parents of
 * @return A stringshared list of the parent devices of @p syspath
 */
EAPI Eina_List       *eeze_udev_syspath_get_parents(const char *syspath);

/**
 * Get the /dev/ path from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return A stringshared char* with the /dev/ path or NULL on failure
 *
 * Takes /sys/$PATH and turns it into the corresponding "/dev/x/y".
 */
EAPI const char      *eeze_udev_syspath_get_devpath(const char *syspath);

/**
 * Get the /dev/ name from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return A stringshared char* of the device name without the /dev/ path, or NULL on failure
 *
 * Takes /sys/$PATH and turns it into the corresponding /dev/x/"y".
 */
EAPI const char      *eeze_udev_syspath_get_devname(const char *syspath);

/**
 * Get the subsystem of a device from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return A stringshared char* with the subsystem of the device or NULL on failure
 *
 * Takes /sys/$PATH and returns the corresponding device subsystem,
 * such as "input" for keyboards/mice.
 */
EAPI const char      *eeze_udev_syspath_get_subsystem(const char *syspath);

/**
 * Get the property value of a device from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @param property The property to get; full list of these is a FIXME
 * @return A stringshared char* with the property or NULL on failure
 */
EAPI const char      *eeze_udev_syspath_get_property(const char *syspath, const char *property);

/**
 * Get the sysattr value of a device from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @param sysattr The sysattr to get; full list of these is a FIXME
 * @return A stringshared char* with the sysattr or NULL on failure
 */
EAPI const char      *eeze_udev_syspath_get_sysattr(const char *syspath, const char *sysattr);

/**
 * Checks whether the device is a mouse.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return If true, the device is a mouse
 */
EAPI Eina_Bool        eeze_udev_syspath_is_mouse(const char *syspath);

/**
 * Checks whether the device is a keyboard.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return If true, the device is a keyboard
 */
EAPI Eina_Bool        eeze_udev_syspath_is_kbd(const char *syspath);

/**
 * Checks whether the device is a touchpad.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return If true, the device is a touchpad
 */
EAPI Eina_Bool        eeze_udev_syspath_is_touchpad(const char *syspath);
   /**
    * @}
    */

   /**
    * @addtogroup walks Walks
    *
    * These are functions which walk up the device chain.
    *
    * @ingroup udev
    *
    * @{
    */

/**
 * Walks up the device chain starting at @p syspath,
 * checking each device for @p sysattr with (optional) @p value.
 *
 * @param syspath The /sys/ path of the device to start at, with or without the /sys/
 * @param sysattr The attribute to find
 * @param value OPTIONAL: The value that @p sysattr should have, or NULL
 *
 * @return If the sysattr (with value) is found, returns TRUE.  Else, false.
 */
EAPI Eina_Bool        eeze_udev_walk_check_sysattr(const char *syspath, const char *sysattr, const char *value);

/**
 * Walks up the device chain starting at @p syspath,
 * checking each device for @p sysattr, and returns the value if found.
 *
 * @param syspath The /sys/ path of the device to start at, with or without the /sys/
 * @param sysattr The attribute to find
 *
 * @return The stringshared value of @p sysattr if found, or NULL
 */
EAPI const char      *eeze_udev_walk_get_sysattr(const char *syspath, const char *sysattr);
   /**
    * @}
    */

   /**
    * @addtogroup watch Watch
    *
    * @brief These are functions which monitor udev for events.
    *
    * Eeze watches are simple: you specify a type of device to watch (or all devices), some events (or all) to watch for, a callback,
    * and some data, and then udev watches those device types for events of the type you specified.  Your callback is called with a
    * syspath of the triggering device and the event that happened to the device, along with the data you associated with the watch and
    * the watch object itself in case you want to stop the watch easily in a callback.
    *
    * @ingroup udev
    *
    * @{
    */

/**
 * Add a watch for a device type
 *
 * @param type The #Eeze_Udev_Type to watch
 * @param event The events to watch; an OR list of #Eeze_Udev_Event (ie (#EEZE_UDEV_EVENT_ADD | #EEZE_UDEV_EVENT_REMOVE)), or 0 for all events
 * @param cb The function to call when the watch receives data of type #Eeze_Udev_Watch_Cb
 * @param user_data Data to pass to the callback function
 *
 * @return A watch struct for the watch type specified, or NULL on failure
 *
 * Eeze watches will monitor udev for changes of type(s) @p event to devices of type @p type.  When these changes occur, the stringshared
 * syspath of the device will be sent to function @p func, along with the bitmask of the event type which can be detected through
 * binary &.
 */
EAPI Eeze_Udev_Watch *eeze_udev_watch_add(Eeze_Udev_Type type, int event, Eeze_Udev_Watch_Cb cb, void *user_data);

/**
 * Deletes a watch.
 *
 * @param watch An Eeze_Udev_Watch object
 * @return The data originally associated with the watch, or NULL
 *
 * Deletes a watch, closing file descriptors and freeing related udev memory.
 */
EAPI void            *eeze_udev_watch_del(Eeze_Udev_Watch *watch);
   /**
    * @}
    */

#ifdef __cplusplus
}
#endif

#endif
