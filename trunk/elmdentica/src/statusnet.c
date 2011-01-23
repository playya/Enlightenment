/* vim:tabstop=4
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cJSON.h"

#include <glib.h>
#include <glib/gprintf.h>

#include <Elementary.h>
#include <Ecore_X.h>

#include <Azy.h>
#include "statusnet_Common.h"

#include <sqlite3.h>

#include <curl/curl.h>

#include "statusnet.h"

#include "gettext.h"
#define _(string) gettext (string)

#include "curl.h"
#include "elmdentica.h"

typedef struct _account_data {
	long long int account_id;
	aStatus *as;
	int	timeline;
	Status_List_Cb update_status_list;
	Repeat_Cb add_repeat;
	void *data;
} accountData;

extern struct sqlite3 *ed_DB;
extern int debug;
extern int current_timeline;
extern char *home;
extern char *dm_to;
extern aStatus *reply_as;
extern long long int user_id;

long long max_status_id=0;

Eina_Hash *userHash=NULL;
Eina_Hash *statusHash=NULL;
Eina_List* newStatuses=NULL;
Eina_Hash *azy_agents=NULL;

extern struct sqlite3 *ed_DB;
extern int debug;
extern char *home, *dm_to;
extern Gui gui;

Eina_Bool user_insert(anUser *au, int account_id) {
	int sqlite_res=0;
	struct sqlite3_stmt *insert_stmt=NULL;
	const char *missed=NULL;
	char *query=NULL;

	if(!au) return(EINA_FALSE);

	if(au->in_db == EINA_TRUE) return(EINA_TRUE);

	sqlite_res = asprintf(&query, "insert into users (uid, account_id, name, screen_name, location, description, profile_image_url, url, protected, followers_count, friends_count, created_at, favorites_count, statuses_count, following, statusnet_blocking) values (%lld, %d, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);", (long long int)au->user->id, account_id);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_prepare_v2(ed_DB, query, 4096, &insert_stmt, &missed);
		if(sqlite_res == 0) {
			sqlite3_bind_text(insert_stmt,  1,  au->user->name, -1, NULL);
			sqlite3_bind_text(insert_stmt,  2,  au->user->screen_name, -1, NULL);
			sqlite3_bind_text(insert_stmt,  3,  au->user->location, -1, NULL);
			sqlite3_bind_text(insert_stmt,  4,  au->user->description, -1, NULL);
			sqlite3_bind_text(insert_stmt,  5,  au->user->profile_image_url, -1, NULL);
			sqlite3_bind_text(insert_stmt,  6,  au->user->url, -1, NULL);
			sqlite3_bind_int64(insert_stmt, 7,  au->user->protected);
			sqlite3_bind_int64(insert_stmt, 8,  au->user->followers_count);
			sqlite3_bind_int64(insert_stmt, 9,  au->user->friends_count);
			sqlite3_bind_int64(insert_stmt, 10, au->created_at);
			sqlite3_bind_int64(insert_stmt, 11, au->user->favourites_count);
			sqlite3_bind_int64(insert_stmt, 12, au->user->statuses_count);
			sqlite3_bind_int64(insert_stmt, 13, au->user->following);
			sqlite3_bind_int64(insert_stmt, 14, au->user->statusnet_blocking);
			sqlite_res = sqlite3_step(insert_stmt);
			if(sqlite_res != 0 && sqlite_res != 101 ) printf("ERROR: %d while inserting user:\n(%s)\n", sqlite_res, au->user->screen_name);
			else au->in_db = EINA_TRUE;

			sqlite3_reset(insert_stmt);
			sqlite3_finalize(insert_stmt);
			return(EINA_TRUE);
		} else {
			fprintf(stderr, "Can't do %s: %d means '%s' was missed in the statement.\n", query, sqlite_res, missed);
		}
		free(query);
	}
	return(EINA_FALSE);

}

aStatus *statusnet_new_status(statusnet_Status *snS, int account_id) {
	aStatus *as=calloc(1, sizeof(aStatus));
	anUser *au=NULL;

	char *uid=NULL;
	int res=0;

	if(!as) {
		fprintf(stderr, _("Not enough memory to create a status\n"));
		return(NULL);
	}

	as->status = snS;
	as->created_at = curl_getdate(snS->created_at, NULL);
	as->account_id = account_id;
	as->account_type = ACCOUNT_TYPE_STATUSNET;
	as->in_db = EINA_FALSE;

	res = asprintf(&uid, "%d/%lld", account_id, (long long int)snS->user->id);
	if(res != -1) {
		au = eina_hash_find(userHash, uid);
		if(au) {
			statusnet_User_free(snS->user);
			snS->user = NULL;
		} else {
			au = calloc(1, sizeof(anUser));
			au->user = snS->user;
			au->account_id = account_id;
			au->account_type = ACCOUNT_TYPE_STATUSNET;
			au->created_at = curl_getdate(snS->user->created_at, NULL);
			eina_hash_add(userHash, uid, au);

			user_insert(au, as->account_id);
		}
		as->au = au;
		as->status->user = NULL;
	}
	return(as);
}

void ed_statusnet_azy_agent_free(void *data) {
	azy_client_free((Azy_Client*)data);
}

void ed_statusnet_account_free(StatusNetBaAccount *account) {
	if(!account) return;

	if(account->screen_name) free(account->screen_name);
	if(account->password) free(account->password);
	if(account->proto) free(account->proto);
	if(account->domain) free(account->domain);
	if(account->base_url) free(account->base_url);
	free(account);
}

Eina_Bool azy_value_to_Array_statusnet_Group(Azy_Value *array, Eina_List **narray) {
    Eina_List *tmp_narray=NULL;
    statusnet_Group *group;

    if (!array)
        return(EINA_FALSE);

    switch(azy_value_type_get(array)) {
        case AZY_VALUE_STRUCT: {
            group=NULL;
            if (azy_value_to_statusnet_Group(array, &group)) {
                tmp_narray = eina_list_append(tmp_narray, group);
            }
            break;
        }
        default: { return(EINA_FALSE); }
    }

    *narray = tmp_narray;
    return(EINA_TRUE);
}

static void Array_statusnet_Group_free(Eina_List *array) {
    statusnet_Group *group;

    EINA_SAFETY_ON_NULL_RETURN(array);
    EINA_LIST_FREE(array, group)
        statusnet_Group_free(group);
}

Eina_Bool groupget_connected(void *data, int type, Azy_Client *cli) {
    Azy_Client_Call_Id id;

    id = azy_client_blank(cli, AZY_NET_TYPE_GET, NULL, (Azy_Content_Cb)azy_value_to_Array_statusnet_Group, NULL);
    azy_client_callback_free_set(cli, id, (Ecore_Cb)Array_statusnet_Group_free);

    return(EINA_TRUE);
}

Eina_Bool groupget_returned(void *data, int type, Azy_Content *content) {
	groupData *gd = (groupData*)data;
	Eina_List *list=NULL;
	statusnet_Group *group=NULL, *groupCopy=NULL;

    if (azy_content_error_is_set(content)) {
        fprintf(stderr, _("Error encountered: %s\n"), azy_content_error_message_get(content));
        return(azy_content_error_code_get(content));
    }

    list = azy_content_return_get(content);

	group = eina_list_data_get(list);
	groupCopy = statusnet_Group_copy(group);
	gd->group_show(gd->as, groupCopy, gd->data);

	return(EINA_TRUE);
}

Eina_Bool groupget_disconnected(void *data, int type, Azy_Client *cli) {
	return(EINA_TRUE);
}

static int ed_statusnet_group_get_handler(void *data, int argc, char **argv, char **azColName) {
	groupData *gd = (groupData*)data;
    char *screen_name=NULL, *password=NULL, *proto=NULL, *domain=NULL, *base_url=NULL, *url_end=NULL, *url_start=NULL;
    int port=0, id=0, res;
	Azy_Client *cli;

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

	cli = azy_client_new();
	if(!cli) return(-1);

	res = asprintf(&url_start, "%s://%s", proto, domain);
	res = asprintf(&url_end, "%s/statusnet/groups/show.json?id=%s&source=elmdentica", base_url, gd->group_name);

	azy_client_host_set(cli, url_start, port);
	azy_client_connect(cli, EINA_TRUE);
	azy_net_auth_set(azy_client_net_get(cli), screen_name, password);
	azy_net_uri_set(azy_client_net_get(cli), url_end);
	azy_net_version_set(azy_client_net_get(cli), 0);


	ecore_event_handler_add(AZY_CLIENT_CONNECTED, (Ecore_Event_Handler_Cb)groupget_connected, gd);
	ecore_event_handler_add(AZY_CLIENT_RETURN, (Ecore_Event_Handler_Cb)groupget_returned, gd);
	ecore_event_handler_add(AZY_CLIENT_DISCONNECTED, (Ecore_Event_Handler_Cb)groupget_disconnected, gd);

	return(0);
}

void ed_statusnet_group_get(aStatus *as, const char *group_name, Group_Show_Cb callback, void *data) {
	char *query=NULL, *db_err=NULL;
	int sqlite_res;
	groupData *gd=calloc(1, sizeof(groupData));

	gd->as = as;
	gd->group_name = group_name;
	gd->group_show = callback;
	gd->data = data;

	sqlite_res = asprintf(&query, "SELECT name,password,type,proto,domain,port,base_url,id FROM accounts WHERE id = %d and type = %d and enabled = 1;", as->account_id, ACCOUNT_TYPE_STATUSNET);

	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, ed_statusnet_group_get_handler, (void*)gd, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't run %s: %d = %s\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);
	}
}

void ed_statusnet_group_free(groupData *gd) {
    if(gd) {
        if(gd->data) free(gd->data);
		if(gd->group) statusnet_Group_free(gd->group);

        free(gd);
    }
}

static int ed_statusnet_group_join_handler(void *data, int argc, char **argv, char **azColName) {
	GroupProfile *group = (GroupProfile*)data;
    char *screen_name=NULL, *password=NULL, *proto=NULL, *domain=NULL, *base_url=NULL;
    int port=0, id=0, res;
	http_request * request=calloc(1, sizeof(http_request));

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

	if(!request) return(-1);

	res = asprintf(&request->url, "%s://%s:%d%s/statusnet/groups/join.json?id=%s", proto, domain, port, base_url, group->name);

	if(request->url) free(request->url);
	if(request->content.memory) free(request->content.memory);
	free(request);

	return(0);
}

void ed_statusnet_group_join(GroupProfile *group) {
	char *query=NULL, *db_err=NULL;
	int sqlite_res;
	
	sqlite_res = asprintf(&query, "SELECT name,password,type,proto,domain,port,base_url,id FROM accounts WHERE id = %d and type = %d and enabled = 1;", group->account_id, ACCOUNT_TYPE_STATUSNET);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, ed_statusnet_group_join_handler, (void*)group, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't run %s: %d = %s\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);
	}
}

static int ed_statusnet_group_leave_handler(void *data, int argc, char **argv, char **azColName) {
	GroupProfile *group =(GroupProfile*)data;
    char *screen_name=NULL, *password=NULL, *proto=NULL, *domain=NULL, *base_url=NULL;
    int port=0, id=0, res;
	http_request * request=calloc(1, sizeof(http_request));

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

	if(!request) return(-1);

	res = asprintf(&request->url, "%s://%s:%d%s/statusnet/groups/leave.json?id=%s", proto, domain, port, base_url, group->name);


	if(request->url) free(request->url);
	if(request->content.memory) free(request->content.memory);
	free(request);

	return(0);
}

void ed_statusnet_group_leave(GroupProfile *group) {
	char *query=NULL, *db_err=NULL;
	int sqlite_res;
	
	sqlite_res = asprintf(&query, "SELECT name,password,type,proto,domain,port,base_url,id FROM accounts WHERE id = %d and type = %d and enabled = 1;", group->account_id, ACCOUNT_TYPE_STATUSNET);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, ed_statusnet_group_leave_handler, (void*)group, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't run %s: %d = %s\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);
	}
}

void status_hash_data_free(void *data) {
	aStatus *s = (aStatus*)data;

    if(s) {
		if(s->status) statusnet_Status_free(s->status);
		free(s);
	}
}

void user_hash_data_free(void *data) {
	anUser *u = (anUser*)data;

	if(u) {
		if(u->user) statusnet_User_free(u->user);
		free(u);
	}
}

static int set_max_status_id(void *notUsed, int argc, char **argv, char **azColName) {
	if(!argv[0])
		max_status_id = 0;
	else
		max_status_id = atoi(argv[0]);
	return(0);
}

void message_insert(statusnet_Status *s, void *user_data) {
	struct sqlite3_stmt **insert_stmt = (struct sqlite3_stmt**)user_data;
	long long int sid=0;
	int sqlite_res=0;

	if(s->id > max_status_id) {
		max_status_id = sid;
		sqlite3_bind_int64(*insert_stmt, 1, s->id);
		sqlite3_bind_text(*insert_stmt,  2, s->text, -1, NULL);
		sqlite3_bind_int64(*insert_stmt, 3, s->truncated);
		sqlite3_bind_int64(*insert_stmt, 4, curl_getdate(s->created_at, NULL));
		sqlite3_bind_int64(*insert_stmt, 5, s->in_reply_to_status_id);
		sqlite3_bind_text(*insert_stmt,  6, s->source, -1, NULL);
		sqlite3_bind_int64(*insert_stmt, 7, s->in_reply_to_user_id);
		sqlite3_bind_int64(*insert_stmt, 8, s->favorited);
		sqlite3_bind_int64(*insert_stmt, 9, s->user->id);

		sqlite_res = sqlite3_step(*insert_stmt);
		if(sqlite_res != 0 && sqlite_res != 101 ) printf("ERROR: %d while inserting message:\n(%s) %s\n", sqlite_res, s->user->screen_name, s->text);

		sqlite3_reset(*insert_stmt);
	}
}



int ed_statusnet_post(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, char *msg) {
	char *ub_status=NULL;
	int res=0;
	http_request * request=NULL;

	request = calloc(1, sizeof(http_request));

	if(request && strlen(msg) > 0) {
		if(reply_as) {
			res = asprintf(&ub_status, "source=%s&status=%s&in_reply_to_status_id=%lld", PACKAGE, msg, (long long int)reply_as->status->id);
			reply_as=NULL;
		} else if(user_id>0 || dm_to)
			res = asprintf(&ub_status, "source=%s&text=%s", PACKAGE, msg);
		else
			res = asprintf(&ub_status, "source=%s&status=%s", PACKAGE, msg);

		if(res != -1) {
			if(user_id) {
				res  = asprintf(&request->url,"%s://%s:%d%s/direct_messages/new.json?user_id=%lld", proto, domain, port, base_url, user_id);
				user_id = 0;
			} else if(dm_to) {
				res  = asprintf(&request->url,"%s://%s:%d%s/direct_messages/new.json?screen_name=%s", proto, domain, port, base_url, dm_to);
				dm_to = NULL;
			} else
				res  = asprintf(&request->url,"%s://%s:%d%s/statuses/update.json", proto, domain, port, base_url);
			if(res != -1) {
				res = ed_curl_post(screen_name, password, request, ub_status, account_id);

				free(ub_status);
				free(request->url);
				free(request);
			}
		}
	}

	return(0);
}

static int ed_statusnet_max_status_id_handler(void *data, int argc, char **argv, char **azColName) {
	long long int *since_id = (long long int*)data;

	if(!argv[0])
		*since_id = 0;
	else
		*since_id = strtoll(argv[0], NULL, 10);

	return(0);
}
void ed_statusnet_max_status_id(int account_id, long long int*since_id, int timeline) {
	char *query=NULL, *db_err=NULL;
	int sqlite_res=0;
	
	sqlite_res = asprintf(&query, "SELECT MAX(status_id) FROM messages WHERE account_id = %d and timeline = %d;", account_id, timeline);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, ed_statusnet_max_status_id_handler, (void*)since_id, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't run %s: %d = %s\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);
	}
}

void ed_statusnet_statuses_get_avatar(char *id, char *url) {
	int res=0;
	char * file_path=NULL;

	if(!url || !id) return;

	res = asprintf(&file_path, "%s/cache/icons/%s", home, id);

	if(res != -1) {
		ed_curl_dump_url_to_file(url, file_path);
		free(file_path);
	}
}


Eina_Bool azy_value_to_Array_statusnet_Status(Azy_Value *array, Eina_List **narray) {
	Eina_List *tmp_narray=NULL, *item;
	Azy_Value *v;
	statusnet_Status *snS;

	if (!array)
		return(EINA_FALSE);

	switch(azy_value_type_get(array)) {
		case AZY_VALUE_ARRAY: {

			EINA_LIST_FOREACH(azy_value_children_items_get(array), item, v) {
				snS=NULL;

				if (azy_value_to_statusnet_Status(v, &snS)) {
					tmp_narray = eina_list_append(tmp_narray, snS);
				}
			}
			break;
		}
		case AZY_VALUE_STRUCT: {
			snS=NULL;
			if (azy_value_to_statusnet_Status(array, &snS)) {
				tmp_narray = eina_list_append(tmp_narray, snS);
			}
			break;
		}
		default: { return(EINA_FALSE); }
	}

	*narray = tmp_narray;
	return(EINA_TRUE);
}

static void Array_statusnet_Status_free(Eina_List *array) {
	statusnet_Status *snS;

	EINA_SAFETY_ON_NULL_RETURN(array);
	EINA_LIST_FREE(array, snS)
		statusnet_Status_free(snS);
}

Eina_Bool timeline_connected(void *data, int type, Azy_Client *cli) {
	Azy_Client_Call_Id id;

	id = azy_client_blank(cli, AZY_NET_TYPE_GET, NULL, (Azy_Content_Cb)azy_value_to_Array_statusnet_Status, NULL);
	azy_client_callback_free_set(cli, id, (Ecore_Cb)Array_statusnet_Status_free);

	return(EINA_TRUE);
}

Eina_Bool timeline_returned(void *data, int type, Azy_Content *content) {
	int sqlite_res=0;
	struct sqlite3_stmt *insert_stmt=NULL;
	const char *missed=NULL;
	char *db_err=NULL, *query=NULL;
	accountData *ad = (accountData*)data;
	statusnet_Status *snS=NULL, *snS2=NULL;
	aStatus *as=NULL;
	Eina_List *l, *list=NULL;

	if (azy_content_error_is_set(content)) {
		fprintf(stderr, _("Error encountered: %s\n"), azy_content_error_message_get(content));
		return(azy_content_error_code_get(content));
	}

	list = azy_content_return_get(content);

	sqlite_res = asprintf(&query, "SELECT max(status_id) FROM messages where account_id = %lld and timeline = %d;", ad->account_id, ad->timeline);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, set_max_status_id, NULL, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't do %s: %d means '%s' was missed in the statement.\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);
	}

	sqlite_res = asprintf(&query, "insert into messages (status_id, account_id, timeline, s_text, s_truncated, s_created_at, s_in_reply_to_status_id, s_source, s_in_reply_to_user_id, s_favorited, s_user) values (?, %lld, %d, ?, ?, ?, ?, ?, ?, ?, ?);", ad->account_id, ad->timeline);;
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_prepare_v2(ed_DB, query, 4096, &insert_stmt, &missed);
		if(sqlite_res == 0) {
			EINA_LIST_REVERSE_FOREACH(list, l, snS) {
				message_insert(snS, &insert_stmt);
				snS2 = statusnet_Status_copy(snS);
				as = statusnet_new_status(snS2, ad->account_id);
				newStatuses = eina_list_append(newStatuses, as);
			}
		} else {
			fprintf(stderr, "Can't do %s: %d means '%s' was missed in the statement.\n", query, sqlite_res, missed);
		}
		free(query);
		sqlite3_finalize(insert_stmt);
	}

	if(ad->as)
		ad->add_repeat(ad->as, ad->data);
	else
		ad->update_status_list(ad->timeline, EINA_FALSE);

	return(EINA_TRUE);
}

Eina_Bool timeline_disconnected(void *data, int type, Azy_Client *cli) {
	return(EINA_TRUE);
}

void ed_statusnet_timeline_get(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, int timeline, Status_List_Cb update_status_list) {
	int res=0;
	long long int since_id=0;
	char *host=NULL, *timeline_str=NULL;
	Azy_Client *cli = azy_client_new();
	accountData *ad=NULL;
	
	if(!cli) return;

	switch(timeline) {
		case TIMELINE_USER:		{
			if(since_id)
				res = asprintf(&timeline_str, "%s/statuses/user_timeline.json?source=elmdentica&since_id=%lld", base_url, since_id);
			else
				res = asprintf(&timeline_str, "%s/statuses/user_timeline.json?source=elmdentica", base_url);
			break;
		}
		case TIMELINE_PUBLIC:	{
			if(since_id)
				res = asprintf(&timeline_str, "%s/statuses/public_timeline.json?source=elmdentica&since_id=%lld", base_url, since_id);
			else
				res = asprintf(&timeline_str, "%s/statuses/public_timeline.json?source=elmdentica", base_url);
			break;
		}
		case TIMELINE_MENTIONS:	{
			if(since_id)
				res = asprintf(&timeline_str, "%s/statuses/mentions.json?source=elmdentica&since_id=%lld", base_url, since_id);
			else
				res = asprintf(&timeline_str, "%s/statuses/mentions.json?source=elmdentica", base_url);
			break;
		}
		case TIMELINE_FAVORITES:	{
			if(since_id)
				res = asprintf(&timeline_str, "%s/favorites.json?source=elmdentica&since_id=%lld", base_url, since_id);
			else
				res = asprintf(&timeline_str, "%s/favorites.json?source=elmdentica", base_url);
			break;
		}
		case TIMELINE_FRIENDS:	{
			if(since_id)
				res = asprintf(&timeline_str, "%s/statuses/friends_timeline.json?source=elmdentica&since_id=%lld", base_url, since_id);
			else
				res = asprintf(&timeline_str, "%s/statuses/friends_timeline.json?source=elmdentica", base_url);
			break;
		}
		case TIMELINE_DMSGS:	{
			if(since_id)
				res = asprintf(&timeline_str, "%s/direct_messages.json?source=elmdentica&since_id=%lld", base_url, since_id);
			else
				res = asprintf(&timeline_str, "%s/direct_messages.json?source=elmdentica", base_url);
			break;
		}
		default:				{
			azy_client_free(cli);
			fprintf(stderr, _("Unknown timeline %d\n"), timeline);
			return;
		}
	}

	if(res == -1) {
			azy_client_free(cli);
			fprintf(stderr, _("Not enough memory to construct timeline URI\n"));
			return;
	}

	ed_statusnet_max_status_id(account_id, &since_id, timeline);

	res = asprintf(&host, "%s://%s", proto, domain);
	if(res == -1) {
			azy_client_free(cli);
			fprintf(stderr, _("Not enough memory to prepare host url\n"));
			return;
	}

	azy_client_host_set(cli, host, port);
	azy_client_connect(cli, EINA_TRUE);
	azy_net_auth_set(azy_client_net_get(cli), screen_name, password);
	azy_net_uri_set(azy_client_net_get(cli), timeline_str);
	azy_net_version_set(azy_client_net_get(cli), 0);

	ad=(accountData*)calloc(1, sizeof(accountData));

	ad->account_id = account_id;
	ad->timeline = timeline;
	ad->update_status_list = update_status_list;

	ecore_event_handler_add(AZY_CLIENT_CONNECTED, (Ecore_Event_Handler_Cb)timeline_connected, ad);
	ecore_event_handler_add(AZY_CLIENT_RETURN, (Ecore_Event_Handler_Cb)timeline_returned, ad);
	ecore_event_handler_add(AZY_CLIENT_DISCONNECTED, (Ecore_Event_Handler_Cb)timeline_disconnected, ad);

	free(timeline_str);
	free(host);
}

void ed_statusnet_toggle_favorite(int account_id, long long int status_id, Eina_Bool favorite) {
	char *query=NULL, *db_err=NULL, *sid_str=NULL;
	int sqlite_res;
	aStatus *as=NULL;

	sqlite_res = asprintf(&query, "UPDATE messages SET s_favorited=%d WHERE account_id = %d and status_id = %lld;", favorite, account_id, status_id);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, NULL, NULL, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't do %s: %d means '%s' was missed in the statement.\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);
	}

	sqlite_res = asprintf(&sid_str, "%lld", status_id);
	if(sqlite_res != -1) {
		as = eina_hash_find(statusHash, sid_str);
		if(as) as->status->favorited=favorite;
		free(sid_str);
	}
}

void ed_statusnet_favorite_create(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, long long int status_id) {
	http_request * request=calloc(1, sizeof(http_request));
	int res;

	res = asprintf(&request->url, "%s://%s:%d%s/favorites/create/%lld.json", proto, domain, port, base_url, status_id);
	if(res != -1) {
		ed_curl_post(screen_name, password, request, "", account_id);
		free(request->url);
		if(request->response_code == 200) ed_statusnet_toggle_favorite(account_id, status_id, EINA_TRUE);
	}
	if(request) free(request);
}

void ed_statusnet_favorite_db_remove(int account_id, long long int status_id) {
	char *query=NULL, *db_err=NULL;;
	int sqlite_res;

	sqlite_res = asprintf(&query, "DELETE FROM messages WHERE account_id = %d and status_id = %lld and timeline = %d;", account_id, status_id, TIMELINE_FAVORITES);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, NULL, NULL, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't do %s: %d means '%s' was missed in the statement.\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);
	}
}

void ed_statusnet_favorite_destroy(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, long long int status_id) {
	http_request * request=calloc(1, sizeof(http_request));
	int res;

	ed_statusnet_favorite_db_remove(account_id, status_id);
	res = asprintf(&request->url, "%s://%s:%d%s/favorites/destroy/%lld.json", proto, domain, port, base_url, status_id);
	if(res != -1) {
		ed_curl_post(screen_name, password, request, "", account_id);
		free(request->url);
		if(request->response_code == 200) ed_statusnet_toggle_favorite(account_id, status_id, EINA_FALSE);
	}
	if(request) free(request);
}

static void ed_sn_single_user_free(statusnet_User *u) {
        statusnet_User_free(u);
}

Eina_Bool ed_sn_userget_parse(Azy_Value *value, Eina_List **_narray) {
        statusnet_User *snU=NULL;
        statusnet_Error *snE=NULL;
        Eina_Strbuf *str=NULL;

        if( (!value) || (azy_value_type_get(value) != AZY_VALUE_STRUCT) ) {
                printf("Didn't get a struct\n");
                return(EINA_FALSE);
        }
        
        if(azy_value_to_statusnet_Error(value, &snE)) {
                str=eina_strbuf_new();
                if(str) {
                        azy_value_dump(value, str, 1);
                        fprintf(stderr, _("Got an error: %s\n"), eina_strbuf_string_get(str));
                        eina_strbuf_free(str);
                } else fprintf(stderr, _("Got an error without content\n"));
                return(EINA_FALSE);
        }

        if(azy_value_to_statusnet_User(value, &snU)) {
                printf("Got an user: %s\n", snU->name);
        } else {
                statusnet_User_free(snU);
                printf("Didn't get an User!\n");
                azy_value_dump(value, str, 1);
                printf("Got: %s\n", eina_strbuf_string_get(str));
        }

        eina_strbuf_free(str);
        return(EINA_TRUE);
}

Eina_Bool ed_statusnet_userget_returned(Azy_Client *cli, int type, Azy_Client *ev) {
        return(EINA_TRUE);
}

Eina_Bool ed_statusnet_userget_connected(Azy_Client *cli, int type, Azy_Client *ev) {
        Azy_Client_Call_Id id;

        id = azy_client_blank(ev, AZY_NET_TYPE_GET, NULL, (Azy_Content_Cb)ed_sn_userget_parse, NULL);
        if(!id) return(EINA_FALSE);

        azy_client_callback_free_set(ev, id, (Ecore_Cb)ed_sn_single_user_free);

        return(EINA_TRUE);
}

static int ed_statusnet_user_get_handler(void *data, int argc, char **argv, char **azColName) {
	UserGet *ug=(UserGet*)data;
	UserProfile *user = ug->user;
    char *screen_name=NULL, *password=NULL, *proto=NULL, *domain=NULL, *base_url=NULL, *url_start=NULL, *url_end=NULL;
    int port=0, id=0, res;
	Azy_Client *cli=NULL;

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

	cli = azy_client_new();
	if(!cli) return(-1);

	res = asprintf(&url_start, "%s://%s", proto, domain);
	if(ug->id)
		res = asprintf(&url_end, "%s/users/show.json?user_id=%lld&source=elmdentica", base_url, ug->id);
	else
		res = asprintf(&url_end, "%s/users/show.json?screen_name=%s&source=elmdentica", base_url, user->screen_name);

	azy_client_host_set(cli, url_start, port);
	free(url_start);

	azy_client_connect(cli, EINA_TRUE);
	azy_net_uri_set(azy_client_net_get(cli), url_end);
	azy_net_auth_set(azy_client_net_get(cli), screen_name, password);

	azy_net_version_set(azy_client_net_get(cli), 0);

	ecore_event_handler_add(AZY_CLIENT_CONNECTED, (Ecore_Event_Handler_Cb)ed_statusnet_userget_connected, NULL);
	ecore_event_handler_add(AZY_CLIENT_RETURN, (Ecore_Event_Handler_Cb)ed_statusnet_userget_returned, NULL);


	return(0);
}

anUser *ed_statusnet_user_get(int account_id, UserProfile *user) {
	char *query=NULL, *db_err=NULL;
	int sqlite_res;
	UserGet *ug=calloc(1, sizeof(UserGet));
	anUser *au=NULL;
	
	ug->user=user;
	ug->account_id=account_id;
	sqlite_res = asprintf(&query, "SELECT name,password,type,proto,domain,port,base_url,id FROM accounts WHERE id = %d and type = %d and enabled = 1;", account_id, ACCOUNT_TYPE_STATUSNET);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, ed_statusnet_user_get_handler, (void*)ug, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't run %s: %d = %s\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);
	}

	au=ug->au;
	free(ug);
	return(ug->au);
}

static int ed_statusnet_user_get_by_id_handler(void *data, int argc, char **argv, char **azColName) {
    statusnet_User *snU = statusnet_User_new();
	anUser **p_au = (anUser**)data;

	*p_au = calloc(1, sizeof(anUser));

    /* In this query handler, these are the current fields:
        argv[0] == id INTEGER
        argv[1] == uid INTEGER
        argv[2] == account_id INTEGER
        argv[3] == name TEXT
        argv[4] == screen_name TEXT
        argv[5] == location TEXT
        argv[6] == description TEXT
        argv[7] == profile_image_url TEXT
        argv[8] == url TEXT
        argv[9] == protected INTEGER
        argv[10] == followers_count INTEGER
        argv[11] == friends_count INTEGER
        argv[12] == created_at INTEGER
        argv[13] == favorites_count INTEGER
        argv[14] == statuses_count INTEGER
        argv[15] == following INTEGER
        argv[16] == statusnet_blocking INTEGER
    */

	snU->id = argv[1]?strtoll(argv[1], NULL, 10):0;
	(*p_au)->account_id = argv[2]?atoi(argv[2]):0;
	snU->name = strdup(argv[3]?argv[3]:"noname");
	snU->screen_name = strdup(argv[4]?argv[4]:"noname");
	snU->location = strdup(argv[5]?argv[5]:"");
	snU->description = strdup(argv[6]?argv[6]:"");
	snU->profile_image_url = strdup(argv[7]?argv[7]:"");
	snU->url = strdup(argv[8]?argv[8]:"");
	snU->protected = argv[9]?atoi(argv[9]):0;
	snU->followers_count = argv[10]?atoi(argv[10]):0;
	snU->friends_count = argv[11]?atoi(argv[11]):0;
	(*p_au)->created_at = argv[12]?atoi(argv[12]):0;
	snU->favourites_count = argv[13]?atoi(argv[13]):0;
	snU->statuses_count = argv[14]?atoi(argv[14]):0;
	snU->following = argv[15]?atoi(argv[15]):0;
	snU->statusnet_blocking = argv[16]?atoi(argv[16]):0;
	(*p_au)->account_type = ACCOUNT_TYPE_STATUSNET;
	(*p_au)->user = snU;

	eina_hash_add(userHash, argv[1], *p_au);

	return(0);
}

anUser *ed_statusnet_user_get_by_id(int account_id, long long int user_id) {
	char *query=NULL, *db_err=NULL;
	int sqlite_res;
	anUser *au=NULL;

	sqlite_res = asprintf(&query, "SELECT * from users WHERE uid = %lld and account_id = %d LIMIT 1;", user_id, account_id);

	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, ed_statusnet_user_get_by_id_handler, (void*)&au, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't run %s: %d = %s\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);
	}

	return(au);
}

void ed_statusnet_user_follow(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, char *user_screen_name) {
	http_request * request=calloc(1, sizeof(http_request));
	int res;

	res = asprintf(&request->url, "%s://%s:%d%s/friendships/create.json?screen_name=%s", proto, domain, port, base_url, user_screen_name);
	if(res != -1) {
		ed_curl_post(screen_name, password, request, "", account_id);
		if(debug && request->response_code != 200)
			printf("User follow failed with response code %ld\n", request->response_code);
		free(request->url);
	}
	if(request) free(request);
}
void ed_statusnet_user_abandon(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, char *user_screen_name) {
	http_request * request=calloc(1, sizeof(http_request));
	int res;

	res = asprintf(&request->url, "%s://%s:%d%s/friendships/destroy.json?screen_name=%s", proto, domain, port, base_url, user_screen_name);
	if(res != -1) {
		ed_curl_post(screen_name, password, request, "", account_id);
		if(debug && request->response_code != 200)
			printf("User abandon failed with response code %ld\n", request->response_code);
		free(request->url);
	}
	if(request) free(request);
}

Eina_Bool ed_statusnet_repeat_disconnected(Azy_Client *cli, int type, Azy_Client *ev) {
	return(EINA_TRUE);
}

Eina_Bool ed_statusnet_repeat_connected(Azy_Client *cli, int type, Azy_Client *ev) {
	Azy_Client_Call_Id id;
	Azy_Net_Data buffer;

	buffer.data = (unsigned char*)"source=elmdentica";
	buffer.size = strlen((const char*)buffer.data);

	id = azy_client_blank(ev, AZY_NET_TYPE_POST, &buffer, (Azy_Content_Cb)azy_value_to_Array_statusnet_Status, NULL);
	if(!id) return(EINA_FALSE);

	azy_client_callback_free_set(ev, id, (Ecore_Cb)Array_statusnet_Status_free);

	return(EINA_TRUE);
}

static int ed_statusnet_repeat_handler(void *data, int argc, char **argv, char **azColName) {
    char *screen_name=NULL, *password=NULL, *proto=NULL, *domain=NULL, *base_url=NULL, *url_start=NULL, *url_end=NULL;
    int port=0, id=0, res;
	Azy_Client *repeat=NULL;
	accountData *ad = (accountData*)data;

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


	res = asprintf(&url_start, "%s://%s", proto, domain);
	res = asprintf(&url_end, "%s/statuses/retweet/%lld.json", base_url, (long long int)ad->as->status->id);

	repeat = azy_client_new();
	azy_client_host_set(repeat, url_start, port);
	free(url_start);

	azy_client_connect(repeat, EINA_TRUE);

	azy_net_uri_set(azy_client_net_get(repeat), url_end);
	azy_net_auth_set(azy_client_net_get(repeat), screen_name, password);

	azy_net_version_set(azy_client_net_get(repeat), 0);

	ecore_event_handler_add(AZY_CLIENT_CONNECTED, (Ecore_Event_Handler_Cb)ed_statusnet_repeat_connected, ad);
	ecore_event_handler_add(AZY_CLIENT_RETURN, (Ecore_Event_Handler_Cb)timeline_returned, ad);
	ecore_event_handler_add(AZY_CLIENT_DISCONNECTED, (Ecore_Event_Handler_Cb)timeline_disconnected, ad);

	return(0);
}

void ed_statusnet_repeat(int account_id, aStatus *as, Repeat_Cb callback, void *data) {
	char *query=NULL, *db_err=NULL;
	int sqlite_res;
	accountData *ad;
	
	sqlite_res = asprintf(&query, "SELECT name,password,type,proto,domain,port,base_url,id FROM accounts WHERE id = %d and type = %d and enabled = 1;", account_id, ACCOUNT_TYPE_STATUSNET);
	if(sqlite_res != -1) {
		ad = calloc(1, sizeof(accountData));
		ad->account_id = account_id;
		ad->as = as;
		ad->add_repeat = callback;
		ad->data = data;
		sqlite_res = sqlite3_exec(ed_DB, query, ed_statusnet_repeat_handler, (void*)ad, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't run %s: %d = %s\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);
	}
}

Eina_Bool ed_sn_single_status_disconnected(void *data, int type, Azy_Client *cli) {
        return(EINA_TRUE);
}

static int ed_statusnet_status_get_handler(void *data, int argc, char **argv, char **azColName) {
    char *screen_name=NULL, *password=NULL, *proto=NULL, *domain=NULL, *base_url=NULL, *url_start=NULL, *url_end=NULL;
    int port=0, id=0, res;
	long long int in_reply_to;
	Azy_Client *cli=NULL;
	aStatus **prelated_status = (aStatus**)data;
	int sqlite_res=0;
	struct sqlite3_stmt *insert_stmt=NULL;
	const char *missed=NULL;
	char *key=NULL, *query=NULL;

    /* In this query handler, these are the current fields:
        argv[0] == name TEXT
        argv[1] == password TEXT
        argv[2] == type INTEGER
        argv[3] == proto TEXT
        argv[4] == domain TEXT
        argv[5] == port INTEGER
        argv[6] == base_url TEXT
        argv[7] == id INTEGER
        argv[8] == in_reply_to INTEGER
    */

    screen_name = argv[0];
    password = argv[1];
    proto = argv[3];
    domain = argv[4];
    port = atoi(argv[5]);
    base_url = argv[6];
    id = atoi(argv[7]);
    in_reply_to = strtoll(argv[8], NULL, 10);

	cli = azy_client_new();

	res = asprintf(&url_start, "%s://%s", proto, domain);
	res = asprintf(&url_end, "%s/statuses/show/%lld.json?source=elmdentica", base_url, in_reply_to);

	azy_client_host_set(cli, url_start, port);
	azy_client_connect(cli, EINA_TRUE);
	azy_net_auth_set(azy_client_net_get(cli), screen_name, password);
	azy_net_uri_set(azy_client_net_get(cli), url_end);
	azy_net_version_set(azy_client_net_get(cli), 0);

    ecore_event_handler_add(AZY_CLIENT_CONNECTED, (Ecore_Event_Handler_Cb)timeline_connected, NULL); // FIXME: FALTAM AQUI accountDatas
    ecore_event_handler_add(AZY_CLIENT_RETURN, (Ecore_Event_Handler_Cb)timeline_returned, NULL);
    ecore_event_handler_add(AZY_CLIENT_DISCONNECTED, (Ecore_Event_Handler_Cb)ed_sn_single_status_disconnected, NULL);

	if(*prelated_status) {
		(*prelated_status)->account_id = id;
		(*prelated_status)->account_type = atoi(argv[2]);

		res = asprintf(&query, "insert into messages (status_id, account_id, timeline, s_text, s_truncated, s_created_at, s_in_reply_to_status_id, s_source, s_in_reply_to_user_id, s_favorited, s_user) values (?, %d, %d, ?, ?, ?, ?, ?, ?, ?, ?);", id, -1);;
		if(sqlite_res != -1) {
			sqlite_res = sqlite3_prepare_v2(ed_DB, query, 4096, &insert_stmt, &missed);
			if(sqlite_res == 0) {
				if(debug > 3) printf("Inserting: %lld\n", (long long int)(*prelated_status)->status->id);
				res = asprintf(&key, "%lld", (long long int)(*prelated_status)->status->id);
				if(res != -1) {
					message_insert((*prelated_status)->status, &insert_stmt);
					free(key);
				}
				sqlite3_finalize(insert_stmt);
			} else {
				fprintf(stderr, "Can't do %s: %d means '%s' was missed in the statement.\n", query, sqlite_res, missed);
			}
			free(query);
		}
	}
	return(0);
}

void ed_statusnet_status_get(int account_id, long long int in_reply_to, aStatus **prelated_status) {
	char *query=NULL, *db_err=NULL, *key=NULL;
	aStatus *as=NULL;
	int sqlite_res;

	sqlite_res = asprintf(&key, "%lld", in_reply_to);
	if(sqlite_res != -1) {
		as = eina_hash_find(statusHash, key);
		free(key);
		if(as) {
			if(debug > 3) printf("Status %lld is present in memory cache\n", in_reply_to);
			*prelated_status = as;
			return;
		}
	}

/*
	sqlite_res = asprintf(&query, "SELECT messages.*, accounts.type, accounts.id, accounts.enabled FROM messages,accounts WHERE account_id = %d and status_id = %lld LIMIT 1;", account_id, in_reply_to);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, ed_statusnet_status_get_from_db, (void**)prelated_status, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't run %s: %d = %s\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);

		if(*prelated_status) {
			if(debug > 3) printf("Status %lld is present in disk cache\n", in_reply_to);
			return;
		} else if(debug > 3) printf("Status %lld is NOT present in disk cache\n", in_reply_to);
	}
*/

	if(debug>3) printf("Downloading status %lld with account %d\n", in_reply_to, account_id);
	sqlite_res = asprintf(&query, "SELECT name,password,type,proto,domain,port,base_url,id,%lld FROM accounts WHERE id = %d and type = %d and enabled = 1;", in_reply_to, account_id, ACCOUNT_TYPE_STATUSNET);
	if(sqlite_res != -1) {
		sqlite_res = sqlite3_exec(ed_DB, query, ed_statusnet_status_get_handler, (void**)prelated_status, &db_err);
		if(sqlite_res != 0) {
			fprintf(stderr, "Can't run %s: %d = %s\n", query, sqlite_res, db_err);
			sqlite3_free(db_err);
		}
		free(query);
	}
}
