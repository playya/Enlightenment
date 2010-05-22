#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eeze.h>
#include "eeze_private.h"
#include "eeze_udev_private.h"

extern _udev *udev;

/* opaque */
struct Eeze_Udev_Watch
{
   _udev_monitor *mon;
   Ecore_Fd_Handler *handler;
   Eeze_Udev_Type type;
};

/* private */
struct _store_data
{
   void (*func)(const char *, int, void *, Eeze_Udev_Watch *);
   void *data;
   int event;
   _udev_monitor *mon;
   Eeze_Udev_Type type;
   Eeze_Udev_Watch *watch;
};

/**
 * @defgroup watch Watch
 *
 * These are functions which monitor udev for events.
 * 
 * @ingroup udev
 */

/* private function to further filter watch results based on Eeze_Udev_Type
 * specified; helpful for new udev versions, but absolutely required for
 * old udev, which does not implement filtering in device monitors.
 */
static int
_get_syspath_from_watch(void *data, Ecore_Fd_Handler * fd_handler)
{
   struct _store_data *store = data;
   _udev_device *device, *parent, *tmpdev;
   const char *ret, *test;
   void (*func)(const char *, int, void *, Eeze_Udev_Watch *) = store->func;
   void *sdata = store->data;
   Eeze_Udev_Watch *watch = store->watch;
   int cap = 0, event = 0;

   if (!ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     return 1;

   device = udev_monitor_receive_device(store->mon);

   if (!device)
     return 1;

   switch (store->type)
     {
        case EEZE_UDEV_TYPE_KEYBOARD:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device)))
              || (strcmp(test, "input")))
            goto error;

          test = udev_device_get_property_value(device, "ID_CLASS");

          if ((_walk_parents_test_attr(device, "bInterfaceProtocol", "01"))
              || ((test) && (!strcmp(test, "kbd"))))
            break;

          goto error;
#endif
          if (!udev_device_get_property_value(device, "ID_INPUT_KEYBOARD"))
            goto error;

          break;
        case EEZE_UDEV_TYPE_MOUSE:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device)))
              || (strcmp(test, "input")))
            goto error;

          test = udev_device_get_property_value(device, "ID_CLASS");

          if ((_walk_parents_test_attr(device, "bInterfaceProtocol", "02"))
              || ((test) && (!strcmp(test, "mouse"))))
            break;

          goto error;
#endif

          if (!udev_device_get_property_value(device, "ID_INPUT_MOUSE"))
            goto error;

          break;
        case EEZE_UDEV_TYPE_TOUCHPAD:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device)))
              || (strcmp(test, "input")))
            goto error;

          if (_walk_parents_test_attr(device, "resolution", NULL))
            break;

          goto error;
#endif
          if (!udev_device_get_property_value(device, "ID_INPUT_TOUCHPAD"))
            goto error;

          break;
        case EEZE_UDEV_TYPE_DRIVE_MOUNTABLE:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device)))
              || (strcmp(test, "block")))
            goto error;
#endif
          test = udev_device_get_sysattr_value(device, "capability");

          if (test)
            cap = atoi(test);

          if (!(test = (udev_device_get_property_value(device, "ID_FS_USAGE"))) ||
              (strcmp("filesystem", test)) || (cap == 52))
            goto error;

          break;
        case EEZE_UDEV_TYPE_DRIVE_INTERNAL:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device)))
              || (strcmp(test, "block")))
            goto error;
#endif
          if (!(test = udev_device_get_property_value(device, "ID_BUS"))
              || (strcmp("ata", test))
              || !(test = udev_device_get_sysattr_value(device, "removable"))
              || (atoi(test)))
            goto error;

          break;
        case EEZE_UDEV_TYPE_DRIVE_REMOVABLE:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device)))
              || (strcmp(test, "block")))
            goto error;
#endif
          if ((!(test = udev_device_get_sysattr_value(device, "removable"))
               || (!atoi(test)))
              && (!(test = udev_device_get_sysattr_value(device, "capability"))
                  || (atoi(test) != 10)))
            goto error;

          break;
        case EEZE_UDEV_TYPE_DRIVE_CDROM:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device)))
              || (strcmp(test, "block")))
            goto error;
#endif
          if (!udev_device_get_property_value(device, "ID_CDROM"))
            goto error;

          break;
        case EEZE_UDEV_TYPE_POWER_AC:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device)))
              || (strcmp(test, "power_supply")))
            goto error;
#endif
          if (!(test = udev_device_get_property_value(device, "POWER_SUPPLY_TYPE"))
              || (strcmp("Mains", test)))
            goto error;
          break;
        case EEZE_UDEV_TYPE_POWER_BAT:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device)))
              || (strcmp(test, "power_supply")))
            goto error;
#endif
          if (!(test = udev_device_get_property_value(device, "POWER_SUPPLY_TYPE"))
              || (strcmp("Battery", test)))
            goto error;
          break;
        case EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device)))
              || (strcmp(test, "hwmon")))
            goto error;
#endif /* have to do stuff up here since we need info from the parent */
          if (!_walk_parents_test_attr(device, "temp1_input", NULL))
            goto error;

          /* if device is not the one which has the temp input, we must go up the chain */
          if (!(test = udev_device_get_sysattr_value(device, "temp1_input")))
            {
              for (parent = udev_device_get_parent(device); parent; parent = udev_device_get_parent(parent)) /*check for parent */
                if (((test =
                        udev_device_get_sysattr_value(parent, "temp1_input"))))
                  {
                    tmpdev = device;

                    if (!(device = _copy_device(parent)))
                      goto error;

                    udev_device_unref(tmpdev);
                    break;
                  }
            }

          break;
        default:
          break;
     }

   if ((!(test = udev_device_get_action(device)))
      || (!(ret = udev_device_get_syspath(device))))
     goto error;

   if (store->event)
     {
      if (!strcmp(test, "add"))
        {
          if ((store->event & EEZE_UDEV_EVENT_ADD) != EEZE_UDEV_EVENT_ADD)
            goto error;

          event |= EEZE_UDEV_EVENT_ADD;
        }
      else
        if (!strcmp(test, "remove"))
          {
            if ((store->event & EEZE_UDEV_EVENT_REMOVE) !=
                EEZE_UDEV_EVENT_REMOVE)
              goto error;

            event |= EEZE_UDEV_EVENT_REMOVE;
          }
        else
          if (!strcmp(test, "change"))
            {
              if ((store->event & EEZE_UDEV_EVENT_CHANGE) !=
                  EEZE_UDEV_EVENT_CHANGE)
                goto error;

              event |= EEZE_UDEV_EVENT_CHANGE;
            }
          else
            if (!strcmp(test, "online"))
              {
                if ((store->event & EEZE_UDEV_EVENT_ONLINE) !=
                    EEZE_UDEV_EVENT_ONLINE)
                  goto error;

                event |= EEZE_UDEV_EVENT_ONLINE;
              }
            else
              {
                if ((store->event & EEZE_UDEV_EVENT_OFFLINE) !=
                    EEZE_UDEV_EVENT_OFFLINE)
                  goto error;

                event |= EEZE_UDEV_EVENT_OFFLINE;
              }
     }

  (*func)(eina_stringshare_add(ret), event, sdata, watch);
error:
   udev_device_unref(device);
   return 1;
}
/**
 * Add a watch for a device type
 *
 * @param type The @ref Eeze_Udev_Type to watch
 * @param event The events to watch; an OR list of @ref event (ie (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE)), or 0 for all events
 * @param func The function to call when the watch receives data;
 * must take (const char *device, const char *event_type, void *data, Eeze_Udev_Watch *watch)
 * @param user_data Data to pass to the callback function
 *
 * @return A watch struct for the watch type specified, or NULL on failure
 *
 * @ingroup watch
 */
EAPI Eeze_Udev_Watch *
eeze_udev_watch_add(Eeze_Udev_Type type, int event,
                    void (*func)(const char *syspath, int event, void *data,
                                 Eeze_Udev_Watch * watch), void *user_data)
{
   _udev_monitor *mon = NULL;
   int fd;
   Ecore_Fd_Handler *handler;
   Eeze_Udev_Watch *watch;
   struct _store_data *store;

   if (!(store = calloc(sizeof(struct _store_data), 1)))
     return NULL;

   if (!(watch = malloc(sizeof(Eeze_Udev_Watch))))
     goto error;

   if (!(mon = udev_monitor_new_from_netlink(udev, "udev")))
     goto error;

#ifndef OLD_UDEV_RRRRRRRRRRRRRR

   switch (type)
     {
        case EEZE_UDEV_TYPE_KEYBOARD:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "input", NULL);
          break;
        case EEZE_UDEV_TYPE_MOUSE:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "input", NULL);
          break;
        case EEZE_UDEV_TYPE_TOUCHPAD:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "input", NULL);
          break;
        case EEZE_UDEV_TYPE_DRIVE_MOUNTABLE:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
          break;
        case EEZE_UDEV_TYPE_DRIVE_INTERNAL:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
          break;
        case EEZE_UDEV_TYPE_DRIVE_REMOVABLE:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
          break;
        case EEZE_UDEV_TYPE_DRIVE_CDROM:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
          break;
        case EEZE_UDEV_TYPE_POWER_AC:
        case EEZE_UDEV_TYPE_POWER_BAT:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "power_supply",
              NULL);
          break;
        case EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "hwmon", NULL);
          break;
          /*
                  case EEZE_UDEV_TYPE_ANDROID:
                    udev_monitor_filter_add_match_subsystem_devtype(mon, "input", "usb_interface");
                    break;
          */
        default:
          break;
     }

#endif

   if (udev_monitor_enable_receiving(mon))
     goto error;

   fd = udev_monitor_get_fd(mon);
   store->func = func;
   store->data = user_data;
   store->mon = mon;
   store->type = type;
   store->watch = watch;
   store->event = event;

   if (!(handler = ecore_main_fd_handler_add(fd, ECORE_FD_READ, 
       _get_syspath_from_watch, store, NULL, NULL)))
     goto error;

   watch->mon = mon;
   watch->handler = handler;
   return watch;
error:
   free(store);
   free(watch);
   udev_monitor_unref(mon);
   return NULL;
}

/**
 * Deletes a watch.
 *
 * @param watch An Eeze_Udev_Watch object
 * @return The data originally associated with the watch, or NULL
 *
 * Deletes a watch, closing file descriptors and freeing related udev memory.
 *
 * @ingroup watch
 */
EAPI void *
eeze_udev_watch_del(Eeze_Udev_Watch * watch)
{
   struct _store_data *sdata;
   void *ret = NULL;

   if ((!watch) || (!watch->mon) || (!watch->handler))
     return NULL;

   udev_monitor_unref(watch->mon);
   sdata = ecore_main_fd_handler_del(watch->handler);

   if (sdata)
     {
        ret = sdata->data;
        free(sdata);
     }

   free(watch);
   return ret;
}
