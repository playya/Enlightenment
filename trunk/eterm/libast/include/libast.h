/*
 * Copyright (C) 1997-2003, Michael Jennings
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file libast.h
 * Global LibAST header file.
 *
 * This file contains all general-purpose macros, function
 * declarations, etc.  for LibAST.  It is also responsible for
 * including all required system headers and LibAST Object headers.
 *
 * @author Michael Jennings <mej@eterm.org>
 */

#ifndef _LIBAST_H_
#define _LIBAST_H_

/* This GNU goop has to go before the system headers */
#ifdef __GNUC__
# ifndef __USE_GNU
#  define __USE_GNU
# endif
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# ifndef _BSD_SOURCE
#  define _BSD_SOURCE
# endif
# ifndef _XOPEN_SOURCE
/* FIXME -- Do some systems still need this? */
/* #  define _XOPEN_SOURCE */
# endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
#endif
#ifdef WITH_DMALLOC
# include <dmalloc.h>
#elif defined(HAVE_MALLOC_H)
# include <malloc.h>
#endif

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#ifdef LIBAST_X11_SUPPORT
# include <X11/Xatom.h>
# include <X11/X.h>
# include <X11/Intrinsic.h>
# ifdef LIBAST_IMLIB2_SUPPORT
#  include <Imlib2.h>
# endif
#endif

#ifdef LIBAST_REGEXP_SUPPORT_PCRE
#  ifdef HAVE_PCRE_H
#    include <pcre.h>
#  elif HAVE_PCRE_PCRE_H
#    include <pcre/pcre.h>
#  endif
#elif defined(LIBAST_REGEXP_SUPPORT_POSIX) || defined(LIBAST_REGEXP_SUPPORT_BSD)
#  ifdef HAVE_REGEX_H
#    include <regex.h>
#  endif
#  ifdef LIBAST_REGEXP_SUPPORT_BSD
extern char *re_comp();
extern int re_exec();
#  endif
#endif

#include <libast/types.h>
#include <libast/obj.h>
#include <libast/regexp.h>
#include <libast/socket.h>
#include <libast/str.h>
#include <libast/tok.h>
#include <libast/url.h>

#include <libast/iterator_if.h>
#include <libast/list_if.h>
#include <libast/vector_if.h>

#include <libast/array.h>
#include <libast/linked_list.h>
#include <libast/dlinked_list.h>

#include <libast/avl_tree.h>

/******************************* GENERIC GOOP *********************************/
/**
 * Mark a variable as used.
 *
 * This macro is used to explicitly mark a variable as "used."  It
 * intentionally generates no real code, but suppresses gcc warnings
 * about unused variables and/or parameters.  That way, the programmer
 * can explicitly acknowledge that certain variables/parameters are
 * intentionally unused, making the warnings more effective by
 * eliminating false positives.
 *
 * @param x Any variable or parameter name.
 */
#define USE_VAR(x)   (void) x

/**
 * @def MIN(a, b)
 * Return the lesser of @a a or @a b.
 *
 * This macro compares its two parameters, @a a and @a b, and returns
 * the lesser of the two (the minimum).  When building under gcc, a
 * GNU-specific extension is used which prevents expressions used as
 * parameters from being evaluated multiple times.
 *
 * @param a Any expression that evaluates to a value.
 * @param b Any expression that evaluates to a value.
 * @return The lesser of the two values.
 */
/**
 * @def MAX(a, b)
 * Return the greater of @a a or @a b.
 *
 * This macro compares its two parameters, @a a and @a b, and returns
 * the greater of the two (the maximum).  When building under gcc, a
 * GNU-specific extension is used which prevents expressions used as
 * parameters from being evaluated multiple times.
 *
 * @param a Any expression that evaluates to a value.
 * @param b Any expression that evaluates to a value.
 * @return The greater of the two values.
 */
/**
 * @def LOWER_BOUND(current, other)
 * Force a lower bound on a variable.
 *
 * This macro checks the value of its first parameter, @a current, and
 * makes sure it is greater than or equal to the value of @a other.
 * If @a current is less than @a other, @a current is assigned the
 * value of @a other.  In essence, this establishes a "lower bound" on
 * @a current equal to the value of @a other.
 *
 * @param current The variable to check.
 * @param other   The value by which @a current will be bound.
 * @return The new value of @a current.
 */
/**
 * @def UPPER_BOUND(current, other)
 * Force an upper bound on a variable.
 *
 * This macro checks the value of its first parameter, @a current, and
 * makes sure it is less than or equal to the value of @a other.  If
 * @a current is greater than @a other, @a current is assigned the
 * value of @a other.  In essence, this establishes an "upper bound"
 * on @a current equal to the value of @a other.
 *
 * @param current The variable to check.
 * @param other   The value by which @a current will be bound.
 * @return The new value of @a current.
 */
/**
 * @def BOUND(val, min, max)
 * Force a variable to be within a given range.
 *
 * This macro checks the value of its first parameter, @a val, and
 * makes sure it is between @a min and @a max, inclusive.  If @a val
 * is above this range, it is assigned the value of @a max.  Likewise,
 * if @a val is below this range, it is assigned the value of @a min.
 * In essence, this establishes both an "upper bound" and a "lower
 * bound" on @a val.
 *
 * @param val The variable to check.
 * @param min The lowest value @a val may have.
 * @param max The highest value @a val may have.
 * @return The new value of @a val.
 */
#ifdef MIN
# undef MIN
#endif
#ifdef MAX
# undef MAX
#endif
#ifdef __GNUC__
# define MIN(a,b)                       __extension__ ({__typeof__(a) aa = (a); __typeof__(b) bb = (b); (aa < bb) ? (aa) : (bb);})
# define MAX(a,b)                       __extension__ ({__typeof__(a) aa = (a); __typeof__(b) bb = (b); (aa > bb) ? (aa) : (bb);})
# define LOWER_BOUND(current, other)    __extension__ ({__typeof__(other) o = (other); ((current) < o) ? ((current) = o) : (current);})
# define UPPER_BOUND(current, other)    __extension__ ({__typeof__(other) o = (other); ((current) > o) ? ((current) = o) : (current);})
# define BOUND(val, min, max)           __extension__ ({__typeof__(min) m1 = (min); __typeof__(max) m2 = (max); ((val) < m1) ? ((val) = m1) : (((val) > m2) ? ((val) = m2) : (val));})
#else
# define MIN(a,b)                       (((a) < (b)) ? (a) : (b))
# define MAX(a,b)                       (((a) > (b)) ? (a) : (b))
# define LOWER_BOUND(current, other)    (((current) < (other)) ? ((current) = (other)) : (current))
# define UPPER_BOUND(current, other)    (((current) > (other)) ? ((current) = (other)) : (current))
# define BOUND(val, min, max)           (((val) < (min)) ? ((val) = (min)) : (((val) > (max)) ? ((val) = (max)) : (val)))
#endif
/** @def AT_LEAST(current, other) Alias for LOWER_BOUND().  This macro is an alias for LOWER_BOUND(). */
#define AT_LEAST(current, other)        LOWER_BOUND(current, other)
/** @def MAX_IT(current, other) Alias for LOWER_BOUND().  This macro is an alias for LOWER_BOUND(). */
#define MAX_IT(current, other)          LOWER_BOUND(current, other)
/** @def AT_MOST(current, other) Alias for UPPER_BOUND().  This macro is an alias for UPPER_BOUND(). */
#define AT_MOST(current, other)         UPPER_BOUND(current, other)
/** @def MIN_IT(current, other) Alias for UPPER_BOUND().  This macro is an alias for UPPER_BOUND(). */
#define MIN_IT(current, other)          UPPER_BOUND(current, other)
/** @def CONTAIN(val, min, max) Alias for BOUND().  This macro is an alias for BOUND(). */
#define CONTAIN(val, min, max)          BOUND(val, min, max)
/**
 * Swaps two values.
 *
 * This macro swaps the values of its first two parameters using the
 * third as temporary storage.
 *
 * @param one The first variable.
 * @param two The second variable.
 * @param tmp A temporary holding spot used during swapping.
 */
#define SWAP_IT(one, two, tmp)          do {(tmp) = (one); (one) = (two); (two) = (tmp);} while (0)

/**
 * @def SWAP(a, b)
 * Swaps two values.
 *
 * This macro performs the same task as the SWAP_IT() macro, except
 * that no temporary variable is required.  Instead, a temporary
 * variable is created by the macro itself.  Under gcc, the
 * __typeof__() extension is used to create a temporary variable of
 * the same type as @a a.  Under other compilers, a void pointer is
 * used.
 *
 * @param a The first variable.
 * @param b The second variable.
 */
#ifdef __GNUC__
# define SWAP(a, b)  (void) __extension__ ({__typeof__(a) tmp = (a); (a) = (b); (b) = tmp;})
#else
# define SWAP(a, b)  do {void *tmp = ((void *)(a)); (a) = (b); (b) = tmp;} while (0)
#endif
/**
 * Swaps two values.
 *
 * This macro swaps the values of @a a and @a b using the now-infamous
 * chained XOR trick.
 *
 * @attention ONLY use this with like variables, and only those which
 * can safely be cast to and from a long.  If you're unsure of whether
 * or not it would be safe, use SWAP() or SWAP_IT() instead!
 *
 * @param a The first variable.
 * @param b The second variable.
 */
#define BINSWAP(a, b)  (((long) (a)) ^= ((long) (b)) ^= ((long) (a)) ^= ((long) (b)))

/**
 * Make sure a char pointer is non-NULL before printing it.
 *
 * This is a convenience macro primarily targetted at systems like
 * Solaris where doing a printf() on a NULL char pointer using %s
 * results in a segmentation fault rather than helpful message.  This
 * macro should be used in any place where a string is printed which
 * could potentially be NULL.
 *
 * @param x A string (char *).
 * @return @a x, or if @a x is NULL, the string "<@a x null>"
 */
#define NONULL(x) (((char *) (x)) ? ((char *) (x)) : ((char *) ("<" #x " null>")))

/****************************** DEBUGGING GOOP ********************************/
#ifndef LIBAST_DEBUG_FD
/**
 * Where to send debugging output.
 *
 * This defines where debugging output should be sent.  Should be
 * either stdout or stderr.
 *
 * @ingroup DOXGRP_DEBUG
 */
# define LIBAST_DEBUG_FD  (stderr)
#endif
#ifndef DEBUG
/**
 * Maximum compile-time debugging level.
 *
 * LibAST supports debugging levels, allowing for progressively more
 * verbosity of debugging output as the level gets higher.  This
 * defines the compile-time maximum; support for higher debugging
 * levels than this will not even be compiled in, so use care when
 * setting this.
 *
 * @ingroup DOXGRP_DEBUG
 */
# define DEBUG 0
#endif

/** UNDOCUMENTED. */
#define DEBUG_LEVEL       (libast_debug_level)
/** UNDOCUMENTED. */
#define DEBUG_FLAGS       (libast_debug_flags)

/** Does nothing.  This macro is a nop (no operation).  It does nothing. */
#define NOP ((void)0)

/**
 * A fix-me NOP.
 *
 * This is the same as NOP(), but is used to mark something needing to
 * be fixed.
 */
#define FIXME_NOP(x)
/**
 * Mark a block of code needing fixing.
 *
 * This marks a block of code needing fixing and removes it.
 */
#define FIXME_BLOCK 0

/**
 * Mark unused blocks of code.
 *
 * This marks a block of code as unused and removes it.
 */
#define UNUSED_BLOCK 0

/**
 * @def __DEBUG()
 * Format and print debugging output.
 *
 * This macro formats and prints debugging output by prepending a
 * timestamp, the filename, the line number, and (if available) the
 * function name.
 *
 * This is an internal macro and should not be used directly.
 * @ingroup DOXGRP_DEBUG
 */
#if defined(__FILE__) && defined(__LINE__)
# ifdef __GNUC__
#  define __DEBUG()  fprintf(LIBAST_DEBUG_FD, "[%lu] %12s | %4d: %s(): ", (unsigned long) time(NULL), __FILE__, __LINE__, __FUNCTION__)
# else
#  define __DEBUG()  fprintf(LIBAST_DEBUG_FD, "[%lu] %12s | %4d: ", (unsigned long) time(NULL), __FILE__, __LINE__)
# endif
#else
# define __DEBUG()   NOP
#endif

/**
 * Assert reaching a line of code.
 *
 * This macro is simply a quick-and-dirty way of printing out a unique
 * message which proves that a particular portion of code was reached
 * and executed properly.
 *
 * @ingroup DOXGRP_DEBUG
 */
#define MOO()  do {__DEBUG(); libast_dprintf("Moo.\n");} while (0)

/**
 * @def ASSERT(x)
 * Asserts that a condition is true.
 *
 * This macro evaluates an expression, @a x, and takes action if the
 * expression evaluates to false (0).  It works similarly to the libc
 * function assert(), with the exception that it will not call abort()
 * if the assertion fails.  Instead, it will either issue a fatal
 * error (generally resulting in a backtrace) if debugging is active,
 * or print a warning if it is not.  In either event, the warning/error
 * message will contain the filename, line number, and (if available)
 * function name where the error occured.
 *
 * If only a warning is generated, the function will return
 * immediately.
 *
 * @param x Any valid boolean expression.
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def ASSERT_RVAL(x, val)
 * Asserts that a condition is true, and provides a return value in
 * case it isn't.
 *
 * This macro is identical to ASSERT(), except that it returns a
 * value, @a val, instead of returning void.
 *
 * @param x   Any valid boolean expression.
 * @param val The return value to use if @a x evaluates to false.
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def ASSERT_NOTREACHED()
 * Asserts that a particular piece of code is not reached.
 *
 * This macro is used in sections of code that should never be
 * reached.  Its actions are similar to those of ASSERT(), but instead
 * of evaluating an expression, it always evaluates to false.
 *
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def ASSERT_NOTREACHED_RVAL(val)
 * Asserts that a particular piece of code is not reached, and
 * provides a return value in case it is.
 *
 * This macro is identical to ASSERT_NOTREACHED(), except that it
 * returns a value, @a val, instead of returning void.
 *
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def ABORT()
 * Throw a fatal exception.
 *
 * This macro is a replacement for the libc abort() function.  This
 * version provides file/line/function information in the fatal error
 * message.
 *
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def REQUIRE(x)
 * Return if an expression is false.
 *
 * This macro is similar to ASSERT(), except that @a x evaluating to
 * false is not necessarily an error.  Normally, this macro simply
 * causes the function to return.  However, if debugging is active, a
 * message is printed noting the expression @a x and the location of
 * the failure.  This macro is often used to test preconditions, such
 * as making sure pointers are non-NULL before using them.
 *
 * @param x Any valid boolean expression.
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def REQUIRE_RVAL(x, v)
 * Return @a v if an expression is false.
 *
 * This macro is identical to REQUIRE(), except that a return value
 * for the function is supplied.
 *
 * @param x Any valid boolean expression
 * @param v The function return value to use if @a x evaluates to
 * false.
 * @ingroup DOXGRP_DEBUG
 */
#if DEBUG >= 1
# if defined(__FILE__) && defined(__LINE__)
#  ifdef __GNUC__
#   define ASSERT(x)  do {if (!(x)) {if (DEBUG_LEVEL>=1) {fatal_error("ASSERT failed in %s() at %s:%d:  %s\n", __FUNCTION__, __FILE__, __LINE__, #x);} \
                                                    else {print_warning("ASSERT failed in %s() at %s:%d:  %s\n", __FUNCTION__, __FILE__, __LINE__, #x); return;}}} while (0)
#   define ASSERT_RVAL(x, val)  do {if (!(x)) {if (DEBUG_LEVEL>=1) {fatal_error("ASSERT failed in %s() at %s:%d:  %s\n", __FUNCTION__, __FILE__, __LINE__, #x);} \
                                                              else {print_warning("ASSERT failed in %s() at %s:%d:  %s\n", __FUNCTION__, __FILE__, __LINE__, #x);} \
                                               return (val);}} while (0)
#   define ASSERT_NOTREACHED()  do {if (DEBUG_LEVEL>=1) {fatal_error("ASSERT failed in %s() at %s:%d:  This code should not be reached.\n", __FUNCTION__, __FILE__, __LINE__);} \
                                                   else {print_warning("ASSERT failed in %s() at %s:%d:  This code should not be reached.\n", __FUNCTION__, __FILE__, __LINE__);} \
                                    } while (0)
#   define ASSERT_NOTREACHED_RVAL(val)  do {if (DEBUG_LEVEL>=1) {fatal_error("ASSERT failed in %s() at %s:%d:  This code should not be reached.\n", __FUNCTION__, __FILE__, __LINE__);} \
                                                           else {print_warning("ASSERT failed in %s() at %s:%d:  This code should not be reached.\n", __FUNCTION__, __FILE__, __LINE__);} \
                                            return (val);} while (0)
#   define ABORT() fatal_error("Aborting in %s() at %s:%d.\n", __FUNCTION__, __FILE__, __LINE__)
#  else
#   define ASSERT(x)  do {if (!(x)) {if (DEBUG_LEVEL>=1) {fatal_error("ASSERT failed at %s:%d:  %s\n", __FILE__, __LINE__, #x);} \
                                                    else {print_warning("ASSERT failed at %s:%d:  %s\n", __FILE__, __LINE__, #x); return;}}} while (0)
#   define ASSERT_RVAL(x, val)  do {if (!(x)) {if (DEBUG_LEVEL>=1) {fatal_error("ASSERT failed at %s:%d:  %s\n", __FILE__, __LINE__, #x);} \
                                                              else {print_warning("ASSERT failed at %s:%d:  %s\n", __FILE__, __LINE__, #x);} \
                                               return (val);}} while (0)
#   define ASSERT_NOTREACHED()  do {if (DEBUG_LEVEL>=1) {fatal_error("ASSERT failed at %s:%d:  This code should not be reached.\n", __FILE__, __LINE__);} \
                                                   else {print_warning("ASSERT failed at %s:%d:  This code should not be reached.\n", __FILE__, __LINE__);} \
                                    } while (0)
#   define ASSERT_NOTREACHED_RVAL(val)  do {if (DEBUG_LEVEL>=1) {fatal_error("ASSERT failed at %s:%d:  This code should not be reached.\n", __FILE__, __LINE__);} \
                                                           else {print_warning("ASSERT failed at %s:%d:  This code should not be reached.\n", __FILE__, __LINE__);} \
                                            return (val);} while (0)
#   define ABORT() fatal_error("Aborting at %s:%d.\n", __FILE__, __LINE__)
#  endif
# else
#  define ASSERT(x)  do {if (!(x)) {if (DEBUG_LEVEL>=1) {fatal_error("ASSERT failed:  %s\n", #x);} \
                                                   else {print_warning("ASSERT failed:  %s\n", #x); return;}}} while (0)
#  define ASSERT_RVAL(x, val)  do {if (!(x)) {if (DEBUG_LEVEL>=1) {fatal_error("ASSERT failed:  %s\n", #x);} \
                                                             else {print_warning("ASSERT failed:  %s\n", #x);} return (val);}} while (0)
#  define ASSERT_NOTREACHED()           return
#  define ASSERT_NOTREACHED_RVAL(x)     return (x)
#  define ABORT()                       fatal_error("Aborting.\n")
# endif
# define REQUIRE(x)                     do {if (!(x)) {if (DEBUG_LEVEL>=1) {__DEBUG(); libast_dprintf("REQUIRE failed:  %s\n", #x);} return;}} while (0)
# define REQUIRE_RVAL(x, v)             do {if (!(x)) {if (DEBUG_LEVEL>=1) {__DEBUG(); libast_dprintf("REQUIRE failed:  %s\n", #x);} return (v);}} while (0)
#else
# define ASSERT(x)                      NOP
# define ASSERT_RVAL(x, val)            NOP
# define ASSERT_NOTREACHED()            return
# define ASSERT_NOTREACHED_RVAL(val)    return (val)
# define ABORT()                        fatal_error("Aborting.\n")
# define REQUIRE(x)                     do {if (!(x)) return;} while (0)
# define REQUIRE_RVAL(x, v)             do {if (!(x)) return (v);} while (0)
#endif

/**
 * @def DPRINTF(x)
 * Print debugging output.
 *
 * This macro can be used for unconditional debugging output.  If any
 * level of debugging support has been compiled in, this macro will
 * print a debugging message.
 *
 * This macro will almost never be used directly; instead, use the
 * D_*() macros.
 *
 * @attention Calls to this and other debugging output macros
 * MUST be double-parenthesized, like so:  DPRINTF((...));
 *
 * @param x A parenthesized argument list suitable for a printf-style
 *          function.
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def DPRINTF1(x)
 * Print level 1 debugging output.
 *
 * This macro is identical to DPRINTF(), except that the message will
 * only be printed if the debug level is 1 or higher.
 *
 * @param x A parenthesized argument list suitable for a printf-style
 *          function.
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def DPRINTF2(x)
 * Print level 2 debugging output.
 *
 * This macro is identical to DPRINTF(), except that the message will
 * only be printed if the debug level is 2 or higher.
 *
 * @param x A parenthesized argument list suitable for a printf-style
 *          function.
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def DPRINTF3(x)
 * Print level 3 debugging output.
 *
 * This macro is identical to DPRINTF(), except that the message will
 * only be printed if the debug level is 3 or higher.
 *
 * @param x A parenthesized argument list suitable for a printf-style
 *          function.
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def DPRINTF4(x)
 * Print level 4 debugging output.
 *
 * This macro is identical to DPRINTF(), except that the message will
 * only be printed if the debug level is 4 or higher.
 *
 * @param x A parenthesized argument list suitable for a printf-style
 *          function.
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def DPRINTF5(x)
 * Print level 5 debugging output.
 *
 * This macro is identical to DPRINTF(), except that the message will
 * only be printed if the debug level is 5 or higher.
 *
 * @param x A parenthesized argument list suitable for a printf-style
 *          function.
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def DPRINTF6(x)
 * Print level 6 debugging output.
 *
 * This macro is identical to DPRINTF(), except that the message will
 * only be printed if the debug level is 6 or higher.
 *
 * @param x A parenthesized argument list suitable for a printf-style
 *          function.
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def DPRINTF7(x)
 * Print level 7 debugging output.
 *
 * This macro is identical to DPRINTF(), except that the message will
 * only be printed if the debug level is 7 or higher.
 *
 * @param x A parenthesized argument list suitable for a printf-style
 *          function.
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def DPRINTF8(x)
 * Print level 8 debugging output.
 *
 * This macro is identical to DPRINTF(), except that the message will
 * only be printed if the debug level is 8 or higher.
 *
 * @param x A parenthesized argument list suitable for a printf-style
 *          function.
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
/**
 * @def DPRINTF9(x)
 * Print level 9 debugging output.
 *
 * This macro is identical to DPRINTF(), except that the message will
 * only be printed if the debug level is 9 or higher.
 *
 * @param x A parenthesized argument list suitable for a printf-style
 *          function.
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
#if DEBUG >= 1
# ifndef DPRINTF
#  define DPRINTF(x)           do { __DEBUG(); libast_dprintf x; } while (0)
# endif
# define DPRINTF1(x)           do { if (DEBUG_LEVEL >= 1) {__DEBUG(); libast_dprintf x;} } while (0)
# define DPRINTF2(x)           do { if (DEBUG_LEVEL >= 2) {__DEBUG(); libast_dprintf x;} } while (0)
# define DPRINTF3(x)           do { if (DEBUG_LEVEL >= 3) {__DEBUG(); libast_dprintf x;} } while (0)
# define DPRINTF4(x)           do { if (DEBUG_LEVEL >= 4) {__DEBUG(); libast_dprintf x;} } while (0)
# define DPRINTF5(x)           do { if (DEBUG_LEVEL >= 5) {__DEBUG(); libast_dprintf x;} } while (0)
# define DPRINTF6(x)           do { if (DEBUG_LEVEL >= 6) {__DEBUG(); libast_dprintf x;} } while (0)
# define DPRINTF7(x)           do { if (DEBUG_LEVEL >= 7) {__DEBUG(); libast_dprintf x;} } while (0)
# define DPRINTF8(x)           do { if (DEBUG_LEVEL >= 8) {__DEBUG(); libast_dprintf x;} } while (0)
# define DPRINTF9(x)           do { if (DEBUG_LEVEL >= 9) {__DEBUG(); libast_dprintf x;} } while (0)
#else
# ifndef DPRINTF
#  define DPRINTF(x)           NOP
# endif
# define DPRINTF1(x)           NOP
# define DPRINTF2(x)           NOP
# define DPRINTF3(x)           NOP
# define DPRINTF4(x)           NOP
# define DPRINTF5(x)           NOP
# define DPRINTF6(x)           NOP
# define DPRINTF7(x)           NOP
# define DPRINTF8(x)           NOP
# define DPRINTF9(x)           NOP
#endif

/**
 * Debugging output you (almost) never want.
 *
 * This macro is used for mapping debugging output you almost never
 * want to see.  Map D_*() macros to this for overly verbose or
 * problematic debugging information, then manually redefine this as
 * needed.
 *
 * @param x A parenthesized argument list suitable for a printf-style
 *          function.
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
#define D_NEVER(x)             NOP

/** Set options debugging to level 1.  @see DOXGRP_DEBUG */
#define DEBUG_OPTIONS          1
/**
 * Option debugging macro.
 *
 * This macro is used for debugging output related to the options
 * subsystem.  It maps to DPRINTF1() so that options-related debugging
 * output will occur at debug level 1 and higher.
 *
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
#define D_OPTIONS(x)           DPRINTF1(x)
/** Set object system debugging to level 2.  @see DOXGRP_DEBUG */
#define DEBUG_OBJ              2
/**
 * Object debugging macro.
 *
 * This macro is used for debugging output related to the object
 * subsystem.  It maps to DPRINTF2() so that object-related debugging
 * output will occur at debug level 2 and higher.
 *
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
#define D_OBJ(x)               DPRINTF2(x)
/** Set config file parser debugging to level 3.  @see DOXGRP_DEBUG */
#define DEBUG_CONF             3
/**
 * Config file parser debugging macro.
 *
 * This macro is used for debugging output related to the config file
 * parser.  It maps to DPRINTF3() so that config-related debugging
 * output will occur at debug level 3 and higher.
 *
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
#define D_CONF(x)              DPRINTF3(x)
/** Set memory allocation debugging to level 5.  @see DOXGRP_DEBUG */
#define DEBUG_MEM              5
/**
 * Memory allocation debugging macro.
 *
 * This macro is used for debugging output related to the memory
 * allocation subsystem.  It maps to DPRINTF1() so that mem-related
 * debugging output will occur at debug level 5 and higher.
 *
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
#define D_MEM(x)               DPRINTF5(x)
/** Set strings module debugging to level 9999.  @see DOXGRP_DEBUG */
#define DEBUG_STRINGS          9999
/**
 * String routine debugging macro.
 *
 * This macro is used for debugging output related to the string
 * manipulation subsystem.  It maps to D_NEVER() so that
 * string-related debugging output can only be activated manually.
 *
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
#define D_STRINGS(x)           D_NEVER(x)
/** Set lexer/parser debugging to level 9999.  @see DOXGRP_DEBUG */
#define DEBUG_PARSE            9999
/**
 * Lexer/parser debugging macro.
 *
 * This macro is used for debugging output related to the lexer/parser
 * portion of the config parser.  It maps to D_NEVER() so that
 * parser-related debugging output can only be activated manually.
 *
 * @see DOXGRP_DEBUG
 * @ingroup DOXGRP_DEBUG
 */
#define D_PARSE(x)             D_NEVER(x)



/********************************* MEM GOOP ***********************************/
/**
 * @def MALLOC(sz)
 * Allocate @a sz bytes of memory.
 * 
 * This macro is a replacement for the libc function malloc().  It
 * allocates the specified number of bytes of memory on the heap and
 * returns a pointer to that memory location.  This macro calls libc's
 * malloc() if memory debugging is off, and libast_malloc() if it's
 * on.
 *
 * @param sz The size in bytes of the block of memory to allocate.
 * @return A pointer to the allocated memory.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def CALLOC(type, n)
 * Allocate enough memory for @a n objects of type @a type.
 * 
 * This macro is a replacement for the libc function calloc().  It
 * allocates a block of memory on the heap large enough to hold @a n
 * objects of type @a type (e.g., a @a type array of size @a n).  The
 * memory area is zeroed out prior to the pointer to it being
 * returned.  This macro calls libc's calloc() if memory debugging is
 * off and libast_calloc() if it's on. 
 *
 * @param type The type of object to be allocated (e.g., int).
 * @param n    The number of objects to be allocated.
 * @return A pointer to the allocated memory.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def REALLOC(mem, sz)
 * Resize the memory block pointed to by @a mem to @a sz bytes.
 *
 * This macro is a replacement for the libc function realloc().  It
 * changes the size of a chunk of memory previously allocated by
 * malloc() or calloc() (or, by extension, the MALLOC()/CALLOC()
 * macros) and returns a pointer to the (possibly moved) memory area.
 * This macro calls libc's realloc() if memory debugging is off and
 * libast_realloc() if it's on.
 *
 * @param mem The old pointer whose size will be changed.
 * @param sz  The new size, in bytes, to be allocated.
 * @return The new pointer value, which may or may not differ from the
 *         old value.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def FREE(ptr)
 * Free a previously-allocated memory block.
 *
 * This macro is a replacement for the libc function free().  It
 * returns the previously-allocated memory block pointed to by @a ptr
 * to the heap.  This macro calls libc's free() if memory debugging is
 * off and libast_free() if it's on.  The @a ptr parameter is assigned
 * the value of NULL after it has been freed.
 *
 * @param ptr The pointer to be freed.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def STRDUP(s)
 * Duplicate a string pointer and return a pointer to the new copy.
 *
 * This macro is a replacement for the libc function strdup().  It
 * allocates a section of memory large enough to hold the string @a s
 * (including the trailing NUL character), copies the contents of @a s
 * into the new buffer, and returns a pointer to the new copy.  This
 * macro calls libc's strdup() of memory debugging is off and
 * libast_strdup() if it's on.
 *
 * @param s The string to duplicate.
 * @return A pointer to the newly-created copy.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def MALLOC_DUMP()
 * Dumps a listing of all allocated pointers along with their sizes
 * and contents in both hex and ASCII.
 *
 * This macro is used to view the status of memory allocated via the
 * LibAST memory management system.  First the pointers used to track
 * allocated memory are dumped (that's what pointer #0 is); then, each
 * allocated pointer is dumped along with its size and contents, the
 * latter being displayed both in hexadecimal form and ASCII form.
 * Non-printable characters are replaced by dots ('.').  You can see
 * a sample of the output in the
 * @link mem_example.c memory management system example @endlink.
 *
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def X_CREATE_PIXMAP(d, win, w, h, depth)
 * Create an X pixmap.
 *
 * This macro is a replacement for the Xlib function XCreatePixmap().
 * It creates a pixmap of the specified size and returns an X resource
 * ID for it.  This macro calls Xlib's XCreatePixmap() if memory
 * debugging is off and libast_x_create_pixmap() if it's on.
 *
 * @param d     The X display connection.
 * @param win   The X drawable on whose display the pixmap will be
 *              created.
 * @param w     The width in pixels of the pixmap.
 * @param h     The height in pixels of the pixmap.
 * @param depth The color depth for the pixmap.
 * @return The Pixmap ID for the new pixmap.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def X_FREE_PIXMAP(d, p)
 * Free the specified X pixmap.
 *
 * This macro is a replacement for the Xlib function XFreePixmap().
 * It frees the specified pixmap.  This macro calls Xlib's
 * XFreePixmap() if memory debugging is off and libast_x_free_pixmap()
 * if it's on.
 *
 * @param d The X display connection.
 * @param p The Pixmap to be freed.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def IMLIB_REGISTER_PIXMAP(p)
 * Register a pixmap generated by Imlib2 so LibAST can track it.
 *
 * Unfortunately, there is no easy way to wrap all the different ways
 * Imlib2 could conceivably create an image.  So instead, simply use
 * this macro to register the pixmaps Imlib2 creates.  Then LibAST
 * will be able to track them.  This macro calls
 * libast_imlib_register_pixmap() if memory debugging is on and if
 * Imlib2 support has been enabled.  Otherwise, it's a NOP().
 *
 * @param p The Pixmap Imlib2 created.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def IMLIB_FREE_PIXMAP(p)
 * Free a pixmap (and its mask) generated by Imlib2.
 *
 * Once an Imlib2-generated pixmap has been registered, you should
 * use this macro to free it.  It calls libast_imlib_free_pixmap() if
 * Imlib2 support has been enabled.  Otherwise, it's a NOP().
 *
 * @param p The Imlib2-generated Pixmap to be freed.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def PIXMAP_DUMP()
 * Dump a listing of allocated pixmaps.
 *
 * This macro is analogous to the MALLOC_DUMP() macro; rather than
 * dumping a list of pointers, however, it dumps a list of allocated
 * pixmaps.  Like MALLOC_DUMP(), this macro is a NOP() if memory
 * debugging support has not been compiled into LibAST.
 *
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def X_CREATE_GC(d, win, f, gcv)
 * Create an X graphics context.
 *
 * This macro is a replacement for the Xlib function XCreateGC().  It
 * creates a graphics context (GC) object and returns its X resource
 * ID.  This macro calls Xlib's XCreateGC() if memory debugging is
 * off and libast_x_create_gc() if it's on.
 *
 * @param d   The X display connection.
 * @param win The X drawable on whose screen the GC will be created.
 * @param f   The GC flags noting which members of @a gcv have set
 *            values.
 * @param gcv The GCValues structure defining properties of the GC.
 * @return The ID of the new GC.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def X_FREE_GC(d, gc)
 * Free an X graphics context.
 *
 * This macro is a replacement for the Xlib function XFreeGC().  It
 * frees a previously allocated graphics context (GC) object.  This
 * macro calls Xlib's XFreeGC() if memory debugging is off and
 * libast_x_free_gc() if it's on.
 *
 * @param d  The X display connection.
 * @param gc The graphics context object to free.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def GC_DUMP()
 * Dump a list of allocated graphics context objects.
 *
 * This macro is analogous to the MALLOC_DUMP() macro; rather than
 * dumping a list of pointers, however, it dumps a list of allocated
 * GC's.  Like MALLOC_DUMP(), this macro is a NOP() if memory
 * debugging support has not been compiled into LibAST.
 *
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def MALLOC_MOD
 * MALLOC() call count interval.
 *
 * LibAST has the ability to count calls to MALLOC(); this defines the
 * interval for reporting the call count.  The default is 25, meaning
 * that LibAST will print the current count every 25 calls.  Note that
 * MALLOC_CALL_DEBUG must be defined when compiling LibAST, in
 * addition to memory debugging, for this feature to work.
 *
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def REALLOC_MOD
 * REALLOC() call count interval.
 *
 * LibAST has the ability to count calls to REALLOC(); this defines
 * the interval for reporting the call count.  The default is 25,
 * meaning that LibAST will print the current count every 25 calls.
 * Note that MALLOC_CALL_DEBUG must be defined when compiling LibAST,
 * in addition to memory debugging, for this feature to work.
 *
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def CALLOC_MOD
 * CALLOC() call count interval.
 *
 * LibAST has the ability to count calls to CALLOC(); this defines the
 * interval for reporting the call count.  The default is 25, meaning
 * that LibAST will print the current count every 25 calls.  Note that
 * MALLOC_CALL_DEBUG must be defined when compiling LibAST, in
 * addition to memory debugging, for this feature to work.
 *
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
/**
 * @def FREE_MOD
 * FREE() call count interval.
 *
 * LibAST has the ability to count calls to FREE(); this defines the
 * interval for reporting the call count.  The default is 25, meaning
 * that LibAST will print the current count every 25 calls.  Note that
 * MALLOC_CALL_DEBUG must be defined when compiling LibAST, in
 * addition to memory debugging, for this feature to work.
 *
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
#if (DEBUG >= DEBUG_MEM)
# define MALLOC(sz)                             libast_malloc(__FILE__, __LINE__, (sz))
# define CALLOC(type,n)                         libast_calloc(__FILE__, __LINE__, (n), (sizeof(type)))
# define REALLOC(mem,sz)                        libast_realloc(#mem, __FILE__, __LINE__, (mem), (sz))
# define FREE(ptr)                              do { libast_free(#ptr, __FILE__, __LINE__, (ptr)); (ptr) = NULL; } while (0)
# define STRDUP(s)                              libast_strdup(#s, __FILE__, __LINE__, (s))
# define MALLOC_DUMP()                          libast_dump_mem_tables()
# define X_CREATE_PIXMAP(d, win, w, h, depth)   libast_x_create_pixmap(__FILE__, __LINE__, (d), (win), (w), (h), (depth))
# define X_FREE_PIXMAP(d, p)                    libast_x_free_pixmap(#p, __FILE__, __LINE__, (d), (p))
# ifdef HAVE_LIBIMLIB2
#  define IMLIB_REGISTER_PIXMAP(p)              libast_imlib_register_pixmap(#p, __FILE__, __LINE__, (p))
#  define IMLIB_FREE_PIXMAP(p)                  libast_imlib_free_pixmap(#p, __FILE__, __LINE__, (p))
# else
#  define IMLIB_REGISTER_PIXMAP(p)              NOP
#  define IMLIB_FREE_PIXMAP(p)                  NOP
# endif
# define PIXMAP_DUMP()                          libast_dump_pixmap_tables()
# define X_CREATE_GC(d, win, f, gcv)            libast_x_create_gc(__FILE__, __LINE__, (d), (win), (f), (gcv))
# define X_FREE_GC(d, gc)                       libast_x_free_gc(#gc, __FILE__, __LINE__, (d), (gc))
# define GC_DUMP()                              libast_dump_gc_tables()
# define MALLOC_MOD 25
# define REALLOC_MOD 25
# define CALLOC_MOD 25
# define FREE_MOD 25
#else
# define MALLOC(sz)                             malloc(sz)
# define CALLOC(type,n)                         calloc((n),(sizeof(type)))
# define REALLOC(mem,sz)                        ((sz) ? ((mem) ? (realloc((mem), (sz))) : (malloc(sz))) : ((mem) ? (free(mem), NULL) : (NULL)))
# define FREE(ptr)                              do { free(ptr); (ptr) = NULL; } while (0)
# define STRDUP(s)                              strdup(s)
# define MALLOC_DUMP()                          NOP
# define X_CREATE_PIXMAP(d, win, w, h, depth)   XCreatePixmap((d), (win), (w), (h), (depth))
# define X_FREE_PIXMAP(d, p)                    XFreePixmap((d), (p))
# ifdef HAVE_LIBIMLIB2
#  define IMLIB_REGISTER_PIXMAP(p)              NOP
#  define IMLIB_FREE_PIXMAP(p)                  imlib_free_pixmap_and_mask(p)
# else
#  define IMLIB_REGISTER_PIXMAP(p)              NOP
#  define IMLIB_FREE_PIXMAP(p)                  NOP
# endif
# define PIXMAP_DUMP()                          NOP
# define X_CREATE_GC(d, win, f, gcv)            XCreateGC((d), (win), (f), (gcv))
# define X_FREE_GC(d, gc)                       XFreeGC((d), (gc))
# define GC_DUMP()                              NOP
#endif

/* Fast memset() macro contributed by vendu */
#if (SIZEOF_LONG == 8)
/** UNDOCUMENTED */
# define MEMSET_LONG() (l |= l<<32)
#else
/** UNDOCUMENTED */
# define MEMSET_LONG() NOP
#endif

/**
 * @def MEMSET(s, c, count)
 * Initialize a memory region to a particular value.
 *
 * This macro is a replacement for the libc function memset().  It
 * initializes the memory region pointed to by @a s to the value
 * specified by @a c.  The size of the memory region is specified by
 * @a count.  Note that @a c must be a byte (char) value.
 *
 * This macro has been optimized to set as many bits simultaneously as
 * the architecture can handle, so it should offer superior
 * performance to libc's memset() function.
 *
 * @param s     A pointer to the memory region to initialize.
 * @param c     The value to which all bytes in the block will be
 *              set.
 * @param count The size, in bytes, of the memory region.
 * @see DOXGRP_MEM
 * @ingroup DOXGRP_MEM
 */
#define MEMSET(s, c, count) do { \
    char *end = (char *)(s) + (count); \
    long l; \
    long *l_dest = (long *)(s); \
    char *c_dest; \
 \
    /* areas of less than 4 * sizeof(long) are set in 1-byte chunks. */ \
    if (((unsigned long) count) >= 4 * sizeof(long)) { \
        /* fill l with c. */ \
        l = (c) | (c)<<8; \
        l |= l<<16; \
        MEMSET_LONG(); \
 \
        /* fill in 1-byte chunks until boundary of long is reached. */ \
        if ((unsigned long)l_dest & (unsigned long)(sizeof(long) -1)) { \
            c_dest = (char *)l_dest; \
            while ((unsigned long)c_dest & (unsigned long)(sizeof(long) -1)) { \
                *(c_dest++) = (c); \
            } \
            l_dest = (long *)c_dest; \
        } \
 \
        /* fill in long-size chunks as long as possible. */ \
        while (((unsigned long) (end - (char *)l_dest)) >= sizeof(long)) { \
            *(l_dest++) = l; \
        } \
    } \
 \
    /* fill the tail in 1-byte chunks. */ \
    if ((char *)l_dest < end) { \
        c_dest = (char *)l_dest; \
        *(c_dest++) = (c); \
        while (c_dest < end) { \
            *(c_dest++) = (c); \
        } \
    } \
  } while (0)



/******************************* STRINGS GOOP *********************************/
/**
 * Returns the length of a literal string.
 *
 * This macro is like libc's strlen() function, except that it
 * requires the string parameter be a literal rather than a variable.
 * This makes calculating the string length for a literal easy without
 * incurring the speed penalty of a call to strlen().
 *
 * @param x The literal string (i.e., a fixed string in quotes, like
 *          "this.").
 * @return The length of the string.
 * @see DOXGRP_STRINGS
 * @ingroup DOXGRP_STRINGS
 */
#define CONST_STRLEN(x)            (sizeof(x) - 1)
/**
 * Compares the beginning of a string with a literal.
 *
 * This macro, like the libc str*cmp() functions, returns an integer
 * less than, equal to, or greater than zero depending on if the
 * initial part of string @a s is found to be less than, to match, or
 * to be greater than the literal string.  Generally, this is used as
 * a boolean value (as !BEG_STRCASECMP()) to determine whether or not
 * @a s starts with @a constr or not.  Note that case is ignored, as
 * the name implies.
 *
 * @param s      The string variable to compare to.
 * @param constr A literal string representing what should be the
 *               beginning of @a s.
 * @return See above.
 * @see DOXGRP_STRINGS
 * @ingroup DOXGRP_STRINGS
 */
#define BEG_STRCASECMP(s, constr)  (strncasecmp(s, constr, CONST_STRLEN(constr)))



/******************************** CONF GOOP ***********************************/
/**
 * @def PATH_MAX
 * The maximum length of a path specifier.
 *
 * LibAST requires PATH_MAX to be properly defined.  Unfortunately,
 * some UNIX versions (namely HP-UX) define it incorrectly.  Most UNIX
 * versions support a PATH_MAX of 1024, but all must support at least
 * 255.  So if PATH_MAX is defined to be less than 255 (like HP-UX and
 * its absolutely ludicrous value of 14), LibAST forceably redefines
 * it to be 255.
 * @see DOXGRP_CONF
 * @ingroup DOXGRP_CONF
 */
#if defined(PATH_MAX) && (PATH_MAX < 255)
#  undef PATH_MAX
#endif
#ifndef PATH_MAX
#  define PATH_MAX 255
#endif

/**
 * Maximum length of a line in a config file.
 *
 * At no time during parsing can any line in a config file exceed this
 * length (20 kB by default).
 * @see DOXGRP_CONF
 * @ingroup DOXGRP_CONF
 */
#define CONFIG_BUFF                     20480

/**
 * Special flag character.
 *
 * This is the special character value passed to a config context
 * parser when the @c begin statement for that context is
 * encountered.
 *
 * @see DOXGRP_CONF
 * @ingroup DOXGRP_CONF
 */
#define CONF_BEGIN_CHAR                 '\001'
/**
 * Special flag character string.
 *
 * This is the string representation of CONF_BEGIN_CHAR.
 *
 * @see DOXGRP_CONF
 * @ingroup DOXGRP_CONF
 */
#define CONF_BEGIN_STRING               "\001"
/**
 * Special flag character.
 *
 * This is the special character value passed to a config context
 * parser when the @c end statement for that context is encountered.
 *
 * @see DOXGRP_CONF
 * @ingroup DOXGRP_CONF
 */
#define CONF_END_CHAR                   '\002'
/**
 * Special flag character string.
 *
 * This is the string representation of CONF_END_CHAR.
 *
 * @see DOXGRP_CONF
 * @ingroup DOXGRP_CONF
 */
#define CONF_END_STRING                 "\002"

/**
 * Compares boolean option value to allowed true values.
 *
 * This macro compares the value of a boolean option against the
 * acceptable boolean "true" values ("1", "on", "yes", and "true").
 *
 * @param s String value of a boolean option.
 * @return Non-zero if a match is found, zero if not.
 * @see DOXGRP_CONF
 * @ingroup DOXGRP_CONF
 */
#define BOOL_OPT_ISTRUE(s)  (!strcasecmp((s), true_vals[0]) || !strcasecmp((s), true_vals[1]) \
                             || !strcasecmp((s), true_vals[2]) || !strcasecmp((s), true_vals[3]))
/**
 * Compares boolean option value to allowed false values.
 *
 * This macro compares the value of a boolean option against the
 * acceptable boolean "false" values ("0", "off", "no", and "false").
 *
 * @param s String value of a boolean option.
 * @return Non-zero if a match is found, zero if not.
 * @see DOXGRP_CONF
 * @ingroup DOXGRP_CONF
 */
#define BOOL_OPT_ISFALSE(s) (!strcasecmp((s), false_vals[0]) || !strcasecmp((s), false_vals[1]) \
                             || !strcasecmp((s), false_vals[2]) || !strcasecmp((s), false_vals[3]))

/**
 * Skip-to-end flag.
 *
 * This symbol represents the bit in the FSS flags which specifies
 * that the parser should skip the rest of the file.
 *
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define FILE_SKIP_TO_END           (0x01)
/**
 * Preprocessing flag.
 *
 * This symbol represents the bit in the FSS flags which specifies
 * that this file should be preprocessed.
 *
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define FILE_PREPROC               (0x02)
/**
 * Push info for a new file onto the state stack.
 *
 * This macro adds a new file state structure to the top of the stack
 * and populates it with the information contained in the macro
 * parameters.  When a new file is opened for parsing, a call is made
 * to this macro to "push" the new file onto the top of the stack.
 *
 * @param f  The file pointer (FILE *) representing the newly-opened
 *           file.
 * @param p  The path to the newly-opened file.
 * @param o  The output file name (for preprocessing).
 * @param l  The current line number for the file.
 * @param fl The flag set for the file.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_push(f, p, o, l, fl)  conf_register_fstate(f, p, o, l, fl)
/**
 * Pop a state structure off the stack.
 *
 * This macro pops a file state structure off the top of the stack.  A
 * call to this macro occurs once the parsing of the current file is
 * completed.
 *
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_pop()                 (fstate_idx--)
/**
 * Return the top file state structure on the stack.
 *
 * This macro is used to access the file state structure currently on
 * top of the stack.
 *
 * @return The file state structure atop the stack.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_peek()                (fstate[fstate_idx])
/**
 * Examine the file pointer on top of the stack.
 *
 * This macro returns the file pointer (FILE *) corresponding to the
 * file currently being parsed.
 *
 * @return The current file pointer.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_peek_fp()             (fstate[fstate_idx].fp)
/**
 * Examine the path of the current file.
 *
 * This macro returns the path for the file currently being parsed.
 *
 * @return The path of the current file.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_peek_path()           (fstate[fstate_idx].path)
/**
 * Examine the path of the current pre-processing output file.
 *
 * This macro returns the path for the preprocessing output file
 * currently being parsed.
 *
 * @return The path of the current preproc output file.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_peek_outfile()        (fstate[fstate_idx].outfile)
/**
 * Examine the line number of the current file.
 *
 * This macro returns the current line number within the current
 * config file.
 *
 * @return The line number of the current file.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_peek_line()           (fstate[fstate_idx].line)
/**
 * Check whether or not we're skipping to the end of the current
 * file.
 *
 * This macro returns zero if the current file is being parsed and
 * non-zero if the parser is skipping to its end.
 *
 * @return The skip-to-end flag for the current file.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_peek_skip()           (fstate[fstate_idx].flags & FILE_SKIP_TO_END)
/**
 * Check whether or not the current file was preprocessed.
 *
 * This macro returns zero if the current file was not preprocessed
 * and non-zero if it was.
 *
 * @return The preprocessing flag for the current file.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_peek_preproc()        (fstate[fstate_idx].flags & FILE_PREPROC)

/**
 * Set the file pointer for the current file.
 *
 * @internal
 * @param f The file pointer (FILE *).
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_poke_fp(f)            ((fstate[fstate_idx].fp) = (f))
/**
 * Set the path for the current file.
 *
 * @internal
 * @param p The path.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_poke_path(p)          ((fstate[fstate_idx].path) = (p))
/**
 * Set the outfile for the current file.
 *
 * @internal
 * @param o The outfile.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_poke_outfile(o)       ((fstate[fstate_idx].outfile) = (o))
/**
 * Set the current line number for the current file.
 *
 * @internal
 * @param l The line number.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_poke_line(l)          ((fstate[fstate_idx].line) = (l))
/**
 * Set the skip-to-end flag for the current file.
 *
 * @internal
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_skip_to_end()         ((fstate[fstate_idx].flags) |= (FILE_SKIP_TO_END))
/**
 * Set/clear the skip-to-end flag for the current file.
 *
 * @internal
 * @param s 0 to clear, non-zero to set.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_poke_skip(s)          do {if (s) {fstate[fstate_idx].flags |= FILE_SKIP_TO_END;} else {fstate[fstate_idx].flags &= ~(FILE_SKIP_TO_END);} } while (0)
/**
 * Set/clear the preprocessing flag for the current file.
 *
 * @internal
 * @param s 0 to clear, non-zero to set.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_poke_preproc(s)       do {if (s) {fstate[fstate_idx].flags |= FILE_PREPROC;} else {fstate[fstate_idx].flags &= ~(FILE_PREPROC);} } while (0)
/**
 * Set all state info for the current file.
 *
 * @internal
 * @param f  The file pointer (FILE *).
 * @param p  The file path.
 * @param o  The outfile.
 * @param l  The line number.
 * @param fl The flags.
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_poke(f, p, o, l, fl)  do {file_poke_fp(f); file_poke_path(p); file_poke_outfile(o); file_poke_line(l); fstate[fstate_idx].flags = (fl);} while (0)

/**
 * Increment the line number for the current file.
 *
 * @internal
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
#define file_inc_line()            (fstate[fstate_idx].line++)

/**
 * File state stack structure.
 *
 * This structure comprises the individual stack elements on the file
 * state stack.  One of these structures is present on the stack for
 * each file being parsed.
 *
 * @see DOXGRP_CONF_FSS
 * @ingroup DOXGRP_CONF_FSS
 */
typedef struct file_state_struct {
    /**
     * File pointer.
     *
     * Contains an open file pointer used to read data from the
     * file.
     */
    FILE *fp;
    /**
     * File path.
     *
     * Contains the path to the file.
     */
    char *path;
    /**
     * Preprocessing output file.
     *
     * Contains the path to the file used for preprocessing
     * output.
     */
    char *outfile;
    /**
     * Line number.
     *
     * Contains the current line number for the file.
     */
    unsigned long line;
    /**
     * File state flags.
     *
     * Contains the skip-to-end (FILE_SKIP_TO_END) and preprocessing
     * (FILE_PREPROC) flags for the file.
     */
    unsigned char flags;
} fstate_t;

/**
 * Typedef for pointers to context handler functions.
 *
 * This function pointer type is used for variables, typecasts,
 * etc. involving context handler functions.  Context handlers must
 * accept two parameters, a char * containing either the config file
 * line or a begin/end magic string, and a void * containing state
 * information; they must return a void * which will be passed to the
 * next invocation of the handler as the aforementioned state
 * information parameter.
 *
 * @see DOXGRP_CONF_CTX
 * @ingroup DOXGRP_CONF_CTX
 */
typedef void * (*ctx_handler_t)(char *, void *);
/**
 */
typedef char * (*conf_func_ptr_t) (char *);

extern fstate_t *fstate;
extern unsigned char fstate_idx;
extern const char *true_vals[], *false_vals[];


/******************************* OPTIONS GOOP **********************************/

/* Flags for individual options */
#define SPIFOPT_FLAG_NONE                 (0)
#define SPIFOPT_FLAG_BOOLEAN              (1UL << 0)
#define SPIFOPT_FLAG_TYPEMASK_NOVALUE     (SPIFOPT_FLAG_BOOLEAN)
#define SPIFOPT_FLAG_INTEGER              (1UL << 1)
#define SPIFOPT_FLAG_STRING               (1UL << 2)
#define SPIFOPT_FLAG_ARGLIST              (1UL << 3)
#define SPIFOPT_FLAG_ABSTRACT             (1UL << 4)
#define SPIFOPT_FLAG_TYPEMASK_VALUE       (SPIFOPT_FLAG_INTEGER | SPIFOPT_FLAG_STRING | SPIFOPT_FLAG_ARGLIST | SPIFOPT_FLAG_ABSTRACT)
#define SPIFOPT_FLAG_TYPEMASK             (SPIFOPT_FLAG_TYPEMASK_NOVALUE | SPIFOPT_FLAG_TYPEMASK_VALUE)

#define SPIFOPT_FLAG_PREPARSE             (1UL << 8)
#define SPIFOPT_FLAG_DEPRECATED           (1UL << 9)

/* Flags that control the parser's behavior */
#define SPIFOPT_SETTING_POSTPARSE         (1UL << 0)

#define SPIFOPT_OPTION(s, l, d, f, p, m)  { s, l, d,                                             (f),  (p), m }
#define SPIFOPT_BOOL(s, l, d, v, m)       { s, l, d,                          (SPIFOPT_FLAG_BOOLEAN), &(v), m }
#define SPIFOPT_BOOL_PP(s, l, d, v, m)    { s, l, d,  (SPIFOPT_FLAG_BOOLEAN | SPIFOPT_FLAG_PREPARSE), &(v), m }
#define SPIFOPT_BOOL_LONG(l, d, v, m)     { 0, l, d,                          (SPIFOPT_FLAG_BOOLEAN), &(v), m }
#define SPIFOPT_BOOL_LONG_PP(l, d, v, m)  { 0, l, d,  (SPIFOPT_FLAG_BOOLEAN | SPIFOPT_FLAG_PREPARSE), &(v), m }
#define SPIFOPT_INT(s, l, d, p)           { s, l, d,                          (SPIFOPT_FLAG_INTEGER), &(p), 0 }
#define SPIFOPT_INT_PP(s, l, d, p)        { s, l, d,  (SPIFOPT_FLAG_INTEGER | SPIFOPT_FLAG_PREPARSE), &(p), 0 }
#define SPIFOPT_INT_LONG(l, d, p)         { 0, l, d,                          (SPIFOPT_FLAG_INTEGER), &(p), 0 }
#define SPIFOPT_INT_LONG_PP(l, d, p)      { 0, l, d,  (SPIFOPT_FLAG_INTEGER | SPIFOPT_FLAG_PREPARSE), &(p), 0 }
#define SPIFOPT_STR(s, l, d, p)           { s, l, d,                           (SPIFOPT_FLAG_STRING), &(p), 0 }
#define SPIFOPT_STR_PP(s, l, d, p)        { s, l, d,   (SPIFOPT_FLAG_STRING | SPIFOPT_FLAG_PREPARSE), &(p), 0 }
#define SPIFOPT_STR_LONG(l, d, p)         { 0, l, d,                           (SPIFOPT_FLAG_STRING), &(p), 0 }
#define SPIFOPT_STR_LONG_PP(l, d, p)      { 0, l, d,   (SPIFOPT_FLAG_STRING | SPIFOPT_FLAG_PREPARSE), &(p), 0 }
#define SPIFOPT_ARGS(s, l, d, p)          { s, l, d,                          (SPIFOPT_FLAG_ARGLIST), &(p), 0 }
#define SPIFOPT_ARGS_PP(s, l, d, p)       { s, l, d,  (SPIFOPT_FLAG_ARGLIST | SPIFOPT_FLAG_PREPARSE), &(p), 0 }
#define SPIFOPT_ARGS_LONG(l, d, p)        { 0, l, d,                          (SPIFOPT_FLAG_ARGLIST), &(p), 0 }
#define SPIFOPT_ARGS_LONG_PP(l, d, p)     { 0, l, d,  (SPIFOPT_FLAG_ARGLIST | SPIFOPT_FLAG_PREPARSE), &(p), 0 }
#define SPIFOPT_ABST(s, l, d, f)          { s, l, d,                         (SPIFOPT_FLAG_ABSTRACT),  (f), 0 }
#define SPIFOPT_ABST_PP(s, l, d, f)       { s, l, d, (SPIFOPT_FLAG_ABSTRACT | SPIFOPT_FLAG_PREPARSE),  (f), 0 }
#define SPIFOPT_ABST_LONG(l, d, f)        { 0, l, d,                         (SPIFOPT_FLAG_ABSTRACT),  (f), 0 }
#define SPIFOPT_ABST_LONG_PP(l, d, f)     { 0, l, d, (SPIFOPT_FLAG_ABSTRACT | SPIFOPT_FLAG_PREPARSE),  (f), 0 }

#define SPIFOPT_TYPE(opt)                 (((spifopt_t) (opt)).flags & SPIFOPT_FLAG_TYPEMASK)
#define SPIFOPT_OPT_TYPE(n)               (SPIFOPT_OPT_FLAGS(n) & SPIFOPT_FLAG_TYPEMASK)
#define SPIFOPT_OPT_IS_BOOLEAN(n)         (SPIFOPT_OPT_FLAGS(n) & SPIFOPT_FLAG_BOOLEAN)
#define SPIFOPT_OPT_IS_STRING(n)          (SPIFOPT_OPT_FLAGS(n) & SPIFOPT_FLAG_STRING)
#define SPIFOPT_OPT_IS_INTEGER(n)         (SPIFOPT_OPT_FLAGS(n) & SPIFOPT_FLAG_INTEGER)
#define SPIFOPT_OPT_IS_ARGLIST(n)         (SPIFOPT_OPT_FLAGS(n) & SPIFOPT_FLAG_ARGLIST)
#define SPIFOPT_OPT_IS_ABSTRACT(n)        (SPIFOPT_OPT_FLAGS(n) & SPIFOPT_FLAG_ABSTRACT)
#define SPIFOPT_OPT_IS_PREPARSE(n)        (SPIFOPT_OPT_FLAGS(n) & SPIFOPT_FLAG_PREPARSE)
#define SPIFOPT_OPT_IS_DEPRECATED(n)      (SPIFOPT_OPT_FLAGS(n) & SPIFOPT_FLAG_DEPRECATED)
#define SPIFOPT_OPT_NEEDS_VALUE(n)        (SPIFOPT_OPT_FLAGS(n) & (SPIFOPT_FLAG_STRING | SPIFOPT_FLAG_INTEGER | SPIFOPT_FLAG_ARGLIST))

#define SPIFOPT_OPT_SHORT(n)              (SPIFOPT_OPTLIST(n).short_opt)
#define SPIFOPT_OPT_LONG(n)               (SPIFOPT_OPTLIST(n).long_opt)
#define SPIFOPT_OPT_DESC(n)               (SPIFOPT_OPTLIST(n).desc)
#define SPIFOPT_OPT_FLAGS(n)              (SPIFOPT_OPTLIST(n).flags)
#define SPIFOPT_OPT_VALUE(n)              (SPIFOPT_OPTLIST(n).value)
#define SPIFOPT_OPT_MASK(n)               (SPIFOPT_OPTLIST(n).mask)

#define SPIFOPT_OPTLIST(n)                (spifopt_settings.opt_list[((n) < (spifopt_settings.num_opts) ? (n) : (0))])
#define SPIFOPT_OPTLIST_SET(l)            (spifopt_settings.opt_list = ((spifopt_t *) (l)))
#define SPIFOPT_NUMOPTS_GET()             (spifopt_settings.num_opts)
#define SPIFOPT_NUMOPTS_SET(n)            (spifopt_settings.num_opts = (n))
#define SPIFOPT_FLAGS_GET()               (spifopt_settings.flags)
#define SPIFOPT_FLAGS_SET(m)              (spifopt_settings.flags |= (m))
#define SPIFOPT_FLAGS_IS_SET(m)           (spifopt_settings.flags & (m))
#define SPIFOPT_FLAGS_CLEAR(m)            (spifopt_settings.flags &= ~(m))
#define SPIFOPT_BADOPTS_GET()             (spifopt_settings.bad_opts)
#define SPIFOPT_BADOPTS_SET(n)            (spifopt_settings.bad_opts = (n))
#define SPIFOPT_ALLOWBAD_GET()            (spifopt_settings.allow_bad)
#define SPIFOPT_ALLOWBAD_SET(n)           (spifopt_settings.allow_bad = (n))
#define SPIFOPT_INDENT_GET()              (spifopt_settings.indent)
#define SPIFOPT_INDENT_SET(n)             (spifopt_settings.indent = (n))
#define SPIFOPT_HELPHANDLER               ((spifopt_settings.help_handler) ? (spifopt_settings.help_handler) : (spifopt_usage))
#define SPIFOPT_HELPHANDLER_SET(f)        (spifopt_settings.help_handler = (f))

/**
 * Typedef for help handler function.
 *
 * This type is used for declaring/typecasting function pointers which
 * will be used for help handlers.  Functions used for this should be
 * declared as returning void (and in reality does not return at all)
 * and should take either no parameters, or a single char * parameter.
 *
 * @see DOXGRP_OPT, SPIFOPT_HELPHANDLER_SET()
 * @ingroup DOXGRP_OPT
 */
typedef void (*spifopt_helphandler_t)();
/**
 * Typedef for abstract option handler function.
 *
 * This type is used for declaring/typecasting function pointers which
 * will be used for abstract option handlers.  Abstract options are
 * those which require special handling; LibAST implements this by
 * allowing for an arbitrary user-specified function be invoked when
 * such an option is encountered.  Functions used for this should be
 * declared as returning void and should take a single char *
 * parameter (the value of the option, or NULL if it had no value).
 *
 * @see DOXGRP_OPT
 * @ingroup DOXGRP_OPT
 */
typedef void (*spifopt_abstract_handler_t)(char *);

/**
 * Option structure.
 *
 * This is the structure that holds the data for each of the command
 * line options for which the parser will be looking.  Client programs
 * must create an array of these structures (a spifopt_t []) and use
 * the SPIFOPT_OPTLIST_SET() macro to tell LibAST which variable it
 * is.
 *
 * @note This structure and its members should NEVER be accessed
 * directly; they are documented solely for informational purposes.
 * The SPIFOPT_* convenience macros provide a streamlined, easy-to-use
 * abstraction layer for declaring the option list, setting option
 * parser parameters, and so forth.  Even the internal code uses these
 * macros!  Consult the macro documentation and the example code for
 * further assistance.
 *
 * @see DOXGRP_OPT, @link opt_example.c example code @endlink
 * @ingroup DOXGRP_OPT
 */
typedef struct spifopt_t_struct {
    /**
     * Short option.
     *
     * The short (one char) form of the option.
     */
    spif_char_t short_opt;
    /**
     * Long option.
     *
     * The long (string) form of the option.
     */
    spif_charptr_t long_opt;
    /**
     * Description.
     *
     * The (brief) description of the option for the help screen.
     */
    spif_charptr_t desc;
    /**
     * Option type/attribute flags.
     *
     * The type and attribute flags for this option.
     */
    spif_uint32_t flags;
    /**
     * Value pointer.
     *
     * A pointer to where the value for this option should be stored.
     * Its exact type, and how it is interpreted, depends on the type
     * of option being defined.
     */
    void *value;
    /**
     * Boolean bitmask.
     *
     * For boolean options, this is the bitmask for the option.  For
     * other option types, it has no meaning.
     */
    spif_uint32_t mask;
} spifopt_t;

/**
 * Option parser settings structure.
 *
 * This is the structure that holds the settings and other internal
 * variables which control how the options parser functions.
 *
 * @note This structure and its members should NEVER be accessed
 * directly; they are documented solely for informational purposes.
 * The SPIFOPT_* convenience macros provide a streamlined, easy-to-use
 * abstraction layer for declaring the option list, setting option
 * parser parameters, and so forth.  Even the internal code uses these
 * macros!  Consult the macro documentation and the example code for
 * further assistance.
 *
 * @see DOXGRP_OPT, @link opt_example.c example code @endlink
 * @ingroup DOXGRP_OPT
 */
typedef struct spifopt_settings_t_struct {
    /**
     * Options list.
     *
     * The array of option structures defining the options to look
     * for.
     */
    spifopt_t *opt_list;
    /**
     * Option count.
     *
     * The total number of options in the options list.
     */
    spif_uint16_t num_opts;
    /**
     * Parser flags.
     *
     * Flags which control the behavior of the parser.
     */
    spif_uint32_t flags;
    /**
     * Bad option count.
     *
     * Keeps track of the number of bad options (i.e., option syntax
     * errors, such as missing values or unknown options)
     * encountered.
     */
    spif_uint8_t bad_opts;
    /**
     * Bad option limit.
     *
     * The maximum number of bad options allowed before giving up and
     * displaying the help text.
     */
    spif_uint8_t allow_bad;
    spif_uint8_t indent; /**< Unused. */
    /**
     * Help handler.
     *
     * Pointer to the function which is responsible for displaying the
     * help text.  If undefined, spifopt_usage() is used.
     */
    spifopt_helphandler_t help_handler;
} spifopt_settings_t;

extern spifopt_settings_t spifopt_settings;




/******************************** PROTOTYPES **********************************/

/* msgs.c */
extern void libast_set_program_name(const char *);
extern void libast_set_program_version(const char *);
extern int libast_dprintf(const char *, ...);
extern void print_error(const char *fmt, ...);
extern void print_warning(const char *fmt, ...);
extern void fatal_error(const char *fmt, ...);

/* debug.c */
extern unsigned int DEBUG_LEVEL;

/* mem.c */
extern void memrec_init(void);
extern void *libast_malloc(const char *, unsigned long, size_t);
extern void *libast_realloc(const char *, const char *, unsigned long, void *, size_t);
extern void *libast_calloc(const char *, unsigned long, size_t, size_t);
extern void libast_free(const char *, const char *, unsigned long, void *);
extern char *libast_strdup(const char *, const char *, unsigned long, const char *);
extern void libast_dump_mem_tables(void);
#ifdef LIBAST_X11_SUPPORT
extern Pixmap libast_x_create_pixmap(const char *, unsigned long, Display *, Drawable, unsigned int, unsigned int, unsigned int);
extern void libast_x_free_pixmap(const char *, const char *, unsigned long, Display *, Pixmap);
# ifdef LIBAST_IMLIB2_SUPPORT
extern void libast_imlib_register_pixmap(const char *var, const char *filename, unsigned long line, Pixmap p);
extern void libast_imlib_free_pixmap(const char *var, const char *filename, unsigned long line, Pixmap p);
# endif
extern void libast_dump_pixmap_tables(void);
extern GC libast_x_create_gc(const char *, unsigned long, Display *, Drawable, unsigned long, XGCValues *);
extern void libast_x_free_gc(const char *, const char *, unsigned long, Display *, GC);
extern void libast_dump_gc_tables(void);
#endif
extern void free_array(void *, size_t);

/* file.c */
extern int libast_temp_file(char *, size_t);

/* strings.c */
extern char *left_str(const char *, unsigned long);
extern char *mid_str(const char *, unsigned long, unsigned long);
extern char *right_str(const char *, unsigned long);
#if defined(LIBAST_REGEXP_SUPPORT_POSIX) && defined(HAVE_REGEX_H)
extern spif_bool_t regexp_match(const char *, const char *);
extern spif_bool_t regexp_match_r(const char *str, const char *pattern, regex_t **rexp);
#endif
extern char **split(const char *, const char *);
extern char **split_regexp(const char *, const char *);
extern char *join(const char *, char **);
extern char *get_word(unsigned long, const char *);
extern char *get_pword(unsigned long, const char *);
extern unsigned long num_words(const char *);
extern char *chomp(char *);
extern char *strip_whitespace(char *);
extern char *downcase_str(char *);
extern char *upcase_str(char *);
#ifndef HAVE_STRCASESTR
extern char *strcasestr(const char *, const char *);
#endif
#ifndef HAVE_STRCASECHR
extern char *strcasechr(const char *, const char);
#endif
#ifndef HAVE_STRCASEPBRK
extern char *strcasepbrk(const char *, const char *);
#endif
#ifndef HAVE_STRREV
extern char *strrev(char *);
#endif
#if !(HAVE_STRSEP)
extern char *strsep(char **, char *);
#endif
extern char *safe_str(char *, unsigned short);
extern char *garbage_collect(char *, size_t);
extern char *file_garbage_collect(char *, size_t);
extern char *condense_whitespace(char *);
extern void hex_dump(void *, size_t);
extern spif_cmp_t version_compare(const char *, const char *);
#ifndef HAVE_MEMMEM
extern void *memmem(const void *, size_t, const void *, size_t);
#endif
#ifndef HAVE_STRNLEN
extern size_t strnlen(const char *, size_t);
#endif
#ifndef HAVE_USLEEP
extern void usleep(unsigned long);
#endif
#ifndef HAVE_SNPRINTF
extern int vsnprintf(char *str, size_t count, const char *fmt, va_list args);
extern int snprintf(char *str, size_t count, const char *fmt, ...);
#endif

/* conf.c */
extern void conf_init_subsystem(void);
extern unsigned char conf_register_context(char *name, ctx_handler_t handler);
extern unsigned char conf_register_fstate(FILE *fp, char *path, char *outfile, unsigned long line, unsigned char flags);
extern unsigned char conf_register_builtin(char *name, conf_func_ptr_t ptr);
extern unsigned char conf_register_context_state(unsigned char ctx_id);
extern void conf_free_subsystem(void);
extern char *shell_expand(char *);
extern char *conf_find_file(const char *file, const char *dir, const char *pathlist);
extern FILE *open_config_file(char *name);
extern void conf_parse_line(FILE *fp, char *buff);
extern char *conf_parse(char *conf_name, const char *dir, const char *path);

/* options.c */
extern void spifopt_parse(int, char **);
extern void spifopt_usage(void);

#endif /* _LIBAST_H_ */
