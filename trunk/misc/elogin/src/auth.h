#ifndef HAVE_AUTH_H
#define HAVE_AUTH_H 1

#include "elogin.h"
#include <security/pam_appl.h>

#define FAIL 0
#define SUCCESS 1

/* structs */
typedef struct _Userinfo Userinfo;

struct _Userinfo
{
   char *user;
   char *passwd;
   char **env;
};

/* funcs */
int elogin_auth_user(char *user, char*passwd);
int elogin_start_client(Userinfo *info);
void elogin_auth_cleanup(void);
#endif
