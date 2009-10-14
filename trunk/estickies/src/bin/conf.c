#include "stickies.h"

#define DEFAULT_THEME "default.edj"

#define NEWD(str, typ) _estickies_data_descriptor(str, sizeof (typ));

#define FREED(eed) \
	 if (eed) \
	     { \
		eet_data_descriptor_free((eed)); \
		(eed) = NULL; \
	     }

#define CFG_GENERAL_NEWI(str, it, type) EET_DATA_DESCRIPTOR_ADD_BASIC(_e_config_gen_edd, E_Config_General, str, it, type)

#define CFG_STICKY_NEWI(str, it, type) EET_DATA_DESCRIPTOR_ADD_BASIC(_e_config_sticky_edd, E_Sticky, str, it, type)

#define CFG_STICKIES_NEWI(str, it, type) EET_DATA_DESCRIPTOR_ADD_BASIC(_e_config_stickies_edd, E_Config_Stickies, str, it, type)

#define CFG_STICKIES_NEWL(str, it, type) EET_DATA_DESCRIPTOR_ADD_LIST(_e_config_stickies_edd, E_Config_Stickies, str, it, type)

#define VER_NEWI(str, it, type) EET_DATA_DESCRIPTOR_ADD_BASIC(_e_config_version_edd, E_Config_Version, str, it, type)


//static Eet_Data_Descriptor *_e_config_gen_edd = NULL;
static Eet_Data_Descriptor *_e_config_sticky_edd = NULL;
static Eet_Data_Descriptor *_e_config_stickies_edd = NULL;
static Eet_Data_Descriptor *_e_config_version_edd = NULL;

static char *
_str_alloc(const char *str)
{
   return (char*) str;
}

static void
_str_free(char *str)
{
   (void) str;
}

static Eina_Hash *
_estickies_hash_add(Eina_Hash *hash, const char *key, void *data)
{
   if (!hash) hash = eina_hash_string_superfast_new(NULL);
   if (!hash) return NULL;

   eina_hash_add(hash, key, data);
   return hash;
}

static Eet_Data_Descriptor *
_estickies_data_descriptor(const char *name, int size)
{
   Eet_Data_Descriptor_Class eddc;

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.name = name;
   eddc.size = size;
   eddc.func.mem_alloc = NULL;
   eddc.func.mem_free = NULL;
   eddc.func.str_alloc = eina_stringshare_add;
   eddc.func.str_free = eina_stringshare_del;
   eddc.func.list_next = eina_list_next;
   eddc.func.list_append = eina_list_append;
   eddc.func.list_data = eina_list_data_get;
   eddc.func.list_free = eina_list_free;
   eddc.func.hash_foreach = eina_hash_foreach;
   eddc.func.hash_add = _estickies_hash_add;
   eddc.func.hash_free = eina_hash_free;
   eddc.func.str_direct_alloc = _str_alloc;
   eddc.func.str_direct_free = _str_free;

   return eet_data_descriptor2_new(&eddc);
}

int
_e_config_init()
{
   char buf[PATH_MAX];
   
   // make sure ~/.e exists and is a dir
   snprintf(buf, sizeof(buf), "%s/.e", home);
   if (!ecore_file_is_dir(buf))
     {
	if (ecore_file_exists(buf) || !ecore_file_mkdir(buf))
	  {
	     ERROR("Cant create config path!");
	     return 0;
	  }
     }
   
   // make sure ~/.e/estickies exists and is a dir
   snprintf(buf, sizeof(buf), "%s/.e/estickies", home);
   if(!ecore_file_is_dir(buf))
     {
	if (ecore_file_exists(buf) || !ecore_file_mkdir(buf))
	  {
	     ERROR("Cant create config path!");
	     return 0;
	  }
     }
   
   _e_config_sticky_edd = NEWD("E_Sticky", E_Sticky);
   CFG_STICKY_NEWI("gx", x, EET_T_INT);
   CFG_STICKY_NEWI("gy", y, EET_T_INT);
   CFG_STICKY_NEWI("gw", w, EET_T_INT);
   CFG_STICKY_NEWI("gh", h, EET_T_INT);
   CFG_STICKY_NEWI("cr", r, EET_T_INT);
   CFG_STICKY_NEWI("cg", g, EET_T_INT);
   CFG_STICKY_NEWI("cb", b, EET_T_INT);
   CFG_STICKY_NEWI("ca", a, EET_T_INT);
   CFG_STICKY_NEWI("st", stick, EET_T_INT);
   CFG_STICKY_NEWI("lk", locked, EET_T_INT);   
   CFG_STICKY_NEWI("tm", theme, EET_T_STRING);   
   CFG_STICKY_NEWI("tx", text, EET_T_STRING);   

   _e_config_stickies_edd = NEWD("E_Config_Stickies", E_Config_Stickies);
   CFG_STICKIES_NEWI("tm", theme, EET_T_STRING);
   CFG_STICKIES_NEWL("st", stickies, _e_config_sticky_edd);
   
   _e_config_version_edd = NEWD("E_Config_Version", E_Config_Version);
   VER_NEWI("mj", major, EET_T_INT);
   VER_NEWI("mn", minor, EET_T_INT);
   VER_NEWI("pa", patch, EET_T_INT);
      
   return 1;
}

int
_e_config_shutdown()
{
   FREED(_e_config_stickies_edd);   
   FREED(_e_config_sticky_edd);
   FREED(_e_config_version_edd);   
   return 1;
}

E_Config_Version *
_e_config_version_parse(char *version)
{
   E_Config_Version *v;
   int res;
   
   v = E_NEW(1, E_Config_Version);
   res = sscanf(version, "%d.%d.%d", &v->major, &v->minor, &v->patch);
   
   if(res < 3)
     return NULL;
   
   return v;
}

/*
 * Compare 2 versions, return 1 if v1 > v2
 *                     return 0 if v1 == v2
 *                     return -1 if v1 < v2
 */
int
_e_config_version_compare(E_Config_Version *v1, E_Config_Version *v2)
{         
   if(v1->major > v2->major)
     return 1;
   else if (v1->major < v2->major)
     return -1;
   
   if(v1->minor > v2->minor)
     return 1;
   else if (v1->minor < v2->minor)
     return -1;
   
   if(v1->patch > v2->patch)
     return 1;
   else if (v1->patch < v2->patch)
     return -1;
   
   return 0;
}

void
_e_config_defaults_apply(E_Stickies *ss)
{
   ss->version = _e_config_version_parse(VERSION);
   ss->stickies = NULL;
   ss->theme = strdup(DEFAULT_THEME);
}     

int
_e_config_load(E_Stickies *ss)
{
   Eet_File *ef;
   char      buf[PATH_MAX];
   int size;
   E_Config_Stickies *stickies = NULL;   
   
   snprintf(buf, sizeof(buf), "%s/.e/estickies/config.eet", home);
   
   if(!ecore_file_exists(buf) || ecore_file_size(buf) == 0)
     {
	/* no saved config */
	_e_about_show();
	_e_config_defaults_apply(ss);	
	return 0;
     }
   
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if(!ef)
     {
	ERROR("Cant open configuration file! Using program defaults.");
	return 0;
     }
      
   ss->version = NULL;
   ss->version = eet_data_read(ef, _e_config_version_edd, "config/version");
   if(!ss->version)
     {
	ERROR("Incompatible configuration file! Creating new one.");
	eet_close(ef);
	_e_config_defaults_apply(ss);
	return 0;
     }
   else
     {
	E_Config_Version *v;
	
	v = _e_config_version_parse(VERSION);
	if(_e_config_version_compare(v, ss->version) != 0)
	  {
	     ERROR("Your version / configuration of E-Stickies is not valid!");
	     eet_close(ef);	     
	     _e_config_defaults_apply(ss);	     
	     return 0;
	  }
     }
   
   stickies = eet_data_read(ef, _e_config_stickies_edd, "config/stickies");
   if(stickies)
     {
	//printf ("found %d stickies in conf!\n", eina_list_count(stickies->stickies));
	ss->stickies = stickies->stickies;	
     }
   //else
     //printf("no stickies found in conf!\n");
   E_FREE(stickies);
   
   ss->theme = NULL;
   ss->theme = eet_read(ef, "config/theme", &size);
   if(size <= 0 || !ss->theme)
     ss->theme = strdup(DEFAULT_THEME);   
   
   eet_close(ef);
   return 1;
}

int
_e_config_save(E_Stickies *ss)
{
   Eet_File  *ef;
   char       buf[PATH_MAX];
   int        ret;
   E_Config_Stickies *stickies = NULL;
   Eina_List *l;
   E_Sticky *s;
   
   snprintf(buf, sizeof(buf), "%s/.e/estickies/config.eet", home);
   
   ef = eet_open(buf, EET_FILE_MODE_WRITE);
   if(!ef)
     return 0;
   
   ret = eet_data_write(ef, _e_config_version_edd, "config/version", ss->version, 1);
   if(!ret)
     DEBUG("Problem saving config!");

   EINA_LIST_FOREACH(ss->stickies, l, s)
     {
	E_FREE(s->text);
	s->text = strdup(elm_entry_entry_get(s->textentry));
     }

   stickies = E_NEW(1, E_Config_Stickies);
   stickies->stickies = ss->stickies;
   //printf("saving %d stickies to conf\n", eina_list_count(ss->stickies));
   ret = eet_data_write(ef, _e_config_stickies_edd, "config/stickies", stickies, 1);
   if(!ret)
     DEBUG("Problem saving config/stickies!");

   E_FREE(stickies);

   if(!eet_write(ef, "config/theme", ss->theme, strlen(ss->theme) + 1, 1))
     DEBUG("Problem saving config/theme!");
      
   eet_close(ef);
   return ret;
}
