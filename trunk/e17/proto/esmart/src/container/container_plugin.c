/* plugin code taken from eplayer and modified */

#include <Evas.h>
#include <config.h>
#include <stdlib.h>
#include <ltdl.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "container.h"
#include "container_private.h"


static char *_find_plugin(const char *dir, const char *name)
{
  Evas_List *files, *l;
  char *ret = NULL, tmp[128];

  if (!(files = _dir_get_files(dir)))
    return NULL;

  for (l = files; l; l = l->next) {
    sscanf((char *)l->data, "%127[^.].so", tmp);

    if (!strcasecmp(name, tmp)) {
      ret = strdup(l->data);
      break;
    }
  }

  while (files)
  {
    free(files->data);
    files = evas_list_remove(files, files->data);
  }
  
  printf("ret: %s\n", ret);
  return ret;
}

Container_Layout_Plugin *
_container_layout_plugin_new(const char *name)
{
  Container_Layout_Plugin *p;

  int (*init)(void *p);
  char path[PATH_MAX + 1];
  char *right_name;
  int numerr;

  if (!name || !*name)
  {
    fprintf(stderr, "ERROR: no name given\n");
    return NULL;
  }
  
  if (!(p = malloc(sizeof(Container_Layout_Plugin))))
    return NULL;

  memset(p, 0, sizeof(Container_Layout_Plugin));

  snprintf(path, sizeof(path), "%s/%s.so", CONTAINER_PLUGIN_DIR, name); 

  if ((numerr = lt_dlinit()))
  {
    fprintf(stderr, "ERROR: initting lt, %d\n", numerr);
    fprintf(stderr, "ERROR - lt: (%s)\n", lt_dlerror());
  }

  if (!(p->handle = lt_dlopen(path))) {
    /* try insensitive search */
    if (!(right_name = _find_plugin(CONTAINER_PLUGIN_DIR, name))) {
      _container_layout_plugin_free(p);
      fprintf(stderr, "ERROR: can't find plugin (%s)\n", name);
      return NULL;
    }

    snprintf(path, sizeof(path), "%s/%s", CONTAINER_PLUGIN_DIR, right_name);
    free(right_name);

    if (!(p->handle = lt_dlopen(path)))
    {
      fprintf(stderr, "ERROR: can't dlopen plugin (%s)\n", path);
      fprintf(stderr, "ERROR - lt: (%s)\n", lt_dlerror());
      _container_layout_plugin_free(p);
      return NULL;
    }
  }
  
  init = lt_dlsym(p->handle, "plugin_init");

  if (!(*init)(p))
  {
    fprintf(stderr, "ERROR: can't init plugin\n");
    fprintf(stderr, "ERROR - lt: (%s)\n", lt_dlerror());
    _container_layout_plugin_free(p);
    return NULL;
  }
  
  return p;
}

void
_container_layout_plugin_free(Container_Layout_Plugin *p)
{
  if (!p) return;

  if (p->shutdown)
  {
    p->shutdown();
  }
  
  if (p->handle)
  {
    int error = lt_dlclose(p->handle);
    if (error)
    {
      fprintf(stderr, "ERROR: lt_dlclose (%s)\n", lt_dlerror());  
    }
  }

  free(p);

  lt_dlexit();
}

/* returns 0 on error, nonzero if ok */
int
e_container_layout_plugin_set(Evas_Object *container, const char *plugin)
{
  Container *cont = NULL;

  if (!(cont = _container_fetch(container)))
  {
    fprintf(stderr, "ERROR: no container\n");
    return 0;
  }

  if (cont->plugin)
  {
    _container_layout_plugin_free(cont->plugin);
    cont->plugin = NULL;
  }
 
  if (!(cont->plugin = _container_layout_plugin_new(plugin)))
  {
    fprintf(stderr, "ERROR: using default layout plugin\n");
    if (!(cont->plugin = _container_layout_plugin_new("default")))
    {
      fprintf(stderr, "ERROR: can't load default plugin\n");
      return 0;
    }
  }
  
  return 1;
}

