#ifndef ELSA_CONFIG_H_
#define ELSA_CONFIG_H_

#define ELSA_CONFIG_FILE        "elsa.cfg"

typedef struct _Elsa_Config Elsa_Config;

struct _Elsa_Config
{
   const char *session_path;
   struct
     {
        const char *xinit_path;
        const char *xinit_args;
        const char *xauth_path;
        const char *xauth_file;
        const char *session_start;
        const char *session_login;
        const char *session_stop;
        const char *shutdown;
        const char *reboot;
        const char *suspend;
     } command;
   Eina_Bool daemonize;// :1;
   Eina_Bool numlock;// :1;
   Eina_Bool xsessions;
   Eina_Bool autologin;
   const char *userlogin;
   const char *lockfile;
   const char *logfile;
   const char *theme;
   char *last_session;
};

Elsa_Config *elsa_config;

void elsa_config_init();
void elsa_config_last_session_set(const char *name);
void elsa_config_shutdown();

 /** @} */
#endif /* ELSA_CONFIG_H_ */
