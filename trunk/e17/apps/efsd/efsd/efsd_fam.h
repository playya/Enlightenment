/*

Copyright (C) 2000, 2001 Christian Kreibich <cK@whoop.org>.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its documentation and acknowledgment shall be
given in the documentation and software packages that this Software was
used.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#ifndef efsd_fam_h
#define efsd_fam_h

#include <fam.h>

#include <efsd.h>
#include <efsd_options.h>
#include <efsd_list.h>

typedef enum
{
  EFSD_FAM_MONITOR_NORMAL   = -1,
  EFSD_FAM_MONITOR_INTERNAL = -2
}
EfsdFamMonType;

typedef struct efsd_fam_request
{
  EfsdFamMonType        type;

  int                   client;
  EfsdCmdId             id;

  int                   num_options;
  EfsdOption           *options;
}
EfsdFamRequest;


typedef struct efsd_fam_monitor
{
  /* filename that is monitored     */
  char                 *filename;

  /* the FAM request                */
  FAMRequest           *fam_req;

  /* use count for this monitor     */
  int                   use_count;

  /* whether this monitor is
     registered internally          */
  char                  registered;

  /* Which clients monitor this file,
     and with what command id.
     list<EfsdFamRequest*>.
  */
  EfsdList             *clients;
}
EfsdFamMonitor;



void             efsd_fam_init(void);
void             efsd_fam_cleanup(void);

/* This one frees the monitor and removes
   it from the list of monitors.
*/
void             efsd_fam_remove_monitor(EfsdFamMonitor *m);


/* High-level API for monitoring stuff -- refcounting
   & co are handled inside. Return >= 0 on success.
*/
int              efsd_fam_start_monitor(EfsdFamMonType type, EfsdCommand *com,
					int client);
int              efsd_fam_stop_monitor(EfsdCommand *cmd, int client);

/* For internal monitoring of files -- specify file name directly.
 */
int              efsd_fam_start_monitor_internal(char *filename);
int              efsd_fam_stop_monitor_internal(char *filename);

/* Monitor filename briefly to get directory listing events.
 */
int              efsd_fam_force_startstop_monitor(EfsdCommand *cmd, int client);

/* Returns value >0 when file is already monitored.
 */
int              efsd_fam_is_monitored(char *filename);

/* Check for all monitors if they are requested by CLIENT
   and in that case release those requests.
*/
int              efsd_fam_cleanup_client(int client);

#endif
