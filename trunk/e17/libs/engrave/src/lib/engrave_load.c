#include <errno.h>

#include "Engrave.h"
#include "engrave_parse.h"

#define MAIN_EDC_NAME "main_edje_source.edc"

char *engrave_filename = NULL;

/**
 * engrave_load_edc - load the given edc file into memory.
 * @param file: The EDC file to load.
 * @param imdir: The image directory for the EDC file.
 * @param fontdir: The font directory for the EDC file.
 *
 * @return Returns a pointer to a newly allocated Engrave_File object on
 * success or NULL on failure.
 */
Engrave_File *
engrave_load_edc(char *file, char *imdir, char *fontdir)
{
  Engrave_File *enf;
  int fd;
  char buf[4096];
  char tmpf[4096];

  if (!file) return NULL;
  strcpy(tmpf, "/tmp/engrave_parse.edc-tmp-XXXXXX");
  fd = mkstemp(tmpf);
  if (fd >= 0)
  {
    int ret;

    snprintf(buf, sizeof(buf), "cat %s | cpp -E -o %s", file, tmpf);
    ret = system(buf);
    if (ret < 0)
    {
      snprintf(buf, sizeof(buf), "gcc -E -o %s %s", tmpf, file);
      ret = system(buf);
    }
    if (ret >= 0) file = tmpf;
    close(fd);
  }

  engrave_filename = strdup(file);
  enf = engrave_parse(file);
  FREE(engrave_filename);
  unlink(tmpf);

  return(enf);
}

/**
 * engrave_load_eet - load the given EET file into memory.
 * @param filename: The filename of the EET file to load.
 *
 * @return Returns a pointer to a newly allocated Engrave_File object on
 * success or NULL on failure.
 */
Engrave_File *
engrave_load_eet(char *filename)
{
  Engrave_File *enf = NULL;
  char *cmd = NULL;
  char *old_fname;
  char *new_fname = NULL;
  char *ptr = NULL;
  int len = 0;
  int ret = 0;
  char *work_dir = NULL;
  static char tmpn[4096];
  char *cpp_extra = NULL;

  if (!filename) return NULL;
  old_fname = strdup(filename);

  memset(tmpn, '\0', sizeof(tmpn));
  strcpy(tmpn, "/tmp/engrave.edc-tmp-XXXXXX");
  if (mkdtemp(tmpn) == NULL) {
    fprintf(stderr, "Can't create working dir: %s",
            strerror(errno));
    return 0;
  }
  work_dir = strdup(tmpn);

  ptr = strrchr(old_fname, '/');
  if (ptr == NULL)
      ptr = old_fname;

  len = strlen(work_dir) + strlen(old_fname) + strlen(ptr) + 6;
  cmd = (char *)calloc(len,sizeof(char));
  snprintf(cmd, len, "cp %s %s/%s", old_fname, work_dir, ptr);
  ret = system(cmd);
  FREE(cmd);

  if (ret < 0) {
    fprintf(stderr, "Unable to copy %s to tmp dir %s: %s\n",
        old_fname, work_dir, strerror(errno));
    return 0;
  }

  /* we change to the work dir because edje_cc will extract into the
   * current directory. 
   */
  getcwd(tmpn, sizeof(tmpn));
  if (chdir(work_dir) == -1) {
    fprintf(stderr, "Can't change to work dir %s: %s\n", work_dir,
            strerror(errno));
    return 0;
  }

  len = strlen(work_dir) + strlen(ptr) + 12;
  cmd = (char *)calloc(len, sizeof(char));
  snprintf(cmd, len, "edje_decc %s/%s", work_dir, ptr);
  ret = system(cmd);
  FREE(cmd);

  if (ret < 0) {
    fprintf(stderr, "Unable to de-compile %s\n", ptr);
    return 0;
  }

  /* change back to the original dir because edje_cc will write into
   * that dir currently
   */
  if (chdir(tmpn) == -1) {
    fprintf(stderr, "Can't change back to current dir: %s\n", 
            strerror(errno));
    return 0;
  }

  cmd = strstr(ptr, ".eet");
  *cmd = '\0';

  /* we need the info on the work dir to pass the cpp so it can
   * include files correctly 
   */
  len = strlen(ptr) + strlen(work_dir) + 4;
  cpp_extra = (char *)calloc(len, sizeof(char));
  snprintf(cpp_extra, len, "-I%s/%s", work_dir, ptr);

  len = strlen(work_dir) + strlen(ptr) +
          strlen(MAIN_EDC_NAME) + 3;
  new_fname = (char *)calloc(len, sizeof(char));
  snprintf(new_fname, len, "%s/%s/%s", work_dir, ptr, 
            MAIN_EDC_NAME);
  FREE(old_fname);

  enf = engrave_load_edc(new_fname, work_dir, work_dir);

  FREE(work_dir);

  return enf;
}

