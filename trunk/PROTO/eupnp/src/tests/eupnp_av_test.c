/* Eupnp - UPnP library
 *
 * Copyright (C) 2009 Andre Dieb Martins <andre.dieb@gmail.com>
 *
 * This file is part of Eupnp.
 *
 * Eupnp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Eupnp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Eupnp.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <string.h>
#include "eupnp_suite.h"

#include "eupnp_av/Eupnp_AV.h"

#ifdef INFO
#undef INFO
#endif
#define INFO(...) printf(__VA_ARGS__)

static void
container_found(void *data, DIDL_Container *container)
{
   INFO("Container found %p\n", container);
}

static void
item_found(void *data, DIDL_Item *item)
{
   INFO("Item found %p\n", item);
}


START_TEST(eupnp_av_didl_containers)
{
   FILE *f;
   long size;
   char *xml;

   eupnp_init();
   eupnp_av_init();

   fail_if(!(f = fopen("src/tests/didl_sample_1.xml", "r")));
   fseek(f, 0, SEEK_END);
   size = ftell(f);
   rewind(f);
   xml = malloc(sizeof(char) * (size + 1));
   fail_if((fread(xml, size, 1, f) != 1));
   xml[size] = '\0';
   fclose(f);

   fail_if(!eupnp_av_didl_parse(xml, size, &item_found, &container_found, NULL));

   eupnp_av_shutdown();
   eupnp_shutdown();
}
END_TEST


void
eupnp_test_av_didl_parser(TCase *t)
{
   tcase_add_test(t, eupnp_av_didl_containers);
//   tcase_add_test(t, eupnp_av_didl_containers_and_items);
}
