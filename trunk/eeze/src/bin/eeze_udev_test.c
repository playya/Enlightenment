#include <E_Udev.h>
#include <Ecore.h>
#include <stdio.h>

typedef struct kbdmouse
{
   Eina_List *kbds;
   Eina_List *mice;
   Eina_Hash *hash;
} kbdmouse;

static void
/* event will always be a syspath starting with /sys */
catch_events(const char *device, const char *event, void *data, Eudev_Watch *watch)
{
   kbdmouse *akbdmouse = data;
   Eina_List *l;
   const char *name, *dev, *type;
   int new = 0;

   /* the device that comes through will be prefixed by "/sys"
    * but the saved name will not, so we check for the saved name
    * inside the device name
    */
   EINA_LIST_FOREACH(akbdmouse->kbds, l, name)
     if (strstr(device, name)) goto end;
   EINA_LIST_FOREACH(akbdmouse->mice, l, name)
     if (strstr(device, name)) goto end;

   /* check to see if the device was just plugged in */
   if (e_udev_syspath_is_kbd(device) || e_udev_syspath_is_mouse(device))
   {
      new = 1;
      goto end;
   }
   /* if we reach here, the device is neither a keyboard nor a mouse that we saw
    * previously, so we print a moderately amusing message and bail
    */
   printf("Sneaky sneaky!  But %s is not a keyboard or a mouse!!\n", device);
   return;

end:
   /* we stored the devpaths for all the syspaths previously so that
    * we can retrieve them now even though the device has been removed and
    * is inaccessible to udev
    */
   if (new)
     {
        dev = e_udev_syspath_get_devpath(device);
        type = "plugged in";
     }
   else
     {
        dev = eina_hash_find(akbdmouse->hash, name);
        type = "unplugged";
     }
   printf("You %s %s!\n", type, dev);
   printf("All tests completed, exiting successfully!\n");
   /* now we free the lists */
   eina_list_free(akbdmouse->kbds);
   eina_list_free(akbdmouse->mice);
   /* and the hash */
   eina_hash_free(akbdmouse->hash);
   /* and the random storage struct */
   free(akbdmouse);
   /* and quit the main loop */
   ecore_main_loop_quit();
   /* and delete the watch */
   e_udev_watch_del(watch);
   /* and shut down eudev */
   e_udev_shutdown();
}

static void
hash_free(void *data)
{
   eina_stringshare_del(data);
}

int main()
{
   Eina_List *type, *l;
   const char *name, *check;
   kbdmouse *akbdmouse;
   Eina_Hash *hash;

   ecore_init();
   e_udev_init();

   hash = eina_hash_stringshared_new(hash_free);
   akbdmouse = malloc(sizeof(kbdmouse));
   akbdmouse->hash = hash;
   
   printf("For my first trick, I will find all of your keyboards and return their syspaths.\n");
   /* find all keyboards using type EUDEV_TYPE_KEYBOARD */
   type = e_udev_find_by_type(EUDEV_TYPE_KEYBOARD, NULL);
   EINA_LIST_FOREACH(type, l, name)
     {  /* add the devpath to the hash for use in the cb later */
        eina_hash_direct_add(hash, name, e_udev_syspath_get_devpath(name));
        printf("Found keyboard: %s\n", name);
     }
   /* we save this list for later, because once a device is unplugged it can
    * no longer be detected by udev, and any related properties are unusable unless
    * they have been previously stored
    */
   akbdmouse->kbds = type;

   printf("\nNext, I will find all of your mice and print the corresponding manufacturer.\n");
   /* find all mice using type EUDEV_TYPE_MOUSE */
   type = e_udev_find_by_type(EUDEV_TYPE_MOUSE, NULL);
   EINA_LIST_FOREACH(type, l, name)
     {  /* add the devpath to the hash for use in the cb later */
        eina_hash_direct_add(hash, name, e_udev_syspath_get_devpath(name)); /* get a property using the device's syspath */
        printf("Found mouse %s with vendor: %s\n", name, e_udev_syspath_get_property(name, "ID_VENDOR"));
     }
   /* we save this list for later, because once a device is unplugged it can
    * no longer be detected by udev, and any related properties are unusable unless
    * they have been previously stored
    */
   akbdmouse->mice = type;

   printf("\nNow let's try something a little more difficult.  Mountable filesystems!\n");
   /* find all mountable drives using type EUDEV_TYPE_DRIVE_MOUNTABLE */
   type = e_udev_find_by_type(EUDEV_TYPE_DRIVE_MOUNTABLE, NULL);
   EINA_LIST_FOREACH(type, l, name)
   {
     printf("Found device: %s\n", name);  /* get a property using the device's syspath */
     printf("\tYou probably know it better as %s\n", e_udev_syspath_get_property(name, "DEVNAME"));
     printf("\tIt's formatted as %s", e_udev_syspath_get_property(name, "ID_FS_TYPE"));
     check = e_udev_syspath_get_property(name, "FSTAB_DIR");
     if (check)
       printf(", and gets mounted at %s", check);
     printf("!\n");
   }
   eina_list_free(type);

   printf("\nInternal drives, anyone?  With serial numbers?\n");
   /* find all internal drives using type EUDEV_TYPE_DRIVE_INTERNAL */
   type = e_udev_find_by_type(EUDEV_TYPE_DRIVE_INTERNAL, NULL);
   EINA_LIST_FOREACH(type, l, name) /* get a property using the device's syspath */
        printf("%s: %s\n", name, e_udev_syspath_get_property(name, "ID_SERIAL"));
   eina_list_free(type);

   printf("\nGot any removables?  I'm gonna find em!\n");
   /* find all removable media using type EUDEV_TYPE_DRIVE_REMOVABLE */
   type = e_udev_find_by_type(EUDEV_TYPE_DRIVE_REMOVABLE, NULL);
   EINA_LIST_FOREACH(type, l, name)  /* get a property using the device's syspath */
     printf("\tOoh, a %s attached on your %s bus!\n", e_udev_syspath_get_property(name, "ID_MODEL"),
       e_udev_syspath_get_property(name, "ID_BUS"));
   eina_list_free(type);


   /* set a udev watch, grab all events because no EUDEV_TYPE filter is specified,
    * set the events to be sent to callback function catch_events(), and attach
    * kbdmouse to the watch as associated data
    */
   e_udev_watch_add(EUDEV_TYPE_NONE, catch_events, akbdmouse);
   printf("\nAnd now for something more complicated.  Plug or unplug your keyboard or mouse for me.\n");

   /* main loop must be started to use ecore fd polling */
   ecore_main_loop_begin();

   return 0;
}
