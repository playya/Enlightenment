#include <config.h>
#include <Evas.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>

int is_dir(const char *dir) {
	struct stat st;

	if (stat(dir, &st))
		return 0;

	return (S_ISDIR(st.st_mode));
}

Evas_List *dir_get_files(const char *directory) {
	Evas_List *list = NULL;
	DIR *dir;
	struct dirent *entry;

	if (!(dir = opendir(directory)))
		return NULL;

	/* ignore "." and ".." */
	while ((entry = readdir(dir))
	       && (!strcmp(entry->d_name, ".")
	       || !strcmp(entry->d_name, "..")));

	if (!entry)
		return NULL;
	
	/* real entries */
	do {
		if (!is_dir(entry->d_name))
			list = evas_list_prepend(list, strdup(entry->d_name));
	} while ((entry = readdir(dir)));

	closedir(dir);

	if (list)
		list = evas_list_reverse(list);

	return list;
}

/**
 * Removes leading and trailing whitespace from a string.
 *
 * @param str String to strip
 * @return Stripped string
 */
char *strstrip(char *str) {
	char *start, *ptr = str;
	
	/* step over leading whitespace */
	for (start = str; isspace(*start); start++);
	
	if (str != start) {
		while ((*ptr++ = *start++));
		*ptr = 0;
	}

	if (!*str)
		return str;

	/* remove trailing whitespace */
	ptr = &str[strlen(str) - 1];

	if (!isspace(*ptr))
		return str;
	
	while (isspace(*ptr) && ptr >= str)
		ptr--;

	ptr[1] = 0;

	return str;
}
