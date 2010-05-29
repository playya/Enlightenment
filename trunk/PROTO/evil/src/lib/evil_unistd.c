
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

# include <sys/time.h>

#ifdef _MSC_VER
# include <direct.h>   /* for _getcwd */
#endif

#include "Evil.h"
#include "evil_private.h"


LONGLONG _evil_time_freq;
LONGLONG _evil_time_count;
long     _evil_time_second;


long
_evil_systemtime_to_time(SYSTEMTIME st)
{
   int days[] = {
     -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
   };
   int day;
   time_t t;

   st.wYear -= 1900;
   if ((st.wYear < 70) || (st.wYear > 138))
     return -1;

   day = st.wDay + days[st.wMonth - 1];

  if (!(st.wYear & 3) && (st.wMonth > 2) )
    day++;

  t = ((st.wYear - 70) * 365 + ((st.wYear - 1) >> 2) - 17 + day) * 24 + st.wHour;
  t = (t * 60 + st.wMinute) * 60 + st.wSecond;

  return (long)t;
}

/*
 * Time related functions
 *
 */

double
evil_time_get(void)
{
   LARGE_INTEGER count;

   QueryPerformanceCounter(&count);

   return (double)_evil_time_second + (double)(count.QuadPart - _evil_time_count)/ (double)_evil_time_freq;
}

int
evil_gettimeofday(struct timeval *tp, void *tzp __UNUSED__)
{
   LARGE_INTEGER count;
   LONGLONG      diff;

   QueryPerformanceCounter(&count);
   diff = count.QuadPart - _evil_time_count;
   tp->tv_sec = _evil_time_second + diff / _evil_time_freq;
   tp->tv_usec = ((diff % _evil_time_freq) * 1000000ll) / _evil_time_freq;

   return 1;
}


/*
 * Process identifer related functions
 *
 */

#ifdef _MSC_VER
pid_t
getpid(void)
{
  return (pid_t)GetCurrentProcessId();
}
#endif

/*
 * File related functions
 *
 */

char *
evil_getcwd(char *buffer, size_t size)
{
#ifdef _WIN32_WCE
   wchar_t wpath[PATH_MAX];
   char   *cpath;
   char   *delim;
   DWORD   ret = 0;

   if (size <= 0)
     return NULL;

   ret = GetModuleFileName(GetModuleHandle(NULL), (LPWSTR)&wpath, PATH_MAX);

   if (!ret)
     {
        _evil_error_display(__FUNCTION__, ret);
        return NULL;
     }

   cpath = evil_wchar_to_char(wpath);
   if (!cpath)
     return NULL;

   if (strlen(cpath) >= (size - 1))
     {
        free(cpath);
        return NULL;
     }

   delim = strrchr(cpath, '\\');
   if (delim)
     *delim = '\0';

   if (!buffer)
     {
        buffer = (char *)malloc(sizeof(char) * size);
        if (!buffer)
          {
             free(cpath);
             return NULL;
          }
     }

   strcpy(buffer, cpath);
   free(cpath);

   return buffer;
#else
   return _getcwd(buffer, (int)size);
#endif /* ! _WIN32_WCE */
}

#ifdef _WIN32_WCE

int
evil_stat(const char *file_name, struct stat *st)
{
   SYSTEMTIME      system_time;
   FILETIME        local_time;
   WIN32_FIND_DATA data;
   HANDLE          handle;
   char           *f;
   char           *tmp;
   wchar_t        *file;
   int             permission = 0;

   if (!file_name || !*file_name)
     return -1;

   f = strdup(file_name);
   if (!f)
     return -1;

   tmp = f;
   while (*tmp)
     {
        if (*tmp == '/') *tmp = '\\';
        tmp++;
     }

   if (!strcmp(file_name, "\\"))
     {
        st->st_size = 1024;
        st->st_mode = S_IFDIR;
        permission = S_IREAD|S_IWRITE|S_IEXEC;

        st->st_mode |= permission | (permission >> 3) | (permission >> 6);
        return 0;
     }

   if (*f != '\\')
     {
        char  buf[PATH_MAX];
        int   l1;
        int   l2;

        evil_getcwd(buf, PATH_MAX);
        l1 = strlen(buf);
        l2 = strlen(file_name);
        tmp = (char *)malloc(l1 + 1 + l2 + 1);
        if (!tmp)
          return -1;
        memcpy(tmp, buf, l1);
        tmp[l1] = '\\';
        memcpy(tmp + l1 + 1, file_name, l2);
        tmp[l1 + 1 + l2] = '\0';
        file = evil_char_to_wchar(tmp);
        free(tmp);
        if (!file)
          return -1;
     }
   else
     {
        file = evil_char_to_wchar(f);
        if (!file)
          return -1;
     }

   free(f);

   handle = FindFirstFile(file, &data);
   if (handle == INVALID_HANDLE_VALUE)
     {
        free(file);
        return -1;
     }

   free(file);

   if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
     {
        st->st_size = 1024;
        st->st_mode = S_IFDIR;
        st->st_nlink = 2;
     }
   else
     {
        st->st_size = data.nFileSizeLow;
        st->st_mode = S_IFREG;
        st->st_nlink = 1;
     }

   permission |= S_IREAD;

   if (!(data.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
     permission |= S_IWRITE;

   if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
     permission |= S_IEXEC;

   st->st_mode |= permission | (permission >> 3) | (permission >> 6);

   FileTimeToLocalFileTime(&data.ftLastWriteTime, &local_time);
   FileTimeToSystemTime(&local_time, &system_time);

   st->st_mtime = _evil_systemtime_to_time(system_time);

   FileTimeToLocalFileTime(&data.ftCreationTime, &local_time);
   FileTimeToSystemTime(&local_time, &system_time);

   st->st_ctime = _evil_systemtime_to_time(system_time);

   FileTimeToLocalFileTime(&data.ftLastAccessTime, &local_time);
   FileTimeToSystemTime(&local_time, &system_time);

   st->st_atime = _evil_systemtime_to_time(system_time);

   if(st->st_atime == 0)
     st->st_atime = st->st_mtime;
   if (st->st_ctime == 0)
     st->st_ctime = st->st_mtime;

   st->st_rdev = 1;
   st->st_ino = 0;

   FindClose(handle);

  return 0;
}

#endif /* _WIN32_WCE */



/*
 * Sockets and pipe related functions
 *
 */

int
evil_sockets_init(void)
{
   WSADATA wsa_data;

   return (WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0) ? 1 : 0;
}

void
evil_sockets_shutdown(void)
{
   WSACleanup();
}

/*
 * The code of the following functions has been kindly offered
 * by Tor Lillqvist.
 */
int
evil_pipe(int *fds)
{
   struct sockaddr_in saddr;
   struct timeval     tv;
   SOCKET             temp;
   SOCKET             socket1 = INVALID_SOCKET;
   SOCKET             socket2 = INVALID_SOCKET;
   u_long             arg;
   fd_set             read_set;
   fd_set             write_set;
   int                len;

   temp = socket (AF_INET, SOCK_STREAM, 0);

   if (temp == INVALID_SOCKET)
     goto out0;

   arg = 1;
   if (ioctlsocket (temp, FIONBIO, &arg) == SOCKET_ERROR)
     goto out0;

   memset (&saddr, 0, sizeof (saddr));
   saddr.sin_family = AF_INET;
   saddr.sin_port = 0;
   saddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

   if (bind (temp, (struct sockaddr *)&saddr, sizeof (saddr)))
     goto out0;

   if (listen (temp, 1) == SOCKET_ERROR)
     goto out0;

   len = sizeof (saddr);
   if (getsockname (temp, (struct sockaddr *)&saddr, &len))
     goto out0;

   socket1 = socket (AF_INET, SOCK_STREAM, 0);

   if (socket1 == INVALID_SOCKET)
     goto out0;

   arg = 1;
   if (ioctlsocket (socket1, FIONBIO, &arg) == SOCKET_ERROR)
      goto out1;

   if ((connect (socket1, (struct sockaddr  *)&saddr, len) == SOCKET_ERROR) &&
       (WSAGetLastError () != WSAEWOULDBLOCK))
     goto out1;

   FD_ZERO (&read_set);
   FD_SET (temp, &read_set);

   tv.tv_sec = 0;
   tv.tv_usec = 0;

   if (select (0, &read_set, NULL, NULL, NULL) == SOCKET_ERROR)
     goto out1;

   if (!FD_ISSET (temp, &read_set))
     goto out1;

   socket2 = accept (temp, (struct sockaddr *) &saddr, &len);
   if (socket2 == INVALID_SOCKET)
     goto out1;

   FD_ZERO (&write_set);
   FD_SET (socket1, &write_set);

   tv.tv_sec = 0;
   tv.tv_usec = 0;

   if (select (0, NULL, &write_set, NULL, NULL) == SOCKET_ERROR)
     goto out2;

   if (!FD_ISSET (socket1, &write_set))
     goto out2;

   arg = 0;
   if (ioctlsocket (socket1, FIONBIO, &arg) == SOCKET_ERROR)
     goto out2;

   arg = 0;
   if (ioctlsocket (socket2, FIONBIO, &arg) == SOCKET_ERROR)
     goto out2;

   fds[0] = socket1;
   fds[1] = socket2;

   closesocket (temp);

   return 0;

 out2:
   closesocket (socket2);
 out1:
   closesocket (socket1);
 out0:
   closesocket (temp);

   fds[0] = INVALID_SOCKET;
   fds[1] = INVALID_SOCKET;

   return -1;
}


/*
 * Exec related functions
 *
 */

#ifdef _WIN32_WCE

int execvp (const char *file __UNUSED__, char *const argv[] __UNUSED__)
{
   return 1;
}

#endif /* _WIN32_WCE */
