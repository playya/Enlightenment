/*
 * vim:ts=4
 * 
 * Copyright © 2009-2010 Rui Miguel Silva Seabra <rms@1407.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <pthread.h>

#ifdef HAVE_LIBFRAMEWORD_GLIB

#include <dbus/dbus.h>

#endif

#include <Elementary.h>
#include <Ecore_X.h>
#include <Azy.h>

#include <sqlite3.h>

#include <curl/curl.h>
#include <glib.h>
#include <glib/gprintf.h>

#include <time.h>

#include <locale.h>
#include "gettext.h"
#define _(string) gettext (string)

#include "elmdentica.h"
#include "settings.h"
#include "statusnet.h"
#include "curl.h"

Gui gui;

Evas_Object *status_list=NULL, *status=NULL, *error_win=NULL, *entry=NULL, *fs=NULL, *count=NULL, *hv=NULL, *related_inwin=NULL;
char theme[PATH_MAX];
char * dm_to=NULL;

StatusesList	*statuses=NULL;
int debug=0, mouse_x=0, mouse_y=0;
Evas_Coord finger_size;
int first_message=1;
int current_timeline=-1;
time_t now;

Eina_Hash * bubble2status=NULL;

long long int reply_id=0;
long long int user_id=0;
const char *url=NULL;
char * url_post = NULL;
char * url_friends = NULL;
extern char * browsers[];
extern char * browserNames[];
char * follow_user=NULL;
char * home=NULL;
Eina_Hash* accounts=NULL;
extern Eina_Hash* statusHash;
extern Eina_Hash* userHash;
extern Settings *settings;

extern CURL * user_agent;

double mouse_held_down=0;

struct sqlite3 *ed_DB=NULL;

GRegex *re_percent=NULL, *re_link=NULL, *re_link_content=NULL, *re_user=NULL, *re_tags=NULL, *re_group=NULL, *re_amp=NULL;
GError *re_err=NULL;

static Elm_Genlist_Item_Class itc1;

void add_status(aStatus *as, Elm_Genlist_Item *gli);

int my_strcmp(const void*a, const void*b) {
	return(strcmp((const char*)a, (const char*)b));
}


void hash_account_del(void *data) {
	Account *a = (Account*)data;

	switch(a->type) {
		case ACCOUNT_TYPE_STATUSNET: { ed_statusnet_account_free(a->details.snba); break; }
		default: break;
	}

	free(a);
}

static int load_account(void *notUsed, int argc, char **argv, char **azColName) {
	Account *account=calloc(1, sizeof(Account));
	char *key=NULL;
	int res=0;

	/* In this query handler, these are the current fields:
		argv[0]	== id INTEGER
		argv[1]	== enabled INTEGER
		argv[2]	== name TEXT
		argv[3]	== password TEXT
		argv[4]	== type INTEGER
		argv[5]	== proto TEXT
		argv[6]	== domain TEXT
		argv[7]	== port INTEGER
		argv[8]	== base_url TEXT
		argv[9]	== receive INTEGER
		argv[10] == send INTEGER
	*/

	if(!account) return(-1);

	account->sqlid = atoi(argv[0]);
	account->disabled = atoi(argv[1])?EINA_FALSE:EINA_TRUE;
	account->type = atoi(argv[4]);

	switch(account->type) {
		case ACCOUNT_TYPE_STATUSNET: {
			account->details.snba = (StatusNetBaAccount*)calloc(1, sizeof(StatusNetBaAccount));

			if(!account->details.snba) {
				free(account);
				return(-1);
			}

			account->details.snba->screen_name = strndup(argv[2], NAME_MAX);
			account->details.snba->password = strndup(argv[3], NAME_MAX);
			account->details.snba->proto = strndup(argv[5], NAME_MAX);
			account->details.snba->domain = strndup(argv[6], NAME_MAX);
			account->details.snba->port = atoi(argv[7]);
			account->details.snba->base_url = strndup(argv[8], NAME_MAX);

			break;
		}
		default: {
			free(account);
			return(-1);
		}
	}

	account->receive = atoi(argv[9]);
	account->send = atoi(argv[10]);

	res = asprintf(&key, "%s@%s", account->details.snba->screen_name, account->details.snba->domain);
	if(res != -1) {
		eina_hash_add(accounts, key, account);
	} else {
		if(account->details.snba->screen_name) free(account->details.snba->screen_name);
		if(account->details.snba->password) free(account->details.snba->password);
		if(account->details.snba->proto) free(account->details.snba->proto);
		if(account->details.snba->domain) free(account->details.snba->domain);
		if(account->details.snba->base_url) free(account->details.snba->base_url);
		free(account->details.snba->screen_name);
		free(account);
		return(-1);
	}
	return(0);
}

void elmdentica_init(void) {
	char *db_path=NULL, *db_err=NULL;
	int sqlite_res=0;
	char *query = NULL;

	sqlite_res = asprintf(&db_path, "%s/.elmdentica/db", getenv("HOME"));
	if(sqlite_res == -1) {
		perror("Too little free memory to even define the db's path\n");
		exit(2);
	}
	sqlite_res = sqlite3_open(db_path, &ed_DB);
	if(sqlite_res != 0) {
		printf("Can't open DB at %s: %d\n", db_path, sqlite_res);
		exit(sqlite_res);
	}
	chmod(db_path, S_IRUSR|S_IWUSR);
	free(db_path);

	query = "CREATE TABLE IF NOT EXISTS gag (id INTEGER PRIMARY KEY, enabled INTEGER, pattern TEXT);";
	sqlite_res = sqlite3_exec(ed_DB, query, NULL, NULL, &db_err);
	if(sqlite_res != 0) {
		printf("Can't run %s: %d => %s\n", query, sqlite_res, db_err);
		exit(sqlite_res);
	}

	query = "CREATE TABLE IF NOT EXISTS accounts (id INTEGER PRIMARY KEY, enabled INTEGER, name TEXT, password TEXT, type INTEGER, proto TEXT, domain TEXT, port INTEGER, base_url TEXT, receive INTEGER, send INTEGER );";
	sqlite_res = sqlite3_exec(ed_DB, query, NULL, NULL, &db_err);
	if(sqlite_res != 0) {
		printf("Can't run %s: %d => %s\n", query, sqlite_res, db_err);
		exit(sqlite_res);
	}

	query = "CREATE TABLE IF NOT EXISTS messages (id INTEGER PRIMARY KEY, status_id INTEGER, account_id INTEGER CONSTRAINT account_id_ref REFERENCES accounts (id), timeline INTEGER, s_text TEXT, s_truncated INTEGER, s_created_at INTEGER, s_in_reply_to_status_id INTEGER, s_source TEXT, s_in_reply_to_user_id INTEGER, s_favorited INTEGER, s_user INTEGER);";
	sqlite_res = sqlite3_exec(ed_DB, query, NULL, NULL, &db_err);
	if(sqlite_res != 0) {
		printf("Can't run %s: %d => %s\n", query, sqlite_res, db_err);
		exit(sqlite_res);
	}

	query = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, uid INTEGER, account_id INTEGER CONSTRAINT account_id_ref REFERENCES accounts (id), name TEXT, screen_name TEXT, location TEXT, description TEXT, profile_image_url TEXT, url TEXT, protected INTEGER, followers_count INTEGER, friends_count INTEGER, created_at INTEGER, favorites_count INTEGER, statuses_count INTEGER, following INTEGER, statusnet_blocking INTEGER);";
	sqlite_res = sqlite3_exec(ed_DB, query, NULL, NULL, &db_err);
	if(sqlite_res != 0) {
		printf("Can't run %s: %d => %s\n", query, sqlite_res, db_err);
		exit(sqlite_res);
	}

	query = "CREATE TABLE IF NOT EXISTS posts (account_id INTEGER CONSTRAINT account_id_ref REFERENCES accounts (id), dm_to TEXT, message TEXT);";
	sqlite_res = sqlite3_exec(ed_DB, query, NULL, NULL, &db_err);
	if(sqlite_res != 0) {
		printf("Can't run %s: %d => %s\n", query, sqlite_res, db_err);
		exit(sqlite_res);
	}

	accounts = eina_hash_string_superfast_new(hash_account_del);

	sqlite3_exec(ed_DB, "SELECT * FROM accounts;", load_account, NULL, &db_err);
	if(sqlite_res != 0) {
		printf("Can't load accounts: %d => %s\n", sqlite_res, db_err);
	}

	if(eina_hash_population(accounts) == 0)
		on_settings(NULL, NULL, NULL);

	sqlite3_free(db_err);

	gui.win=NULL;
	gui.win_evas=NULL;
	gui.timeline=NULL;
	gui.pager=NULL;
	gui.main=NULL;
	gui.status_detail=NULL;
}

void toggle_fullscreen(Eina_Bool new_fullscreen) {
	elm_win_fullscreen_set(gui.win, new_fullscreen);
	settings->fullscreen=new_fullscreen;
}

void make_status_list(int timeline) {
	char *label;

	if(timeline != current_timeline) {
		switch(timeline) {
			case TIMELINE_USER:		{	label = _("Last messages...");		break; }
			case TIMELINE_PUBLIC:	{	label = _("Everyone...");	break; }
			case TIMELINE_FAVORITES: {	label = _("My favorites...");	break; }
			case TIMELINE_MENTIONS: {	label = _("Replies / mentions...");	break; }
			case TIMELINE_FRIENDS:
			default:				{	label = _("My friends...");	break; }
		}

		elm_win_title_set(gui.win, label);

		elm_genlist_clear(gui.timeline);
		current_timeline = timeline;
	}
}

void error_win_del(void *data, Evas_Object *zbr, void *event_info) {
	evas_object_del(error_win);
}

static int ed_mark_favorite(void *data, int argc, char **argv, char **azColName) {
	Evas_Object *bubble = (Evas_Object*)data;
	aStatus *as = eina_hash_find(bubble2status, &bubble);
	char *screen_name=NULL, *password=NULL, *proto=NULL, *domain=NULL, *base_url=NULL;
	int port=0, id=0;

	/* In this query handler, these are the current fields:
		argv[0] == name TEXT
		argv[1] == password TEXT
		argv[2] == type INTEGER
		argv[3] == proto TEXT
		argv[4] == domain TEXT
		argv[5] == port INTEGER
		argv[6] == base_url TEXT
		argv[7] == id INTEGER
	*/

	screen_name = argv[0];
	password = argv[1];
	proto = argv[3];
	domain = argv[4];
	port = atoi(argv[5]);
	base_url = argv[6];
	id = atoi(argv[7]);

	if(as->status->favorited) {
		switch(atoi(argv[2])) {
			case ACCOUNT_TYPE_STATUSNET:
			default: { ed_statusnet_favorite_destroy(id, screen_name, password, proto, domain, port, base_url, as->status->id); break; }
		}
	} else {
		switch(atoi(argv[2])) {
			case ACCOUNT_TYPE_STATUSNET:
			default: { ed_statusnet_favorite_create(id, screen_name, password, proto, domain, port, base_url, as->status->id); break; }
		}
	}
	return(0);
}

static void on_mark_favorite(void *data, Evas_Object *obj, void *event_info) {
	Elm_Genlist_Item *gli = (Elm_Genlist_Item*)data;
	aStatus *as = (aStatus*)elm_genlist_item_data_get(gli);
	int sqlite_res=0;
	char *db_err=NULL, *query=NULL;

	if(!as) return;

	sqlite_res = asprintf(&query, "SELECT name,password,type,proto,domain,port,base_url,id FROM accounts WHERE enabled = 1 and id = %d;", as->account_id);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, ed_mark_favorite, data, &db_err);
		if(sqlite_res != 0) {
			printf("Can't run %s: %d => %s\n", query, sqlite_res, db_err);
		}
		sqlite3_free(db_err);
		free(query);

		if(gui.hover) {
			evas_object_del(gui.hover);
			gui.hover = NULL;
		}
	}
}

static void on_repeat(void *data, Evas_Object *obj, void *event_info) {
	Elm_Genlist_Item *gli = (Elm_Genlist_Item*)data;
	aStatus *as = (aStatus*)elm_genlist_item_data_get(gli);

	if(as) {
		switch(as->account_type) {
			case ACCOUNT_TYPE_STATUSNET:
			default: { ed_statusnet_repeat(as->account_id, as->status->id); break; }
		}
	}

		if(gui.hover) {
			evas_object_del(gui.hover);
			gui.hover = NULL;
		}
}

static void on_view_related(void *data, Evas_Object *obj, void *event_info) {
	Elm_Genlist_Item *gli = (Elm_Genlist_Item*)data;
	aStatus *as = (aStatus*)elm_genlist_item_data_get(gli), *related_status=NULL;

	if(as) {
		switch(as->account_type) {
			case ACCOUNT_TYPE_STATUSNET:
			default: { ed_statusnet_status_get(as->account_id, as->status->in_reply_to_status_id, &related_status); break; }
		}
	}

	if(related_status)
		add_status(related_status, gli);
	else
		printf(_("Error importing related status\n"));

	if(gui.hover) {
		evas_object_del(gui.hover);
		gui.hover = NULL;
	}

}

static void on_reply(void *data, Evas_Object *obj, void *event_info) {
	Elm_Genlist_Item *gli = (Elm_Genlist_Item*)data;
	aStatus *as = (aStatus*)elm_genlist_item_data_get(gli);
	anUser *au = NULL;
	char * entry_str=NULL, *uid_str=NULL;
	int res = 0;

	if(as) {
		res = asprintf(&uid_str, "%lld", as->user);
		if(res != -1) {
			au = eina_hash_find(userHash, uid_str);
			if(au) {
				res = asprintf(&entry_str, "@%s: ", au->user->screen_name);
				if(res != -1) {
					elm_entry_entry_set(entry, entry_str);
					free(entry_str);
					elm_object_focus(entry);
					reply_id=as->status->id;
					elm_entry_cursor_end_set(entry);
				}
			}
			free(uid_str);
		}
	}

	if(gui.hover) {
		evas_object_del(gui.hover);
		gui.hover = NULL;
	}

	evas_object_show(hv);
}

static void on_dm(void *data, Evas_Object *obj, void *event_info) {
	Elm_Genlist_Item *gli = (Elm_Genlist_Item*)data;
	aStatus *as = (aStatus*)elm_genlist_item_data_get(gli);

	if(as) {
		elm_object_focus(entry);
		user_id=as->user;
	}

	if(gui.hover) {
		evas_object_del(gui.hover);
		gui.hover = NULL;
	}

	evas_object_show(hv);
}


static void on_open_url(void *data, Evas_Object *obj, void *event_info) {
	Evas_Object *url_win=(Evas_Object*)data;
	char * cmd = NULL;
	int sys_result = 0;

	evas_object_del(url_win);

	if (settings->browser < 0)
		return;

	sys_result = asprintf(&cmd, settings->browser_cmd, url);
	if(sys_result != -1) {
		sys_result = system(cmd);
		if(sys_result == -1) {
			fprintf(stderr, _("Error %d: %s\n"), errno, strerror(errno));
		} else if(sys_result != 0) {
			if(! WIFEXITED(sys_result)) {
				fprintf(stderr, _("System failed, child exited with status %d\n"), WEXITSTATUS(sys_result));
			}
		}
		free(cmd);
	}
}

anUser *user_info_get(aStatus *as, UserProfile *user) {
	anUser *au=NULL;

	switch(as->account_type) {
		case ACCOUNT_TYPE_STATUSNET:
		default: { au = ed_statusnet_user_get(as->account_id, user); break; }
	}

	return(au);
}

static int ed_user_follow(void *data, int argc, char **argv, char **azColName) {
	char *screen_name=NULL, *password=NULL, *proto=NULL, *domain=NULL, *base_url=NULL;
	int port=0, id=0;

	/* In this query handler, these are the current fields:
		argv[0] == name TEXT
		argv[1] == password TEXT
		argv[2] == type INTEGER
		argv[3] == proto TEXT
		argv[4] == domain TEXT
		argv[5] == port INTEGER
		argv[6] == base_url TEXT
		argv[7] == id INTEGER
	*/

	screen_name = argv[0];
	password = argv[1];
	proto = argv[3];
	domain = argv[4];
	port = atoi(argv[5]);
	base_url = argv[6];
	id = atoi(argv[7]);

	switch(atoi(argv[2])) {
		case ACCOUNT_TYPE_STATUSNET:
		default: { ed_statusnet_user_follow(id, screen_name, password, proto, domain, port, base_url, follow_user); break; }
	}

	follow_user=NULL;

	return(0);
}

static void on_user_follow(void *data, Evas_Object *obj, void *event_info) {
	Evas *e = evas_object_evas_get(obj);
	Evas_Object *user_win=NULL;
	aStatus *as = (aStatus*)data;
	int sqlite_res=0;
	char *db_err=NULL, *query=NULL;

	if(!as) return;

	sqlite_res = asprintf(&query, "SELECT name,password,type,proto,domain,port,base_url,id FROM accounts WHERE enabled = 1 and id = %d;", as->account_id);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, ed_user_follow, NULL, &db_err);
		if(sqlite_res != 0) {
			printf("Can't run %s: %d => %s\n", query, sqlite_res, db_err);
		}
		sqlite3_free(db_err);
		free(query);
		if(e) {
			user_win = evas_object_name_find(e, "user_win");
			evas_object_del(user_win);
		}
	}
}

static int ed_user_abandon(void *data, int argc, char **argv, char **azColName) {
	char *screen_name=NULL, *password=NULL, *proto=NULL, *domain=NULL, *base_url=NULL;
	int port=0, id=0;

	/* In this query handler, these are the current fields:
		argv[0] == name TEXT
		argv[1] == password TEXT
		argv[2] == type INTEGER
		argv[3] == proto TEXT
		argv[4] == domain TEXT
		argv[5] == port INTEGER
		argv[6] == base_url TEXT
		argv[7] == id INTEGER
	*/

	screen_name = argv[0];
	password = argv[1];
	proto = argv[3];
	domain = argv[4];
	port = atoi(argv[5]);
	base_url = argv[6];
	id = atoi(argv[7]);

	switch(atoi(argv[2])) {
		case ACCOUNT_TYPE_STATUSNET:
		default: { ed_statusnet_user_abandon(id, screen_name, password, proto, domain, port, base_url, follow_user); break; }
	}

	follow_user=NULL;

	return(0);
}

static void on_user_abandon(void *data, Evas_Object *obj, void *event_info) {
	Evas *e = evas_object_evas_get(obj);
	Evas_Object *user_win=NULL;
	aStatus *as = (aStatus*)data;
	int sqlite_res=0;
	char *db_err=NULL, *query=NULL;

	if(!as) return;

	sqlite_res = asprintf(&query, "SELECT name,password,type,proto,domain,port,base_url,id FROM accounts WHERE enabled = 1 and id = %d;", as->account_id);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, ed_user_abandon, NULL, &db_err);
		if(sqlite_res != 0) {
			printf("Can't run %s: %d => %s\n", query, sqlite_res, db_err);
		}
		sqlite3_free(db_err);
		free(query);
		user_win = evas_object_name_find(e, "user_win");
		if(user_win) evas_object_del(user_win);
	}
}

static void user_free(UserProfile *user) {
	if(user) {
		if(user->name) free(user->name);
		if(user->tmp) free(user->tmp);
		if(user->text) free(user->text);

		free(user);
	}
}

Evas_Object *ed_get_icon(long long int id, Evas_Object *parent) {
	Evas_Object *icon=NULL;
	int res=0;
	char *file_path=NULL;

	res = asprintf(&file_path, "%s/cache/icons/%lld", home, id);
	if(res != -1 && (icon = elm_icon_add(parent))) {
		elm_icon_file_set(icon, file_path, "fubar?");
		free(file_path);
	}
	return(icon);
}

static void user_win_del(void *data, Evas_Object *obj, void *event_info) {
	UserProfile *user = (UserProfile*)data;
	if(user) user_free(user);
	evas_object_del(obj);
}

static void on_handle_user(void *data, Evas_Object *obj, void *event_info) {
	Elm_List_Item *li = elm_list_selected_item_get(obj);
	Evas_Object *user_win=NULL, *icon=NULL, *bg=NULL, *table=NULL, *button=NULL, *label=NULL;
	UserProfile *user=NULL;
	aStatus *as = (aStatus*)data;
	char *description=NULL, *path=NULL;
	int res=0;
	struct stat buf;
	anUser *au=NULL;

	elm_list_item_selected_set(li, EINA_FALSE);

	if(!settings->online) return;

	follow_user=(char*)elm_list_item_label_get(li);

	user = calloc(1,sizeof(UserProfile));
	user->screen_name=follow_user;
	au = user_info_get(as, user);

	if(!au) return;

	user_win = elm_win_add(NULL, au->user->name, ELM_WIN_BASIC);
		evas_object_name_set(user_win, "user_win");
		evas_object_size_hint_min_set(user_win, 480, 480);
		evas_object_size_hint_max_set(user_win, 640, 640);
		elm_win_title_set(user_win, au->user->name);
		evas_object_smart_callback_add(user_win, "delete-request", user_win_del, user);

		bg = elm_bg_add(user_win);
			evas_object_size_hint_weight_set(bg, 1, 1);
			evas_object_size_hint_align_set(bg, -1, -1);
			elm_win_resize_object_add(user_win, bg);
		evas_object_show(bg);

		table=elm_table_add(user_win);
			evas_object_size_hint_weight_set(table, 1, 0);
			evas_object_size_hint_align_set(table, -1, 0);
			elm_win_resize_object_add(user_win, table);
			elm_table_padding_set(table, 20, 20);

			res = asprintf(&path, "%s/cache/icons/%lld", home, (long long int)au->user->id);

			if(res!=-1 && stat(path, &buf) == 0 ) {
				icon = elm_icon_add(user_win);
					evas_object_size_hint_weight_set(icon, 1, 1);
					evas_object_size_hint_align_set(icon, -1, -1);
					elm_icon_file_set(icon, path, "fubar?");
					elm_table_pack(table, icon, 0, 0, 1, 2);
				evas_object_show(icon);
				free(path);
			}

			label = elm_entry_add(user_win);
				evas_object_size_hint_weight_set(label, 1, 1);
				evas_object_size_hint_align_set(label, -1, -1);

				res = asprintf(&description, "%s is following %d and has %d followers.", au->user->name, au->user->friends_count, au->user->followers_count);
				elm_entry_line_wrap_set(label, EINA_TRUE);
				if(res!=-1) {
					elm_entry_entry_set(label, description);
					free(description);
				} else
					elm_entry_entry_set(label, _("Couldn't parse user info message..."));

				elm_table_pack(table, label, 1, 0, 1, 1);
			evas_object_show(label);

			button = elm_button_add(user_win);
				evas_object_size_hint_weight_set(button, 1, 1);
				evas_object_size_hint_align_set(button, 0.5, 0);

				if(!au->user->following && !au->user->protected) {
					elm_button_label_set(button, _("Follow"));
					evas_object_smart_callback_add(button, "clicked", on_user_follow, as);
				} else {
					elm_button_label_set(button, _("Stop following"));
					evas_object_smart_callback_add(button, "clicked", on_user_abandon, as);
				}
				elm_table_pack(table, button, 1, 1, 1, 1);
			evas_object_show(button);

			if(au->user->description) {
					label = elm_label_add(user_win);
						evas_object_size_hint_weight_set(label, 1, 1);
						evas_object_size_hint_align_set(label, -1, -1);
						elm_label_line_wrap_set(label, EINA_TRUE);
						elm_label_label_set(label, au->user->description);
					evas_object_show(label);
					elm_table_pack(table, label, 0, 2, 2, 1);
			}

		evas_object_show(table);

	    evas_object_resize(user_win, 480, 640);
	evas_object_show(user_win);

}

static void on_group_messages_view(void *data, Evas_Object *obj, void *event_info) {
	Evas_Object *s = (Evas_Object*)data, *label=NULL, *group_win = evas_object_smart_parent_get(obj);

	label = elm_label_add(group_win);
		evas_object_size_hint_weight_set(label, 1, 1);
		evas_object_size_hint_align_set(label, -1, -1);
		elm_label_line_wrap_set(label, EINA_TRUE);
		elm_label_label_set(label, _("Not implemented yet"));
		elm_scroller_content_set(s, label);
	evas_object_show(s);
}

static void on_group_leave(void *data, Evas_Object *obj, void *event_info);

static void on_group_join(void *data, Evas_Object *obj, void *event_info) {
	Evas *e = evas_object_evas_get(obj);
	Evas_Object *group_desc = NULL;
	GroupProfile *gp = (GroupProfile*)data;
	char *m;
	int res = 0;

	gp->failed = 0;
	ed_statusnet_group_join(gp);

	if(gp->failed) {
		printf("Error joining group %s: %s\n", gp->name, gp->error);
	} else {
		if(gp->member)
			res = asprintf(&m, _("You are a member of group %s along with %d other people.<br>«%s»"), gp->fullname, gp->member_count -1, gp->description);
		else
			res = asprintf(&m, _("You are not a member of group %s but %d people are.<br>«%s»"), gp->fullname, gp->member_count, gp->description);

		if(res != -1) {
			group_desc = evas_object_name_find(e, "group_desc");
			elm_label_label_set(group_desc, m);
			elm_button_label_set(obj, _("Leave"));
			evas_object_smart_callback_del(obj, "clicked", on_group_join);
			evas_object_smart_callback_add(obj, "clicked", on_group_leave, gp);
			free(m);
		}
	}
}

static void on_group_leave(void *data, Evas_Object *obj, void *event_info) {
	Evas *e = evas_object_evas_get(obj);
	Evas_Object *group_desc = NULL;
	GroupProfile *gp = (GroupProfile*)data;
	char *m;
	int res = 0;

	gp->failed = 0;
	ed_statusnet_group_leave(gp);

	if(gp->failed && debug)
		printf("Error leaving group %s: %s\n", gp->name, gp->error);
	else {
		if(gp->member)
			res = asprintf(&m, _("You are a member of group %s along with %d other people.<br>«%s»"), gp->fullname, gp->member_count -1, gp->description);
		else
			res = asprintf(&m, _("You are not a member of group %s but %d people are.<br>«%s»"), gp->fullname, gp->member_count, gp->description);

		if(res != -1) {
			group_desc = evas_object_name_find(e, "group_desc");
			elm_label_label_set(group_desc, m);
			elm_button_label_set(obj, _("Join"));
			evas_object_smart_callback_del(obj, "clicked", on_group_leave);
			evas_object_smart_callback_add(obj, "clicked", on_group_join, gp);
			free(m);
		}
	}
}

static void group_win_del(void *data, Evas_Object *obj, void *event_info) {
	GroupProfile *gp = (GroupProfile*)data;
	ed_statusnet_group_free(gp);
	evas_object_del(obj);
}

static void on_handle_group(void *data, Evas_Object *obj, void *event_info) {
	aStatus* as = (aStatus*)data;
	Elm_List_Item *li = elm_list_selected_item_get(obj);
	Evas_Object *group_win=NULL, *bg=NULL, *box=NULL, *label=NULL, *notify=NULL, *s=NULL, *box2=NULL, *icon=NULL, *button=NULL, *frame=NULL;
	GroupProfile *gp = (GroupProfile*)calloc(1, sizeof(GroupProfile));
	char *m, *path;
	int res = 0;
	struct stat buf;

	elm_list_item_selected_set(li, EINA_FALSE);

	if(!settings->online) return;

	gp->name=strdup(elm_list_item_label_get(li));
	gp->account_id = as->account_id;

	ed_statusnet_group_get(gp);

	if(gp->failed) {
		notify = elm_notify_add(gui.win);
			evas_object_size_hint_weight_set(notify, 1, 1);
			evas_object_size_hint_align_set(notify, -1, -1);
			label = elm_label_add(gui.win);
				evas_object_size_hint_weight_set(label, 1, 1);
				evas_object_size_hint_align_set(label, -1, -1);
				elm_label_label_set(label, gp->error);
				elm_label_line_wrap_set(label, EINA_TRUE);
			evas_object_show(label);
			elm_notify_content_set(notify, label);
			elm_notify_orient_set(notify, ELM_NOTIFY_ORIENT_TOP_RIGHT);
			elm_notify_parent_set(notify, gui.win);
			elm_notify_timeout_set(notify, 5);
		evas_object_show(notify);

		return;
	}

	group_win = elm_win_add(NULL, gp->name, ELM_WIN_BASIC);
		evas_object_name_set(group_win, "group_win");
		elm_win_title_set(group_win, gp->name);
		evas_object_smart_callback_add(group_win, "delete-request", group_win_del, gp);

		bg = elm_bg_add(group_win);
			evas_object_size_hint_weight_set(bg, 1, 1);
			evas_object_size_hint_align_set(bg, -1, 0);
			elm_win_resize_object_add(group_win, bg);
		evas_object_show(bg);

		box = elm_box_add(group_win);
			evas_object_size_hint_weight_set(box, 1, 1);
			evas_object_size_hint_align_set(box, -1, 0);
			elm_win_resize_object_add(group_win, box);

			frame = elm_frame_add(group_win);
				evas_object_name_set(frame, "group");
				evas_object_size_hint_weight_set(frame, 1, 1);
				evas_object_size_hint_align_set(frame, -1, 0);
				elm_frame_label_set(frame, gp->name);

			box2 = elm_box_add(group_win);
				evas_object_size_hint_weight_set(box2, 1, 1);
				evas_object_size_hint_align_set(box2, -1, 0);
				elm_box_horizontal_set(box2, EINA_TRUE);

				res = asprintf(&path, "%s/cache/icons/%s", home, gp->name);

				if(res != -1 && stat(path, &buf) == 0 ) {
					icon = elm_icon_add(group_win);
						evas_object_size_hint_weight_set(icon, 1, 1);
						evas_object_size_hint_align_set(icon, -1, -1);
						elm_icon_file_set(icon, path, "fubar?");
						elm_box_pack_end(box2, icon);
						free(path);
					evas_object_show(icon);
				}

				if(gp->member)
					res = asprintf(&m, _("You are a member of group %s along with %d other people.<br>«%s»"), gp->fullname, gp->member_count -1, gp->description);
				else
					res = asprintf(&m, _("You are not a member of group %s but %d people are.<br>«%s»"), gp->fullname, gp->member_count, gp->description);

				if(res != -1) {
					label = elm_label_add(group_win);
						evas_object_name_set(label, "group_desc");
						evas_object_size_hint_weight_set(label, 1, 1);
						evas_object_size_hint_align_set(label, -1, -1);
						elm_label_line_wrap_set(label, EINA_TRUE);

						elm_label_label_set(label, m);
						free(m);
						elm_box_pack_end(box2, label);
					evas_object_show(label);

				}
				elm_frame_content_set(frame, box2);
				elm_box_pack_end(box, frame);
			evas_object_show(frame);

			s = elm_scroller_add(group_win);
				evas_object_size_hint_weight_set(s, 1, 1);
				evas_object_size_hint_align_set(s, -1, -1);


			box2 = elm_box_add(group_win);
				evas_object_size_hint_weight_set(box2, 1, 1);
				evas_object_size_hint_align_set(box2, -1, 0);
				elm_box_horizontal_set(box2, EINA_TRUE);

				button = elm_button_add(group_win);
					evas_object_size_hint_weight_set(button, 1, 1);
					evas_object_size_hint_align_set(button, -1, 0);
					if(gp->member) {
						elm_button_label_set(button, _("Leave"));
						evas_object_smart_callback_add(button, "clicked", on_group_leave, gp);
					} else {
						elm_button_label_set(button, _("Join"));
						evas_object_smart_callback_add(button, "clicked", on_group_join, gp);
					}
					elm_box_pack_end(box2, button);
				evas_object_show(button);
				
				res = asprintf(&m, _("Last %d messages"), settings->max_messages);
				if(res != -1) {
					button = elm_button_add(group_win);
						evas_object_size_hint_weight_set(button, 1, 1);
						evas_object_size_hint_align_set(button, -1, 0);
						elm_button_label_set(button, m);
						evas_object_smart_callback_add(button, "clicked", on_group_messages_view, s);
						elm_box_pack_end(box2, button);
					evas_object_show(button);
					free(m);
				}

				elm_box_pack_start(box, box2);
				elm_box_pack_end(box, s);
			evas_object_show(box2);


		evas_object_show(box);


	evas_object_resize(group_win, 480, 640);
	evas_object_show(group_win);
}

static void on_url_dismissed(void *data, Evas_Object *obj, void *event_info) {
	evas_object_del(obj);
}

static void on_handle_url(void *data, Evas_Object *obj, void *event_info) {
	Elm_List_Item *li = elm_list_selected_item_get(obj);
	Evas_Object *url_win=NULL, *button=NULL, *frame=NULL;

	elm_list_item_selected_set(li, EINA_FALSE);

	if(!settings->online) return;

	url = elm_list_item_label_get(li);

	url_win = elm_hover_add(gui.win);
		elm_hover_target_set(url_win, gui.status_detail_links);
		
		frame = elm_frame_add(gui.win);
			elm_frame_label_set(frame, url);

			button = elm_button_add(gui.win);
				evas_object_size_hint_weight_set(button, 1, 1);
				evas_object_size_hint_align_set(button, 0.5, 0.5);
				elm_button_label_set(button, _("Check it out!"));
				evas_object_smart_callback_add(button, "clicked", on_open_url, url_win);
				elm_frame_content_set(frame, button);
			evas_object_show(button);
		evas_object_show(frame);

		elm_hover_content_set(url_win, "middle", frame);

		evas_object_smart_callback_add(url_win, "clicked", on_url_dismissed, NULL);
	evas_object_show(url_win);
}

static int ed_check_gag_message(void *user_data, int argc, char **argv, char **azColName) {
	GagData *gd = (GagData*)user_data;
	char *sn = NULL, *pattern=argv[0];
	int res = -1;

	/* In this query handler, these are the current fields:
		argv[0] == patern STRING
	*/

	if(strchr(pattern, '@')) {
		res = asprintf(&sn, "@%s", gd->screen_name);
		if(res == -1)
			sn = (char*)gd->screen_name;
	}

	if(debug > 2) printf("(%s|%s|%s) ~ %s ?", sn?sn:gd->screen_name, gd->name, gd->message, pattern);

	// only do costly matches if there's isn't a match already
	if(	gd->match == EINA_FALSE &&
			(g_regex_match_simple(pattern, sn?sn:gd->screen_name, G_REGEX_CASELESS, 0) ||
			 g_regex_match_simple(pattern, gd->name, G_REGEX_CASELESS, 0)        ||
			 g_regex_match_simple(pattern, gd->message, G_REGEX_CASELESS, 0))) {
		gd->match = EINA_TRUE;
		if(debug > 2) printf(" %s\n", "Yes");
	} else if(debug > 2) printf(" %s\n", "No");

	if(res != -1) free(sn);

	return(0);
}

Eina_Bool ed_check_gag(const char *screen_name, const char *name, const char *message) {
	GagData gd;
	char *query, *db_err=NULL;
	int sqlite_res = 0;

	gd.screen_name = screen_name;
	gd.name = name;
	gd.message = message;
	gd.match = EINA_FALSE;

	query = "SELECT pattern FROM gag where enabled = 1;";
	sqlite_res = sqlite3_exec(ed_DB, query, ed_check_gag_message, &gd, &db_err);
	if(sqlite_res != 0) {
		printf("Can't run %s: %s\n", query, db_err);
		sqlite3_free(db_err);
	}

	return(gd.match);
}

static int fetch_user_from_db_handler(void *user_data, int argc, char **argv, char **azColName) {
	anUser **pau = (anUser**)user_data;
	anUser *au=NULL;

	if(*pau) return(-1);
	*pau = calloc(1, sizeof(anUser));

	au = *pau;

	au->user = calloc(1, sizeof(statusnet_User));

/*
 argv[0]  := id                 INTEGER
 argv[1]  := uid                INTEGER
 argv[2]  := account_id         INTEGER
 argv[3]  := name               TEXT
 argv[4]  := screen_name        TEXT
 argv[5]  := location           TEXT
 argv[6]  := description        TEXT
 argv[7]  := profile_image_url  TEXT
 argv[8]  := url                TEXT
 argv[9]  := protected          INTEGER
 argv[10] := followers_count    INTEGER
 argv[11] := friends_count      INTEGER
 argv[12] := created_at         INTEGER
 argv[13] := favorites_count    INTEGER
 argv[14] := statuses_count     INTEGER
 argv[15] := following          INTEGER
 argv[16] := statusnet_blocking INTEGER

*/

	if(argv[3]) au->user->name = strndup(argv[3], PIPE_BUF);
	if(argv[4]) au->user->screen_name = strndup(argv[4], PIPE_BUF);
	if(argv[5]) au->user->location = strndup(argv[5], PIPE_BUF);
	if(argv[6]) au->user->description = strndup(argv[6], PIPE_BUF);
	if(argv[7]) au->user->profile_image_url = strndup(argv[7], PIPE_BUF);
	if(argv[8]) au->user->url = strndup(argv[8], PIPE_BUF);
	if(argv[9]) au->user->protected = atoi(argv[9]);
	if(argv[10]) au->user->followers_count = atoi(argv[10]);
	if(argv[11]) au->user->friends_count = atoi(argv[11]);
	if(argv[12]) au->created_at = atoi(argv[12]);
	if(argv[13]) au->user->favourites_count = atoi(argv[13]);
	if(argv[14]) au->user->statuses_count = atoi(argv[14]);
	if(argv[15]) au->user->following = atoi(argv[15]);
	if(argv[16]) au->user->statusnet_blocking = atoi(argv[16]);
	au->in_db = EINA_TRUE;

	return(0);
}

anUser *fetch_user_from_db(long long int uid) {
	char *query, *db_err=NULL;
	int sqlite_res = 0, res = 0;
	anUser *au = NULL;

	res = asprintf(&query, "SELECT users.* FROM users WHERE uid = %lld LIMIT 1;", uid);
	if(res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, fetch_user_from_db_handler, &au, &db_err);
		if(sqlite_res != 0) {
			printf("Can't run %s: %s\n", query, db_err);
			sqlite3_free(db_err);
		}
	} 
	return(au);
}

char *ed_shorten_text(const char *text) {
	GError *re_err=NULL;
	char *shortened_text=NULL;
	char *tmp=NULL;

	if(!re_percent)	re_percent  = g_regex_new("\%", G_REGEX_OPTIMIZE, 0, &re_err);
	tmp = g_regex_replace(re_percent, text, strlen(text), 0, "\%\%", 0, &re_err);
	shortened_text = tmp;

	if(!re_link)	re_link  = g_regex_new("([a-z]+://.*?)(?=\\s|$)", G_REGEX_OPTIMIZE, 0, &re_err);
	tmp = g_regex_replace(re_link, text, strlen(text), 0, "<u>[link]</u>", 0, &re_err);
	free(shortened_text);
	shortened_text = tmp;

	if(!re_amp)	re_amp  = g_regex_new("(&)(?!amp;)", G_REGEX_OPTIMIZE, 0, &re_err);
	tmp = g_regex_replace(re_amp, shortened_text, strlen(shortened_text), 0, "&amp;", 0, &re_err);
	free(shortened_text);
	shortened_text = tmp;

	return(shortened_text);
}

char *ed_status_label_get(void *data, Evas_Object *obj, const char *part) {
	aStatus *as = (aStatus *)data;
	char *shortened_text;
	int res=0,h=0;
	anUser *au;
	char buf[256], *key=NULL;
	time_t now, dt;
	struct tm date_tm, date_now;

	if (!strcmp(part, "text")) {
		shortened_text = ed_shorten_text(as->status->text);
		snprintf(buf, sizeof(buf), "%s", shortened_text);
		free(shortened_text);
	} else if (!strcmp(part, "name")) {
		res = asprintf(&key, "%lld", as->user);
		if(res != -1)  {
			au = eina_hash_find(userHash, key);
			snprintf(buf, sizeof(buf), "%s", au->user->name);
		} else snprintf(buf, sizeof(buf), "unknown");
	} else if (!strcmp(part, "date")) {
		time(&now);
		localtime_r(&now, &date_now);

		if(settings->rel_timestamps && localtime_r(&(as->created_at), &date_tm)) {
			dt = now - as->created_at;
			if(dt < 60)
				snprintf(buf, sizeof(buf), _("a few seconds ago"));
			else if(dt < 600)
				snprintf(buf, sizeof(buf), _("a few minutes ago"));
			else if(dt < 3600)
				snprintf(buf, sizeof(buf), _("~ %d minutes ago"), (int)dt/60);
			else if(dt < 86400) {
				if( (h = (int)dt/3600) > 1)
					snprintf(buf, sizeof(buf), _("~ %d hours ago"), h);
				else
					snprintf(buf, sizeof(buf), _("~ 1 hour ago"));
			} else
				strftime(buf, sizeof(buf), _("on %x"), &date_tm);
		} else if(!settings->rel_timestamps) {
				if(localtime_r(&(as->created_at), &date_tm)) {
					strftime(buf, sizeof(buf), _("%x %X"), &date_tm);
				} else snprintf(buf, sizeof(buf), _("some-when"));
		} else snprintf(buf, sizeof(buf), _("some-when"));
	}

	if(res != -1 && key) free(key);

	return(strdup(buf));
}

Evas_Object *ed_status_icon_get(void *data, Evas_Object *obj, const char *part) {
	aStatus *as = (aStatus *)data;
	Evas_Object *icon=NULL;

	if (!strcmp(part, "icon")) {
		icon = ed_get_icon(as->user, gui.win);
		//evas_object_smart_callback_add(icon, "clicked", on_bubble_icon_clicked, as);
	}
	return(icon);
}

Eina_Bool ed_status_state_get(void *data, Evas_Object *obj, const char *part) {
	return(EINA_FALSE);
}

void ed_status_del(void *data, Evas_Object *obj) {
}

static void on_close_status_status_action(void *data, Evas_Object *obj, void *event_info) {
	evas_object_del(gui.status_detail);
}

static void on_status_show_page_users(void *data, Evas_Object *obj, void *event_info) {
	Evas_Object *pager = (Evas_Object*)data;
	elm_pager_content_promote(pager, gui.status_detail_users);
}

static void on_status_show_page_links(void *data, Evas_Object *obj, void *event_info) {
	Evas_Object *pager = (Evas_Object*)data;
	elm_pager_content_promote(pager, gui.status_detail_links);
}

static void on_status_show_page_tags(void *data, Evas_Object *obj, void *event_info) {
	Evas_Object *pager = (Evas_Object*)data;
	elm_pager_content_promote(pager, gui.status_detail_tags);
}

static void on_status_show_page_groups(void *data, Evas_Object *obj, void *event_info) {
	Evas_Object *pager = (Evas_Object*)data;
	elm_pager_content_promote(pager, gui.status_detail_groups);
}

static void ed_status_status_action(void *data, Evas_Object *obj, void *event_info) {
	Elm_Genlist_Item *gli = (Elm_Genlist_Item*)data;
	Elm_List_Item *li=NULL;
	Elm_Toolbar_Item *ti=NULL;
	Evas_Object *box=NULL, *toolbar=NULL, *pager=NULL, *list=NULL, *button=NULL;
	aStatus *as = (aStatus*)elm_genlist_item_data_get(gli);
	anUser *au=NULL;
	GMatchInfo *user_matches=NULL, *link_matches=NULL, *tags_matches=NULL, *group_matches=NULL;
	char *match=NULL, *key=NULL;
	int res=0;

	if(!re_link)	re_link  = g_regex_new("([a-z]+://.*?)(?=\\s|$)", G_REGEX_OPTIMIZE, 0, &re_err);
	if(!re_user)	re_user  = g_regex_new("@([a-zA-Z0-9_]+)",      G_REGEX_OPTIMIZE, 0, &re_err);
	if(!re_tags)	re_tags  = g_regex_new("#([a-zA-Z0-9_]+)",      G_REGEX_OPTIMIZE, 0, &re_err);
	if(!re_group)	re_group = g_regex_new("!([a-zA-Z0-9_]+)",      G_REGEX_OPTIMIZE, 0, &re_err);

	if(gui.hover) {
		evas_object_del(gui.hover);
		gui.hover = NULL;
	}

	gui.status_detail=elm_win_inwin_add(gui.win);

	pager = elm_pager_add(gui.win);
		elm_object_style_set(pager, "fade");
		evas_object_size_hint_weight_set(pager, 1, 1);
		evas_object_size_hint_align_set(pager, -1, -1);
	evas_object_show(pager);

	box = elm_box_add(gui.win);
		elm_box_homogenous_set(box, EINA_FALSE);
		evas_object_size_hint_weight_set(box, 1, 1);
		evas_object_size_hint_align_set(box, -1, -1);

		toolbar = elm_toolbar_add(gui.win);
			evas_object_size_hint_weight_set(toolbar, 1, 1);
			evas_object_size_hint_align_set(toolbar, -1, 0);
			ti = elm_toolbar_item_append(toolbar, NULL, _("Users"), on_status_show_page_users, pager);

			list = elm_list_add(gui.win);
				evas_object_size_hint_weight_set(list, 1, 1);
				evas_object_size_hint_align_set(list, -1, -1);
				evas_object_smart_callback_add(list, "selected", on_handle_user, as);

				if(g_regex_match(re_user, as->status->text, 0, &user_matches)) {
					while((match = g_match_info_fetch(user_matches, 1))) {
						li = elm_list_item_sorted_insert(list, match, NULL, NULL, NULL, NULL, my_strcmp);
						g_match_info_next(user_matches, &re_err);
					}
				}
				g_match_info_free(user_matches);

				res = asprintf(&key, "%lld", as->user);
				if(res != -1) {
					au = eina_hash_find(userHash, key);
					if(au) li = elm_list_item_prepend(list, au->user->screen_name, NULL, NULL, NULL, NULL);
					free(key);
				}

				elm_list_go(list);
				gui.status_detail_users=list;
				elm_pager_content_push(pager, gui.status_detail_users);
				elm_pager_content_promote(pager, gui.status_detail_users);
			evas_object_show(list);

			if(g_regex_match(re_link, as->status->text, 0, &link_matches)) {
				elm_toolbar_item_append(toolbar, NULL, _("Links"), on_status_show_page_links, pager);

				list = elm_list_add(gui.win);
					evas_object_size_hint_weight_set(list, 1, 1);
					evas_object_size_hint_align_set(list, -1, -1);
					evas_object_smart_callback_add(list, "selected", on_handle_url, as);

					while((match = g_match_info_fetch(link_matches, 1))) {
						li = elm_list_item_sorted_insert(list, match, NULL, NULL, NULL, NULL, my_strcmp);
						g_match_info_next(link_matches, &re_err);
					}
					elm_list_go(list);
					elm_pager_content_push(pager, list);
					gui.status_detail_links=list;
				evas_object_show(list);
			}
			g_match_info_free(link_matches);

			if(re_tags && g_regex_match(re_tags, as->status->text, 0, &tags_matches)) {
				elm_toolbar_item_append(toolbar, NULL, _("Tags"), on_status_show_page_tags, pager);

				list = elm_list_add(gui.win);
					evas_object_size_hint_weight_set(list, 1, 1);
					evas_object_size_hint_align_set(list, -1, -1);

					while((match = g_match_info_fetch(tags_matches, 1))) {
						li = elm_list_item_sorted_insert(list, match, NULL, NULL, NULL, NULL, my_strcmp);
						g_match_info_next(tags_matches, &re_err);
					}
					elm_list_go(list);
					elm_pager_content_push(pager, list);
					gui.status_detail_tags=list;
				evas_object_show(list);
			}
			g_match_info_free(tags_matches);

			if(re_group && g_regex_match(re_group, as->status->text, 0, &group_matches)) {
				elm_toolbar_item_append(toolbar, NULL, _("Groups"), on_status_show_page_groups, pager);

				list = elm_list_add(gui.win);
					evas_object_size_hint_weight_set(list, 1, 1);
					evas_object_size_hint_align_set(list, -1, -1);
					evas_object_smart_callback_add(list, "selected", on_handle_group, as);

					while((match = g_match_info_fetch(group_matches, 1))) {
						li = elm_list_item_sorted_insert(list, match, NULL, NULL, NULL, NULL, my_strcmp);
						g_match_info_next(group_matches, &re_err);
					}
					elm_list_go(list);
					elm_pager_content_push(pager, list);
					gui.status_detail_groups=list;
				evas_object_show(list);
			}
			g_match_info_free(group_matches);

			elm_box_pack_end(box, toolbar);
		evas_object_show(toolbar);

		elm_box_pack_end(box, pager);

		button = elm_button_add(gui.win);
			evas_object_size_hint_weight_set(button, 1, 1);
			evas_object_size_hint_align_set(button, 0.5, 1);
			elm_button_label_set(button, _("Close"));
			evas_object_smart_callback_add(button, "clicked", on_close_status_status_action, NULL);
			elm_box_pack_end(box, button);
		evas_object_show(button);

	evas_object_show(box);

	elm_win_inwin_content_set(gui.status_detail, box);

	elm_toolbar_item_selected_set(ti, EINA_TRUE);
	evas_object_show(gui.status_detail);
}

void on_status_action_close(void *data, Evas_Object *obj, void *event_info) {
	evas_object_del((Evas_Object*)data);
}

void on_status_action(void *data, Evas_Object *obj, void *event_info) {
	Elm_Genlist_Item *gli = (Elm_Genlist_Item*)event_info;
	aStatus *as = (aStatus*)elm_genlist_item_data_get(gli);
	Evas_Object *box=NULL, *table=NULL, *button=NULL;

	if(elm_genlist_item_selected_get(gli) == EINA_FALSE) return;

	gui.hover = elm_hover_add(gui.win);
		evas_object_name_set(gui.hover, "hover_actions");
		box = elm_box_add(gui.win);
			table = elm_table_add(gui.win);

				button = elm_button_add(gui.win);
					evas_object_size_hint_weight_set(button, 1, 1);
					evas_object_size_hint_align_set(button, -1, 0);
					elm_button_label_set(button, _("Reply"));
					evas_object_smart_callback_add(button, "clicked", on_reply, event_info);
					elm_table_pack(table, button, 0, 0, 1, 1);
				evas_object_show(button);

				button = elm_button_add(gui.win);
					evas_object_size_hint_weight_set(button, 1, 1);
					evas_object_size_hint_align_set(button, -1, 0);
					elm_button_label_set(button, _("Repeat"));
					evas_object_smart_callback_add(button, "clicked", on_repeat, event_info);
					elm_table_pack(table, button, 1, 0, 1, 1);
				evas_object_show(button);

				button = elm_button_add(gui.win);
					evas_object_size_hint_weight_set(button, 1, 1);
					evas_object_size_hint_align_set(button, -1, 0);
					elm_button_label_set(button, _("DM"));
					evas_object_smart_callback_add(button, "clicked", on_dm, event_info);
					elm_table_pack(table, button, 2, 0, 1, 1);
				evas_object_show(button);

				button = elm_button_add(gui.win);
					evas_object_size_hint_weight_set(button, 1, 1);
					evas_object_size_hint_align_set(button, -1, 0);
					if(as->status->favorited)
						elm_button_label_set(button, _("Unmark favorite"));
					else
						elm_button_label_set(button, _("Mark favorite"));
					evas_object_smart_callback_add(button, "clicked", on_mark_favorite, event_info);
					elm_table_pack(table, button, 0, 1, 3, 1);
				evas_object_show(button);

				if(as->status->in_reply_to_status_id != 0) {
					button = elm_button_add(gui.win);
						evas_object_size_hint_weight_set(button, 1, 1);
						evas_object_size_hint_align_set(button, -1, 0);
						elm_button_label_set(button, _("View related"));
						evas_object_smart_callback_add(button, "clicked", on_view_related, event_info);
						elm_table_pack(table, button, 0, 2, 3, 1);
					evas_object_show(button);
				}

				button = elm_button_add(gui.win);
					evas_object_size_hint_weight_set(button, 1, 1);
					evas_object_size_hint_align_set(button, -1, 0);
					elm_button_label_set(button, _("Status details"));
					evas_object_smart_callback_add(button, "clicked", ed_status_status_action, event_info);
					elm_table_pack(table, button, 0, 3, 3, 1);
				evas_object_show(button);

				evas_object_show(table);
				elm_box_pack_end(box, table);

		evas_object_show(box);

		elm_hover_parent_set(gui.hover, gui.win);
		elm_hover_target_set(gui.hover, gui.timeline);
		elm_hover_content_set(gui.hover, "middle", box);
	evas_object_show(gui.hover);
}

void add_status(aStatus *as, Elm_Genlist_Item *gli) {
	Elm_Genlist_Item *li=NULL;
	Evas_Object *icon=NULL;
	char uid_str[PIPE_BUF];
	anUser *au=NULL;

	snprintf(uid_str, PIPE_BUF, "%lld", as->user);
	au = eina_hash_find(userHash, uid_str);
	if(!au) {
		au = fetch_user_from_db(as->user);
		if(au) eina_hash_add(userHash, uid_str, au);
		else return;
	}

	if(ed_check_gag(au->user->screen_name, au->user->name, as->status->text)) return;

	icon = ed_get_icon(as->user, gui.win);

	itc1.item_style		= "elmdentica";
	itc1.func.label_get	= ed_status_label_get;
	itc1.func.icon_get	= ed_status_icon_get;
	itc1.func.state_get	= ed_status_state_get;
	itc1.func.del		= ed_status_del;

	if(gli) {
		li = elm_genlist_item_insert_after(gui.timeline, &itc1, as, gli, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_show(li);
	} else
		li = elm_genlist_item_append(gui.timeline, &itc1, as, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	eina_hash_add(bubble2status, &li, as);
}

static int add_status_from_db(void *data, int argc, char **argv, char **azColName) {
	aStatus *as=NULL;

	char *sid_str=NULL;

	/* In this query handler, these are the current fields:
        argv[0]  := messages.id                       INTEGER
        argv[1]  := messages.s_id                     INTEGER
        argv[2]  := messages.account_id               INTEGER
        argv[3]  := messages.timeline                 INTEGER
        argv[4]  := messages.s_text                   TEXT
        argv[5]  := messages.s_truncated              INTEGER
        argv[6]  := messages.s_created_at             INTEGER
        argv[7]  := messages.s_in_reply_to_status_id  INTEGER
        argv[8]  := messages.s_source                 TEXT
        argv[9]  := messages.s_in_reply_to_user_id    INTEGER
        argv[10] := messages.s_favorited              INTEGER
        argv[11] := messages.s_user                   INTEGER
        argv[12] := accounts.type                     INTEGER
		argv[13] := accounts.id                       INTEGER
		argv[14] := accounts.enabled                  INTEGER
	*/

	sid_str = strndup(argv[1], PIPE_BUF);
	as = eina_hash_find(statusHash, sid_str);
	if(!as) {
		as = (aStatus*)calloc(1, sizeof(aStatus));
		if(!as) return(-1);
		as->status = (statusnet_Status*)calloc(1, sizeof(statusnet_Status));

		as->status->id = strtoll(argv[1], NULL, 10);
		as->status->text = strndup(argv[4], PIPE_BUF);
		as->created_at = atoi(argv[6]);
		as->status->in_reply_to_status_id = strtoll(argv[7], NULL, 10);
		as->status->in_reply_to_user_id = strtoll(argv[9], NULL, 10);
		as->status->favorited = atoi(argv[10]);
		as->user = strtoll(argv[11], NULL, 10);
		as->account_type = atoi(argv[12]);
		as->account_id = atoi(argv[13]);
		as->in_db = EINA_TRUE;
		eina_hash_add(statusHash, sid_str, as);
	}

	add_status(as, NULL);

	return(0);
}

void show_error(StatusesList * statuses) {
	Evas_Object *bg=NULL, *box=NULL, *frame1=NULL, *frame2=NULL, *label=NULL, *button=NULL;

	/* Error Window */
	error_win = elm_win_add(NULL, _("Error"), ELM_WIN_DIALOG_BASIC);
	elm_win_title_set(error_win, _("µ-blog Error"));
	evas_object_smart_callback_add(error_win, "delete-request", error_win_del, (void*)error_win);

	/* Background */
	bg = elm_bg_add(error_win);
	evas_object_size_hint_weight_set(bg, 1.0, 1.0);
	elm_win_resize_object_add(error_win, bg);
	evas_object_show(bg);

	/* Vertical Box */
	box = elm_box_add(error_win);
	evas_object_size_hint_weight_set(box, 1, 1);
	elm_win_resize_object_add(error_win, box);
	evas_object_show(box);
	
		/* First frame (with message) */
		frame1 = elm_frame_add(box);
			elm_frame_label_set(frame1, statuses->hash_request);
			label = elm_label_add(frame1);
				elm_label_label_set(label, statuses->hash_error);
				elm_frame_content_set(frame1, label);
			evas_object_show(label);
		elm_box_pack_end(box, frame1);
		evas_object_show(frame1);

		/* Second frame (with close button) */
		frame2 = elm_frame_add(box);
			button = elm_button_add(frame2);
				evas_object_smart_callback_add(button, "clicked", error_win_del, NULL);
				elm_button_label_set(button, _("Close"));
				elm_frame_content_set(frame2, button);
			evas_object_show(button);
		elm_box_pack_end(box, frame2);
		evas_object_show(frame2);

	evas_object_show(error_win);
}

void update_status_list(int timeline);

static int get_messages_for_account(void *pTimeline, int argc, char **argv, char **azColName) {
	char *screen_name=NULL, *password=NULL, *proto=NULL, *domain=NULL, *base_url=NULL;
	int port=0, id=0;
	int timeline = *(int*)pTimeline;
	/* In this query handler, these are the current fields:
		argv[0] == name TEXT
		argv[1] == password TEXT
		argv[2] == type INTEGER
		argv[3] == proto TEXT
		argv[4] == domain TEXT
		argv[5] == port INTEGER
		argv[6] == base_url TEXT
		argv[7] == id INTEGER
	*/

	screen_name = argv[0];
	password = argv[1];
	proto = argv[3];
	domain = argv[4];
	port = atoi(argv[5]);
	base_url = argv[6];
	id = atoi(argv[7]);

	switch(atoi(argv[2])) {
		case ACCOUNT_TYPE_STATUSNET:
		default: { ed_statusnet_timeline_get(id, screen_name, password, proto, domain, port, base_url, timeline, update_status_list); break; }
	}

	return(0);
}

static void get_messages(int timeline) {
	int sqlite_res=0;
	char *db_err=NULL, *query=NULL;

	elm_toolbar_item_disabled_set(gui.timelines, EINA_TRUE);
	elm_toolbar_item_disabled_set(gui.post, EINA_TRUE);
	query = "SELECT name,password,type,proto,domain,port,base_url,id FROM accounts WHERE enabled = 1 and receive = 1;";
	sqlite3_exec(ed_DB, query, get_messages_for_account, &timeline, &db_err);
	if(sqlite_res != 0) {
		printf("Can't run %s: %d => %s\n", query, sqlite_res, db_err);
	}
	sqlite3_free(db_err);
}

void fill_message_list(int timeline) {
	int sqlite_res=0;
	char *db_err=NULL, *query=NULL;

	if(bubble2status) {
		eina_hash_free(bubble2status);
		bubble2status = NULL;
	}

	bubble2status = eina_hash_pointer_new(NULL);
	if(!statusHash) statusHash = eina_hash_string_superfast_new(status_hash_data_free);
    if(!userHash) userHash = eina_hash_string_superfast_new(user_hash_data_free);


	sqlite_res = asprintf(&query, "SELECT messages.*, accounts.type, accounts.id, accounts.enabled FROM messages,accounts where messages.timeline = %d and messages.account_id=accounts.id and accounts.enabled=1 ORDER BY messages.s_created_at DESC LIMIT %d;", timeline, settings->max_messages);
	if(sqlite_res != -1) {
		sqlite_res = 0;
		sqlite3_exec(ed_DB, query, add_status_from_db, (void*)(long)timeline, &db_err);
		if(sqlite_res != 0) {
			printf("Can't run %s: %d => %s\n", query, sqlite_res, db_err);
		}
		sqlite3_free(db_err);
		free(query);
	}
}

void update_status_list(int timeline) {
	make_status_list(timeline);
	fill_message_list(timeline);
	elm_toolbar_item_disabled_set(gui.timelines, EINA_FALSE);
	elm_toolbar_item_disabled_set(gui.post, EINA_FALSE);
}

/* ********** CALLBACKS *********** */
static void on_timeline_friends_reload(void *data, Evas_Object *obj, void *event_info) {
	if(settings->online) get_messages(TIMELINE_FRIENDS);
	else
		update_status_list(TIMELINE_FRIENDS);
}

static void on_timeline_mentions_reload(void *data, Evas_Object *obj, void *event_info) {
	if(settings->online) get_messages(TIMELINE_MENTIONS);
	else
		update_status_list(TIMELINE_MENTIONS);
}

static void on_timeline_user_reload(void *data, Evas_Object *obj, void *event_info) {
	if(settings->online) get_messages(TIMELINE_USER);
	else
		update_status_list(TIMELINE_USER);
}

static void on_timeline_dmsgs_reload(void *data, Evas_Object *obj, void *event_info) {
	if(settings->online) get_messages(TIMELINE_DMSGS);
	else
		update_status_list(TIMELINE_DMSGS);
}

static void on_timeline_public_reload(void *data, Evas_Object *obj, void *event_info) {
	if(settings->online) get_messages(TIMELINE_PUBLIC);
	else
		update_status_list(TIMELINE_PUBLIC);
}

static void on_timeline_favorites_reload(void *data, Evas_Object *obj, void *event_info) {
	if(settings->online) get_messages(TIMELINE_FAVORITES);
	else
		update_status_list(TIMELINE_FAVORITES);
}

static void on_fs(void *data, Evas_Object *obj, void *event_info) {
	if(settings->fullscreen)
		settings->fullscreen=FALSE;
	else
		settings->fullscreen=TRUE;

	toggle_fullscreen(settings->fullscreen);

	elm_toolbar_item_selected_set(elm_toolbar_selected_item_get(obj), EINA_FALSE);
}

static int do_post(void *notUsed, int argc, char **argv, char **azColName) {
	struct sqlite3_stmt *post_stmt=NULL;
	char *msg = NULL, *query = NULL, *entry_str=NULL;
	const char *missed = NULL;
	int sqlite_res = 0, res=0;
	Eina_Strbuf *buf = NULL;

	char *screen_name=NULL, *password=NULL, *proto=NULL, *domain=NULL, *base_url=NULL;
	int port=0, id=0, type=0;
	/* In this query handler, these are the current fields:
		argv[0] == id INTEGER
		argv[1] == enabled INTEGER
		argv[2] == name TEXT
		argv[3] == password TEXT
		argv[4] == type INTEGER
		argv[5] == proto TEXT
		argv[6] == domain TEXT
		argv[7] == port INTEGER
		argv[8] == base_url TEXT
	*/

	id = atoi(argv[0]);
	screen_name = argv[2];
	password = argv[3];
	type = atoi(argv[4]);
	proto = argv[5];
	domain = argv[6];
	port = atoi(argv[7]);
	base_url = argv[8];

	buf = eina_strbuf_new();
	entry_str = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));
	eina_strbuf_append(buf, entry_str);
	eina_strbuf_replace_all(buf, "<br>", " ");
	msg=ed_curl_escape(eina_strbuf_string_steal(buf));
	eina_strbuf_free(buf);

	switch(type) {
		case ACCOUNT_TYPE_STATUSNET:
		default: { res = ed_statusnet_post(id, screen_name, password, proto, domain, port, base_url, msg); break; }
	}

	if(res != 0) {
		if( msg && strlen(msg) > 0 )  {
			if(dm_to) {
				sqlite_res = asprintf(&query, "INSERT INTO posts (account_id, dm_to, message) VALUES (%d, '%s', ?);", id, dm_to);
				dm_to=NULL;
			} else {
				sqlite_res = asprintf(&query, "INSERT INTO posts (account_id, message) VALUES (%d, ?);", id);
			}

			if(sqlite_res != -1) {
				sqlite_res = sqlite3_prepare(ed_DB, query, 4096, &post_stmt, &missed);
				if(sqlite_res == 0) {
					sqlite3_bind_text(post_stmt, 1, msg, -1, NULL);
					sqlite3_step(post_stmt);
					sqlite3_reset(post_stmt);
					sqlite3_finalize(post_stmt);
					} else
						fprintf(stderr, "Can't do %s: %d means '%s' was missed in the statement.\n", query, sqlite_res, missed);
				free(query);
			}
		}
	}

	if(msg) free(msg);
	if(entry_str) free(entry_str);

	return(0);
}

static void on_post_dm_set(void *data, Evas_Object *obj, void *event_info) {
	Evas *e = evas_object_evas_get(obj);
	Evas_Object *dm_entry = e?evas_object_name_find(e, "dm_entry"):NULL;
	char* dm_entry_str = dm_entry?elm_entry_markup_to_utf8(elm_entry_entry_get(dm_entry)):NULL;

	if(dm_entry_str && strlen(dm_entry_str) > 0) {
		dm_to = strdup(dm_entry_str);
		if(debug) printf("Sending a DM to %s\n", dm_to);
	}
	evas_object_del((Evas_Object*)data);
	if(dm_entry_str) free(dm_entry_str);
}

static void on_post_dm_cancel(void *data, Evas_Object *obj, void *event_info) {
	evas_object_del((Evas_Object*)data);
}

static void on_post_dm(void *data, Evas_Object *obj, void *event_info) {
	//Evas_Object *entry = data;
	Evas_Object *inwin=NULL, *frame=NULL, *entry=NULL, *box=NULL, *buttons=NULL, *button=NULL;

	inwin = elm_win_inwin_add(gui.win);
		elm_object_style_set(inwin, "minimal_vertical");

		frame = elm_frame_add(gui.win);
			elm_frame_label_set(frame, _("Send a DM to..."));
			box = elm_box_add(gui.win);
				entry = elm_entry_add(gui.win);
					evas_object_name_set(entry, "dm_entry");
					elm_box_pack_end(box, entry);
				evas_object_show(entry);

				buttons = elm_box_add(gui.win);
					elm_box_horizontal_set(buttons, EINA_TRUE);
					elm_box_homogenous_set(buttons, EINA_TRUE);

					button = elm_button_add(gui.win);
						elm_button_label_set(button, _("OK"));
						evas_object_smart_callback_add(button, "clicked", on_post_dm_set, inwin);
						elm_box_pack_end(buttons, button);
					evas_object_show(button);

					button = elm_button_add(gui.win);
						elm_button_label_set(button, _("Cancel"));
						evas_object_smart_callback_add(button, "clicked", on_post_dm_cancel, inwin);
						elm_box_pack_end(buttons, button);
					evas_object_show(button);

					elm_box_pack_end(box, buttons);
				evas_object_show(buttons);
			elm_frame_content_set(frame, box);
		elm_win_inwin_content_set(inwin, frame);
	evas_object_show(inwin);
}

static void on_post_clear(void *data, Evas_Object *obj, void *event_info) {
	Evas_Object *entry = data;
	elm_entry_entry_set(entry, "");
}

static void on_post(void *data, Evas_Object *obj, void *event_info) {
	char *query = NULL, *db_err=NULL;
	int sqlite_res = 0;

	query = "SELECT * FROM accounts WHERE enabled = 1 and send = 1;";
	sqlite_res = sqlite3_exec(ed_DB, query, do_post, NULL, &db_err);
	if(sqlite_res == 0) {
		if(dm_to) dm_to = NULL;
	} else
		fprintf(stderr, "Can't run %s: %d = %s\n", query, sqlite_res, db_err);

	elm_entry_entry_set(entry, "");
	evas_object_focus_set(entry, 0);

	evas_object_hide(hv);
}

static void on_timelines_hv(void *data, Evas_Object *obj, void *event_info) {
	evas_object_show((Evas_Object*)data);
}

static void on_post_hv(void *data, Evas_Object *obj, void *event_info) {
	elm_toolbar_item_selected_set(elm_toolbar_selected_item_get(obj), EINA_FALSE);
	evas_object_show(hv);
}

static void win_del(void *data, Evas_Object *obj, void *event_info) {
	elm_exit();
}

static void on_entry_changed(void *data, Evas_Object *entry, void *event_info) {
	int i = 140, len=0, res=0;
	char * count_str = NULL;
	char * entry_text = NULL;

	entry_text = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));

	if(!entry_text) {
		len=0;
		if(debug) printf("Entry content was NULL?!\n");
	} else {
 		if(g_utf8_validate(entry_text, -1, NULL))
			len = g_utf8_strlen(entry_text, -1);
		else
			len = strlen(entry_text);
	}

	i-=len;

	if(i<0) {
		res = asprintf(&count_str, "-% dc | ", -1*i);
	} else
		res = asprintf(&count_str, " % dc | ", i);
	if(res != -1) {
		elm_label_label_set(count, count_str);
		free(count_str);
	}
	free(entry_text);
}

static void on_entry_clicked(void *data, Evas_Object *entry, void *event_info) {
	int len;
	const char *first = _("Type your status here...");
	char *msg=elm_entry_markup_to_utf8(elm_entry_entry_get(entry));

	if(g_utf8_validate(first, -1, NULL))
		len = g_utf8_strlen(first, -1);
	else
		len = strlen(first);

	if(first_message && strncmp(msg,first,len) == 0) {
		elm_entry_entry_set(entry, "");
		first_message=0;
	}

	if(msg) free(msg);
}

Eina_Bool ed_statuses_update_time(void *data) {
	Elm_Genlist_Item *gli=NULL;

	Eina_List *l=NULL, *statuses_list = elm_genlist_realized_items_get(gui.timeline);

	EINA_LIST_FOREACH(statuses_list, l, gli)
		elm_genlist_item_update(gli);

	return(EINA_TRUE);
}

EAPI int elm_main(int argc, char **argv)
{
	Evas_Object *bg=NULL, *toolbar=NULL, *bt=NULL, *icon=NULL, *box2=NULL, *edit_panel=NULL, *menu=NULL;
	Elm_Toolbar_Item *it=NULL;
	char *tmp=NULL;
	int res = 0;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	if((tmp = getenv("HOME")))
		res = asprintf(&home, "%s/.elmdentica", tmp);
	else
		home = ".elmdentica";
	if(res == -1)
		home = ".elmdentica";

	ed_settings_init(argc, argv);

	azy_init();

	elmdentica_init();

	gui.win = elm_win_add(NULL, "elmdentica", ELM_WIN_BASIC);
	evas_object_smart_callback_add(gui.win, "delete-request", win_del, NULL);
	evas_object_size_hint_min_set(gui.win, 480, 480);
	evas_object_size_hint_max_set(gui.win, 640, 640);

	bg = elm_bg_add(gui.win);
	evas_object_size_hint_weight_set(bg, 1.0, 1.0);
	elm_win_resize_object_add(gui.win, bg);
	evas_object_show(bg);

	snprintf(theme, sizeof(theme), "%s/themes/default.edj", PKGDATADIR);
	elm_theme_extension_add(NULL, theme);

	gui.main = elm_layout_add(gui.win);
		elm_layout_file_set(gui.main, theme, "elmdentica/vertical_layout");
		evas_object_size_hint_weight_set(gui.main, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(gui.win, gui.main);
	evas_object_show(gui.main);

	//gui.pager = elm_pager_add(gui.win);
		//elm_object_style_set(gui.pager, "fade");
		//elm_win_resize_object_add(gui.win, gui.pager);
	//evas_object_show(gui.pager);

	//elm_pager_content_promote(gui.pager, gui.main);

	gui.timeline = elm_genlist_add(gui.win);
		evas_object_size_hint_weight_set(gui.timeline, 1, 1);
		evas_object_size_hint_align_set(gui.timeline, -1, -1);
		elm_genlist_bounce_set(gui.timeline, EINA_FALSE, EINA_TRUE);
		elm_genlist_height_for_width_mode_set(gui.timeline, EINA_TRUE);
		//elm_genlist_no_select_mode_set(gui.timeline, EINA_TRUE);
		elm_genlist_compress_mode_set(gui.timeline, EINA_TRUE);
		elm_genlist_longpress_timeout_set(gui.timeline, 0.5);

		evas_object_smart_callback_add(gui.timeline, "longpressed", on_status_action, NULL);
		// Statuses list
		make_status_list(TIMELINE_FRIENDS);
		fill_message_list(TIMELINE_FRIENDS);

		elm_layout_content_set(gui.main, "timeline", gui.timeline);
	evas_object_show(gui.timeline);

	if(settings->rel_timestamps)
		settings->rel_ts_timer = ecore_timer_add(60, ed_statuses_update_time, NULL);

	edit_panel = elm_box_add(gui.win);
		elm_box_homogenous_set(edit_panel, 0);
		elm_box_horizontal_set(edit_panel, 1);
		evas_object_size_hint_align_set(edit_panel, -1, 1);

		count = elm_label_add(gui.win);
			elm_label_label_set(count, " 140c | ");
			evas_object_size_hint_weight_set(count, 0, 1);
			evas_object_size_hint_align_set(count, 0, 0);
		evas_object_show(count);
		elm_box_pack_end(edit_panel, count);

		entry = elm_entry_add(gui.win);
			elm_entry_entry_set(entry, _("Type your status here..."));
			elm_entry_single_line_set(entry, 0);
			elm_entry_line_char_wrap_set(entry, 1);
			evas_object_size_hint_weight_set(entry, 1, 0);
			evas_object_size_hint_align_set(entry, -1, 1);
			evas_object_smart_callback_add(entry, "cursor,changed", on_entry_clicked, NULL);
			evas_object_smart_callback_add(entry, "changed", on_entry_changed, NULL);
		elm_box_pack_end(edit_panel, entry);
		evas_object_show(entry);
	evas_object_show(edit_panel);

	/* toolbar (horizontal box object) */
	toolbar = elm_toolbar_add(gui.win);
		evas_object_size_hint_weight_set(toolbar, 1.0, 0.0);
		evas_object_size_hint_align_set(toolbar, -1, 0);
		elm_toolbar_homogenous_set(toolbar, EINA_TRUE);

		gui.timelines = elm_toolbar_item_append(toolbar, "chat", _("Timelines"), on_timelines_hv, NULL);
		elm_toolbar_item_menu_set(gui.timelines, EINA_TRUE);
		elm_toolbar_item_priority_set(gui.timelines, 0);
		elm_toolbar_menu_parent_set(toolbar, gui.timeline);
		menu = elm_toolbar_item_menu_get(gui.timelines);

		elm_menu_item_add(menu, NULL, NULL, _("Direct Messages"), on_timeline_dmsgs_reload, NULL);
		elm_menu_item_add(menu, NULL, NULL, _("Everyone"), on_timeline_public_reload, NULL);
		elm_menu_item_add(menu, NULL, NULL, _("Just me"), on_timeline_user_reload, NULL);
		elm_menu_item_add(menu, NULL, NULL, _("Favorites"), on_timeline_favorites_reload, NULL);
		elm_menu_item_add(menu, NULL, NULL, _("Replies/Mentions"), on_timeline_mentions_reload, NULL);
		elm_menu_item_add(menu, NULL, NULL, _("Friends & I"), on_timeline_friends_reload, NULL);

		hv = elm_hover_add(gui.win);
			elm_object_style_set(hv, "popout");
			elm_hover_parent_set(hv, gui.win);
			elm_hover_content_set(hv, "top", edit_panel);

			gui.post = elm_toolbar_item_append(toolbar, "edit", _("Post"), on_post_hv, NULL);

		elm_hover_target_set(hv, toolbar);

		box2 = elm_box_add(gui.win);
		elm_box_horizontal_set(box2, EINA_TRUE);

			icon = elm_icon_add(gui.win);
			elm_icon_standard_set(icon, "edit");
			evas_object_show(icon);

			bt = elm_button_add(gui.win);
				evas_object_size_hint_weight_set(bt, 1, 1);
				evas_object_size_hint_align_set(bt, -1, 0);
				elm_button_label_set(bt, _("Send"));
				elm_button_icon_set(bt, icon);
				evas_object_smart_callback_add(bt, "clicked", on_post, hv);
				elm_box_pack_end(box2, bt);
			evas_object_show(bt);

			icon = elm_photo_add(gui.win);
			elm_photo_file_set(icon, "head.png");
			evas_object_show(icon);

			bt = elm_button_add(gui.win);
				evas_object_size_hint_weight_set(bt, 1, 1);
				evas_object_size_hint_align_set(bt, -1, 0);
				elm_button_label_set(bt, _("DM"));
				elm_button_icon_set(bt, icon);
				evas_object_smart_callback_add(bt, "clicked", on_post_dm, hv);
				elm_box_pack_end(box2, bt);
			evas_object_show(bt);

			icon = elm_icon_add(gui.win);
			elm_icon_standard_set(icon, "delete");
			evas_object_show(icon);

			bt = elm_button_add(gui.win);
				evas_object_size_hint_weight_set(bt, 1, 1);
				evas_object_size_hint_align_set(bt, -1, 0);
				elm_button_label_set(bt, _("Clear"));
				elm_button_icon_set(bt, icon);
				evas_object_smart_callback_add(bt, "clicked", on_post_clear, entry);
				elm_box_pack_end(box2, bt);
			evas_object_show(bt);

			elm_hover_content_set(hv, "middle", box2);
		evas_object_show(box2);

		it = elm_toolbar_item_append(toolbar, NULL, _("Fullscreen"), on_fs, NULL);
		it = elm_toolbar_item_append(toolbar, NULL, _("Settings"), on_settings, NULL);

	elm_layout_content_set(gui.main, "toolbar", toolbar);
	evas_object_show(toolbar);

	evas_object_resize(gui.win, 480, 640);
	evas_object_show(gui.win);


	if(settings->fullscreen) toggle_fullscreen(settings->fullscreen);

	curl_global_init(CURL_GLOBAL_ALL);

	finger_size =  elm_finger_size_get();

	elm_run();

	azy_shutdown();

	elm_shutdown();

	ed_settings_shutdown();

	if(re_percent) g_regex_unref(re_percent);
	if(re_link) g_regex_unref(re_link);
	if(re_user) g_regex_unref(re_user);
	if(re_tags) g_regex_unref(re_tags);
	if(re_group) g_regex_unref(re_group);
	if(re_link_content) g_regex_unref(re_link_content);
	if(re_amp) g_regex_unref(re_amp);

	return 0;
}

ELM_MAIN()
