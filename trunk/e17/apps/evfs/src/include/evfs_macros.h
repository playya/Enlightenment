#ifndef __EVFS_MACROS_H_
#define __EVFS_MACROS_H_

#define NEW(X)    ((X*) calloc(1, sizeof(X)))
#define LOCK(X)    (pthread_mutex_lock(X))
#define UNLOCK(X)  (pthread_mutex_unlock(X))

#define READDIR(dir, de, de_ptr)  (de_ptr = readdir(dir))
#define CTIME(time_ptr, buf)      (buf = ctime(time_ptr))

#endif
