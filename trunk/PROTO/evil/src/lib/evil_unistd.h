#ifndef __EVIL_UNISTD_H__
#define __EVIL_UNISTD_H__


/*
 * Process identifer related functions
 *
 */


/**
 * @brief Return the process identifier of the calling process.
 *
 * @return The process ID.
 *
 * Return the process identifier of the calling process. Until
 * the process terminates, the process identifier uniquely
 * identifies the process throughout the system.
 *
 * Conformity: Not appliclable.
 *
 * Supported OS: Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI pid_t getpid(void);


/*
 * Symbolic links and directory related functions
 *
 */

/**
 * @brief Create a shell link.
 *
 * @param oldpath The file name to be linked.
 * @param newpath The file name to create.
 * @return 0 on success, -1 otherwise.
 *
 * Create a shell link @p newpath to @p oldpath (@p newpath is the
 * name of the file created, @p oldpath is the string used in
 * creating the shell link).
 *
 * On success, this function returns 0. Otherwise, it returns -1 and
 * errno may be set to the following value:
 * - ENOMEM: Not enough memory.
 *
 * On Windows, the symbolic links do not exist. Nevertheless
 * shell links can be created. This function is named like the Unix
 * function for portability reasons.
 *
 * Conformity: None.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI int symlink(const char *oldpath, const char *newpath);

/**
 * @brief Read value of a shell link.
 *
 * @param path The file name to be linked.
 * @param buf The file name to create.
 * @param bufsiz The size of the buffer.
 * @return 0 on success, -1 otherwise.
 *
 * Place the content of the shell link @p path in the buffer
 * @p buf, which has size @p bufzsiz.
 *
 * On success, this function returns the count of characters
 * placed in the buffer. Otherwise, it returns -1 and errno may
 * be set to the following value:
 * - ENOMEM: Not enough memory.
 *
 * On Windows, the symbolic links do not exist. Nevertheless
 * shell links can be managed. This function is named like the Unix
 * function for portability reasons.
 *
 * Conformity: None.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI ssize_t readlink(const char *path, char *buf, size_t bufsiz);

/**
 * @brief Get the current directory.
 *
 * @param buffer Buffer to store the current directory.
 * @param size Size of the buffer.
 * @return The current directory.
 *
 * On Windows desktop, use the _getcwd function in MSVCRT.
 *
 * On Windows CE, get the current directory by extracting the path
 * from the executable that is running and put the result in @p buffer
 * of length @p size. If @p size is less or equal than 0, return NULL.
 * If the current absolute path would require a buffer longer than
 * @p size elements, NULL is returned. If @p buffer is NULL, a buffer
 * of length @p size is allocated and is returned. If the allocation
 * fails, NULL is returned. On success, @p buffer is returned and
 * contains the current directory. The last '\' is not included.
 * If @p buffer is NULL, the returned value must be freed if not NULL.
 *
 * Specially usefull on WinCE where the current directory functionality
 * is not supported.
 *
 * Conformity: Almost POSIX.1 (no errno set)
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP, WinCE.
 *
 * @ingroup Evil
 */
EAPI char *evil_getcwd(char *buffer, size_t size);


/*
 * Sockets and pipe related functions
 *
 */

/**
 * @brief Initiates the use of Windows sockets.
 *
 * @return 1 on success, 0 otherwise.
 *
 * Initiates the use of Windows sockets. If the function succeeds,
 * it returns 1, otherwise it returns 0.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI int evil_sockets_init(void);

/**
 * @brief Shutdown the Windows socket system.
 *
 * Shutdown the Windows socket system.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI void evil_sockets_shutdown(void);

/**
 * @brief Create a pair of sockets.
 *
 * @param fds A pointer that contains two sockets.
 *
 * Create a pair of sockets that can be use with select().
 * Hence, evil_sockets_init() must have been caled at least
 * once before. Contrary to Unix, that functions does not
 * create a pair of file descriptors.
 *
 * Conformity: Not applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI int evil_pipe(int *fds);


#endif /* __EVIL_UNISTD_H__ */
