#ifndef E_KEYS_H
#define E_KEYS_H

#include "e.h"

void     e_keys_init(void);
void     e_keys_grab(char *key, Ecore_Event_Key_Modifiers mods, int anymod);
void     e_keys_ungrab(char *key, Ecore_Event_Key_Modifiers mods, int anymod);

#endif
