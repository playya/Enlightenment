#ifndef _ENTRANCE_AUTH
#define _ENTRANCE_AUTH

#include "../config.h"
#include "entrance_config.h"
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#ifdef HAVE_PAM
#   include <security/pam_appl.h>
#endif

#ifdef HAVE_CRYPT_H
#   include <crypt.h>
#endif

#ifdef HAVE_SHADOW
#   include <shadow.h>
#endif

/**
@file entrance_auth.h
@brief Declares Entrance_Auth struct, and return value types
*/

#define AUTH_SUCCESS 0
#define E_SUCCESS 0
#define ERROR_NO_PAM_INIT 1
#define ERROR_BAD_PASS 2
#define ERROR_PAM_SET 3
#define ERROR_NO_PERMS 4
#define ERROR_CRED_EXPIRED 5
#define ERROR_BAD_CRED 6

/**
 * Collection of data relating to authenticating off of the system
 */
struct _Entrance_Auth
{
#ifdef HAVE_PAM
   struct
   {
      struct pam_conv conv;
      pam_handle_t *handle;
   }
   pam;
#endif

   struct passwd *pw;
   char   user[PATH_MAX];
   char   pass[PATH_MAX];
   char   **env;
};
typedef struct _Entrance_Auth Entrance_Auth;

Entrance_Auth *entrance_auth_new(void);
void entrance_auth_free(Entrance_Auth * e);
void entrance_auth_session_end(Entrance_Auth * e);
void entrance_auth_clear_pass(Entrance_Auth * e);

/* 0 on success, 1 on failure */
int entrance_auth_cmp_pam(Entrance_Auth * e);
int entrance_auth_cmp_crypt(Entrance_Auth * e, Entrance_Config * cfg);
void entrance_auth_set_pass(Entrance_Auth * e, const char *str);

/* 0 on success, 1 on no user by that name */
int entrance_auth_set_user(Entrance_Auth * e, const char *str);
void entrance_auth_setup_environment(Entrance_Auth * e);

#endif
