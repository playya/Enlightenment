#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Disk.h>
#include "eeze_udev_private.h"
#include "eeze_disk_private.h"

_udev *udev;

int _eeze_udev_log_dom = -1;
int _eeze_init_count = 0;

static Eeze_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Eeze_Version *eeze_version = &_version;

EAPI int
eeze_init(void)
{
   if (++_eeze_init_count != 1)
     return _eeze_init_count;

   if (!eina_init())
     return 0;

   _eeze_udev_log_dom = eina_log_domain_register
       ("eeze_udev", EINA_COLOR_CYAN);

   if (_eeze_udev_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'eeze_udev' log domain.");
        goto eina_fail;
     }

   if (!ecore_init())
     goto fail;
#ifdef HAVE_EEZE_MOUNT
   if (!eeze_disk_init())
     goto eeze_fail;
#endif
   if (!((udev) = udev_new()))
     {
        EINA_LOG_ERR("Could not initialize udev library!");
        goto ecore_fail;
     }

   return _eeze_init_count;

ecore_fail:
#ifdef HAVE_EEZE_MOUNT
   eeze_disk_shutdown();
eeze_fail:
#endif
   ecore_shutdown();
fail:
   eina_log_domain_unregister(_eeze_udev_log_dom);
   _eeze_udev_log_dom = -1;
eina_fail:
   eina_shutdown();
   return 0;
}

EAPI int
eeze_shutdown(void)
{
   if (--_eeze_init_count != 0)
     return _eeze_init_count;

   udev_unref(udev);
#ifdef HAVE_EEZE_MOUNT
   eeze_disk_shutdown();
#endif
   ecore_shutdown();
   eina_log_domain_unregister(_eeze_udev_log_dom);
   _eeze_udev_log_dom = -1;
   eina_shutdown();
   return _eeze_init_count;
}

