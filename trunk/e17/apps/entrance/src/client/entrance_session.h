#ifndef _ENTRANCE_SESSION_H
#define _ENTRANCE_SESSION_H

#include<Edje.h>
#include<Evas.h>
#include<Ecore.h>
#include<Ecore_X.h>
#include<Ecore_Evas.h>
#include<stdio.h>
#include<limits.h>
#include<string.h>
#include<unistd.h>
#include<syslog.h>

#include "entrance_auth.h"
#include "entrance_config.h"
#include "entrance_user.h"

struct _Entrance_Session
{
   char *session;
   Ecore_Evas *ee;              /* the ecore_evas */
   Evas_Object *edje;           /* main theme edje */
   Entrance_Auth *auth;         /* encapsulated auth shit */
   Entrance_Config *config;     /* configuration options */

   int authed;
};

typedef struct _Entrance_Session Entrance_Session;

Entrance_Session *entrance_session_new(const char *config);
void entrance_session_ecore_evas_set(Entrance_Session * e, Ecore_Evas * ee);
void entrance_session_free(Entrance_Session * e);
void entrance_session_run(Entrance_Session * e);
int entrance_session_auth_user(Entrance_Session * e);
void entrance_session_user_reset(Entrance_Session * e);
void entrance_session_user_set(Entrance_Session * e, Entrance_User * user);
void entrance_session_user_session_default_set(Entrance_Session * e);
void entrance_session_start_user_session(Entrance_Session * e);
void entrance_session_xsession_set(Entrance_Session * e,
                                   const char *xsession);
void entrance_session_edje_object_set(Entrance_Session * e,
                                      Evas_Object * obj);
void entrance_session_list_add(Entrance_Session * e);
void entrance_session_user_list_add(Entrance_Session * e);
const char *entrance_session_default_xsession_get(Entrance_Session * e);

#endif
