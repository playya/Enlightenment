#ifndef __ENTRANCED_AUTH_H
#define __ENTRANCED_AUTH_H

#include <Entranced.h>
#include <X11/Xauth.h>
#include <X11/Xos.h>

char  *entranced_cookie_new(void);
Xauth *entranced_auth_mit_get(void);
int    entranced_auth_display_secure(Entranced_Display *d);
int    entranced_auth_user_add(Entranced_Display *d, const char *homedir);
void   entranced_auth_user_remove(Entranced_Display *d);

#endif

