#ifdef EM_TYPEDEFS

typedef struct _Em_Gui Em_Gui;
typedef struct _Em_Gui_Server Em_Gui_Server;
typedef struct _Em_Gui_Channel Em_Gui_Channel;

#else
# ifndef EM_GUI_H
#  define EM_GUI_H

struct _Em_Gui
{
   Em_Object em_obj_inherit;

   Evas_Object *win;
   Evas_Object *o_tb;
   Evas_Object *o_chansel;
   Evas_Object *o_box;
   Evas_Object *o_block;
   Eina_Hash   *servers;
};

struct _Em_Gui_Server
{
   Em_Object em_obj_inherit;

   Evas_Object *box;
   Evas_Object *text;
   Evas_Object *entry;
   Evas_Object *nick;
   Eina_Hash   *channels;

   Elm_Hoversel_Item *select;
   Emote_Protocol *protocol;
};

struct _Em_Gui_Channel
{
   Em_Object em_obj_inherit;

   Em_Gui_Server *server;
   Evas_Object *box;
   Evas_Object *text;
   Evas_Object *entry;
   Evas_Object *nick;

   Elm_Hoversel_Item *select;
};

EM_INTERN int em_gui_init(void);
EM_INTERN void em_gui_server_add(const char *server, Emote_Protocol *p);
EM_INTERN void em_gui_server_del(const char *server, Emote_Protocol *p __UNUSED__);
EM_INTERN void em_gui_channel_add(const char *server, const char *channel, Emote_Protocol *p);
EM_INTERN void em_gui_channel_del(const char *server, const char *channel, Emote_Protocol *p);
EM_INTERN void em_gui_message_add(const char *server, const char *channel, const char *user, const char *text);
EM_INTERN void em_gui_nick_update(const char *server, const char *old, const char *new);
EM_INTERN void em_gui_channel_status_update(const char *server, const char *channel, const char *user, Emote_Protocol *p, Eina_Bool joined);
EM_INTERN void em_gui_topic_show(const char *server, const char *channel, const char *user, const char *message);
EM_INTERN void em_gui_user_list_add(const char *server, const char *channel, const char *users);
EM_INTERN int em_gui_shutdown(void);

# endif
#endif
