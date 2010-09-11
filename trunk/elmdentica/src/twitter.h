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


#ifndef TWITTER_H
#define TWITTER_H

#include <Elementary.h>
#include <Ecore_X.h>

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

typedef struct _ub_Status {
    char * screen_name;
    char * name;
    char * text;
    time_t created_at;
    long long int    id;
    long long int   user_id;
    long long int   in_reply_to;
} ub_Status;

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
	int account_type;
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

int ed_twitter_post(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, char *msg);
void ed_twitter_timeline_get(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, int timeline);
void ed_twitter_favorite_create(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, long long int status_id);
void ed_twitter_favorite_destroy(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, long long int status_id);
void ed_twitter_init_friends(void);
anUser *ed_twitter_user_get(int account_id, UserProfile *user);
void ed_twitter_user_follow(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, char *user_screen_name);
void ed_twitter_user_abandon(int account_id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, char *user_screen_name);
void ed_twitter_repeat(int account_id, long long int status_id);
void ed_twitter_status_get(int account_id, long long int in_reply_to, aStatus **related_status);

void status_hash_data_free(void *data);
void user_hash_data_free(void *data);

void ed_twitter_statuses_get_avatar(char *id, char *url);

#endif
