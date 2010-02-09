/* EINA - EFL data type library
 * Copyright (C) 2010 Sebastian Dransfeld
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "eina_suite.h"
#include "Eina.h"

START_TEST(eina_strbuf_simple)
{
   Eina_Strbuf *buf;
   char text[] = "This test should be so long that it is longer than the initial size of strbuf";

   eina_init();

   buf = eina_strbuf_new();
   fail_if(!buf);

   eina_strbuf_append(buf, text);
   fail_if(strcmp(eina_strbuf_string_get(buf), text));

   eina_strbuf_free(buf);

   eina_shutdown();
}
END_TEST

void
eina_test_strbuf(TCase *tc)
{
   tcase_add_test(tc, eina_strbuf_simple);
}
