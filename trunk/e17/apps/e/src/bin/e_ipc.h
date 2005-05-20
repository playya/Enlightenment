/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS

typedef enum _E_Ipc_Domain
{
   E_IPC_DOMAIN_NONE,
   E_IPC_DOMAIN_SETUP,
   E_IPC_DOMAIN_REQUEST,
   E_IPC_DOMAIN_REPLY,
   E_IPC_DOMAIN_EVENT,
   E_IPC_DOMAIN_LAST
} E_Ipc_Domain;

typedef enum _E_Ipc_Op
{
   E_IPC_OP_NONE,
   E_IPC_OP_MODULE_LOAD,
   E_IPC_OP_MODULE_UNLOAD,
   E_IPC_OP_MODULE_ENABLE,
   E_IPC_OP_MODULE_DISABLE,
   E_IPC_OP_MODULE_LIST,
   E_IPC_OP_MODULE_LIST_REPLY,
   E_IPC_OP_BG_SET,
   E_IPC_OP_BG_GET,
   E_IPC_OP_BG_GET_REPLY,

   E_IPC_OP_FONT_AVAILABLE_LIST,
   E_IPC_OP_FONT_AVAILABLE_LIST_REPLY,
   E_IPC_OP_FONT_APPLY,
   E_IPC_OP_FONT_FALLBACK_CLEAR,
   E_IPC_OP_FONT_FALLBACK_APPEND,
   E_IPC_OP_FONT_FALLBACK_PREPEND,
   E_IPC_OP_FONT_FALLBACK_LIST,
   E_IPC_OP_FONT_FALLBACK_LIST_REPLY,
   E_IPC_OP_FONT_FALLBACK_REMOVE,
   E_IPC_OP_FONT_DEFAULT_SET,
   E_IPC_OP_FONT_DEFAULT_GET,
   E_IPC_OP_FONT_DEFAULT_GET_REPLY,
   E_IPC_OP_FONT_DEFAULT_REMOVE,
   E_IPC_OP_FONT_DEFAULT_LIST,
   E_IPC_OP_FONT_DEFAULT_LIST_REPLY,
   E_IPC_OP_RESTART,
   E_IPC_OP_SHUTDOWN,
   E_IPC_OP_LANG_LIST,
   E_IPC_OP_LANG_LIST_REPLY,
   E_IPC_OP_LANG_SET,
   E_IPC_OP_LANG_GET,
   E_IPC_OP_LANG_GET_REPLY,
   E_IPC_OP_BINDING_MOUSE_LIST,
   E_IPC_OP_BINDING_MOUSE_LIST_REPLY,
   E_IPC_OP_BINDING_MOUSE_ADD,
   E_IPC_OP_BINDING_MOUSE_DEL,
   E_IPC_OP_BINDING_KEY_LIST,
   E_IPC_OP_BINDING_KEY_LIST_REPLY,
   E_IPC_OP_BINDING_KEY_ADD,
   E_IPC_OP_BINDING_KEY_DEL,
   E_IPC_OP_MENUS_SCROLL_SPEED_SET,
   E_IPC_OP_MENUS_SCROLL_SPEED_GET,
   E_IPC_OP_MENUS_SCROLL_SPEED_GET_REPLY,
   E_IPC_OP_MENUS_FAST_MOVE_THRESHHOLD_SET,
   E_IPC_OP_MENUS_FAST_MOVE_THRESHHOLD_GET,
   E_IPC_OP_MENUS_FAST_MOVE_THRESHHOLD_GET_REPLY,
   E_IPC_OP_MENUS_CLICK_DRAG_TIMEOUT_SET,
   E_IPC_OP_MENUS_CLICK_DRAG_TIMEOUT_GET,
   E_IPC_OP_MENUS_CLICK_DRAG_TIMEOUT_GET_REPLY,
   E_IPC_OP_BORDER_SHADE_ANIMATE_SET,
   E_IPC_OP_BORDER_SHADE_ANIMATE_GET,
   E_IPC_OP_BORDER_SHADE_ANIMATE_GET_REPLY,
   E_IPC_OP_BORDER_SHADE_TRANSITION_SET,
   E_IPC_OP_BORDER_SHADE_TRANSITION_GET,
   E_IPC_OP_BORDER_SHADE_TRANSITION_GET_REPLY,
   E_IPC_OP_BORDER_SHADE_SPEED_SET,
   E_IPC_OP_BORDER_SHADE_SPEED_GET,
   E_IPC_OP_BORDER_SHADE_SPEED_GET_REPLY,
   E_IPC_OP_FRAMERATE_SET,
   E_IPC_OP_FRAMERATE_GET,
   E_IPC_OP_FRAMERATE_GET_REPLY,
   E_IPC_OP_IMAGE_CACHE_SET,
   E_IPC_OP_IMAGE_CACHE_GET,
   E_IPC_OP_IMAGE_CACHE_GET_REPLY,
   E_IPC_OP_FONT_CACHE_SET,
   E_IPC_OP_FONT_CACHE_GET,
   E_IPC_OP_FONT_CACHE_GET_REPLY,
   E_IPC_OP_USE_EDGE_FLIP_SET,
   E_IPC_OP_USE_EDGE_FLIP_GET,
   E_IPC_OP_USE_EDGE_FLIP_GET_REPLY,
   E_IPC_OP_EDGE_FLIP_TIMEOUT_SET,
   E_IPC_OP_EDGE_FLIP_TIMEOUT_GET,
   E_IPC_OP_EDGE_FLIP_TIMEOUT_GET_REPLY,
   
   /* Module PATH IPC */
   E_IPC_OP_MODULE_DIRS_LIST,
   E_IPC_OP_MODULE_DIRS_LIST_REPLY,
   E_IPC_OP_MODULE_DIRS_APPEND,
   E_IPC_OP_MODULE_DIRS_PREPEND,
   E_IPC_OP_MODULE_DIRS_REMOVE,

   /* Theme PATH IPC */
   E_IPC_OP_THEME_DIRS_LIST,
   E_IPC_OP_THEME_DIRS_LIST_REPLY,
   E_IPC_OP_THEME_DIRS_APPEND,
   E_IPC_OP_THEME_DIRS_PREPEND,
   E_IPC_OP_THEME_DIRS_REMOVE,

   /* Font Path IPC */
   E_IPC_OP_FONT_DIRS_LIST,
   E_IPC_OP_FONT_DIRS_LIST_REPLY,
   E_IPC_OP_FONT_DIRS_APPEND,
   E_IPC_OP_FONT_DIRS_PREPEND,
   E_IPC_OP_FONT_DIRS_REMOVE,

   /* data Path IPC */
   E_IPC_OP_DATA_DIRS_LIST,
   E_IPC_OP_DATA_DIRS_LIST_REPLY,
   E_IPC_OP_DATA_DIRS_APPEND,
   E_IPC_OP_DATA_DIRS_PREPEND,
   E_IPC_OP_DATA_DIRS_REMOVE,

   /* Images Path IPC */
   E_IPC_OP_IMAGE_DIRS_LIST,
   E_IPC_OP_IMAGE_DIRS_LIST_REPLY,
   E_IPC_OP_IMAGE_DIRS_APPEND,
   E_IPC_OP_IMAGE_DIRS_PREPEND,
   E_IPC_OP_IMAGE_DIRS_REMOVE,

  /* Init Path IPC */
   E_IPC_OP_INIT_DIRS_LIST,
   E_IPC_OP_INIT_DIRS_LIST_REPLY,
   E_IPC_OP_INIT_DIRS_APPEND,
   E_IPC_OP_INIT_DIRS_PREPEND,
   E_IPC_OP_INIT_DIRS_REMOVE,

  /* Icon Path IPC */
   E_IPC_OP_ICON_DIRS_LIST,
   E_IPC_OP_ICON_DIRS_LIST_REPLY,
   E_IPC_OP_ICON_DIRS_APPEND,
   E_IPC_OP_ICON_DIRS_PREPEND,
   E_IPC_OP_ICON_DIRS_REMOVE,

   /* Background Path IPC */
   E_IPC_OP_BG_DIRS_LIST,
   E_IPC_OP_BG_DIRS_LIST_REPLY,
   E_IPC_OP_BG_DIRS_APPEND,
   E_IPC_OP_BG_DIRS_PREPEND,
   E_IPC_OP_BG_DIRS_REMOVE,

   E_IPC_OP_DESKS_SET,
   E_IPC_OP_DESKS_GET,
   E_IPC_OP_DESKS_GET_REPLY,
     
   E_IPC_OP_FOCUS_POLICY_SET,
   E_IPC_OP_FOCUS_POLICY_GET,
   E_IPC_OP_FOCUS_POLICY_GET_REPLY,
     
   E_IPC_OP_LAST
     /* FIXME: add ipc: */
     /* get list of actions */
} E_Ipc_Op;

#else
#ifndef E_IPC_H
#define E_IPC_H

EAPI int  e_ipc_init(void);
EAPI void e_ipc_shutdown(void);

#endif
#endif
