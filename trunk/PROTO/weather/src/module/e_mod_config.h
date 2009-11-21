#ifndef E_MOD_CONFIG_H
# define E_MOD_CONFIG_H


#define MOD_CONFIG_FILE_EPOCH 0x0001
#define MOD_CONFIG_FILE_GENERATION 0x008d
#define MOD_CONFIG_FILE_VERSION \
   ((MOD_CONFIG_FILE_EPOCH << 16) | MOD_CONFIG_FILE_GENERATION)

typedef struct _Config Config;
typedef struct _Config_Item Config_Item;

#include "e_mod_main.h"

struct _Config 
{
   int version;
   Eina_List *instances, *items;
   const char *mod_dir;
};

struct _Config_Item 
{
   const char *id;
   int celcius;
   const char *location;
   const char *plugin;
   double poll_time;
   Instance *inst;
};

EAPI Config_Item *_weather_config_item_get(Instance *inst, const char *id);
EAPI void _weather_config_updated(Config_Item *ci);
EAPI void _weather_config_new(void);
EAPI void _weather_config_free(void);

EAPI E_Config_Dialog *weather_config_dialog(Config_Item *ci);


extern Config *weather_cfg;

#endif
