#include "Ecore_Config.h"
#include "config.h"
#include "ecore_config_private.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <util.h>

Ecore_Config_Server *__ecore_config_server_global = NULL;
Ecore_Config_Server *__ecore_config_server_local = NULL;
Ecore_Config_Bundle *__ecore_config_bundle_local = NULL;
char               *__ecore_config_app_name = NULL;

Ecore_Config_Server *_ecore_config_ipc_init(char *name);
int                 _ecore_config_ipc_exit(void);

static char        *_ecore_config_type[] =
   { "undefined", "integer", "float", "string", "colour", "theme" };

/**
 * Removes the given property from the local configuration and destroys it.
 * @param  e Property to destroy.
 * @return @c NULL
 */
Ecore_Config_Prop  *
ecore_config_dst(Ecore_Config_Prop * e)
{
   Ecore_Config_Bundle *t;
   Ecore_Config_Prop  *p, *c;
   Ecore_Config_Listener_List *l;

   p = NULL;
   c = e;
   t = __ecore_config_bundle_local;

   if (!e || !e->key)
      return NULL;
   if (t)
     {
	if (t->data == e)
	   t->data = e->next;
	else
	  {
	     do
	       {
		  p = c;
		  c = c->next;
	       }
	     while (c && (c != e));
	     if (c)		/* remove from list if even in there */
		p->next = c->next;
	  }
     }

   while (e->listeners)
     {
	l = e->listeners;
	e->listeners = e->listeners->next;
	free(l);
     }

   if (e->key)
      free(e->key);
   if (e->ptr && (e->type == PT_STR))
      free(e->ptr);

   memset(e, 0, sizeof(Ecore_Config_Prop));
   free(e);

   return NULL;
}

/**
 * Returns the property with the given key.
 * @param   key The unique name of the wanted property.
 * @return  The property that corresponds to the given key.  @c NULL if the
 *          key could not be found.
 * @ingroup Ecore_Config_Get_Group
 */
Ecore_Config_Prop  *
ecore_config_get(const char *key)
{
   Ecore_Config_Bundle *t;
   Ecore_Config_Prop  *e;

   t = __ecore_config_bundle_local;
   if (!t || !key)
      return NULL;
   e = t->data;
   while (e)
     {
	if (!strcmp(key, e->key))
	   return e;
	e = e->next;
     }
   return NULL;
}

/**
 * Returns the type of the property.
 * @param   e Property to get the type of.
 * @returns The type of the property.  If the property is invalid, then the
 *          string "not found" is returned.
 * @ingroup Ecore_Config_Property_Group
 */
const char         *
ecore_config_type_get(const Ecore_Config_Prop * e)
{
   if (e)
     {
	return _ecore_config_type[e->type];
     }
   return "not found";
}

/**
 * Obtains the data pointed to by the specified property.
 * @param   key The property key.
 * @return  Data pointer used by the property.
 * @ingroup Ecore_Config_Get_Group
 */
void               *
ecore_config_data_get(const char *key)
{
   Ecore_Config_Prop  *e;

   e = ecore_config_get(key);
   return (e ? ((e->type == PT_STR) ? ((void *)&e->ptr) : ((void *)&e->val))
	   : NULL);
}

/**
 * Returns the specified property as a string.
 * @param   key The property key.
 * @return  The string value of the property.  The function returns @c NULL if
 *          the property is not a string or is not set.
 * @ingroup Ecore_Config_Get_Group
 */
char               *
ecore_config_string_get(const char *key)
{
   Ecore_Config_Prop  *e;

   e = ecore_config_get(key);
   return (e && (e->type == PT_STR)) ? strdup(e->ptr) : NULL;
}

/**
 * Returns the specified property as a long integer.
 * @param   key The property key.
 * @return  The integer value of the property.  The function returns 0 if the
 *          property is not an integer or is not set.
 * @ingroup Ecore_Config_Get_Group
 */
long
ecore_config_int_get(const char *key)
{
   Ecore_Config_Prop  *e;

   e = ecore_config_get(key);
   return (e && ((e->type == PT_INT) || (e->type == PT_RGB))) ? e->val : 0L;
}

/**
 * Returns the specified property as a float.
 * @param   key The property key.
 * @return  The float value of the property.  The function returns 0.0 if the 
 *          property is not a float or is not set.
 * @ingroup Ecore_Config_Get_Group
 */
float
ecore_config_float_get(const char *key)
{
   Ecore_Config_Prop  *e;

   e = ecore_config_get(key);
   return (e
	   && (e->type ==
	       PT_FLT)) ? ((float)e->val / ECORE_CONFIG_FLOAT_PRECISION) : 0.0;
}

/**
 * Finds the red, green and blue values of a color property.
 * @param   key The property key.
 * @param   r   A pointer to an integer to store the red value into.
 * @param   g   A pointer to an integer to store the green value into.
 * @param   b   A pointer to an integer to store the blue value into.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.  @c ECORE_CONFIG_ERR_FAIL
 *          otherwise.
 * @ingroup Ecore_Config_Get_Group
 */
int
ecore_config_rgb_get(const char *key, int *r, int *g, int *b)
{
   Ecore_Config_Prop  *e;

   e = ecore_config_get(key);

   if (e && ((e->type == PT_RGB)))
     {
	*r = (e->val >> 16) & 0xff;
	*g = (e->val >> 8) & 0xff;
	*b = e->val & 0xff;
	return ECORE_CONFIG_ERR_SUCC;
     }
   return ECORE_CONFIG_ERR_FAIL;
}

/**
 * Returns a color property as a string of hexadecimal characters.
 * @param   key The property key.
 * @return  A string of hexadecimal characters in the format #rrggbb.
 * @ingroup Ecore_Config_Get_Group
 */
char               *
ecore_config_rgbstr_get(const char *key)
{
   char               *r;

   r = NULL;
   esprintf(&r, "#%06x", ecore_config_int_get(key));
   return r;
}

/**
 * Returns a theme property.
 * @param   key The property key.
 * @return  The name of the theme the property refers to.  The function returns
 *          @c NULL if the property is not a theme or is not set.
 * @ingroup Ecore_Config_Get_Group
 */
char               *
ecore_config_theme_get(const char *key)
{
   Ecore_Config_Prop  *e;

   e = ecore_config_get(key);
   return (e && (e->type == PT_THM)) ? strdup(e->ptr) : NULL;
}

/**
 * Retrieves the key as a string.
 * @param   key The property key.
 * @return  Returns a character array in the form of 'key:type=value'.  @c NULL
 *          is returned if the property does not exist.
 * @ingroup Ecore_Config_Get_Group
 */
char               *
ecore_config_as_string_get(const char *key)
{
   Ecore_Config_Prop  *e;
   char               *r;

   r = NULL;
   if (!(e = ecore_config_get(key)))
      E(0, "no such property, \"%s\"...\n", key);
   else
     {
	const char         *type = ecore_config_type_get(e);

	switch (e->type)
	  {
	  case PT_NIL:
	     esprintf(&r, "%s:%s=<nil>", key, type);
	     break;
	  case PT_INT:
	     esprintf(&r, "%s:%s=%ld", key, type, ecore_config_int_get(key));
	     break;
	  case PT_FLT:
	     esprintf(&r, "%s:%s=%lf", key, type, ecore_config_float_get(key));
	     break;
	  case PT_STR:
	     esprintf(&r, "%s:%s=\"%s\"", key, type,
		      ecore_config_string_get(key));
	     break;
	  case PT_RGB:
	     esprintf(&r, "%s:%s=#%06x", key, type, ecore_config_int_get(key));
	     break;
	  case PT_THM:
	     esprintf(&r, "%s:%s=\"%s\"", key, type,
		      ecore_config_theme_get(key));
	     break;
	  default:
	     esprintf(&r, "%s:unknown_type", key);
	     break;
	  }
     }
   return r;
}

int
ecore_config_bound(Ecore_Config_Prop * e)
{
   int                 ret;
   long                v;

   ret = ECORE_CONFIG_ERR_SUCC;

   if (!e)
      return ECORE_CONFIG_ERR_FAIL;
   if (e->flags & PF_BOUNDS)
     {
	if ((e->val < e->lo))
	  {
	     E(0,
	       "ecore_config_bounds(\"%s\",%ld): value out of range; adjusted to %ld...\n",
	       e->key, e->val, e->lo);
	     e->val = e->lo;
	  }
	else if ((e->val > e->hi))
	  {
	     E(0,
	       "ecore_config_bounds(\"%s\",%ld): value out of range; adjusted to %ld...\n",
	       e->key, e->val, e->hi);
	     e->val = e->hi;
	  }
	else
	   ret = ECORE_CONFIG_ERR_IGNORED;
     }
   else
      ret = ECORE_CONFIG_ERR_IGNORED;

   if (e->step)
     {
	v = ((int)(e->val / e->step)) * e->step;
	if (v != e->val)
	  {
	     if (e->type == PT_FLT)
		E(0,
		  "ecore_config_bound(\"%s\"): float value %f not a multiple of %f, adjusted to %f...\n",
		  e->key, ((double)e->val) / ECORE_CONFIG_FLOAT_PRECISION,
		  ((double)e->step) / ECORE_CONFIG_FLOAT_PRECISION,
		  ((double)v) / ECORE_CONFIG_FLOAT_PRECISION);
	     else
		E(0,
		  "ecore_config_bound(\"%s\"): integer value %ld not a multiple of %ld, adjusted to %ld...\n",
		  e->key, e->val, e->step, v);
	     ret = ECORE_CONFIG_ERR_SUCC;
	     e->val = v;
	  }
     }

   return ret;
}

/**
 * Tries to guess the type of a property.
 *
 * This function first checks to see if the property exists.  If it does, then
 * the type of the stored property is returned.  Otherwise, the function tries
 * to guess the type of the property based on @p val.
 *
 * @param  key The property key.
 * @param  val The value in string form.
 * @return The type of the property determined by the function.  Note that if
 *         val is @c NULL, @c PT_NIL will be returned.
 */
int
ecore_config_type_guess(const char *key, char *val)
{
   Ecore_Config_Prop  *p;
   char               *l;
   long                v;

   l = NULL;

   if ((p = ecore_config_get(key)) && p->type != PT_NIL)
      return p->type;

   if (!val)
      return PT_NIL;
   if (val[0] == '#')
      return PT_RGB;
   v = strtol(val, &l, 10);
   if (*l)
     {
	float               f;

	if (sscanf(val, "%f%*s", &f) != 1)
	   return PT_STR;
	return PT_FLT;
     }
   return PT_INT;
}

static int
ecore_config_typed_val(Ecore_Config_Prop * e, void *val, int type)
{
   char               *l;
   long                v;
   int                *i;
   float              *f;

   l = NULL;
   v = 0;

   if (!(val))
      e->ptr = NULL;
   else
     {
	if (type == PT_INT)
	  {
	     i = (int *)val;
	     e->val = (long)*i;
	     e->type = PT_INT;
	  }
	else if (type == PT_STR || type == PT_THM)
	  {
	     if (!(e->ptr = strdup(val)))
		return ECORE_CONFIG_ERR_OOM;
	     if (e->type == PT_NIL)
		e->type = type;
	  }
	else if (type == PT_RGB)
	  {
	     if (((char *)val)[0] == '#')
	       {
		  if ((v = strtol(&((char *)val)[1], &l, 16)) < 0)
		    {
		       v = 0;
		       E(0,
			 "ecore_config_val: key \"%s\" -- hexadecimal value less than zero, bound to zero...\n",
			 (char *)val);
		       l = (char *)val;
		    }
	       }
	     else
	       {
		  E(0,
		    "ecore_config_val: key \"%s\" -- value \"%s\" not a valid hexadecimal RGB value?\n",
		    e->key, (char *)val);
		  return ECORE_CONFIG_ERR_FAIL;
	       }
	     if (*l)
		E(0,
		  "ecore_config_val: key \"%s\" -- value \"%s\" not a valid hexadecimal RGB value?\n",
		  e->key, (char *)val);
	     else
	       {
		  e->val = v;
		  e->type = PT_RGB;
	       }
	  }
	else if (type == PT_FLT)
	  {
	     f = (float *)val;
	     e->val = (long)((*f) * ECORE_CONFIG_FLOAT_PRECISION);
	     e->type = PT_FLT;
	  }
	else
	   e->type = PT_NIL;

	ecore_config_bound(e);
	e->flags |= PF_MODIFIED;
	return ECORE_CONFIG_ERR_SUCC;
     }
   return ECORE_CONFIG_ERR_IGNORED;
}

static int
ecore_config_typed_add(const char *key, void *val, int type)
{
   Ecore_Config_Prop  *e;
   Ecore_Config_Bundle *t;

   t = __ecore_config_bundle_local;
   if (!key)
      return ECORE_CONFIG_ERR_NODATA;

   if (!(e = malloc(sizeof(Ecore_Config_Prop))))
      goto ret;
   memset(e, 0, sizeof(Ecore_Config_Prop));

   if (!(e->key = strdup(key)))
      goto ret_free_nte;

   if (!val)
      e->type = PT_NIL;
   else if (ecore_config_typed_val(e, val, type) == ECORE_CONFIG_ERR_OOM)
      goto ret_free_key;

   e->next = t ? t->data : NULL;
   if (t)
      t->data = e;

   return ECORE_CONFIG_ERR_SUCC;

 ret_free_key:
   free(e->key);
 ret_free_nte:
   free(e);
 ret:
   return ECORE_CONFIG_ERR_OOM;
}

static int
ecore_config_add(const char *key, char *val)
{
   int                 type;

   type = ecore_config_type_guess(key, val);
   return ecore_config_typed_add(key, val, type);
}

/**
 * Sets the description field of the indicated property.
 * @param  key  The property key.
 * @param  desc Description string.
 * @note   The description string is copied for the property's use.  You can
 *         free @p desc once this function is called.
 * @ingroup Ecore_Config_Property_Group
 */
int
ecore_config_describe(const char *key, char *desc)
{
   Ecore_Config_Prop  *e;

   if (!(e = ecore_config_get(key)))
      return ECORE_CONFIG_ERR_NODATA;
   e->description = strdup(desc);
   return ECORE_CONFIG_ERR_SUCC;
}

/**
 * Set the short option character of a property.
 * @param  key       The property key.
 * @param  short_opt Character used to indicate the value of a property
 *                   given on the command line.
 * @return @c ECORE_CONFIG_ERR_SUCC on success.  @c ECORE_CONFIG_ERR_NODATA
 *         is returned if the property does not exist.
 */
int
ecore_config_short_opt_set(const char *key, char short_opt)
{
   Ecore_Config_Prop  *e;

   if (!(e = ecore_config_get(key)))
      return ECORE_CONFIG_ERR_NODATA;
   e->short_opt = short_opt;
   return ECORE_CONFIG_ERR_SUCC;
}

/**
 * Set the long option string of the property.
 * @param  key      The property key.
 * @param  long_opt String used to indicate the value of a property given
 *                  on the command line.
 * @return @c ECORE_CONFIG_ERR_SUCC on success.  @c ECORE_CONFIG_ERR_NODATA
 *         is returned if the property does not exist.
 */
int
ecore_config_long_opt_set(const char *key, char *long_opt)
{
   Ecore_Config_Prop  *e;

   if (!(e = ecore_config_get(key)))
      return ECORE_CONFIG_ERR_NODATA;
   if (e->long_opt)
      free(e->long_opt);
   e->long_opt = strdup(long_opt);
   return ECORE_CONFIG_ERR_SUCC;
}

/**
 * Sets the indicated property to the given value and type.
 * @param  key  The property key.
 * @param  val  A pointer to the value to set the property to.
 * @param  type The type of the property.
 * @return @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 */
int
ecore_config_typed_set(const char *key, void *val, int type)
{
   Ecore_Config_Prop  *e;
   Ecore_Config_Listener_List *l;
   int                 ret;

   if (!key)
      return ECORE_CONFIG_ERR_NODATA;
/*  if (!t) { * global prop *
    e=ecore_config_get(key);
    if (e)
      for(l=e->listeners;l;l=l->next)
        l->listener(e->key,e->type,l->tag,l->data,t);
    return ECORE_CONFIG_ERR_SUCC;
  }
*/
   if (!(e = ecore_config_get(key)))
      return ecore_config_typed_add(key, val, type);

   if ((ret = ecore_config_typed_val(e, val, type)) == ECORE_CONFIG_ERR_SUCC)
     {
	for (l = e->listeners; l; l = l->next)
 	   l->listener(e->key, e->type, l->tag, l->data);
     }
   else
     {
	E(0,
	  "ecore_config_typed_set(\"%s\"): ecore_config_typed_val() failed: %d\n",
	  key, ret);
     }

   return ret;
}

/**
 * Sets the indicated property to the value indicated by @a val.
 * @param   key The property key.
 * @param   val String representation of value to set.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
int
ecore_config_set(const char *key, char *val)
{
   int                 type;
   int                 tmpi;
   float               tmpf;

   type = ecore_config_type_guess(key, val);
   if (type == PT_INT)
     {
	tmpi = atoi(val);
	return ecore_config_typed_set(key, (void *)&tmpi, type);
     }
   else if (type == PT_FLT)
     {
	tmpf = atof(val);
	return ecore_config_typed_set(key, (void *)&tmpf, type);
     }
   else
      return ecore_config_typed_set(key, (void *)val, type);
}

/**
 * Sets the indicated property to the value given in the string.
 * @param   key The property key.
 * @param   val String representation of the value.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
int
ecore_config_as_string_set(const char *key, char *val)
{
   return ecore_config_set(key, val);
}

/**
 * Sets the indicated property to the given integer.
 * @param   key The property key.
 * @param   val Integer to set the property to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
int
ecore_config_int_set(const char *key, int val)
{
   return ecore_config_typed_set(key, (void *)&val, PT_INT);
}

/**
 * Sets the indicated property to the given string.
 * @param   key The property key.
 * @param   val String to set the property to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
int
ecore_config_string_set(const char *key, char *val)
{
   return ecore_config_typed_set(key, (void *)val, PT_STR);
}

/**
 * Sets the indicated property to the given float value.
 * @param   key The property key.
 * @param   val Float to set the property to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
int
ecore_config_float_set(const char *key, float val)
{
   return ecore_config_typed_set(key, (void *)&val, PT_FLT);
}

/**
 * Sets the indicated property to a color value.
 * @param   key The property key
 * @param   val Color value in RGB format.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
int
ecore_config_rgb_set(const char *key, char *val)
{
   return ecore_config_typed_set(key, (void *)val, PT_RGB);
}

/**
 * Sets the indicated property to a theme name.
 * @param   key The property key.
 * @param   val String giving the name of the theme.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
int
ecore_config_theme_set(const char *key, char *val)
{
   return ecore_config_typed_set(key, (void *)val, PT_THM);
}

/**
 * Sets the theme preview group of an indicated property.
 * @param   key   The property key.
 * @param   group The group name.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Set_Group
 */
int
ecore_config_theme_preview_group_set(const char *key, char *group)
{
   int                 ret;
   Ecore_Config_Prop  *e;

   ret = ECORE_CONFIG_ERR_SUCC;
   if (!(e = ecore_config_get(key)))
     {				/* prop doesn't exist yet */
	if ((ret = ecore_config_typed_add(key, "", PT_THM)) != ECORE_CONFIG_ERR_SUCC)	/* try to add it */
	   return ret;		/* ...failed */
	if (!(e = ecore_config_get(key)))	/* get handle */
	   return ECORE_CONFIG_ERR_FAIL;
     }
   if (e->data)
      free(e->data);
   e->data = strdup(group);

   return ret;
}

int
ecore_config_typed_default(const char *key, void *val, int type)
{
   int                 ret;
   Ecore_Config_Prop  *e;

   ret = ECORE_CONFIG_ERR_SUCC;

   if (!(e = ecore_config_get(key)))
     {				/* prop doesn't exist yet */
	if ((ret = ecore_config_typed_add(key, val, type)) != ECORE_CONFIG_ERR_SUCC)	/* try to add it */
	   return ret;		/* ...failed */
	if (!(e = ecore_config_get(key)))	/* get handle */
	   return ECORE_CONFIG_ERR_FAIL;
	e->flags = e->flags & ~PF_MODIFIED;
     }

   return ret;
}

/**
 * Sets the indicated property if it has not already been set or loaded.
 * @param   key  The property key.
 * @param   val  Default value of the key.
 * @param   lo   Lowest valid value for the key.
 * @param   hi   Highest valid value for the key.
 * @param   step Used by integer and float values.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there are no errors.
 * @note    The @p lo, @p hi and @p step parameters are only used when storing
 *          integer and float properties.
 * @ingroup Ecore_Config_Default_Group
 */
int
ecore_config_default(const char *key, char *val, float lo, float hi, float step)
{
   int                 ret, type;
   Ecore_Config_Prop  *e;

   type = ecore_config_type_guess(key, val);
   ret = ecore_config_typed_default(key, val, type);
   e = ecore_config_get(key);
   if (e)
     {
	if (type == PT_INT)
	  {
	     e->step = step;
	     e->flags |= PF_BOUNDS;
	     e->lo = lo;
	     e->hi = hi;
	     ecore_config_bound(e);
	  }
	else if (type == PT_FLT)
	  {
	     e->step = (int)(step * ECORE_CONFIG_FLOAT_PRECISION);
	     e->flags |= PF_BOUNDS;
	     e->lo = (int)(lo * ECORE_CONFIG_FLOAT_PRECISION);
	     e->hi = (int)(hi * ECORE_CONFIG_FLOAT_PRECISION);
	     ecore_config_bound(e);
	  }
     }

   return ret;
}

/**
 * Sets the indicated property to the given integer if the property has not yet
 * been set.
 * @param   key The property key.
 * @param   val Integer to set the value to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there are no problems.
 * @ingroup Ecore_Config_Default_Group
 */
int
ecore_config_int_default(const char *key, int val)
{
   return ecore_config_typed_default(key, (void *)&val, PT_INT);
}

/**
 * Sets the indicated property to the given integer if the property has not yet
 * been set.
 *
 * The bounds and step values are set regardless.
 *
 * @param   key  The property key.
 * @param   val  Integer to set the property to.
 * @param   low  Lowest valid integer value for the property.
 * @param   high Highest valid integer value for the property.
 * @param   step Increment value for the property.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there were no problems.
 * @ingroup Ecore_Config_Default_Group
 */
int
ecore_config_int_default_bound(const char *key, int val, int low, int high,
			       int step)
{
   Ecore_Config_Prop  *e;
   int                 ret;

   ret = ecore_config_typed_default(key, (void *)&val, PT_INT);
   e = ecore_config_get(key);
   if (e)
     {
	e->step = step;
	e->flags |= PF_BOUNDS;
	e->lo = low;
	e->hi = high;
	ecore_config_bound(e);
     }

   return ret;
}

/**
 * Sets the indicated property to the given string if the property has not yet
 * been set.
 * @param   key The property key.
 * @param   val String to set the property to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there were no problems.
 * @ingroup Ecore_Config_Default_Group
 */
int
ecore_config_string_default(const char *key, char *val)
{
   return ecore_config_typed_default(key, (void *)val, PT_STR);
}

/**
 * Sets the indicated property to the given float if the property has not yet
 * been set.
 * @param   key The property key.
 * @param   val Float to set the property to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there were no problems.
 * @ingroup Ecore_Config_Default_Group
 */
int
ecore_config_float_default(const char *key, float val)
{
   return ecore_config_typed_default(key, (void *)&val, PT_FLT);
}

/**
 * Sets the indicated property to the given float if the property has not yet
 * been set.
 *
 * The bounds and step values are set regardless.
 *
 * @param   key  The property key.
 * @param   val  Float to set the property to.
 * @param   low  Lowest valid integer value for the property.
 * @param   high Highest valid float value for the property.
 * @param   step Increment value for the property.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there were no problems.
 * @ingroup Ecore_Config_Default_Group
 */
int
ecore_config_float_default_bound(const char *key, float val, float low,
				 float high, float step)
{
   Ecore_Config_Prop  *e;
   int                 ret;

   ret = ecore_config_typed_default(key, (void *)&val, PT_FLT);
   e = ecore_config_get(key);
   if (e)
     {
	e->step = (int)(step * ECORE_CONFIG_FLOAT_PRECISION);
	e->flags |= PF_BOUNDS;
	e->lo = (int)(low * ECORE_CONFIG_FLOAT_PRECISION);
	e->hi = (int)(high * ECORE_CONFIG_FLOAT_PRECISION);
	ecore_config_bound(e);
     }

   return ret;
}

/**
 * Sets the indicated property to a color value if the property has not yet
 * been set.
 * @param  key The property key.
 * @param  val Color value in RGB format.
 * @return @c ECORE_CONFIG_ERR_SUCC if there are no problems.
 * @ingroup Ecore_Config_Default_Group
 */
int
ecore_config_rgb_default(const char *key, char *val)
{
   return ecore_config_typed_default(key, (void *)val, PT_RGB);
}

/**
 * Sets the indicated property to a theme name if the property has not yet
 * been set.
 * @param   key The property key.
 * @param   val String giving the name of the theme.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Default_Group
 */
int
ecore_config_theme_default(const char *key, char *val)
{
   return ecore_config_typed_default(key, (void *)val, PT_THM);
}

/**
 * Adds a callback function to the list of functions called when a property
 * changes.
 * @param   name     Name of the callback.
 * @param   key      The key of the property to listen to.
 * @param   listener Listener callback function.
 * @param   tag      Tag to pass to @p listener when it is called.
 * @param   data     Data to pass to @p listener when it is called.
 * @return  @c ECORE_CONFIG_ERR_SUCC if successful in setting up the callback.
 * @ingroup Ecore_Config_Listeners_Group
 */
int
ecore_config_listen(const char *name, const char *key,
		    Ecore_Config_Listener listener, int tag, void *data)
{
   Ecore_Config_Prop  *e;
   Ecore_Config_Listener_List *l;

   if (!key)
      return ECORE_CONFIG_ERR_NODATA;

   if (!(e = ecore_config_get(key)))
     {
	int                 ret = ecore_config_add(key, "");

	if (ret != ECORE_CONFIG_ERR_SUCC)
	  {
	     E(0, "ecore_config_listen: ecore_config_add(\"%s\") failed: %d\n",
	       key, ret);
	     return ret;
	  }
	if (!(e = ecore_config_get(key)))
	  {
	     E(0, "ecore_config_listen: list of properties corrupted!?\n");
	     return ECORE_CONFIG_ERR_FAIL;
	  }
     }

   for (l = e->listeners; l; l = l->next)
      if (!strcmp(l->name, name) || (l->listener == listener))
	{
	   E(1,
	     "ecore_config_listen: %s is already listening for changes of %s...\n",
	     name, key);
	   return ECORE_CONFIG_ERR_IGNORED;
	}

   if (!(l = malloc(sizeof(Ecore_Config_Listener_List))))
      return ECORE_CONFIG_ERR_OOM;

   E(1, "registering listener \"%s\" for \"%s\" (%d)...\n", name, key, e->type);

   memset(l, 0, sizeof(Ecore_Config_Listener_List));

   l->listener = listener;
   l->name = name;
   l->data = data;
   l->tag = tag;
   l->next = e->listeners;
   e->listeners = l;

   if (e->type != PT_NIL)	/* call right on creation if prop exists and has val */
      listener(key, e->type, tag, data);

   return ECORE_CONFIG_ERR_SUCC;
}

/**
 * Removes a listener callback.
 * @param  name     Name of the callback to remove.
 * @param  key      The property key the callback is listening to.
 * @param  listener The callback function to remove.
 * @return @c ECORE_CONFIG_ERR_SUCC if successful in removing the callback.
 *         If no callback matches the given parameters, then
 *         @c ECORE_CONFIG_ERR_NOTFOUND is returned.  If @c NULL is passed
 *         for the key pointer, @c ECORE_CONFIG_ERR_NODATA is returned.
 * @ingroup Ecore_Config_Listeners_Group
 */
int
ecore_config_deaf(const char *name, const char *key,
		  Ecore_Config_Listener listener)
{
   Ecore_Config_Prop  *e;
   Ecore_Config_Listener_List *l, *p;
   int                 ret;

   ret = ECORE_CONFIG_ERR_NOTFOUND;

   if (!key)
      return ECORE_CONFIG_ERR_NODATA;

   if (!(e = ecore_config_get(key)))
      return ECORE_CONFIG_ERR_NOTFOUND;

   for (p = NULL, l = e->listeners; l; p = l, l = l->next)
     {
	if ((name && !strcmp(l->name, name)) || (l->listener == listener))
	  {
	     ret = ECORE_CONFIG_ERR_SUCC;
	     if (!p)
		e->listeners = e->listeners->next;
	     else
		p->next = l->next;
	     memset(l, 0, sizeof(Ecore_Config_Listener));
	     free(l);
	  }
     }

   return ret;
}

/**
 * Locates the first configuration bundle on the given server.
 * @param  srv The configuration server.
 * @return Pointer to the first configuration bundle.
 */
Ecore_Config_Bundle *
ecore_config_bundle_1st_get(Ecore_Config_Server * srv)
{				/* anchor: global, but read-only */
   return srv->bundles;
}

/**
 * Locates the configuration bundle after the given one.
 * @param  ns  The configuration bundle.
 * @return The next configuration bundle.
 */
Ecore_Config_Bundle *
ecore_config_bundle_next_get(Ecore_Config_Bundle * ns)
{
   return ns ? ns->next : NULL;
}

/**
 * Locates a configuration bundle on a configuration server based on its serial
 * number.
 * @param  srv    The configuration server.
 * @param  serial Serial number.
 * @return The configuration bundle with the given serial number.
 */
Ecore_Config_Bundle *
ecore_config_bundle_by_serial_get(Ecore_Config_Server * srv, long serial)
{
   Ecore_Config_Bundle *eb;

   eb = srv->bundles;

   if (serial < 0)
      return NULL;
   else if (serial == 0)
     {
	Ecore_Config_Bundle *r = eb;

	return r;
     }

   while (eb)
     {
	if (eb->serial == serial)
	   return eb;
	eb = eb->next;
     }
   return NULL;
}

/**
 * Gets the Ecore_Config_Bundle with the given identifier from the given
 * server.
 * @param  srv   The configuration server.
 * @param  label The bundle's identifier string.
 * @return The bundle with the given identifier string, or @c NULL if it
 *         could not be found.
 */
Ecore_Config_Bundle *
ecore_config_bundle_by_label_get(Ecore_Config_Server * srv, const char *label)
{
   Ecore_Config_Bundle *ns;

   ns = srv->bundles;

   while (ns)
     {
	if (ns->identifier && !strcmp(ns->identifier, label))
	   return ns;
	ns = ns->next;
     }
   return NULL;
}

/**
 * Retrieves the bundle's serial number.
 * @param  ns The configuration bundle.
 * @return The bundle's identifier string, or -1 if ns is @c NULL.
 */
long
ecore_config_bundle_serial_get(Ecore_Config_Bundle * ns)
{
   return ns ? ns->serial : -1;
}

/**
 * Retrieves the bundle's identifier.
 * @param  ns The configuration bundle.
 * @return The bundle's identifer string.
 */
char               *
ecore_config_bundle_label_get(Ecore_Config_Bundle * ns)
{
   return ns ? ns->identifier : NULL;
}

/**
 * Creates a new Ecore_Config_Bundle.
 * @param  srv        Config server.
 * @param  identifier Identifier string for the new bundle.
 * @return A pointer to a new Ecore_Config_Bundle.  @c NULL is returned if the
 *         structure couldn't be allocated.
 */
Ecore_Config_Bundle *
ecore_config_bundle_new(Ecore_Config_Server * srv, const char *identifier)
{
   Ecore_Config_Bundle *t;
   static long         ss;

   ss = 0;			/* bundle unique serial */

   if ((t = malloc(sizeof(Ecore_Config_Bundle))))
     {
	memset(t, 0, sizeof(Ecore_Config_Bundle));

	t->identifier = (char *)identifier;
	t->serial = ++ss;
	t->owner = srv->name;
	t->next = srv->bundles;
	srv->bundles = t;
     }
   return t;
}

Ecore_Config_Server *
do_init(char *name)
{
   return _ecore_config_ipc_init(name);
}

Ecore_Config_Server *
ecore_config_init_local(char *name)
{
   char               *p;
   char               *buf;

   if ((p = getenv("HOME")))
     {				/* debug-only ### FIXME */
	if (!(buf = malloc(PATH_MAX * sizeof(char))))
	   return NULL;
	snprintf(buf, PATH_MAX, "%s/.ecore/%s/.global", p, name);
	unlink(buf);

	free(buf);
     }

   return do_init(name);
}

Ecore_Config_Server *
ecore_config_init_global(char *name)
{
   char               *p;
   int global;
   char               *buf;
   global = 0;

   if ((p = getenv("HOME")))
     {				/* debug-only ### FIXME */
	if (!(buf = malloc(PATH_MAX * sizeof(char))))
	   return NULL;
	snprintf(buf, PATH_MAX, "%s/.ecore/%s/.global", p, name);
	global = creat(buf, S_IRWXU);

	if (global)
	   close(global);

	free(buf);
     }

   return do_init(name);
}

/**
 * Initializes the Enlightened Property Library.
 *
 * This function must be run before any other function in the
 * Enlightened Property Library, even if you have run @ref ecore_init .
 * The name given is used to determine the default configuration to
 * load.
 *
 * @param  name Application name
 * @return @c ECORE_CONFIG_ERR_SUCC if the library is successfully set up.
 *         @c ECORE_CONFIG_ERR_FAIL otherwise.
 * @ingroup Ecore_Config_Lib_Group
 */
int
ecore_config_init(char *name)
{
   char               *buf, *p, *path;
   Ecore_Config_Prop  *sys;

   DEBUG = -1;
   if ((p = getenv("ECORE_CONFIG_DEBUG")) && strlen(p) > 0)
     {
	DEBUG = atoi(p);
     }

   __ecore_config_app_name = strdup(name);
   __ecore_config_server_local = ecore_config_init_local(name);
   if (!__ecore_config_server_local)
      return ECORE_CONFIG_ERR_FAIL;

   __ecore_config_server_global =
      ecore_config_init_global(ECORE_CONFIG_GLOBAL_ID);
   if (!__ecore_config_server_global)
      return ECORE_CONFIG_ERR_FAIL;

   __ecore_config_bundle_local =
      ecore_config_bundle_new(__ecore_config_server_local, "config");

   path = ecore_config_theme_default_path_get();
   if (path)
     {
	ecore_config_string_default("/e/themes/search_path", path);
	free(path);
     }

   if ((p = getenv("HOME")))
     {				/* debug-only ### FIXME */
	if ((buf = malloc(PATH_MAX * sizeof(char))))
	  {
	     snprintf(buf, PATH_MAX, "%s/.e/config.db", p);
	     if (ecore_config_file_load(buf) != 0)
		if (ecore_config_file_load(PACKAGE_DATA_DIR "/system.db") != 0)
		   return ECORE_CONFIG_ERR_NOFILE;
	     sys = __ecore_config_bundle_local->data;
	     while (sys)
	       {
		  /* unmark it modified - modification will mean it has been overridden */
		  sys->flags &= ~PF_MODIFIED;
		  /* mark as system so that examine can hide them */
		  sys->flags |= PF_SYSTEM;
		  sys = sys->next;
	       }
	  }
	free(buf);
     }

   return ECORE_CONFIG_ERR_SUCC;
}

/**
 * Frees memory and shuts down the library.
 * @return @c ECORE_CONFIG_ERR_IGNORED
 * @ingroup Ecore_Config_Lib_Group
 */
int
ecore_config_shutdown(void)
{
   int                 ret;

   ret = _ecore_config_ipc_exit();
   free(__ecore_config_app_name);
   free(__ecore_config_bundle_local);
   free(__ecore_config_server_local);
   free(__ecore_config_server_global);
   return ret;
}
