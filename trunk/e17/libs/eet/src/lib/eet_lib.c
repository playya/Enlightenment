#include "Eet.h"
#include "Eet_private.h"

#ifdef HAVE_REALPATH
#undef HAVE_REALPATH
#endif

#define EET_MAGIC_FILE                  0x1ee7ff00
#define EET_MAGIC_FILE_HEADER           0x1ee7ff01
#define EET_MAGIC_FILE_NODE             0x1ee7ff02
#define EET_MAGIC_FILE_DIRECTORY        0x1ee7ff03
#define EET_MAGIC_FILE_DIRECTORY_HASH   0x1ee7ff04   

typedef struct _Eet_File_Header         Eet_File_Header;
typedef struct _Eet_File_Node           Eet_File_Node;
typedef struct _Eet_File_Directory      Eet_File_Directory;
typedef struct _Eet_File_Directory_Hash Eet_File_Directory_Hash;

struct _Eet_File
{
   int              magic;
   int              references;
   
   char            *path;
   char            *real_path;
   
   FILE            *fp;
   Eet_File_Mode    mode;
   
   int              writes_pending : 1;
   
   Eet_File_Header *header;
};

struct _Eet_File_Header
{
   int                 magic;
   Eet_File_Directory *directory;
};

struct _Eet_File_Node
{
   char *name;
   int   offset;
   int   compression;
   int   size;
   int   data_size;
   void *data;
};

struct _Eet_File_Directory
{
   int                       size;
   Eet_File_Directory_Hash  *hash;
};

struct _Eet_File_Directory_Hash
{
   int            size;
   Eet_File_Node *node;
};

#if 0
/* NB: all int's are stored in network byte order on disk */
/* file format: */
int magic; /* magic number ie 0x1ee7ff00 */
int num_directory_entries; /* number of directory entries to follow */
int bytes_directory_entries; /* bytes of directory entries to follow */
struct 
{
   int offset; /* bytes offset into file for data chunk */
   int flags; /* flags - for now 0 = uncompressed, 1 = compressed */
   int size; /* size of the data chunk */
   int data_size; /* size of the (uncompressed) data chunk */
   int name_size; /* length in bytes of the name field */
   char name[name_size]; /* name string (variable length) */
} directory[num_directory_entries];
/* and now startes the data stream... */
#endif	  

/* prototypes of internal calls */
static Eet_File *eet_cache_find(char *real_path, Eet_File **cache, int cache_num);
static void      eet_cache_add(Eet_File *ef, Eet_File ***cache, int *cache_num);
static void      eet_cache_del(Eet_File *ef, Eet_File ***cache, int *cache_num);
static int       eet_string_match(char *s1, char *s2);
static int       eet_hash_gen(char *key, int hash_size);
static void      eet_flush(Eet_File *ef);

/* cache. i don't expect this to ever be large, so arrays will do */
static int        eet_writers_num = 0;
static Eet_File **eet_writers     = NULL;
static int        eet_readers_num = 0;
static Eet_File **eet_readers     = NULL;
static int        eet_initcount   = 0;

/* find an eet file in the currently in use cache */
static Eet_File *
eet_cache_find(char *real_path, Eet_File **cache, int cache_num)
{
   int i;
   
   /* walk list */
   for (i = 0; i < cache_num; i++)
     {
	/* if matches real path - return it */
	if (eet_string_match(cache[i]->real_path, real_path)) return cache[i];
     }
   /* not found */
   return NULL;
}

/* add to end of cache */
static void
eet_cache_add(Eet_File *ef, Eet_File ***cache, int *cache_num)
{
   Eet_File **new_cache;
   int new_cache_num;
   
   new_cache_num = *cache_num;
   new_cache = *cache;
   new_cache_num++;
   new_cache = realloc(new_cache, new_cache_num * sizeof(Eet_File *));
   if (!new_cache)
     {
	fprintf(stderr, "BAD ERROR! Eet realloc of cache list failed. Abort\n");
	abort();
     }
   if (!new_cache) return;
   new_cache[new_cache_num - 1] = ef;
   *cache = new_cache;
   *cache_num = new_cache_num;   
}

/* delete from cache */
static void
eet_cache_del(Eet_File *ef, Eet_File ***cache, int *cache_num)
{
   Eet_File **new_cache;
   int new_cache_num;
   int i, j;

   new_cache_num = *cache_num;
   new_cache = *cache;
   if (new_cache_num <= 0)
     {
	return;
     }
   for (i = 0; i < new_cache_num; i++)
     {
	if (new_cache[i] == ef) break;
     }
   if (i >= new_cache_num)
     {
	return;
     }
   new_cache_num--;
   for (j = i; j < new_cache_num; j++) new_cache[j] = new_cache[j + 1];   
   if (new_cache_num > 0)
     {
        new_cache = realloc(new_cache, new_cache_num * sizeof(Eet_File *));
	if (!new_cache)
	  {
	     fprintf(stderr, "BAD ERROR! Eet realloc of cache list failed. Abort\n");
	     abort();
	  }
     }
   else
     {
	free(new_cache);
	new_cache = NULL;
     }
   *cache_num = new_cache_num;   
   *cache = new_cache;
}

/* internal string match. bails out at first mismatch - not comparing all */
/* bytes in strings */
static int
eet_string_match(char *s1, char *s2)
{
   /* both null- no match */
   if ((!s1) || (!s2)) return 0;
   /* go thru - first mismatch - exit with 0 */
   do
     {
	if (*s1 != *s2) return 0;
	s1++;
	s2++;
     }   
   while ((*s1) || (*s2));
   /* got this far. match */
   return 1;
}

/* caluclate hash table entry valu with bitmask size of hash_size */
static int
eet_hash_gen(char *key, int hash_size)
{
   int hash_num = 0;
   unsigned char *ptr;
   const int masks[9] =
     {
	0x00,
	0x01,
	0x03,
	0x07,
	0x0f,
	0x1f,
	0x3f,
	0x7f,
	0xff
     };
   
   /* no string - index 0 */
   if (!key) return 0;
   
   /* calc hash num */
   for (ptr = key; *ptr; ptr++) hash_num ^= (int)(*ptr);
   
   /* mask it */
   hash_num &= masks[hash_size];
   /* return it */
   return hash_num;
}

/* flush out writes to an eet file */
static void
eet_flush(Eet_File *ef)
{
   int i, j, count, size, num, offset;
   int head[3];
   unsigned long int i1, i2;
   
   /* check to see its' an eet file pointer */   
   if ((!ef) || (ef->magic != EET_MAGIC_FILE))
     return;
   if (!ef->header) return;
   if (!ef->header->directory) return;
   if ((ef->mode != EET_FILE_MODE_WRITE) && (ef->mode != EET_FILE_MODE_RW)) return;
   if (!ef->writes_pending) return;

   /* calculate total size in bytes of directory block */
   size = 0;
   count = 0;
   num = (1 << (ef->header->directory->size - 1));
   for (i = 0; i < num; i++)
     {
	for (j = 0; j < ef->header->directory->hash[i].size; j++)
	  {
	     if (ef->header->directory->hash[i].node[j].compression >= 0)
	       {  
		  size += 20 + strlen(ef->header->directory->hash[i].node[j].name);
		  count++;
	       }
	  }
     }
   /* caluclate offsets per entry */
   offset = 0;
   for (i = 0; i < num; i++)
     {
	for (j = 0; j < ef->header->directory->hash[i].size; j++)
	  {
             if (ef->header->directory->hash[i].node[j].compression >= 0)
	       {
		  ef->header->directory->hash[i].node[j].offset = 12 + size + offset;
		  offset += ef->header->directory->hash[i].node[j].size;
	       }
	  }
     }
   /* go thru and write the header */
   i1 = (unsigned long int)EET_MAGIC_FILE;
   i2 = htonl(i1);
   head[0] = (int)i2;
   i1 = (unsigned long int)count;
   i2 = htonl(i1);
   head[1] = (int)i2;
   i1 = (unsigned long int)size;
   i2 = htonl(i1);
   head[2] = (int)i2;
   fseek(ef->fp, 0, SEEK_SET);
   if (fwrite(head, 12, 1, ef->fp) != 1) return;
   offset = 12;
   for (i = 0; i < num; i++)
     {
	for (j = 0; j < ef->header->directory->hash[i].size; j++)
	  {
	     unsigned char *buf;
	     int buf_size;
	     int name_size;

	     if (ef->header->directory->hash[i].node[j].compression >= 0)
	       {
		  name_size = strlen(ef->header->directory->hash[i].node[j].name);
		  buf_size = 20 + name_size;
		  buf = malloc(buf_size);
		  if (!buf) return;
		  i1 = (unsigned long int)ef->header->directory->hash[i].node[j].offset;
		  i2 = htonl(i1);
		  *((int *)(buf + 0)) = (int)i2;
		  i1 = (unsigned long int)ef->header->directory->hash[i].node[j].compression;
		  i2 = htonl(i1);
		  *((int *)(buf + 4)) = (int)i2;
		  i1 = (unsigned long int)ef->header->directory->hash[i].node[j].size;
		  i2 = htonl(i1);
		  *((int *)(buf + 8)) = (int)i2;
		  i1 = (unsigned long int)ef->header->directory->hash[i].node[j].data_size;
		  i2 = htonl(i1);
		  *((int *)(buf + 12)) = (int)i2;
		  i1 = (unsigned long int)name_size;
		  i2 = htonl(i1);
		  *((int *)(buf + 16)) = (int)i2;
		  memcpy(buf + 20, ef->header->directory->hash[i].node[j].name, name_size);
		  if (fwrite(buf, buf_size, 1, ef->fp) != 1) 
		    {
		       free(buf);
		       return;
		    }
		  offset += buf_size;
		  free(buf);
	       }
	  }
     }
   /* write data */
   for (i = 0; i < num; i++)
     {
	for (j = 0; j < ef->header->directory->hash[i].size; j++)
	  {
	     if (ef->header->directory->hash[i].node[j].compression >= 0)
	       {
		  if (fwrite(ef->header->directory->hash[i].node[j].data, 
			     ef->header->directory->hash[i].node[j].size,
			     1, ef->fp) != 1)
		    return;
	       }
	  }
     }
   /* no more writes pending */
   ef->writes_pending = 0;
}

Eet_File *
eet_open(char *file, Eet_File_Mode mode)
{
   Eet_File *ef;
   char buf[PATH_MAX];

   if (!file) return NULL;

#ifdef HAVE_REALPATH
   /* in case this is a symlink... find out where it REALLY points */
   if (!realpath(file, buf)) 
     {
	if (mode == EET_FILE_MODE_READ) return NULL;
     }
#else
   strncpy(buf, file, sizeof(buf));
   buf[sizeof(buf) - 1] = 0;
#endif

   /* find the current file handle in cache*/
   ef = NULL;
   if (mode == EET_FILE_MODE_READ)
     ef = eet_cache_find(buf, eet_readers, eet_readers_num);
   else if ((mode == EET_FILE_MODE_WRITE) || (mode == EET_FILE_MODE_RW))
     ef = eet_cache_find(buf, eet_writers, eet_writers_num);
   /* we found one */
   if (ef)
     {
	/* reference it up and return it */
	ef->references++;
	return ef;
     }
   
   /* allocate struct for eet file and have it zero'd out */
   ef = calloc(sizeof(Eet_File), 1);
   if (!ef) return NULL;
   
   /* fill some of the members */
   ef->path = strdup(file);
   ef->real_path = strdup(buf);
   ef->magic = EET_MAGIC_FILE;
   ef->references = 1;
   ef->mode = mode;

   /* try open the file based on mode */
   if ((ef->mode == EET_FILE_MODE_READ) || (ef->mode == EET_FILE_MODE_RW))
     ef->fp = fopen(ef->path, "rb");
   else if (ef->mode == EET_FILE_MODE_WRITE)
     {
	/* opening for write - delete old copy of file right away */
	unlink(ef->real_path);
	ef->fp = fopen(ef->path, "wb");
     }
   else
     {
	eet_close(ef);
	return NULL;
     }
   
   /* if we can't open - bail out */
   if (!ef->fp)
     {
	eet_close(ef);
	return NULL;
     }
   
   /* if we opened for read or read-write */
   if ((mode == EET_FILE_MODE_READ) || (mode == EET_FILE_MODE_RW))
     {
	unsigned char buf[12];
	unsigned char *dyn_buf, *p;
	unsigned long int i1, i2;
	int num_entries, byte_entries, i;
	size_t count;
	
	/* build header table if read mode */
	/* geat header */
	count = fread(buf, 12, 1, ef->fp);
	if (count != 1)
	  {
	     eet_close(ef);
	     return NULL;	     
	  }
	/* get magic no */
	memcpy(&i1, buf + 0, sizeof(int));
	i2 = ntohl(i1);
	if (i2 != EET_MAGIC_FILE)
	  {
	     eet_close(ef);
	     return NULL;	     
	  }
	/* get entries count and byte count */
	memcpy(&i1, buf + 4, sizeof(int));
	i2 = ntohl(i1);
	num_entries = (int)i2;
	memcpy(&i1, buf + 8, sizeof(int));
	i2 = ntohl(i1);
	byte_entries = (int)i2;
	/* we cant have <= 0 values here - invalid */
	if ((num_entries <= 0) || (byte_entries <= 0))
	  {
	     eet_close(ef);
	     return NULL;	     
	  }
	/* we can't have more entires than minimum bytes for those! invalid! */
	if ((num_entries * 20) > byte_entries)
	  {
	     eet_close(ef);
	     return NULL;	     
	  }
	/* allocate dynamic buffer for entire directory block */
	dyn_buf = malloc(byte_entries);
	if (!dyn_buf)
	  {
	     eet_close(ef);
	     return NULL;	     
	  }
	/* allocate header */
	ef->header = calloc(sizeof(Eet_File_Header), 1);
	if (!ef->header)
	  {
	     free(dyn_buf);
	     eet_close(ef);
	     return NULL;
	  }
	ef->header->magic = EET_MAGIC_FILE_HEADER;
	/* allocate directory block in ram */
	ef->header->directory = calloc(sizeof(Eet_File_Directory), 1);
	if (!ef->header->directory)
	  {
	     free(dyn_buf);
	     eet_close(ef);
	     return NULL;
	  }
	/* 8 bit hash table (256 buckets) */
	ef->header->directory->size = 8;
	/* allocate base hash table */
	ef->header->directory->hash = calloc(sizeof(Eet_File_Directory_Hash), (1 << (ef->header->directory->size - 1)));
	if (!ef->header->directory->hash)
	  {
	     free(dyn_buf);
	     eet_close(ef);
	     return NULL;
	  }
	/* actually read the directory block - all of it, into ram */
	count = fread(dyn_buf, byte_entries, 1, ef->fp);
	if (count != 1)
	  {
	     free(dyn_buf);
	     eet_close(ef);
	     return NULL;
	  }
	/* parse directory block */
	p = dyn_buf;
	for (i = 0; i < num_entries; i++)
	  {
	     int offset;
	     int flags;
	     int size;
	     int data_size;
	     int name_size;
	     char *name;
	     int hash;
	     Eet_File_Node *node;
	     int node_size;
	     void *data = NULL;
	     
	     /* out directory block is inconsistent - we have oveerun our */
	     /* dynamic block buffer before we finished scanning dir entries */	     
	     if (p >= (dyn_buf + byte_entries))
	       {
		  free(dyn_buf);
		  eet_close(ef);
		  return NULL;
	       }
	     /* get entrie header */
	     memcpy(&i1, p + 0, sizeof(int));
	     i2 = ntohl(i1);
	     offset = (int)i2;
	     memcpy(&i1, p + 4, sizeof(int));
	     i2 = ntohl(i1);
	     flags = (int)i2;
	     memcpy(&i1, p + 8, sizeof(int));
	     i2 = ntohl(i1);
	     size = (int)i2;
	     memcpy(&i1, p + 12, sizeof(int));
	     i2 = ntohl(i1);
	     data_size = (int)i2;
	     memcpy(&i1, p + 16, sizeof(int));
	     i2 = ntohl(i1);
	     name_size = (int)i2;
	     /* invalid size */
	     if (size <= 0)
	       {
		  free(dyn_buf);
		  eet_close(ef);
		  return NULL;		  
	       }
	     /* invalid name_size */
	     if (name_size <= 0)
	       {
		  free(dyn_buf);
		  eet_close(ef);
		  return NULL;		  
	       }
	     /* reading name would mean falling off end of dyn_buf - invalid */
	     if ((p + 16 + name_size) > (dyn_buf + byte_entries))
	       {
		  free(dyn_buf);
		  eet_close(ef);
		  return NULL;		  
	       }
	     /* allocate name string */
	     name = malloc(name_size + 1);
	     if (!name)
	       {
		  free(dyn_buf);
		  eet_close(ef);
		  return NULL;		  
	       }
	     /* copy name in and terminate it */
	     strncpy(name, p + 20, name_size);
	     name[name_size] = 0;
	     /* get hask bucket it should go in */
	     hash = eet_hash_gen(name, ef->header->directory->size);
	     /* resize hask bucket */
	     node = realloc(ef->header->directory->hash[hash].node, 
			    (ef->header->directory->hash[hash].size  + 1) * 
			    sizeof(Eet_File_Node));
	     if (!node)
	       {
		  free(dyn_buf);
		  eet_close(ef);
		  return NULL;
	       }
	     /* current node size */
	     node_size = ef->header->directory->hash[hash].size;
	     /* resized node list set up */
	     ef->header->directory->hash[hash].node = node;
	     /* new node at end */
	     ef->header->directory->hash[hash].node[node_size].name = name;
	     ef->header->directory->hash[hash].node[node_size].offset = offset;
	     ef->header->directory->hash[hash].node[node_size].compression = flags;
	     ef->header->directory->hash[hash].node[node_size].size = size;
	     ef->header->directory->hash[hash].node[node_size].data_size = data_size;

	     /* read-only mode, so currently we have no data loaded */
	     if (mode == EET_FILE_MODE_READ)
	       {
		  ef->header->directory->hash[hash].node[node_size].data = NULL;
	       }
	     /* read-write mode - read everything into ram */
	     else
	       {
		  data = malloc(size);
		  if (data)
		    {
		       if (fseek(ef->fp, ef->header->directory->hash[hash].node[node_size].offset, SEEK_SET) < 0)
			 {
			    free(data);
			    data = NULL;
			    /* XXX die gracefully somehow */
			    break;
			 }
		       if (fread(data, size, 1, ef->fp) != 1)
			 {
			    free(data);
			    data = NULL;
			    /* XXX die gracefully somehow */
			    break;
			 }
		    }
		  
                  ef->header->directory->hash[hash].node[node_size].data = data;
	       }

	     /* increment number of nodes */
	     ef->header->directory->hash[hash].size++;
	     /* advance */
	     p += 20 + name_size;
	  }
	/* done - free dynamic buffer */
	free(dyn_buf);
     }

   /* we need to delete the original file in read-write mode and re-open for writing */
   if (ef->mode == EET_FILE_MODE_RW)
     {
	fclose(ef->fp);
	unlink(ef->real_path);
	ef->fp = fopen(ef->path, "wb");
     }

   /* add to cache */
   if (ef->mode == EET_FILE_MODE_READ)
     eet_cache_add(ef, &eet_readers, &eet_readers_num);
   else if ((ef->mode == EET_FILE_MODE_WRITE) || (ef->mode == EET_FILE_MODE_RW))
     eet_cache_add(ef, &eet_writers, &eet_writers_num);
   return ef;
}

void
eet_close(Eet_File *ef)
{
   /* check to see its' an eet file pointer */   
   if ((!ef) || (ef->magic != EET_MAGIC_FILE))
     return;
   /* deref */
   ef->references--;
   /* if its still referenced - dont go any further */
   if (ef->references > 0) return;
   /* remove from cache */
   if (ef->mode == EET_FILE_MODE_READ)
     eet_cache_del(ef, &eet_readers, &eet_readers_num);
   else if ((ef->mode == EET_FILE_MODE_WRITE) || (ef->mode == EET_FILE_MODE_RW))
     eet_cache_del(ef, &eet_writers, &eet_writers_num);
   /* flush any writes */
   eet_flush(ef);

   /* free up members */
   if (ef->fp) fclose(ef->fp);
   if (ef->path) free(ef->path);
   if (ef->real_path) free(ef->real_path);

   /* free up data */
   if (ef->header) 
     {
	if (ef->header->directory)
	  {
	     if (ef->header->directory->hash)
	       {
		  int i, num;
		  
		  num = (1 << (ef->header->directory->size - 1));
		  for (i = 0; i < num; i++)
		    {
		       if (ef->header->directory->hash[i].node)
			 {
			    int j;
			    int num2;
			    
			    num2 = ef->header->directory->hash[i].size;
			    for (j = 0; j < num2; j++)
			      {
				 if (ef->header->directory->hash[i].node[j].name)
				   free(ef->header->directory->hash[i].node[j].name);
				 if (ef->header->directory->hash[i].node[j].data)
				   free(ef->header->directory->hash[i].node[j].data);
			      }
			    free(ef->header->directory->hash[i].node);
			 }
		    }
		  free(ef->header->directory->hash);
	       }
	     free(ef->header->directory);
	  }
	free(ef->header);
     }
   
   /* zero out ram for struct - caution tactic against stale memory use */
   memset(ef, 0, sizeof(Eet_File));
   /* free it */
   free(ef);
}

void *
eet_read(Eet_File *ef, char *name, int *size_ret)
{
   void *data = NULL;
   int   size = 0, tmp_size;
   int   hash, i, num;

   /* check to see its' an eet file pointer */
   if ((!ef) || (ef->magic != EET_MAGIC_FILE) || (!name))
     {
	if (size_ret) *size_ret = 0;
	return NULL;
     }
   /* get hash bucket this should be in */
   hash = eet_hash_gen(name, ef->header->directory->size);
   /* no header, return NULL */
   if (!ef->header) return NULL;
   /* no directory, return NULL */
   if (!ef->header->directory) return NULL;
   /* hunt hash bucket */
   num = ef->header->directory->hash[hash].size;
   for (i = 0; i < num; i++)
     {
	/* if it matches */
	if (eet_string_match(ef->header->directory->hash[hash].node[i].name, name))
	  {
	     /* uncompressed data */
	     if (ef->header->directory->hash[hash].node[i].compression == 0)
	       {
		  /* get size */
		  size = ef->header->directory->hash[hash].node[i].size;
		  /* allocate data */
		  data = malloc(size);
		  if (data)
		    {
		       /* if we alreayd have the data in ram... copy that */
		       if (ef->header->directory->hash[hash].node[i].data)
			 memcpy(data, 
				ef->header->directory->hash[hash].node[i].data, 
				ef->header->directory->hash[hash].node[i].size);
		       /* or get data from disk */
		       else
			 {
			    /* seek to data location */
			    if (fseek(ef->fp, ef->header->directory->hash[hash].node[i].offset, SEEK_SET) < 0)
			      {
				 free(data);
				 data = NULL;
				 break;
			      }
			    /* read it */
			    if (fread(data, size, 1, ef->fp) != 1)
			      {
				 free(data);
				 data = NULL;
				 break;
			      }
			 }
		    }
		  break;
	       }
	     /* compressed data */
	     else
	       {
		  void *tmp_data;
		  
		  /* get size of data in file */
		  tmp_size = ef->header->directory->hash[hash].node[i].size;
		  tmp_data = malloc(tmp_size);
		  if (!tmp_data) break;
		  /* get size uncompressed */
		  size = ef->header->directory->hash[hash].node[i].data_size;
		  /* allocate data */
		  data = malloc(size);
		  if (data)
		    {
		       uLongf dlen;
		       
		       /* if we already have the data in ram... copy that */
		       if (ef->header->directory->hash[hash].node[i].data)
			 memcpy(tmp_data, 
				ef->header->directory->hash[hash].node[i].data, 
				tmp_size);
		       /* or get data from disk */
		       else
			 {
			    /* seek to data location */
			    if (fseek(ef->fp, ef->header->directory->hash[hash].node[i].offset, SEEK_SET) < 0)
			      {
				 free(tmp_data);
				 free(data);
				 data = NULL;
				 break;
			      }
			    /* read it */
			    if (fread(tmp_data, tmp_size, 1, ef->fp) != 1)
			      {
				 free(tmp_data);
				 free(data);
				 data = NULL;
				 break;
			      }
			 }
		       /* decompress it */
		       dlen = size;
		       if (uncompress((Bytef *)data, &dlen, 
				      tmp_data, (uLongf)tmp_size))
			 {
			    free(tmp_data);
			    free(data);
			    data = NULL;
			    break;
			 }
		    }
		  free(tmp_data);
		  break;
	       }
	  }
     }
   /* fill in return values */
   *size_ret = size;
   /* update access time */
   return data;
}

int
eet_write(Eet_File *ef, char *name, void *data, int size, int compress)
{
   int data_size;
   int hash, node_size;
   Eet_File_Node *node;
   char *name2;
   void *data2;
   int exists_already = 0;
   
   /* check to see its' an eet file pointer */   
   if ((!ef) || (ef->magic != EET_MAGIC_FILE)
       || (!name) || (!data) || (size <= 0) || 
       ((ef->mode != EET_FILE_MODE_WRITE) &&
        (ef->mode != EET_FILE_MODE_RW)))
	
     return 0;

   if (!ef->header)
     {
	/* allocate header */
	ef->header = calloc(sizeof(Eet_File_Header), 1);
	if (!ef->header) return 0;
	ef->header->magic = EET_MAGIC_FILE_HEADER;
	/* allocate directory block in ram */
	ef->header->directory = calloc(sizeof(Eet_File_Directory), 1);
	if (!ef->header->directory) return 0;
	/* 8 bit hash table (256 buckets) */
	ef->header->directory->size = 8;
	/* allocate base hash table */
	ef->header->directory->hash = calloc(sizeof(Eet_File_Directory_Hash), (1 << (ef->header->directory->size - 1)));
	if (!ef->header->directory->hash) return 0;
     }
   /* figure hash bucket */
   hash = eet_hash_gen(name, ef->header->directory->size);
   node_size = ef->header->directory->hash[hash].size;
   /* dup name */
   name2 = strdup(name);
   if (!name2) return 0;
   /* dup data */
   data_size = size;
   /* have bigger buffer for compress */
   if (compress == 1)
     data_size = 12 + ((size * 101) / 100);
   data2 = malloc(data_size);
   if (!data2)
     {
	free(name2);
	return 0;
     }
   /* if we want to compress */
   if (compress == 1)
     {
	uLongf buflen;
	int ok;
	  
	/* compress the data with max compression */
	buflen = (uLongf)data_size;
	if ((ok = compress2((Bytef *)data2, &buflen, (Bytef *)data, 
			   (uLong)size, 9)) != Z_OK)
	  {
	     free(name2);
	     free(data2);
	     return 0;
	  }
	/* record compressed chunk size */
	data_size = (int)buflen;
	if (data_size >= size) 
	  {
	     compress = 0;
	     data_size = size;
	  }
	else
	  {
	     void *data3;
	     
	     data3 = realloc(data2, data_size);
	     if (data3) data2 = data3;
	  }
     }
   if (!compress)
     memcpy(data2, data, size);

   /* Does this node already exist? */
   if (ef->mode == EET_FILE_MODE_RW)
     {
	int i;
	for (i = 0; i < node_size; i++)
	  {
	     /* if it matches */
	     if (eet_string_match(ef->header->directory->hash[hash].node[i].name, name))
	       {
		  free(ef->header->directory->hash[hash].node[i].data);
		  ef->header->directory->hash[hash].node[i].compression = compress;
		  ef->header->directory->hash[hash].node[i].size = data_size;
		  ef->header->directory->hash[hash].node[i].data_size = size;
		  ef->header->directory->hash[hash].node[i].data = data2;
		  exists_already = 1;
		  free(name2);
		  break;
	       }
	  }
     }
   if (!exists_already)
     {
	/* increase hash bucket size */
	node = realloc(ef->header->directory->hash[hash].node, 
		       (node_size  + 1) * sizeof(Eet_File_Node));
	if (!node) 
	  {
	     free(name2);
	     free(data2);
	     return 0;
	  }
	/* resized node list set up */
	ef->header->directory->hash[hash].node = node;
	/* new node at end */
	ef->header->directory->hash[hash].node[node_size].name = name2;
	ef->header->directory->hash[hash].node[node_size].offset = 0;
	ef->header->directory->hash[hash].node[node_size].compression = compress;
	ef->header->directory->hash[hash].node[node_size].size = data_size;
	ef->header->directory->hash[hash].node[node_size].data_size = size;
	ef->header->directory->hash[hash].node[node_size].data = data2;
	ef->header->directory->hash[hash].size++;
     }

   /* flags that writes are pending */
   ef->writes_pending = 1;
   /* update access time */
   return data_size;
}

int
eet_delete(Eet_File *ef, char *name)
{
   int hash, node_size;
   int exists_already = 0;
   
   /* check to see its' an eet file pointer */   
   if ((!ef) || (ef->magic != EET_MAGIC_FILE) || (!name))
     return 0;

   if (!ef->header) return 0;
   
   /* figure hash bucket */
   hash = eet_hash_gen(name, ef->header->directory->size);
   node_size = ef->header->directory->hash[hash].size;
   
   /* Does this node already exist? */
   if (ef->mode == EET_FILE_MODE_RW)
     {
	int i;
	for (i = 0; i < node_size; i++)
	  {
	     /* if it matches */
	     if (eet_string_match(ef->header->directory->hash[hash].node[i].name, name))
	       {
		  free(ef->header->directory->hash[hash].node[i].data);
		  ef->header->directory->hash[hash].node[i].compression = -1;
		  ef->header->directory->hash[hash].node[i].size = 0;
		  ef->header->directory->hash[hash].node[i].data_size = 0;
		  ef->header->directory->hash[hash].node[i].data = NULL;
		  exists_already = 1;
		  break;
	       }
	  }
     }
   /* flags that writes are pending */
   if (exists_already) ef->writes_pending = 1;
   /* update access time */
   return exists_already;
}

char **
eet_list(Eet_File *ef, char *glob, int *count_ret)
{
   char **list_ret = NULL;
   int list_count = 0;
   int list_count_alloc = 0;
   int i, j, num;

   /* check to see its' an eet file pointer */   
   if ((!ef) || (ef->magic != EET_MAGIC_FILE) || (!glob) ||
       (!ef->header) || (!ef->header->directory))
     {
	if (count_ret) *count_ret = 0;
	return NULL;
     }
   /* loop through all entries */
   num = (1 << (ef->header->directory->size - 1));
   for (i = 0; i < num; i++)
     {
	for (j = 0; j < ef->header->directory->hash[i].size; j++)
	  {
	     /* if the entry matches the input glob */
	     if (!fnmatch(glob, ef->header->directory->hash[i].node[j].name, 0))
	       {
		  char **new_list;
		  
		  /* add it to our list */
		  list_count++;
		  /* only realloc in 32 entry chunks */
		  if (list_count > list_count_alloc)
		    {
		       list_count_alloc += 32;
		       new_list = realloc(list_ret, list_count_alloc * (sizeof(char *)));
		       if (!new_list)
			 {
			    free(list_ret);
			    if (count_ret) *count_ret = 0;
			    return NULL;		       
			 }
		       list_ret = new_list;
		    }
		  /* put pointer of name string in */
		  list_ret[list_count - 1] = ef->header->directory->hash[i].node[j].name;
	       }
	  }
     }
   /* return count and list */
   if (count_ret) *count_ret = list_count;   
   return list_ret;
}

int eet_init(void)
{
   return ++eet_initcount;
}

int eet_shutdown(void)
{
   if (--eet_initcount == 0)
      _eet_memfile_shutdown();

   return eet_initcount;
}
