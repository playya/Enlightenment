#include <config.h>

#include "poppler.h"


static const char poppler_version[] = PACKAGE_VERSION;

const char *
evas_poppler_version_get (void)
{
  return poppler_version;
}
