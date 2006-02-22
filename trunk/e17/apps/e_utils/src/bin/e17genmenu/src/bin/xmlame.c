#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Ecore_File.h>

#include "xmlame.h"

/** xmlame.c Extensively Mocked Language Approximately Mangled for Enlightenment. 
 *
 * This is NOT a real XML parser.  There were a few ways we could go when it came
 * to parsing the freedesktop.org (fdo) XML menu files.  Whatever way we went, we 
 * needed some sort of XML parser if we wanted to fully support fdo menu files.  
 * Nothing we can do about that, fdo set the standard and they choose XML to do it.
 *
 * After a discussion with raster, three things led to the decision to do it this 
 * way.  It is likely that this will get included as a core part of the E17 window 
 * manager (E17) coz E17 needs this functionality.  E17 is in a dependency freeze
 * and there is no XML parser in it's current dependencies.  The fdo XML menu files
 * look to be simple enough to parse that this sort of fake, brain dead, XML parser
 * may get away with it.  Much testing on lots of systems is highly recommended.
 *
 * The final '>' of a tag is replaced with a '\0', but it's existance can be implied.
 */

static char *_xmlame_parse(Dumb_Tree *tree, char *buffer);


Dumb_Tree *
xmlame_new(char *buffer)
{
   Dumb_Tree *tree;

   tree = dumb_tree_new(buffer);
   return tree;
}

Dumb_Tree *
xmlame_get(char *file)
{
   int size;
   char *buffer;
   Dumb_Tree *tree = NULL;

   size = ecore_file_size(file);
   buffer = (char *) malloc(size + 1);
   if (buffer)
      {
         int fd;

         buffer[0] = '\0';
	 fd = open(file, O_RDONLY);
	 if (fd != -1)
	    {
	       if (read(fd, buffer, size) == size)
		     buffer[size] = '\0';
	    }
         tree = xmlame_new(buffer);
	 if (tree)
	    {
	       /* Have the file name as the first item on the tree, for later reference. */
	       dumb_tree_extend(tree, file);
	       _xmlame_parse(tree, buffer);
	    }
      }
   return tree;
}

static char *
_xmlame_parse(Dumb_Tree *tree, char *buffer)
{
   do
      {
	 char *text;

         /* Skip any white space at the beginning. */
         while ((*buffer != '\0') && (isspace(*buffer)))
            buffer++;
         text = buffer;
         /* Find the beginning of a tag. */
         while ((*buffer != '<') && (*buffer != '\0'))
            buffer++;
         /* Check for data between tags. */
	 if (buffer != text)
	    {
	       char t;

               t = *buffer;
	       *buffer = '\0';
	       dumb_tree_extend(tree, strdup(text));
	       *buffer = t;
	    }
         if (*buffer != '\0')
	    {
               char *begin;

               begin = buffer++;
               /* Find the end of the tag. */
               while ((*buffer != '>') && (*buffer != '\0'))
                  buffer++;
               /* We have our tag, do something with it. */
               if (*buffer != '\0')
                  {
                     *buffer++ = '\0';
	             if (begin[1] == '/')
	                {   /* The end of an element. */
	                   dumb_tree_add(tree, begin);
                           break;
		        }
                     else if ((begin[1] == '!') || (begin[1] == '-') || (*(buffer - 2) == '/'))
	                {   /* This is a script, a comment, or a stand alone tag. */
	                   dumb_tree_add(tree, begin);
	                }
	             else
	                {   /* The beginning of an element. */
                           Dumb_Tree *new_tree;

                           new_tree = xmlame_new(NULL);
	                   if (new_tree)
	                      {
		                 dumb_tree_add_child(tree, new_tree);
	                         dumb_tree_add(new_tree, begin);
	                         buffer = _xmlame_parse(new_tree, buffer);
		              }
	                }
	          }
            }
      } while (*buffer != '\0');

   return buffer;
}
