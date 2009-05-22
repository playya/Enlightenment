/** @file exalt_dns.c */
#include "exalt_dns.h"
#include "libexalt_private.h"

/**
 * @addtogroup Exalt_DNS
 * @{
 */


Eina_List* exalt_dns_get_list()
{
    FILE* f;
    char buf[1024];
    char *addr;
    Eina_List* l = NULL;

    f = fopen(EXALT_RESOLVCONF_FILE, "ro");
    EXALT_ASSERT_RETURN(f!=NULL);

    while(fgets(buf,1024,f))
    {
        buf[strlen(buf)-1] = '\0';
        //jump nameserver
        if(strlen(buf) > 13)
        {
            addr = buf + 11;
            if(exalt_address_is(addr))
                l = eina_list_append(l, strdup(addr));
        }
    }
    EXALT_FCLOSE(f);
    return l;
}




int exalt_dns_add(const char* dns)
{
    char buf[1024];
    FILE* f;
    int ret;

    EXALT_ASSERT_RETURN(dns!=NULL);
    EXALT_ASSERT_RETURN(exalt_address_is(dns));

    f = fopen(EXALT_RESOLVCONF_FILE, "a");
    EXALT_ASSERT_RETURN(f!=NULL);

    sprintf(buf,"nameserver %s\n", dns);
    ret = fwrite( buf, sizeof(char), strlen(buf), f);

    EXALT_FCLOSE(f);
    return 1;
}




int exalt_dns_delete(const char* dns)
{
    char buf[1024], buf2[1024];
    FILE* fw, *fr;
    int ret;

    EXALT_ASSERT_RETURN(dns!=NULL);

    ecore_file_cp(EXALT_RESOLVCONF_FILE, EXALT_TEMP_FILE);

    fr = fopen(EXALT_TEMP_FILE, "ro");
    EXALT_ASSERT_RETURN(fr!=NULL);

    fw = fopen(EXALT_RESOLVCONF_FILE, "w");
    EXALT_ASSERT_ADV(fw!=NULL,EXALT_FCLOSE(fr);return 0,"f!=NULL failed");

    sprintf(buf,"nameserver %s\n",dns);
    while(fgets(buf2,1024,fr))
        if( strcmp(buf,buf2) != 0)
            ret = fwrite( buf2, sizeof(char), strlen(buf2), fw);
    EXALT_FCLOSE(fr);
    EXALT_FCLOSE(fw);
    remove(EXALT_TEMP_FILE);
    return 1;
}




int exalt_dns_replace(const char* old_dns, const char* new_dns)
{
    char buf[1024], buf2[1024], buf3[1024];;
    FILE* fw, *fr;
    int ret;

    EXALT_ASSERT_RETURN(old_dns!=NULL);
    EXALT_ASSERT_RETURN(new_dns!=NULL);
    EXALT_ASSERT_RETURN(exalt_address_is(new_dns));

    ecore_file_cp(EXALT_RESOLVCONF_FILE, EXALT_TEMP_FILE);
    fr = fopen(EXALT_TEMP_FILE, "ro");
    EXALT_ASSERT_RETURN(fr!=NULL);

    fw = fopen(EXALT_RESOLVCONF_FILE, "w");
    EXALT_ASSERT_ADV(fw!=NULL,EXALT_FCLOSE(fr);return 0,"f!=NULL failed");

    sprintf(buf,"nameserver %s\n",old_dns);
    sprintf(buf3,"nameserver %s\n",new_dns);
    while(fgets(buf2,1024,fr))
        if( strcmp(buf,buf2) != 0)
            ret = fwrite( buf2, sizeof(char), strlen(buf2), fw);
        else
            ret = fwrite( buf3, sizeof(char), strlen(buf3), fw);
    EXALT_FCLOSE(fr);
    EXALT_FCLOSE(fw);
    remove(EXALT_TEMP_FILE);
    return 1;
}



void exalt_dns_printf()
{
    Eina_List* l = exalt_dns_get_list();
    Eina_List *l2;
    char *dns;

    printf("## DNS LIST ##\n");
    EINA_LIST_FOREACH(l,l2,dns)
        printf("%s\n",dns);
    eina_list_free(l);
}


/** @} */

