#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "auth.h"
#include "util.h"
#include "md5.h"
#include "../config.h"

#define BUFSIZE 512

#define AUTH_DATA_LEN 16

char * entranced_cookie_new (void) 
{
   int                     fd;
   int                     r, i;
   unsigned char           buf[BUFSIZE];
   unsigned char           digest[MD5_HASHBYTES];
   double                  ctime;
   pid_t                   pid;
   char                    *cookie;
   Entranced_MD5_Context   *ctx = NULL;

   entranced_md5_init(&ctx);

   ctime = ecore_time_get();
   entranced_md5_update(ctx, (unsigned char *) &ctime, sizeof(ctime));
   pid = getpid();
   entranced_md5_update(ctx, (unsigned char *) &pid, sizeof(pid));
   pid = getppid();
   entranced_md5_update(ctx, (unsigned char *) &pid, sizeof(pid));
   

   if ((fd = open("/dev/random", O_RDONLY|O_NONBLOCK)) < 0) {
      entranced_debug("Cookie generation failed: could not open /dev/urandom\n");
      return NULL;
   }

   if((r = read(fd, buf, sizeof(buf))) <= 0) {
      entranced_debug("Cookie generation failed: could not read /dev/urandom\n");
      return NULL;
   }

   entranced_md5_update(ctx, buf, r);
   entranced_md5_final(digest, ctx);

   cookie = calloc(40, 1);
   
   for (i = 0; i < MD5_HASHBYTES; ++i) {
      char tmp[3];
      snprintf(tmp, sizeof(tmp), "%02x", (unsigned int) digest[i]);
      strncat(cookie, tmp, 2);
   }

   return cookie;
}

static void
_entranced_auth_purge(Entranced_Display *d, FILE *auth_file)
{
   Xauth             *auth;
   Ecore_List_Node   *li;
   Ecore_List        *auth_keep = NULL;
   
   if (!d || !auth_file)
      return;

   entranced_debug("entranced_auth_purge: %s\n", d->name);
   fseek (auth_file, 0L, SEEK_SET);
   
   auth_keep = ecore_list_new();
   ecore_list_set_free_cb(auth_keep, ECORE_FREE_CB(XauDisposeAuth));

   /* Read each auth entry and check if it matches one for this
    * display */
   while ((auth = XauReadAuth(auth_file)))
   {
      int match;
      match = FALSE;

      for (li = d->auths->first; li; li = li->next)
      {
         Xauth *disp_auth = (Xauth *) li->data;
         if (!memcmp(disp_auth->address, auth->address, auth->address_length) &&
               !memcmp(disp_auth->number, auth->number, auth->number_length))
            match = TRUE;
      }

      if (match)
         XauDisposeAuth(auth);
      else
         ecore_list_append(auth_keep, auth);
   }

   /* Write remaining entries to auth file */
   if (!(auth_file = freopen(d->client.authfile, "w", auth_file)))
   {
      entranced_debug("entranced_auth_purge: Write failed!\n");
      return;
   }
   
   for(li = auth_keep->first; li; li = li->next)
   {
      Xauth *xa;
      xa = (Xauth *) li->data;
      XauWriteAuth(auth_file, (Xauth *) xa);
   }

   ecore_list_destroy(auth_keep);

}

/**
 * Generate a new MIT-MAGIC-COOKIE-1 Xauth struct
 *
 * @return  A pointer to a new Xauth struct with cookie data, or NULL if
 *          generation failed.
 */
Xauth * 
entranced_auth_mit_get(void)
{
   Xauth    *new;
   char     *cookie;

   new = (Xauth *) malloc(sizeof(Xauth));

   if (!new) return NULL;

   new->family = FamilyWild;
   new->address_length = 0;
   new->address = 0;
   new->number_length = 0;
   new->number = 0;

   new->data = (char *) malloc(AUTH_DATA_LEN);
   if(!new->data)
   {
      free(new);
      return NULL;
   }
   
   new->name = strdup("MIT-MAGIC-COOKIE-1");
   new->name_length = 18;
   cookie = entranced_cookie_new();
   if (!cookie)
   {
      free(new->name);
      free(new->data);
      free(new);
      return NULL;
   }
   
   memcpy(new->data, cookie, AUTH_DATA_LEN);
   new->data_length = AUTH_DATA_LEN;

   free(cookie);
   
   return new;
}

static Xauth *
_entranced_auth_generate(void)
{
   Xauth          *auth = NULL;
   unsigned short i;

   auth = entranced_auth_mit_get();
   if (auth)
   {
      entranced_debug("Generated MIT-MAGIC-COOKIE-1 ");
      for (i = 0; i < auth->data_length; ++i)
         entranced_debug(" %02x", auth->data[i] & 0xff);
      entranced_debug("\n");
   }
   else
      entranced_debug("Auth generation failed.\n");

   return auth;
}

static int 
_entranced_auth_entry_add(Entranced_Display *d, FILE *auth_file, const char *addr, int addrlen) 
{
   Xauth       *auth = NULL;
   char        dispnum[8];

   if (!d)
      return FALSE;

   /* Generate a new Xauth and set the address */
   if (!(auth = _entranced_auth_generate()))
      return FALSE;
   auth->address = calloc(1, addrlen);
   if(!auth->address)
   {
      free(auth);
      return FALSE;
   }
   memcpy(auth->address, addr, addrlen);
   auth->address_length = addrlen;

   /* Set the display number */
   snprintf(dispnum, 8, "%d", d->dispnum);
   auth->number = strdup(dispnum);
   auth->number_length = strlen(dispnum);

   if (!XauWriteAuth(auth_file, auth))
   {
      entranced_debug("_entrance_auth_entry_add: Auth write failed!\n");
      return FALSE;
   }

   if (!d->auths)
   {
      d->auths = ecore_list_new();
      ecore_list_set_free_cb(d->auths, ECORE_FREE_CB(XauDisposeAuth));
   }

   if (!ecore_list_append(d->auths, auth))
   {
      entranced_debug("_entrance_auth_entry_add: Could not add auth entry to list!\n");
      return FALSE;
   }

   return TRUE;

}

int
entranced_auth_display_secure (Entranced_Display *d)
{
   FILE              *auth_file;
/* FILE              *host_file; */
   char              buf[PATH_MAX];
   char              hostname[1024];

   if (!d)
      return FALSE;

   umask(022);

   entranced_debug("entranced_auth_display_secure: Setting up access for display %s\n", d->name);

   if (d->authfile)
      free(d->authfile);

   /* FIXME: Config-ize */
   snprintf(buf, PATH_MAX, PACKAGE_STATE_DIR"/%s%s", d->name, ".Xauth");
   d->authfile = strdup(buf);
   unlink(d->authfile);

   if(!(auth_file = fopen(d->authfile, "w")))
   {
      free(d->authfile);
      d->authfile = NULL;
      return FALSE;
   }
   
   /* XXX: This code assumes X server is running on localhost and
    *      may need to be modified for Xdmcp
    */
   ecore_list_goto_first(d->auths);
   while(d->auths->nodes)
   {
      Xauth *tmp;
      tmp = (Xauth *) ecore_list_remove(d->auths);
      XauDisposeAuth(tmp);
   }
   
   memset(hostname, 0, sizeof(hostname));
   if (!gethostname(hostname, 1023))
   {
      if (d->hostname)
         free(d->hostname);
      d->hostname = strdup(hostname);
   }

   /* Add auth entry for localhost */
   if (!_entranced_auth_entry_add(d, auth_file, d->hostname, 
                                 strlen(d->hostname)))
      return FALSE;

   fclose(auth_file);
   setenv("XAUTHORITY", d->authfile, TRUE);
#if 0
   /* Write host access file */
   snprintf(buf, PATH_MAX, "/etc/X%d.hosts", d->dispnum);
   if (!(host_file = fopen(buf, "w")))
   {
      entranced_debug("entranced_auth_display_secure: Unable to open %s for writing\n", buf);
      return FALSE;
   }
   fprintf(host_file, "%s\n", d->hostname);
   fclose(host_file);
#endif
   entranced_debug("entranced_auth_display_secure: Successfully set up access for %s (localhost)\n", d->name);

   return TRUE;
}

int
entranced_auth_user_add(Entranced_Display *d, const char *homedir)
{
   FILE              *auth_file;
   int               ret = TRUE;
   char              buf[PATH_MAX];
   Ecore_List_Node   *li;

   if (!d || !homedir)
      return FALSE;

   entranced_debug("entranced_auth_user_add: Adding auth cookie\n");

   while(1)
   {
      umask (077);

      if (!homedir)
         d->client.authfile = NULL;
      else
      {
         snprintf(buf, PATH_MAX, "%s/.Xauthority", homedir);
         d->client.authfile = strdup(buf);
      }

      /* Make sure the file can be written to */
      if((auth_file = fopen(d->client.authfile, "a+")))
         fclose(auth_file);
      else
      {
         entranced_debug("entranced_auth_user_add: Unable to write auth file %s\n", d->client.authfile);
         free(d->client.authfile);
         d->client.authfile = NULL;
         return FALSE;
      }
      /* TODO: May need a permissions/paranoia check */

      /* Lock the authorization file */
      /* FIXME: What if for some reason we never succeed in getting
       *        a lock ?
       */
      if (XauLockAuth(d->client.authfile, 3, 3, 0) != LOCK_SUCCESS)
      {
         syslog(LOG_CRIT, "entranced_auth_user_add: Unable to lock auth file %s", d->client.authfile);
         free(d->client.authfile);
         d->client.authfile = NULL;

         umask (022);
      }
      else
         break;
   }

   /* Open file and write auth entries */
   if(!(auth_file = fopen(d->client.authfile, "a+")))
   {
      syslog(LOG_CRIT, "entranced_auth_user_add: Open auth file %s failed after lock", d->client.authfile);
      XauUnlockAuth (d->client.authfile);
      free(d->client.authfile);
      d->client.authfile = NULL;

      umask (022);

      return FALSE;
   }

   entranced_debug("entranced_auth_user_add: Opened %s for writing cookies\n", d->client.authfile);

   /* Remove any existing old entries for this display */
   _entranced_auth_purge(d, auth_file);
   
   for (li = d->auths->first; li; li = li->next)
   {
      if (!XauWriteAuth (auth_file, (Xauth *) li->data))
      {
         syslog(LOG_CRIT, "entranced_user_auth_add: Unable to write cookie");
         ret = FALSE;
         break;
      }
   }

   fclose(auth_file);
   XauUnlockAuth(d->client.authfile);
   entranced_debug("entranced_auth_user_add: Finished writing auth entries to %s\n", d->client.authfile);

   return ret;
      
}

void
entranced_auth_user_remove (Entranced_Display *d)
{
   FILE     *auth_file;

   if (!d || !d->client.authfile)
      return;

   entranced_debug("entranced_auth_user_remove: Removing cookie from %s\n", d->client.authfile);

   /* TODO: Permissions check on auth file */
   /* Get a lock */
   if (XauLockAuth(d->client.authfile, 3, 3, 0) != LOCK_SUCCESS)
   {
      free(d->client.authfile);
      d->client.authfile = NULL;
      return;
   }

   /* Open the file */
   if (!(auth_file = fopen(d->client.authfile, "a+")))
   {
      XauUnlockAuth(d->client.authfile);
      free(d->client.authfile);
      d->client.authfile = NULL;
      return;
   }

   /* Remove cookies for this display */
   _entranced_auth_purge(d, auth_file);
   
   /* Close and unlock */
   fclose(auth_file);
   XauUnlockAuth(d->client.authfile);

   free(d->client.authfile);
   d->client.authfile = NULL;
}


