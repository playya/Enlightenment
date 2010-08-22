#ifndef IRC_H
# define IRC_H

#include <Ecore.h>
#include <Ecore_Con.h>

EMAPI extern Emote_Protocol_Api emote_protocol_api;

EMAPI int emote_protocol_init(Emote_Protocol *p);
EMAPI int emote_protocol_shutdown(Emote_Protocol *p);

#endif
