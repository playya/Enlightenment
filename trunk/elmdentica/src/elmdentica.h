/* vim:tabstop=4
 * Copyright © 2009 Rui Miguel Silva Seabra <rms@1407.org>
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


#ifndef ELMDENTICA_H

#define ELMDENTICA_H

typedef enum _TimeLineStates {
	HASH,
	HASH_ERROR,
	HASH_REQUEST,
	FT_NULL,
	FT_START,
	FT_STATUS,
	FT_ID,
	FT_USER,
	FT_NAME,
	FT_AVATAR,
	FT_SCREEN_NAME,
	FT_TEXT,
	FT_CREATED_AT,
	FT_RT,
	FT_PLACE
} TimeLineStates;

typedef enum _UsersShowStates {
	US_HASH,
	US_HASH_ERROR,
	US_HASH_REQUEST,
	US_ERROR,
	US_NULL,
	US_START,
	US_NAME,
	US_DESCRIPTION,
	US_PROFILE_IMAGE,
	US_PROTECTED,
	US_FOLLOWERS_COUNT,
	US_FRIENDS_COUNT,
	US_FOLLOWING,
	US_TEXT,
} UsersShowStates;

typedef struct _ub_Status {
	char * screen_name;
	char * name;
	char * text;
	time_t created_at;
	long long int    id;
} ub_Status;

typedef struct _ub_bubble {
	char *screen_name;
	char *message;
	int   account_id;
	int   account_type;
	long int   status_id;
	Eina_Bool	favorite;
} ub_Bubble;

typedef struct anchor_data {
	Evas_Object *bubble;
	char        *url;
} AnchorData;

typedef struct gag_data {
	char        *screen_name;
	char        *name;
	char        *message;
	Eina_Bool	match;
} GagData;

typedef struct _StatusesList {
	Eina_List	*list;
	ub_Status 	*current;
	TimeLineStates	state;
	char		*hash_error;
	char		*hash_request;
} StatusesList;

typedef struct _User_Profile {
	UsersShowStates	state;
	char		*name;
	char		*screen_name;
	char		*description;
	char		*tmp;
	Eina_Bool	protected;
	int			followers_count;
	int			friends_count;
	Eina_Bool	following;
} UserProfile;

#define ACCOUNT_TYPE_NONE 0
#define ACCOUNT_TYPE_STATUSNET 1
#define ACCOUNT_TYPE_TWITTER 2

#define TIMELINE_FRIENDS 0
#define TIMELINE_USER 1
#define TIMELINE_PUBLIC 2
#define TIMELINE_FAVORITES 3

#define BROWSER_XDG		0
#define	BROWSER_VENTURA	1
#define	BROWSER_MIDORI	2
#define	BROWSER_WOOSH	3
#define	BROWSER_DILLO	4

void error_win_del(void *data, Evas_Object *zbr, void *event_info);

void set_urls(void);

void fill_message_list(int timeline);

#endif
