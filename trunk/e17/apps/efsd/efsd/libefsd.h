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
#ifndef libefsd_h
#define libefsd_h

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <efsd.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Efsd connection: */
typedef struct efsd_connection EfsdConnection;


/* Creates and returns an efsd connection. 
*/
EfsdConnection *efsd_open(void);


/* Use this to close an efsd connection.
   Frees the allocated EfsdConnection.
   Returns value < 0 if the the final
   command could not be sent to Efsd.
 */
int            efsd_close(EfsdConnection *ec);


/* Use this to get the file descriptor of an efsd
   connection.
*/
int            efsd_get_connection_fd(EfsdConnection *ec);


/* Returns value > 0 if a following efsd_next_event() would
   succeed.
*/
int            efsd_events_pending(EfsdConnection *ec);


/* If available, reads an event the efsd daemon sent. It does
   not block. You want to use this if you select() efsd's file
   descriptor. Returns -1 when called on closed connection,
   >= 0 otherwise.
*/
int            efsd_next_event(EfsdConnection *ec, EfsdEvent *ev);


/* Blocks until an efsd event arrives, then returns it.
   Returns -1 when called on closed connection, >= 0
   otherwise.
*/
int            efsd_wait_event(EfsdConnection *ec, EfsdEvent *ev);


/* Events may contain allocated data -- clean that up here.
   To be called before any other calls to efsd_next_event
   on the same EfsdEvent instance.

   NOTE -- this does not free the EfsdEvent itself -- only
   data that was read into it. If you want to get rid of
   the EfsdEvent entirely, call efsd_cleanup_event() first
   and then simply free() the pointer.
*/
void           efsd_event_cleanup(EfsdEvent *ev);


/* Various commands to operate on the fs.

   UNLESS there's a communication problem (e.g. clogged
   buffers), each command returns and efsd command ID, which
   is also contained in the efsd events returned by the server
   so that commands and the generated replies can be associated.
   In case of an error, a value < 0 is returned.

   Filenames are internally converted to fully canonical
   path names if not fully specified.
*/

EfsdCmdId      efsd_remove(EfsdConnection *ec, char *filename);
EfsdCmdId      efsd_move(EfsdConnection *ec, char *from_file, char *to_file);

/* Creates a symbolic link from the from_file to the
   to_file
*/
EfsdCmdId      efsd_symlink(EfsdConnection *ec, char *from_file, char *to_file);

/* Lists the contents of a directory by starting a FAM
   monitor for the directory, thus generating an "exists"
   FAM event for each file in the directory, and then
   stopping the monitor afterwards.
*/
EfsdCmdId      efsd_listdir(EfsdConnection *ec, char *dirname);

/* Create a directory. Behaves like "mkdir -p", i.e. it can
   create directories recursively. Multiple slashes are
   treated as one, prefixed or suffixed slashes don't matter.
*/
EfsdCmdId      efsd_makedir(EfsdConnection *ec, char *dirname);

/* Chmods the given file to the given mode.
 */
EfsdCmdId      efsd_chmod(EfsdConnection *ec, char *filename,  mode_t mode);

/* Metadata operations.
 */
EfsdCmdId      efsd_set_metadata(EfsdConnection *ec, char *key, char *filename,
				 EfsdDatatype datatype, int datalength, void *data);
EfsdCmdId      efsd_del_metadata(EfsdConnection *ec, char *key, char *filename);
EfsdCmdId      efsd_get_metadata(EfsdConnection *ec, char *key, char *filename);

/* Start/stop a FAM monitor for a given file or directory.
 */
EfsdCmdId      efsd_start_monitor(EfsdConnection *ec, char *filename);
EfsdCmdId      efsd_stop_monitor(EfsdConnection *ec, char *filename);

/* Returns the full file stats in the generated reply,
   as returned by the lstat() command.
 */
EfsdCmdId      efsd_stat(EfsdConnection *ec, char *filename);

/* Returns the file a symlink points to */
EfsdCmdId      efsd_readlink(EfsdConnection *ec, char *filename);

/* Returns the mimetype for a file */
EfsdCmdId      efsd_get_mimetype(EfsdConnection *ec, char *filename);

#ifdef __cplusplus
}
#endif

#endif
