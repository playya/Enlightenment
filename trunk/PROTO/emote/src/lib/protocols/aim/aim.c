#include "aim.h"
#include "emote_private.h"

#include <Ecore.h>
#include <Ecore_Con.h>

EMAPI Emote_Protocol_Api protocol_api =
{
   /* version, name, label */
   EMOTE_PROTOCOL_API_VERSION, "aim", "AIM"
};

static Eina_Bool _aim_msg_send(void *data, int type, void *event);

static Emote_Protocol *m;

EMAPI int
protocol_init(Emote_Protocol *p)
{
   m = p;
   return 1;
}

EMAPI int
protocol_shutdown(void)
{
   return 1;
}

EMAPI int
protocol_connect(const char *server, int port, const char *user, const char *pass)
{
   return 1;
}

EMAPI int
protocol_disconnect(const char *server)
{
   return 1;
}

static Eina_Bool
_aim_msg_send(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   return EINA_TRUE;
}
