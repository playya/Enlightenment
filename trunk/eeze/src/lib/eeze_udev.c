#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eeze.h>
#include "eeze_private.h"
#include "eeze_udev_private.h"

extern _udev *udev;

/**
 * @defgroup udev udev
 *
 * These are functions which interact directly with udev.
 */

/**
 * Find the root device of a device from its syspath.
 *
 * @param syspath The syspath of a device, with or without "/sys/"
 * @return The syspath of the parent device
 *
 * Return a syspath (/sys/$syspath) for the parent device.
 *
 * @ingroup udev
 */
EAPI const char *
eeze_udev_syspath_get_parent(const char *syspath)
{
   _udev_device *device, *parent;
   const char *ret;
   Eina_Strbuf *sbuf;

   if (!syspath)
     return NULL;

   sbuf = eina_strbuf_new();

   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
   parent = udev_device_get_parent(device);
   ret = eina_stringshare_add(udev_device_get_property_value(parent, "DEVPATH"));
   udev_device_unref(device);
   eina_strbuf_free(sbuf);
   return ret;
}

/**
 * Returns a list of all parent device syspaths for @p syspath.
 *
 * @param syspath The device to find parents of
 * @return A list of the parent devices of @p syspath
 *
 * @ingroup udev
 */
EAPI Eina_List *
eeze_udev_syspath_get_parents(const char *syspath)
{
   _udev_device *child, *parent, *device;
   const char *path;
   Eina_Strbuf *sbuf;
   Eina_List *devlist = NULL;

   if (!syspath)
     return NULL;

   sbuf = eina_strbuf_new();

   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));

   if (!(parent = udev_device_get_parent(device)))
     return NULL;

   for (; parent; child = parent, parent = udev_device_get_parent(child))
     {
        path = udev_device_get_syspath(parent);
        devlist = eina_list_append(devlist, eina_stringshare_add(path));
     }

   udev_device_unref(device);
   return devlist;
}

/**
 * Get the /dev/ path from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return A const char* with the /dev/ path or NULL on failure
 *
 * Takes /sys/$PATH and turns it into the corresponding "/dev/x/y".
 *
 * @ingroup udev
 */
EAPI const char *
eeze_udev_syspath_get_devpath(const char *syspath)
{
   _udev_device *device;
   const char *name = NULL;
   Eina_Strbuf *sbuf;

   if (!syspath)
     return NULL;

   sbuf = eina_strbuf_new();

   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));

   if (!(name = udev_device_get_property_value(device, "DEVNAME")))
     return NULL;

   name = eina_stringshare_add(name);
   udev_device_unref(device);
   eina_strbuf_free(sbuf);
   return name;
}

/**
 * Get the subsystem of a device from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return A const char* with the subsystem of the device or NULL on failure
 *
 * Takes /sys/$PATH and returns the corresponding device subsystem,
 * such as "input" for keyboards/mice.
 *
 * @ingroup udev
 */
EAPI const char *
eeze_udev_syspath_get_subsystem(const char *syspath)
{
   _udev_device *device;
   const char *subsystem;
   Eina_Strbuf *sbuf;

   if (!syspath)
     return NULL;

   sbuf = eina_strbuf_new();

   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
   subsystem =
     eina_stringshare_add(udev_device_get_property_value(device, "SUBSYSTEM"));
   udev_device_unref(device);
   eina_strbuf_free(sbuf);
   return subsystem;
}

/**
 * Get the property value of a device from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @param property The property to get; full list of these is a FIXME
 * @return A const char* with the property or NULL on failure
 *
 * @ingroup udev
 */
EAPI const char *
eeze_udev_syspath_get_property(const char *syspath, const char *property)
{
   _udev_device *device;
   const char *value = NULL, *test;
   Eina_Strbuf *sbuf;

   if (!syspath || !property)
     return NULL;

   sbuf = eina_strbuf_new();

   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));

   if ((test = udev_device_get_property_value(device, property)))
     value = eina_stringshare_add(test);

   udev_device_unref(device);
   eina_strbuf_free(sbuf);
   return value;
}

/**
 * Get the sysattr value of a device from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @param sysattr The sysattr to get; full list of these is a FIXME
 * @return A const char* with the sysattr or NULL on failure
 *
 * @ingroup udev
 */
EAPI const char *
eeze_udev_syspath_get_sysattr(const char *syspath, const char *sysattr)
{
   _udev_device *device;
   const char *value = NULL, *test;
   Eina_Strbuf *sbuf;

   if (!syspath || !sysattr)
     return NULL;

   sbuf = eina_strbuf_new();

   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));

   if ((test = udev_device_get_sysattr_value(device, sysattr)))
     value = eina_stringshare_add(test);

   udev_device_unref(device);
   eina_strbuf_free(sbuf);
   return value;
}

/**
 * Get the syspath of a device from the /dev/ path.
 *
 * @param devpath The /dev/ path of the device
 * @return A const char* which corresponds to the /sys/ path of the device or NULL on failure
 *
 * Takes "/dev/path" and returns the corresponding /sys/ path (without the "/sys/")
 *
 * @ingroup udev
 */
EAPI const char *
eeze_udev_devpath_get_syspath(const char *devpath)
{
   _udev_enumerate *en;
   _udev_list_entry *devs, *cur;
   _udev_device *device;
   const char *name, *ret;

   if (!devpath)
     return NULL;

   en = udev_enumerate_new((udev));

   if (!en)
     return NULL;

   udev_enumerate_add_match_property(en, "DEVNAME", devpath);
   udev_enumerate_scan_devices(en);
   devs = udev_enumerate_get_list_entry(en);
   udev_list_entry_foreach(cur, devs)
     {
        name = udev_list_entry_get_name(cur);
        device = udev_device_new_from_syspath(udev, name);
        ret = eina_stringshare_add(udev_device_get_sysname(device));
        udev_device_unref(device);
        break;      /*just in case there's more than one somehow */
     }
   udev_enumerate_unref(en);
   return ret;
}

/**
 * Get the subsystem of a device from the /dev/ path.
 *
 * @param devpath The /dev/ path of the device
 * @return A const char* with the subsystem of the device or NULL on failure
 *
 * Takes "/dev/path" and returns the subsystem of the device.
 *
 * @ingroup udev
 */
EAPI const char *
eeze_udev_devpath_get_subsystem(const char *devpath)
{
   _udev_enumerate *en;
   _udev_list_entry *devs, *cur;
   _udev_device *device;
   const char *name, *ret;

   if (!devpath)
     return NULL;

   en = udev_enumerate_new((udev));

   if (!en)
     return NULL;

   udev_enumerate_add_match_property(en, "DEVNAME", devpath);
   udev_enumerate_scan_devices(en);
   devs = udev_enumerate_get_list_entry(en);
   udev_list_entry_foreach(cur, devs)
     {
        name = udev_list_entry_get_name(cur);
        device = udev_device_new_from_syspath(udev, name);
        ret = eina_stringshare_add(udev_device_get_subsystem(device));
        udev_device_unref(device);
        break;      /*just in case there's more than one somehow */
     }
   udev_enumerate_unref(en);
   return ret;
}

/**
 * Checks whether the device is a mouse.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return If true, the device is a mouse
 *
 * @ingroup udev
 */
EAPI Eina_Bool
eeze_udev_syspath_is_mouse(const char *syspath)
{
   _udev_device *device;
   Eina_Bool mouse = 0;
   Eina_Strbuf *sbuf;
   const char *test = NULL;

   if (!syspath)
     return 0;

   sbuf = eina_strbuf_new();

   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
   mouse = _walk_parents_test_attr(device, "bInterfaceProtocol", "02");

   if (!mouse)
     {
        test = udev_device_get_property_value(device, "ID_CLASS");

        if ((test) && (!strcmp(test, "mouse")))
          mouse = 1;
     }

#else
   test = udev_device_get_property_value(device, "ID_INPUT_MOUSE");

   if (test)
     mouse = atoi(test);

#endif
   udev_device_unref(device);
   eina_strbuf_free(sbuf);
   return mouse;
}

/**
 * Checks whether the device is a keyboard.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return If true, the device is a keyboard
 *
 * @ingroup udev
 */
EAPI Eina_Bool
eeze_udev_syspath_is_kbd(const char *syspath)
{
   _udev_device *device;
   Eina_Bool kbd = 0;
   Eina_Strbuf *sbuf;
   const char *test = NULL;

   if (!syspath)
     return 0;

   sbuf = eina_strbuf_new();

   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
   kbd = _walk_parents_test_attr(device, "bInterfaceProtocol", "01");

   if (!kbd)
     {
        test = udev_device_get_property_value(device, "ID_CLASS");

        if ((test) && (!strcmp(test, "kbd")))
          kbd = 1;
     }

#else
   test = udev_device_get_property_value(device, "ID_INPUT_KEYBOARD");

   if (test)
     kbd = atoi(test);

#endif
   udev_device_unref(device);
   eina_strbuf_free(sbuf);
   return kbd;
}

/**
 * Checks whether the device is a touchpad.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return If true, the device is a touchpad
 *
 * @ingroup udev
 */
EAPI Eina_Bool
eeze_udev_syspath_is_touchpad(const char *syspath)
{
   _udev_device *device;
   Eina_Bool touchpad = 0;
   Eina_Strbuf *sbuf;

   if (!syspath)
     return 0;

   sbuf = eina_strbuf_new();

   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
   touchpad = _walk_parents_test_attr(device, "resolution", NULL);
#else
   const char *test;
   test = udev_device_get_property_value(device, "ID_INPUT_TOUCHPAD");

   if (test)
     touchpad = atoi(test);

#endif
   udev_device_unref(device);
   eina_strbuf_free(sbuf);
   return touchpad;
}
