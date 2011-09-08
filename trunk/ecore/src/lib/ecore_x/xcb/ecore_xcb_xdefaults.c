#include "ecore_xcb_private.h"
#include <fnmatch.h>

/* local function prototypes */
static Eina_Bool _ecore_xcb_xdefaults_glob_match(const char *str, const char *glob);

/* local variables */
static Eina_File *_ecore_xcb_xdefaults_file = NULL;
static char *_ecore_xcb_xdefaults_data = NULL;

void 
_ecore_xcb_xdefaults_init(void) 
{
   char buff[PATH_MAX];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   snprintf(buff, sizeof(buff), "%s/.Xdefaults", getenv("HOME"));
   if ((_ecore_xcb_xdefaults_file = eina_file_open(buff, EINA_FALSE)))
     {
        eina_mmap_safety_enabled_set(EINA_TRUE);

        _ecore_xcb_xdefaults_data = 
          eina_file_map_all(_ecore_xcb_xdefaults_file, EINA_FILE_SEQUENTIAL);
     }
}

void 
_ecore_xcb_xdefaults_shutdown(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!_ecore_xcb_xdefaults_file) return;
   if (_ecore_xcb_xdefaults_data) 
     eina_file_map_free(_ecore_xcb_xdefaults_file, _ecore_xcb_xdefaults_data);
   if (_ecore_xcb_xdefaults_file) eina_file_close(_ecore_xcb_xdefaults_file);
}

char *
_ecore_xcb_xdefaults_string_get(const char *prog, const char *param) 
{
   char buff[1024], ret[1024];
   char *str = NULL;
   char **ea = NULL;
   unsigned int count = 0, i = 0;

   if ((!_ecore_xcb_xdefaults_data) || (!_ecore_xcb_xdefaults_file))
     return NULL;

   snprintf(buff, sizeof(buff), "*%s*.*%s*", prog, param);

   str = _ecore_xcb_xdefaults_data;
   ea = eina_str_split_full(str, "\n", -1, &count);
   for (i = 0; i < count; i++) 
     {
        if (_ecore_xcb_xdefaults_glob_match(ea[i], buff)) 
          sscanf(ea[i], "%*[^:]:%*[ ]%s", ret);
     }
   if ((ea) && (ea[0]))
     {
        free(ea[0]);
        free(ea);
     }

   return strdup(ret);
}

int 
_ecore_xcb_xdefaults_int_get(const char *prog, const char *param) 
{
   char buff[1024];
   char *str = NULL;
   char **ea = NULL;
   unsigned int count = 0, i = 0;
   int ret = -1;

   if ((!_ecore_xcb_xdefaults_data) || (!_ecore_xcb_xdefaults_file))
     return 0;

   snprintf(buff, sizeof(buff), "*%s*.*%s*", prog, param);

   str = _ecore_xcb_xdefaults_data;
   ea = eina_str_split_full(str, "\n", -1, &count);
   for (i = 0; i < count; i++) 
     {
        if (_ecore_xcb_xdefaults_glob_match(ea[i], buff)) 
          sscanf(ea[i], "%*[^:]:%*[ ]%d", &ret);
     }
   if ((ea) && (ea[0]))
     {
        free(ea[0]);
        free(ea);
     }

   return ret;
}

/* local functions */
static Eina_Bool 
_ecore_xcb_xdefaults_glob_match(const char *str, const char *glob) 
{
   if ((!str) || (!glob)) return EINA_FALSE;
   if (glob[0] == 0) 
     {
        if (str[0] == 0) return EINA_TRUE;
        return EINA_FALSE;
     }
   if (!strcmp(glob, "*")) return EINA_TRUE;
   if (!fnmatch(glob, str, 0)) return EINA_TRUE;
   return EINA_FALSE;
}
