#ifndef __EWL_CONFIG_H__
#define __EWL_CONFIG_H__

/**
 * @defgroup Ewl_Config Config: Functions for Manipulating Configuration Data
 *
 * @{
 */

typedef struct _ewl_config Ewl_Config;

struct _ewl_config
{
	time_t                  mtime;
	struct {
		int             enable;
		int             level;
	} debug;
	struct {
		int             font_cache;
		int             image_cache;
		char           *render_method;
	} evas;
	struct {
		char           *name;
		int             cache;
		int             cclass_override;
	} theme;
};

extern Ewl_Config      ewl_config;

int             ewl_config_init(void);
void            ewl_config_shutdown(void);
int             ewl_config_set_str(const char *k, char *v);
int             ewl_config_set_int(const char *k, int v);
int             ewl_config_set_float(const char *k, float v);
char           *ewl_config_get_str(const char *k);
int             ewl_config_get_int(const char *k);
float           ewl_config_get_float(const char *k);
char *          ewl_config_get_render_method(void);

/**
 * @}
 */

#endif				/* __EWL_CONFIG_H__ */
