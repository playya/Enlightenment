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
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#ifdef __FreeBSD__
#include <sys/mount.h>
#else
#include <sys/statfs.h>
#endif
#include <sys/un.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fnmatch.h>
#include <Edb.h>

#ifdef __EMX__
#include <strings.h>
#endif

#include <efsd.h>
#include <efsd_debug.h>
#include <efsd_lock.h>
#include <efsd_macros.h>
#include <efsd_misc.h>
#include <efsd_filetype.h>
#include <efsd_hash.h>
#include <efsd_statcache.h>

const char unknown_string[] = "document/unknown";

typedef enum efsd_magic_type
{
  EFSD_MAGIC_8              = 0,
  EFSD_MAGIC_16             = 1,
  EFSD_MAGIC_32             = 2,
  EFSD_MAGIC_STRING         = 3
}
EfsdMagicType;

typedef enum efsd_magic_test
{
  EFSD_MAGIC_TEST_EQUAL     = 0,
  EFSD_MAGIC_TEST_NOTEQUAL  = 1,
  EFSD_MAGIC_TEST_SMALLER   = 2,
  EFSD_MAGIC_TEST_LARGER    = 3,
  EFSD_MAGIC_TEST_MASK      = 4,
  EFSD_MAGIC_TEST_NOTMASK   = 5
}
EfsdMagicTest;

typedef enum efsd_byteorder
{
  EFSD_BYTEORDER_HOST       = 0,
  EFSD_BYTEORDER_BIG        = 1,
  EFSD_BYTEORDER_SMALL      = 2
}
EfsdByteorder;


/* This is the data structure used to define a file magic
   test. All tests are stored in a tree in order to
   be able to represent the hierarchical nature of the
   tests -- all tests below a node are more specialized,
   like ones with more ">"'s in a magic file, check the
   manpage for details.

   The tests are stored by node indices in the db --
   the first test is located by /1/FIELDS, where FIELDS
   is 'offset', 'type', 'value' etc. The other tests on the
   same layer are /2/FIELDS ... /n/FIELDS. The specialized
   tests are located by simply adding another index level,
   1/1/FIELDS, 1/2/FIELDS ... 1/m/FIELDS, etc.
*/

typedef struct efsd_magic
{
  u_int16_t           offset;
  EfsdMagicType       type;
  void               *value;
  int                 value_len;
  EfsdByteorder       byteorder;

  char                use_mask;
  int                 mask;
  EfsdMagicTest       test;

  char               *filetype;

  struct efsd_magic  *next;
  struct efsd_magic  *kids;
  struct efsd_magic  *last_kid;
}
EfsdMagic;


typedef struct efsd_filetype_cache_item
{
  char    *filetype; /* Cached filetype */
  time_t   time;     /* Timestamp of last calculation */ 
}
EfsdFiletypeCacheItem;

#ifdef WORDS_BIGENDIAN
static EfsdByteorder host_byteorder = EFSD_BYTEORDER_BIG;
#else
static EfsdByteorder host_byteorder = EFSD_BYTEORDER_SMALL;
#endif

/* These are mostly taken from the stat(1) source code: */

#define	AFFS_SUPER_MAGIC      0xADFF
#define EXT_SUPER_MAGIC       0x137D
#define EXT2_OLD_SUPER_MAGIC  0xEF51
#define EXT2_SUPER_MAGIC      0xEF53
#define HPFS_SUPER_MAGIC      0xF995E849
#define ISOFS_SUPER_MAGIC     0x9660
#define MINIX_SUPER_MAGIC     0x137F
#define MINIX_SUPER_MAGIC2    0x138F
#define MINIX2_SUPER_MAGIC    0x2468
#define MINIX2_SUPER_MAGIC2   0x2478
#define MSDOS_SUPER_MAGIC     0x4d44
#define NCP_SUPER_MAGIC       0x564c
#define NFS_SUPER_MAGIC       0x6969
#define PROC_SUPER_MAGIC      0x9fa0
#define SMB_SUPER_MAGIC       0x517B
#define XENIX_SUPER_MAGIC     0x012FF7B4
#define SYSV4_SUPER_MAGIC     0x012FF7B5
#define SYSV2_SUPER_MAGIC     0x012FF7B6
#define COH_SUPER_MAGIC       0x012FF7B7
#define UFS_MAGIC             0x00011954
#define _XIAFS_SUPER_MAGIC    0x012FD16D
#define	NTFS_SUPER_MAGIC      0x5346544e
#define XFS_SUPER_MAGIC       0x58465342
#define REISERFS_SUPER_MAGIC  0x52654973

/* The root node of the magic checks tree. Its test-related
   entries aren't used, it's only a container for the first
   level's list of EfsdMagics.
*/
static EfsdMagic  magic;

static EfsdLock  *filetype_lock;

/* The db where everything is stored. */
static E_DB_File *magic_db = NULL;

/* Filename patterns */
static char     **patterns = NULL;
static char     **pattern_filetypes = NULL;
static int        num_patterns;

static char     **patterns_user = NULL;
static char     **pattern_filetypes_user = NULL;
static int        num_patterns_user;

static EfsdHash  *filetype_cache;

/* db helper functions */
static int        filetype_edb_int8_t_get(E_DB_File * db, char *key, u_int8_t *val);
static int        filetype_edb_int16_t_get(E_DB_File * db, char *key, u_int16_t *val);
static int        filetype_edb_int32_t_get(E_DB_File * db, char *key, u_int32_t *val);

static EfsdMagic *filetype_magic_new(char *key, char *params);
static void       filetype_magic_free(EfsdMagic *em);
static void       filetype_magic_cleanup_level(EfsdMagic *em);
static void       filetype_magic_add_child(EfsdMagic *em_dad, EfsdMagic *em_kid);
static char      *filetype_magic_test_level(EfsdMagic *em, FILE *f, char *ptr, char stop_when_found);
static char      *filetype_magic_test_perform(EfsdMagic *em, FILE *f);
static void       filetype_magic_init_level(char *key, char *ptr, EfsdMagic *em_parent);
static int        filetype_patterns_init(void);
static void       filetype_fix_byteorder(EfsdMagic *em);

static int        filetype_test_fs(char *filename, struct stat *st, char *type, int len);
static int        filetype_test_magic(char *filename, char *type, int len);
static int        filetype_test_pattern(char *filename, char *type, int len);

static char      *filetype_get_magic_db(void);
static char      *filetype_get_sys_patterns_db(void);
static char      *filetype_get_user_patterns_db(void);

static void       filetype_cache_init(void);
static void       filetype_cache_update(char *filename, time_t time, const char *filetype);
static EfsdFiletypeCacheItem *filetype_cache_lookup(char *filename);
static void       filetype_hash_item_free(EfsdHashItem *it);


static char   *
filetype_get_magic_db(void)
{
  static char s[4096] = "\0";
  
  D_ENTER;

  if (s[0] != '\0')
    D_RETURN_(s);

  snprintf(s, sizeof(s), "%s/magic.db", efsd_misc_get_sys_dir());
  s[sizeof(s)-1] = '\0';

  if (efsd_misc_file_exists(s))
    D_RETURN_(s);

  D_RETURN_(NULL);
}


static char   *
filetype_get_sys_patterns_db(void)
{
  static char s[4096] = "\0";
  
  D_ENTER;

  if (s[0] != '\0')
    D_RETURN_(s);

  snprintf(s, sizeof(s), "%s/pattern.db", efsd_misc_get_sys_dir());
  s[sizeof(s)-1] = '\0';

  if (efsd_misc_file_exists(s))
    D_RETURN_(s);

  D_RETURN_(NULL);
}


static char   *
filetype_get_user_patterns_db(void)
{
  static char s[4096] = "\0";
  
  D_ENTER;

  if (s[0] != '\0')
    D_RETURN_(s);

  snprintf(s, sizeof(s), "%s/pattern.db", efsd_misc_get_user_dir());
  s[sizeof(s)-1] = '\0';

  if (efsd_misc_file_exists(s))
    D_RETURN_(s);

  D_RETURN_(NULL);
}



static int
filetype_edb_int8_t_get(E_DB_File * db, char *key, u_int8_t *val)
{
  int result;
  int v;
  
  D_ENTER;
  
  result = e_db_int_get(db, key, &v);
  *val = (u_int8_t)v;
  D_RETURN_(result);
}


static int               
filetype_edb_int16_t_get(E_DB_File * db, char *key, u_int16_t *val)
{
  int result;
  int v;
  
  D_ENTER;
  
  result = e_db_int_get(db, key, &v);
  *val = (u_int16_t)v;
  D_RETURN_(result);
}


static int               
filetype_edb_int32_t_get(E_DB_File * db, char *key, u_int32_t *val)
{
  int result;
  int v;
  
  D_ENTER;
  
  result = e_db_int_get(db, key, &v);
  *val = (u_int32_t)v;
  D_RETURN_(result);
}


static EfsdMagic *
filetype_magic_new(char *key, char *params)
{
  int        dummy;
  EfsdMagic *em;

  D_ENTER;

  em = NEW(EfsdMagic);
  memset(em, 0, sizeof(EfsdMagic));

  sprintf(params, "%s", "/offset");
  e_db_int_get(magic_db, key, &dummy);
  em->offset = (u_int16_t)dummy;

  sprintf(params, "%s", "/type");
  e_db_int_get(magic_db, key, (int*)&em->type);

  sprintf(params, "%s", "/byteorder");
  e_db_int_get(magic_db, key, (int*)&em->byteorder);

  sprintf(params, "%s", "/value");
  switch (em->type)
    {
    case EFSD_MAGIC_8:
      em->value = NEW(u_int8_t);
      filetype_edb_int8_t_get(magic_db, key, (u_int8_t*)em->value);
      break;
    case EFSD_MAGIC_16:
      em->value = NEW(u_int16_t);
      filetype_edb_int16_t_get(magic_db, key, (u_int16_t*)em->value);
      filetype_fix_byteorder(em);
      break;
    case EFSD_MAGIC_32:
      em->value = NEW(u_int32_t);
      filetype_edb_int32_t_get(magic_db, key, (u_int32_t*)em->value);
      filetype_fix_byteorder(em);
      break;
    case EFSD_MAGIC_STRING:
      em->value = (char*)e_db_data_get(magic_db, key, &em->value_len);
      break;
    default:
    }

  sprintf(params, "%s", "/mask");
  if (e_db_int_get(magic_db, key, &em->mask))
    em->use_mask = TRUE;
  else
    em->use_mask = FALSE;

  sprintf(params, "%s", "/test");
  e_db_int_get(magic_db, key, (int*)&em->test);

  sprintf(params, "%s", "/filetype");
  em->filetype = e_db_str_get(magic_db, key);

  D_RETURN_(em);
}


static void       
filetype_magic_free(EfsdMagic *em)
{
  D_ENTER;

  if (!em)
    { D_RETURN; }

  FREE(em->value);
  FREE(em->value);
  FREE(em);

  D_RETURN;
}


static void       
filetype_magic_cleanup_level(EfsdMagic *em)
{
  EfsdMagic *m, *m2;

  D_ENTER;

  if (!em)
    { D_RETURN; }

  for (m = em->kids; m; m = m->next)
    filetype_magic_cleanup_level(m);

  m = em->kids;

  while (m)
    {
      m2 = m;
      m = m->next;

      filetype_magic_free(m2);
    }

  D_RETURN;
}


static void       
filetype_magic_add_child(EfsdMagic *em_dad, EfsdMagic *em_kid)
{
  D_ENTER;

  if (!em_dad || !em_kid)
    { D_RETURN; }

  if (em_dad->kids)
    {
      em_dad->last_kid->next = em_kid;
      em_dad->last_kid = em_kid;
    }
  else
    {
      em_dad->kids = em_kid;
      em_dad->last_kid = em_kid;
    }

  D_RETURN;
}


static void
filetype_fix_byteorder(EfsdMagic *em)
{
  int    size = 0;
  int    i;
  char   tmp[4];
  char  *data;

  D_ENTER;

  if ((em->type == EFSD_MAGIC_8) ||
      (em->type == EFSD_MAGIC_16) ||
      (em->type == EFSD_MAGIC_32))
    {
      if ((em->byteorder == host_byteorder)  || 
	  (em->byteorder == EFSD_BYTEORDER_HOST))
	{
	  /* D(("Not changing byteorder.\n")); */
	  D_RETURN;
	}

      switch (em->type)
	{
	case EFSD_MAGIC_8:
	  size = sizeof(u_int8_t);
	  break;
	case EFSD_MAGIC_16:
	  size = sizeof(u_int16_t);
	  break;
	case EFSD_MAGIC_32:
	  size = sizeof(u_int32_t);
	  break;
	default:
	}

      data = (char*)em->value;

      for (i = 0; i < size; i++)
	tmp[i] = data[size-1-i];
      
      memcpy(data, tmp, size);
    }
  else
    {
      /* D(("Not changing byteorder.\n")); */
    }
  
  D_RETURN;
}


static char      *
filetype_magic_test_perform(EfsdMagic *em, FILE *f)
{
  D_ENTER;

  if (!em || !f)
    { D_RETURN_(NULL); }

  fseek(f, em->offset, SEEK_SET);

  switch (em->type)
    {
    case EFSD_MAGIC_8:
      {
	u_int8_t val, val_test;

	val_test = *((u_int8_t*)em->value);

	fread(&val, sizeof(val), 1, f);
	if (em->use_mask)
	  {
	    val &= (u_int8_t)em->mask;
	  }

	switch (em->test)
	  {
	  case EFSD_MAGIC_TEST_EQUAL:
	    if (val == val_test)
	      {
		D(("Equality test ...succeeded.\n"));
		D_RETURN_(em->filetype);
	      }
	    break;
	  case EFSD_MAGIC_TEST_NOTEQUAL:
	    if (val != val_test)
	      { 
		D(("Unequality test ...succeeded.\n"));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  case EFSD_MAGIC_TEST_SMALLER:
	    if (val < val_test)
	      {
		D(("Smaller test ...succeeded.\n"));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  case EFSD_MAGIC_TEST_LARGER:
	    if (val > val_test)
	      {
		D(("Larger test ...succeeded.\n"));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  case EFSD_MAGIC_TEST_MASK:
	    if ((val & val_test) == val_test)
	      {
		D(("Mask test: %x == %x? succeeded.\n",
		   (val & val_test),
		   val_test));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  case EFSD_MAGIC_TEST_NOTMASK:
	    if ((val & val_test) == 0)
	      {
		D(("Notmask test succeeded.\n"));
		D_RETURN_(em->filetype);
	      }
	    break;
	  default:
	    D(("UNKNOWN test type!\n"));
	  }
      }
      break;
    case EFSD_MAGIC_16:
      {
	u_int16_t val, val_test;

	val_test = *((u_int16_t*)em->value);

	fread(&val, sizeof(val), 1, f);
	if (em->use_mask)
	  {
	    val &= (u_int16_t)em->mask;
	  }

	switch (em->test)
	  {
	  case EFSD_MAGIC_TEST_EQUAL:
	    if (val == val_test)
	      {
		D(("Equality test ...succeeded.\n"));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  case EFSD_MAGIC_TEST_NOTEQUAL:
	    if (val != val_test)
	      {
		D(("Unequality test ...succeeded.\n"));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  case EFSD_MAGIC_TEST_SMALLER:
	    if (val < val_test)
	      {
		D(("Smaller test ...succeeded.\n"));
		D_RETURN_(em->filetype);
	      }
	    break;
	  case EFSD_MAGIC_TEST_LARGER:
	    if (val > val_test)
	      {
		D(("Larger test ...succeeded.\n"));
		D_RETURN_(em->filetype);
	      }
	    break;
	  case EFSD_MAGIC_TEST_MASK:
	    if ((val & val_test) == val_test)
	      {
		D(("Mask test: %x == %x? succeeded.\n",
		   (val & val_test),
		   val_test));
		D_RETURN_(em->filetype);
	      }
	    break;
	  case EFSD_MAGIC_TEST_NOTMASK:
	    if ((val & val_test) == 0)
	      {
		D(("Notmask test ...succeeded.\n"));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  default:
	    D(("UNKNOWN test type!\n"));
	  }
      }
      break;
    case EFSD_MAGIC_32:
      {
	u_int32_t val, val_test;

	val_test = *((u_int32_t*)em->value);

	fread(&val, sizeof(val), 1, f);
	if (em->use_mask)
	  val &= (u_int32_t)em->mask;

	switch (em->test)
	  {
	  case EFSD_MAGIC_TEST_EQUAL:
	    if (val == val_test)
	      {
		D(("Long test: %x == %x succeeded.\n", val, *((u_int32_t*)em->value)));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  case EFSD_MAGIC_TEST_NOTEQUAL:
	    if (val != val_test)
	      {
		D(("Long test: %x != %x succeeded.\n", val, *((u_int32_t*)em->value)));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  case EFSD_MAGIC_TEST_SMALLER:
	    if (val < val_test)
	      {
		D(("Long test: %x < %x succeeded.\n", val, *((u_int32_t*)em->value)));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  case EFSD_MAGIC_TEST_LARGER:
	    if (val > val_test)
	      {
		D(("Long test: %x > %x succeeded.\n", val, *((u_int32_t*)em->value)));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  case EFSD_MAGIC_TEST_MASK:
	    if ((val & val_test) == val_test)
	      {
		D(("Long test: %x & %x succeeded.\n", val, *((u_int32_t*)em->value)));
		D_RETURN_(em->filetype); 
	      }
	    break;
	  case EFSD_MAGIC_TEST_NOTMASK:
	    if ((val & val_test) == 0)
	      {
		D(("Long test: %x & %x == 0 succeeded.\n", val, *((u_int32_t*)em->value)));
		D_RETURN_(em->filetype);
	      }
	    break;
	  default:
	    D(("UNKNOWN test type!\n"));
	  }
      }
      break;
    case EFSD_MAGIC_STRING:
      {
	int   i;
	char  s[MAXPATHLEN];

	for (i = 0; i < em->value_len; i++)
	  s[i] = fgetc(f);

	/* Fixme: add remaining string tests. */

	if (memcmp(s, em->value, em->value_len) == 0)
	  {
	    D(("String test for '%s', len = %i succeeded.\n", (char*)em->value, em->value_len));
	    D_RETURN_(em->filetype);
	  }
      }
      break;
    default:
    }

  D_RETURN_(NULL);
}


static char *
filetype_magic_test_level(EfsdMagic *level, FILE *f, char *ptr, char stop_when_found)
{
  EfsdMagic *em;
  char      *s, *ptr2;
  char      *result = NULL;

  D_ENTER;

  for (em = level; em; em = em->next)
    {
      if ((s = filetype_magic_test_perform(em, f)) != NULL)
	{
	  sprintf(ptr, "%s", s);
	  ptr = ptr + strlen(s);
	  result = ptr;

	  if ((ptr2 = filetype_magic_test_level(em->kids, f, ptr, FALSE)))
	    {
	      result = ptr = ptr2;
	    }

	  if (stop_when_found)
	    {
	      D_RETURN_(result);
	    }
	}
    }

  D_RETURN_(result);
}


static void
filetype_magic_init_level(char *key, char *ptr, EfsdMagic *em_parent)
{
  char        *item_ptr;
  int          i = 0, dummy;

  D_ENTER;

  for (i = 0; 1; i++)
    {
      sprintf(ptr, "/%i", i);
      item_ptr = ptr + strlen(ptr);

      sprintf(item_ptr, "%s", "/offset");

      if (e_db_int_get(magic_db, key, &dummy))
	{
	  EfsdMagic *em;

	  em = filetype_magic_new(key, item_ptr);
	  filetype_magic_add_child(em_parent, em);
	  filetype_magic_init_level(key, item_ptr, em);
	}
      else
	{
	  D_RETURN;
	}
    }
}


static int
filetype_test_fs(char *filename, struct stat *st, char *type, int len)
{
  char         *ptr;
  char          broken_link = FALSE;
  struct statfs stfs;
  int           fslen;

  D_ENTER;

  if (!st)
    D_RETURN_(FALSE);

#ifdef __EMX__
   snprintf(type, len, "%s", "hpfs");
#else
  if (statfs(filename, &stfs) < 0)
    {
      if (S_ISLNK(st->st_mode))
	{
	  char *lastslash;

	  lastslash = strrchr(filename, '/');

	  if (lastslash)
	    {
	      char old = *(lastslash+1);

	      *(lastslash+1) = '\0';
	      if (statfs(filename, &stfs) < 0)
		{
		  *(lastslash+1) = old;
		  D_RETURN_(FALSE);
		}

	      *(lastslash+1) = old;
	      broken_link = TRUE;
	    }
	}
      else
	{
	  D_RETURN_(FALSE);
	}
    }
#ifdef __FreeBSD__
  if (stfs.f_fstypename < 0)
    snprintf(type, len, "%s", "unknown-fs");
  else
    snprintf(type, len, "%s", stfs.f_fstypename);
#else
  switch (stfs.f_type)
    {
    case AFFS_SUPER_MAGIC:
      snprintf(type, len, "%s", "affs");
      break;
    case EXT_SUPER_MAGIC:
      snprintf(type, len, "%s", "ext");
    break;
    case EXT2_OLD_SUPER_MAGIC:
    case EXT2_SUPER_MAGIC:
      snprintf(type, len, "%s", "ext2");
      break;
    case HPFS_SUPER_MAGIC:
      snprintf(type, len, "%s", "hpfs");
      break;
    case ISOFS_SUPER_MAGIC:
      snprintf(type, len, "%s", "isofs");
      break;
    case MINIX_SUPER_MAGIC:
    case MINIX_SUPER_MAGIC2:
      snprintf(type, len, "%s", "minix");
      break;
    case MINIX2_SUPER_MAGIC:
    case MINIX2_SUPER_MAGIC2:
      snprintf(type, len, "%s", "minix-v2");
      break;
    case MSDOS_SUPER_MAGIC:
      snprintf(type, len, "%s", "msdos");
      break;
    case NCP_SUPER_MAGIC:
      snprintf(type, len, "%s", "novell");
      break;
    case NFS_SUPER_MAGIC:
      snprintf(type, len, "%s", "nfs");
      break;
    case PROC_SUPER_MAGIC:
      snprintf(type, len, "%s", "proc");
      break;
    case SMB_SUPER_MAGIC:
      snprintf(type, len, "%s", "smb");
      break;
    case XENIX_SUPER_MAGIC:
      snprintf(type, len, "%s", "xenix");
      break;
    case SYSV4_SUPER_MAGIC:
      snprintf(type, len, "%s", "sysv4");
      break;
    case SYSV2_SUPER_MAGIC:
      snprintf(type, len, "%s", "sysv2");
      break;
    case COH_SUPER_MAGIC:
      snprintf(type, len, "%s", "coh");
      break;
    case UFS_MAGIC:
      snprintf(type, len, "%s", "ufs");
      break;
    case _XIAFS_SUPER_MAGIC:
      snprintf(type, len, "%s", "xia");
      break;
    case NTFS_SUPER_MAGIC:
      snprintf(type, len, "%s", "ntfs");
      break;
    case XFS_SUPER_MAGIC:
      snprintf(type, len, "%s", "xfs");
      break;
    case REISERFS_SUPER_MAGIC:
      snprintf(type, len, "%s", "reiserfs");
      break;
    default:
      snprintf(type, len, "%s", "unknown-fs");
    }
#endif
#endif

  fslen = strlen(type);
  ptr = type + fslen;
    
  if (S_ISLNK(st->st_mode))
    {
      if (broken_link)	
	snprintf(ptr, len - fslen, "%s", "/link/broken");
      else
	snprintf(ptr, len - fslen, "%s", "/link");
    }
  else if (S_ISDIR(st->st_mode))
    {
      snprintf(ptr, len - fslen, "%s", "/dir");
    }
  else if (S_ISCHR(st->st_mode))
    {
      snprintf(ptr, len - fslen, "%s", "/chardev");
    }
#ifndef __EMX__
  else if (S_ISBLK(st->st_mode))
    {
      snprintf(ptr, len - fslen, "%s", "/block");
    }
#endif
  else if (S_ISFIFO(st->st_mode))
    {
      snprintf(ptr, len - fslen, "%s", "/fifo");
    }
  else if (S_ISSOCK(st->st_mode))
    {
      snprintf(ptr, len - fslen, "%s", "/socket");
    }
  else
    {
      /* If it's not a specific file type,
	 the fs test should fail! */
      D_RETURN_(FALSE);
    }

  D_RETURN_(TRUE);
}


static int
filetype_test_magic(char *filename, char *type, int len)
{
  FILE        *f = NULL;
  char        *result = NULL;
  char         s[MAXPATHLEN];

  D_ENTER;

  if ((f = fopen(filename, "r")) == NULL)
    { D_RETURN_(FALSE); }
  
  result = filetype_magic_test_level(magic.kids, f, s, TRUE);

  fclose(f);

  if (result)
    {
      int last;

      last = strlen(s)-1;

      if (s[last] == '-' || s[last] == '/')
	s[last] = '\0';

      strncpy(type, s, len);

      D_RETURN_(TRUE);
    }

  D_RETURN_(FALSE);
}


static int
filetype_patterns_init(void)
{
  char      *s;
  E_DB_File *db;
  int        i;
 
  D_ENTER;

  s = filetype_get_user_patterns_db();

  if (s)
    {
      patterns_user = e_db_dump_key_list(s, &num_patterns_user);
      
      if (num_patterns_user > 0)
	{
	  pattern_filetypes_user = malloc(sizeof(char*) * num_patterns_user);
	  
	  db = e_db_open_read(s);
	  
	  for (i = 0; i < num_patterns_user; i++)
	    pattern_filetypes_user[i] = e_db_str_get(db, patterns_user[i]);
	  
	  e_db_close(db);
	}  
    }
  else
    {
      num_patterns_user = 0;
      D(("User pattern db not found.\n"));
    }

  s = filetype_get_sys_patterns_db();

  if (!s)
    {
      D(("System pattern db not found.\n"));
      D_RETURN_(0);
    }

  D(("System pattern db at %s\n", s));

  if (s)
    {
      patterns = e_db_dump_key_list(s, &num_patterns);

      if (num_patterns > 0)
	{
	  pattern_filetypes = malloc(sizeof(char*) * num_patterns);
	  
	  D(("opening '%s'\n", s));
	  db = e_db_open_read(s);
      
	  for (i = 0; i < num_patterns; i++)
	    pattern_filetypes[i] = e_db_str_get(db, patterns[i]);
      
	  e_db_close(db);
	}  
    }

  D(("%i keys in user pattern db, %i keys in system pattern db.\n",
     num_patterns_user, num_patterns));

  D_RETURN_(1);
}


static int
filetype_test_pattern(char *filename, char *type, int len)
{
  char *ptr;
  int   i;

  D_ENTER;

  if (!filename)
    D_RETURN_(FALSE);

  /* Test user-defined patterns first: */

  for (i = 0; i < num_patterns_user; i++)
    {
      ptr = strrchr(filename, '/');
      if (!ptr)
	ptr = filename;
      else
	ptr++;

      if (!fnmatch(patterns_user[i], ptr, FNM_PATHNAME | FNM_PERIOD))
	{
	  strncpy(type, pattern_filetypes_user[i], len);
	  D_RETURN_(TRUE);
	}
    }

  /* If not found, use system-wide definitions. */

  for (i = 0; i < num_patterns; i++)
    {
      ptr = strrchr(filename, '/');
      if (!ptr)
	ptr = filename;
      else
	ptr++;

      if (!fnmatch(patterns[i], ptr, FNM_PATHNAME | FNM_PERIOD))
	{
	  strncpy(type, pattern_filetypes[i], len);
	  D_RETURN_(TRUE);
	}
    }

  D_RETURN_(FALSE);
}


static void       
filetype_cache_init(void)
{
  D_ENTER;

  filetype_cache = efsd_hash_new(1023, 10, (EfsdHashFunc)efsd_hash_string,
				 (EfsdCmpFunc)strcmp,
				 filetype_hash_item_free);

  D_RETURN;
}


static void       
filetype_cache_update(char *filename, time_t time,
		      const char *filetype)
{
  EfsdFiletypeCacheItem *it;

  D_ENTER;

  it = filetype_cache_lookup(filename);

  efsd_lock_get_write_access(filetype_lock);

  if (it)
    {
      if (time > it->time)
	{
	  FREE(it->filetype);
	  it->filetype = strdup(filetype);
	  it->time = time;
	}
    }
  else
    {
      char *key = strdup(filename);

      it = NEW(EfsdFiletypeCacheItem);
      it->filetype = strdup(filetype);
      it->time = time;

      if (!efsd_hash_insert(filetype_cache, key, it))
	{
	  FREE(key);
	  FREE(it->filetype);
	  FREE(it);
	}
    }

  efsd_lock_release_write_access(filetype_lock);

  D_RETURN;
}


static EfsdFiletypeCacheItem *
filetype_cache_lookup(char *filename)
{
  EfsdFiletypeCacheItem *filetype_it;

  D_ENTER;

  filetype_it = efsd_hash_find(filetype_cache, filename);

  D_RETURN_(filetype_it);
}


static void
filetype_hash_item_free(EfsdHashItem *it)
{
  EfsdFiletypeCacheItem *filetype_it;

  D_ENTER;

  filetype_it = (EfsdFiletypeCacheItem*)it->data;

  FREE(filetype_it->filetype);
  FREE(it->data);
  FREE(it->key);
  FREE(it);

  D_RETURN;
}


int       
efsd_filetype_init(void)
{
  char        key[MAXPATHLEN];
  char       *ptr;

  D_ENTER;

  ptr = filetype_get_magic_db();
  
  if (!ptr)
    {
      D(("System magic db not found.\n"));
      D_RETURN_(0);
    }
     
  magic_db = e_db_open_read(ptr);

  if (!magic_db)
    { 
      D(("Could not open magic db!\n"));
      D_RETURN_(0);
    }

  memset(&magic, 0, sizeof(EfsdMagic));
  ptr = key;

  filetype_magic_init_level(key, ptr, &magic);
  e_db_close(magic_db);

  filetype_cache_init();
  filetype_lock = efsd_lock_new();

  D_RETURN_(filetype_patterns_init());
}


void       
efsd_filetype_cleanup(void)
{
  int i;

  D_ENTER;

  filetype_magic_cleanup_level(&magic);
  magic.kids = NULL;

  for (i = 0; i < num_patterns; i++)
    {
      FREE(patterns[i]);
      FREE(pattern_filetypes[i]);
    }

  FREE(patterns);
  FREE(pattern_filetypes);

  efsd_lock_free(filetype_lock);
  filetype_lock = NULL;

  D_RETURN;
}


int
efsd_filetype_get(char *filename, char *type, int len)
{
  struct stat     st;
  char            realfile[MAXPATHLEN];
  EfsdFiletypeCacheItem *cached_result = NULL;

  D_ENTER;

  efsd_misc_remove_trailing_slashes(filename);

  /* Okay -- if filetype is in cache, check file
     modification time to see if regeneration of
     filetype is necessary.
  */


  if (!efsd_lstat(filename, &st))
    {
      /* Ouch -- couldn't stat the file. Testing doesn't
	 make much sense now. */
      D_RETURN_(FALSE);
    }

  /* If it's a link, get stat of link target instead */
  if (S_ISLNK(st.st_mode))
    {
      if (realpath(filename, realfile))
	{
	  filename = realfile;

	  if (!efsd_stat(filename, &st))
	    {
	      strncpy(type, unknown_string, len);
	      D_RETURN_(TRUE);
	    }

	  D(("Link substitution succeeded.\n"));
	}
      else
	{
	  D(("Link substitution failed.\n"));
	}
    }

  efsd_lock_get_read_access(filetype_lock);
  cached_result = filetype_cache_lookup(filename);

  if (cached_result)
    {
      D(("Cached result found for %s\n", filename));
      if (cached_result->time == st.st_mtime)
	{
	  /* File has not been changed -- use cached value. */
	  D(("Using cached filetype on %s\n", filename));
	  strncpy(type, cached_result->filetype, len);
	  efsd_lock_release_read_access(filetype_lock);
	  D_RETURN_(TRUE);
	}
    }

  efsd_lock_release_read_access(filetype_lock);
  D(("Calculating filetype on %s\n", filename));

  /* Filetype is not in cache or file has been modified, re-test: */

  if(filetype_test_fs(filename, &st, type, len))
    {
      filetype_cache_update(filename, st.st_mtime, type);
      D_RETURN_(TRUE);
    }

  D(("magic: fs check failed.\n"));

  if (filetype_test_magic(filename, type, len))
    {
      filetype_cache_update(filename, st.st_mtime, type);
      D_RETURN_(TRUE);
    }

  D(("magic: data check failed.\n"));

  if (filetype_test_pattern(filename, type, len))
    {
      filetype_cache_update(filename, st.st_mtime, type);      
      D_RETURN_(TRUE);
    }

  D(("magic: file pattern check failed.\n"));

  strncpy(type, unknown_string, len);
  filetype_cache_update(filename, st.st_mtime, unknown_string);

  D_RETURN_(TRUE);
}


