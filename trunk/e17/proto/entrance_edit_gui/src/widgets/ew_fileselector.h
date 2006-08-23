#ifndef _EW_FILESELECTOR_H
#define _EW_FILESELECTOR_H

Entrance_Widget ew_fileselector_new(const char *title, const char *folder, int multiple,
		int showdot, void (*response_callback)(void *, int, void *), void *data);

const char *ew_fileselector_file_get(Entrance_Widget);
Evas_List *ew_fileselector_file_list_get(Entrance_Widget);

#endif
