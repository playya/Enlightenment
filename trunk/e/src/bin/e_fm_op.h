/*
 * vim:cindent:ts=8:sw=3:sts=8:expandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS

#define E_FM_OP_DEBUG(...) fprintf(stderr, __VA_ARGS__)

#define E_FM_OP_MAGIC 314

typedef enum _E_Fm_Op_Type
{
   E_FM_OP_COPY = 0,
   E_FM_OP_MOVE = 1,
   E_FM_OP_REMOVE = 2,
   E_FM_OP_ABORT = 3,
   E_FM_OP_ERROR = 4,
   E_FM_OP_ERROR_RESPONSE_IGNORE_THIS = 5,
   E_FM_OP_ERROR_RESPONSE_IGNORE_ALL = 6,
   E_FM_OP_ERROR_RESPONSE_ABORT = 7,
   E_FM_OP_PROGRESS = 8,
   E_FM_OP_NONE = 9,
   E_FM_OP_ERROR_RESPONSE_RETRY = 10,
   E_FM_OP_OVERWRITE = 11,
   E_FM_OP_OVERWRITE_RESPONSE_NO = 12,
   E_FM_OP_OVERWRITE_RESPONSE_NO_ALL = 13,
   E_FM_OP_OVERWRITE_RESPONSE_YES = 14,
   E_FM_OP_OVERWRITE_RESPONSE_YES_ALL = 15,
   E_FM_OP_COPY_STAT_INFO = 16,
   E_FM_OP_MKDIR,
   E_FM_OP_TRASH,
   E_FM_OP_MONITOR_START,
   E_FM_OP_MONITOR_SYNC,
   E_FM_OP_MONITOR_END,
   E_FM_OP_MOUNT,
   E_FM_OP_UNMOUNT,
   E_FM_OP_HELLO,
   E_FM_OP_FILE_ADD,
   E_FM_OP_FILE_CHANGE,
   E_FM_OP_FILE_DEL,
   E_FM_OP_STORAGE_ADD,
   E_FM_OP_STORAGE_DEL,
   E_FM_OP_VOLUME_ADD,
   E_FM_OP_VOLUME_DEL,
   E_FM_OP_MOUNT_DONE,
   E_FM_OP_UNMOUNT_DONE,
   E_FM_OP_QUIT,
   E_FM_OP_SYMLINK,
   E_FM_OP_OK,
   E_FM_OP_ERROR_RETRY_ABORT,
   E_FM_OP_REORDER
} E_Fm_Op_Type;

#else
#ifndef E_FM_OP_H
#define E_FM_OP_H

#endif
#endif
