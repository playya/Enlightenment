#include <sys/stat.h>
#include "container.h"
#include "container_private.h"

Container *
_container_fetch(Evas_Object *obj)
{
  Container *cont;
  char *type;

  type = (char *)evas_object_type_get(obj);
  if (!type) return NULL;
  if (strcmp(type, "container")) return NULL;
  cont = evas_object_smart_data_get(obj); 
  return cont;
}

void
_container_scale_scroll(Container *cont, double old_length)
{
  int new_scroll;
  double new_length;
  
  new_length = e_container_elements_length_get(cont->obj);

  if (new_length < (cont->direction ? cont->h : cont->w))
  {
    new_scroll = 0;
  }
  else 
    new_scroll = cont->scroll_offset * (new_length / old_length);

  e_container_scroll_offset_set(cont->obj, new_scroll);

  
}

int
is_dir(const char *dir)
{
	struct stat st;

	if (stat(dir, &st))
		return 0;

	return (S_ISDIR(st.st_mode));
}


Evas_List *_dir_get_files(const char *directory) {
	Evas_List *list = NULL;
	DIR *dir;
	struct dirent *entry;

	if (!(dir = opendir(directory)))
		return NULL;

	while ((entry = readdir(dir))) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;

		if (!is_dir(entry->d_name))
			list = evas_list_prepend(list, strdup(entry->d_name));
	}

	closedir(dir);

	if (list)
		list = evas_list_reverse(list);

	return list;
}

