#include <definitions>

__E_CFG_VERSION 0

/*
 * Global button bindings... specially named actionclass that applies to
 * all client windows - so you can bind "alt - left click" to move the
 * window, raise it or something.... 
 */

__ACLASS __BGN
  __NAME BUTTONBINDINGS
  __TYPE __TYPE_NORMAL
    __EVENT __MOUSE_PRESS
    __BUTTON 1
    __MODIFIER_KEY __ALT
    __ACTION __A_MOVE
  __NEXT_ACTION
    __EVENT __MOUSE_PRESS
    __BUTTON 1
    __MODIFIER_KEY __ALT_SHIFT
    __ACTION __A_SWAPMOVE
  __NEXT_ACTION
    __EVENT __DOUBLE_CLICK
    __BUTTON 1
    __MODIFIER_KEY __ALT
    __ACTION __A_SHADE
  __NEXT_ACTION
    __EVENT __MOUSE_PRESS
    __BUTTON 2
    __MODIFIER_KEY __ALT
    __ACTION __A_RESIZE
  __NEXT_ACTION
    __EVENT __DOUBLE_CLICK
    __BUTTON 2
    __MODIFIER_KEY __ALT
    __ACTION __A_MAX_HEIGHT available
  __NEXT_ACTION
    __EVENT __MOUSE_PRESS
    __BUTTON 3
    __MODIFIER_KEY __ALT
    __ACTION __A_SHOW_MENU "named WINOPS_MENU"
__END

/* what mouse presses do on the desktop background .... */
__ACLASS __BGN
  __NAME DESKBINDINGS
  __TYPE __TYPE_NORMAL
  __TOOLTIP_TEXT "Kliknięcie myszą na tym Pulpicie spowoduje"
  __TOOLTIP_TEXT "wykonanie następujących operacji"
    __TOOLTIP_ACTION_TEXT "Wyświetlenie Menu Użytkownika"
    __EVENT __MOUSE_PRESS
    __BUTTON 1
    __MODIFIER_KEY __NONE
    __ACTION __A_SHOW_MENU "named APPS_SUBMENU"
  __NEXT_ACTION
    __TOOLTIP_ACTION_TEXT "Wyświetlenie Menu Enlightenment"
    __EVENT __MOUSE_PRESS
    __BUTTON 1
    __MODIFIER_KEY __CTRL
    __ACTION __A_SHOW_MENU "named ROOT_2"
  __NEXT_ACTION
    __TOOLTIP_ACTION_TEXT "Wyświetlenie Menu Ustawień"
    __EVENT __MOUSE_PRESS
    __BUTTON 1
    __MODIFIER_KEY __WINDOWS_KEY
    __ACTION __A_SHOW_MENU "named CONFIG_SUBMENU"
  __NEXT_ACTION
    __TOOLTIP_ACTION_TEXT "Wyświetlenie Menu Enlightenment"
    __EVENT __MOUSE_PRESS
    __BUTTON 2
    __MODIFIER_KEY __NONE
    __ACTION __A_SHOW_MENU "named ROOT_2"
  __NEXT_ACTION
    __TOOLTIP_ACTION_TEXT "Wyświetlenia Menu Listy Zadań"
    __EVENT __MOUSE_PRESS
    __BUTTON 2
    __MODIFIER_KEY __ALT
    __ACTION __A_SHOW_MENU "taskmenu"
  __NEXT_ACTION
    __TOOLTIP_ACTION_TEXT "Wyświetlenie Menu Pulpitu"
    __EVENT __MOUSE_PRESS
    __BUTTON 2
    __MODIFIER_KEY __CTRL
    __ACTION __A_SHOW_MENU "deskmenu"
  __NEXT_ACTION
    __TOOLTIP_ACTION_TEXT "Wyświetlenie Menu Grupy"
    __EVENT __MOUSE_PRESS
    __BUTTON 2
    __MODIFIER_KEY __SHIFT
    __ACTION __A_SHOW_MENU "groupmenu"
  __NEXT_ACTION
    __TOOLTIP_ACTION_TEXT "Wyświetlenie Menu Ustawień"
    __EVENT __MOUSE_PRESS
    __BUTTON 3
    __MODIFIER_KEY __NONE
    __ACTION __A_SHOW_MENU "named CONFIG_SUBMENU"
  __NEXT_ACTION
    __TOOLTIP_ACTION_TEXT "Przejście do poprzedniego Pulpitu"
    __EVENT __MOUSE_PRESS
    __BUTTON 4
    __ALLOW_ANY_MODIFIER_KEYS __ON
    __ACTION __A_DESKTOP_PREV
  __NEXT_ACTION
    __TOOLTIP_ACTION_TEXT "Przejście do następnego Pulpitu"
    __EVENT __MOUSE_PRESS
    __BUTTON 5
    __ALLOW_ANY_MODIFIER_KEYS __ON
    __ACTION __A_DESKTOP_NEXT
__END


/*
******************************************************************************
* Actionclasses used for global keybindings
******************************************************************************
*
* Some Keys you can use to bind to (mainly standard English PC keyboard here)
* This is a SMALL list to make this file readable. it in no way lists ALL 
* possible keys that can be bound. This is just the useful keys from a normal
* PC-style 101 key keyboard.
*
* BackSpace
* Tab
* Return
* Pause
* Scroll_Lock
* Sys_Req
* Escape
* Delete
* Home
* Left
* Up
* Right
* Down
* Page_Up
* Page_Down
* End
* Print
* Insert
* Num_Lock
* KP_Enter
* KP_Multiply
* KP_Add
* KP_Separator
* KP_Subtract
* KP_Decimal
* KP_Divide
* KP_0
* KP_1
* KP_2
* KP_3
* KP_4
* KP_5
* KP_6
* KP_7
* KP_8
* KP_9
* F1
* F2
* F3
* F4
* F5
* F6
* F7
* F8
* F9
* F10
* F11
* F12
* space
* quoteright
* comma
* minus
* slash
* semicolon
* equal
* bracketleft
* bracketright
* backslash
* quoteleft
*
* There are more keys. please see /usr/X11R6/include/X11/keysymdef.h for a
* complete list fo keys (LOTS of them)
*/

__ACLASS __BGN
/*
* The action name is what you use to bind an actionclass to an
* object elsewhere in the configuration
*/
  __NAME KEYBINDINGS
  __TYPE __TYPE_GLOBAL
/*  
* The following key should be pretty self explanitory, but you should
* really see how the Keysym is set. in your keysym.h file in your X11
* distribution
*/
    __KEY Home
/*  
* Can I use this with any modifier? (default is no)
*/
    __MODIFIER_KEY __CTRL_ALT
/*  
* you can also use modifier to specify a specific mod mask (alt,ctrl,qshift,etc)
*/
    __EVENT __KEY_PRESS
    __ACTION __A_CLEANUP size
  __NEXT_ACTION
    __KEY Insert
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __CTRL_ALT
    __ACTION __A_EXEC Eterm
  __NEXT_ACTION
    __KEY v
    __MODIFIER_KEY __CTRL_ALT
    __EVENT __KEY_PRESS
    __ACTION __A_DESKRAY
  __NEXT_ACTION
    __KEY Delete
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __CTRL_ALT
    __ACTION __A_EXIT logout
  __NEXT_ACTION
    __KEY End
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __CTRL_ALT
    __ACTION __A_EXIT restart
  __NEXT_ACTION
    __KEY Right
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __CTRL_ALT
    __ACTION __A_DESKTOP_NEXT
  __NEXT_ACTION
    __KEY Left
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __CTRL_ALT
    __ACTION __A_DESKTOP_PREV
  __NEXT_ACTION
    __KEY Return
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __CTRL_ALT
    __ACTION __A_DESKTOP_INPLACE
  __NEXT_ACTION
    __KEY Down
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT_SHIFT
    __ACTION __A_AREA_MOVE_BY 0 1
  __NEXT_ACTION
    __KEY Up
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT_SHIFT
    __ACTION __A_AREA_MOVE_BY 0 -1
  __NEXT_ACTION
    __KEY Left
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT_SHIFT
    __ACTION __A_AREA_MOVE_BY -1 0
  __NEXT_ACTION
    __KEY Right
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT_SHIFT
    __ACTION __A_AREA_MOVE_BY 1 0
  __NEXT_ACTION
    __KEY F1
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 0
  __NEXT_ACTION
    __KEY F2
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 1
  __NEXT_ACTION
    __KEY F3
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 2
  __NEXT_ACTION
    __KEY F4
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 3
  __NEXT_ACTION
    __KEY F5
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 4
  __NEXT_ACTION
    __KEY F6
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 5
  __NEXT_ACTION
    __KEY F7
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 6
  __NEXT_ACTION
    __KEY F8
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 7
  __NEXT_ACTION
    __KEY F9
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 8
  __NEXT_ACTION
    __KEY F10
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 9
  __NEXT_ACTION
    __KEY F11
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 10
  __NEXT_ACTION
    __KEY F12
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __ALT
    __ACTION __A_GOTO_DESK 11
  __NEXT_ACTION
    __KEY Tab
    __MODIFIER_KEY __ALT
    __EVENT __KEY_PRESS
    __ACTION __A_FOCUS_NEXT
  __NEXT_ACTION
    __MODIFIER_KEY __CTRL_ALT
    __KEY Up
    __EVENT __KEY_PRESS
    __ACTION __A_RAISE
  __NEXT_ACTION
    __MODIFIER_KEY __CTRL_ALT
    __KEY Down
    __EVENT __KEY_PRESS
    __ACTION __A_LOWER
  __NEXT_ACTION
    __MODIFIER_KEY __CTRL_ALT
    __KEY x
    __EVENT __KEY_PRESS
    __ACTION __A_KILL
  __NEXT_ACTION
    __MODIFIER_KEY __CTRL_ALT
    __KEY k
    __EVENT __KEY_PRESS
    __ACTION __A_KILL_NASTY
  __NEXT_ACTION
    __MODIFIER_KEY __CTRL_ALT
    __KEY s
    __EVENT __KEY_PRESS
    __ACTION __A_STICK
  __NEXT_ACTION
    __MODIFIER_KEY __CTRL_ALT
    __KEY i
    __EVENT __KEY_PRESS
    __ACTION __A_ICONIFY
  __NEXT_ACTION
    __MODIFIER_KEY __CTRL_ALT
    __KEY r
    __EVENT __KEY_PRESS
    __ACTION __A_SHADE
  __NEXT_ACTION
    __MODIFIER_KEY __ALT
    __KEY Return
    __EVENT __KEY_PRESS
    __ACTION __A_ZOOM
__END

/*
 * These keybindings cant be edited because they aren't called "KEYBINDINGS" thus
 * they also can never be lost or accidentally deleted by users
 */
__ACLASS __BGN
  __NAME KEYBINDINGS_UNCHANGABLE
  __TYPE __TYPE_GLOBAL
    __KEY d
    __EVENT __KEY_PRESS
    __MODIFIER_KEY __CTRL_ALT
    __ACTION __A_DRAGDIR_SET
  __NEXT_ACTION
    __KEY o
    __MODIFIER_KEY __CTRL_ALT
    __EVENT __KEY_PRESS
    __ACTION __A_DRAGBAR_ORDER_SET
  __NEXT_ACTION
    __KEY c
    __MODIFIER_KEY __CTRL_ALT
    __EVENT __KEY_PRESS
    __ACTION __A_HIDESHOW_BUTTON buttons CONFIG*
  __NEXT_ACTION
    __KEY b
    __MODIFIER_KEY __CTRL_ALT
    __EVENT __KEY_PRESS
    __ACTION __A_HIDESHOW_BUTTON
  __NEXT_ACTION
    __KEY a
    __MODIFIER_KEY __CTRL_ALT
    __EVENT __KEY_PRESS
    __ACTION __A_HIDESHOW_BUTTON all
__END
