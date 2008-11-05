#include <E_Nm.h>
#include <Ecore_Data.h>

E_NM *nm = NULL;

static int
cb_get_devices(void *data, void *reply)
{
    Ecore_List *list;
    E_NM_Device *device;

    list = reply;
    if (list)
    {
        ecore_list_first_goto(list);
        while ((device = ecore_list_next(list)))
            e_nm_device_dump(device);
        ecore_list_destroy(list);
    }
    ecore_main_loop_quit();
    return 1;
}

static int
cb_nm(void *data, void *reply)
{
    if (!reply)
    {
        ecore_main_loop_quit();
        return 1;
    }
    nm = reply;
    e_nm_dump(nm);
    e_nm_get_devices(nm, cb_get_devices, nm);
    return 1;
}
   

int 
main(int argc, char **argv)
{
    ecore_init();
    eina_stringshare_init();
    e_dbus_init();
   
    if (!e_nm_get(cb_nm, (void *)0xdeadbeef))
    {
        printf("Error connecting to system bus. Is it running?\n");
        return 1;
    }
   
    ecore_main_loop_begin();
    e_nm_free(nm);
   
    e_dbus_shutdown();
    eina_stringshare_shutdown();
    ecore_shutdown();
    return 0;
}
