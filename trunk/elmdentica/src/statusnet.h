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


#ifndef STATUSNET_H
#define STATUSNET_H

#include <elmdentica.h>

typedef struct _Group_Profile {
    char        *name;
    char        *fullname;
    Eina_Bool   member;
    int         member_count;
    char        *original_logo;
    char        *description;
	Eina_Bool	failed;
	char		*error;
} GroupProfile;

typedef struct _group_get {
	GroupProfile *group;
	int account_id;
} GroupGet;

void ed_statusnet_group_get(int account_id, GroupProfile *group);
int ed_statusnet_post(int id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, char *msg);
void ed_statusnet_timeline_get(int id, char *screen_name, char *password, char *proto, char *domain, int port, char *base_url, int timeline);

void ed_statusnet_group_free(GroupProfile *group);

void ed_statusnet_group_join(int account_id, GroupProfile *group);
void ed_statusnet_group_leave(int account_id, GroupProfile *group);

#endif
