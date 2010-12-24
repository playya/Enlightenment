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


#ifndef STATUSNET_H
#define STATUSNET_H

#include <Elementary.h>
#include <Ecore_X.h>

typedef struct _StatusNetBaAccount {
	double id;
	char *screen_name;
	char *password;
	char *proto;
	char *domain;
	short port;
	char *base_url;
} StatusNetBaAccount;

typedef struct _Group_Profile {
    char  *name;
    char        *fullname;
    Eina_Bool   member;
    int         member_count;
    char        *original_logo;
    char        *description;
	Eina_Bool	failed;
	char		*error;
	int account_id;
} GroupProfile;

typedef struct _group_get {
	GroupProfile *group;
	int account_id;
} GroupGet;

typedef struct _User_Profile {
    char        *name;
    char        *screen_name;
    char        *description;
    char        *text;
    time_t      status_created_at;
    char        *tmp;
    Eina_Bool   protected;
    int         followers_count;
    int         friends_count;
    Eina_Bool   following;
} UserProfile;

typedef struct _a_Status {
	long long int sid;
	char *text;
	Eina_Bool truncated;
	time_t	created_at;
	long long int in_reply_to_status_id;
	char *source;
	long long int in_reply_to_user_id;
	Eina_Bool favorited;
	long long int user;
	int account_id;
	short account_type;
	Evas_Object *bubble;
	Eina_Bool	in_db;
} aStatus;

typedef struct _an_User {
	long long int uid;
	char *name;
	char *screen_name;
	char *location;
	char *description;
	char *profile_image_url;
	char *url;
	Eina_Bool protected;
	int followers_count;
	int friends_count;
	time_t created_at;
	int favorites_count;
	int statuses_count;
	Eina_Bool following;
	Eina_Bool statusnet_blocking;
	Eina_Bool	in_db;
} anUser;

typedef struct _user_get {
	UserProfile *user;
	int account_id;
	anUser *au;
} UserGet;

void ed_statusnet_account_free(StatusNetBaAccount *account);

void ed_statusnet_group_get(GroupProfile *group);

void ed_statusnet_group_free(GroupProfile *group);

void ed_statusnet_group_join(GroupProfile *group);
void ed_statusnet_group_leave(GroupProfile *group);

int ed_statusnet_post(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, char *msg);
void ed_statusnet_timeline_get(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, int timeline);
void ed_statusnet_favorite_create(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, long long int status_id);
void ed_statusnet_favorite_destroy(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, long long int status_id);
void ed_statusnet_init_friends(void);
anUser *ed_statusnet_user_get(int account_id, UserProfile *user);
void ed_statusnet_user_follow(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, char *user_screen_name);
void ed_statusnet_user_abandon(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, char *user_screen_name);
void ed_statusnet_repeat(int account_id, long long int status_id);
void ed_statusnet_status_get(int account_id, long long int in_reply_to, aStatus **related_status);

void status_hash_data_free(void *data);
void user_hash_data_free(void *data);

void ed_statusnet_statuses_get_avatar(char *id, char *url);

#endif
