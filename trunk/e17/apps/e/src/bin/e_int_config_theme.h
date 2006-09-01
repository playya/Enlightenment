/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS
#else
#ifndef E_INT_CONFIG_THEME_H
#define E_INT_CONFIG_THEME_H

EAPI E_Config_Dialog *e_int_config_theme(E_Container *con);
EAPI void             e_int_config_theme_import_done(E_Config_Dialog *dia);
EAPI void             e_int_config_theme_update(E_Config_Dialog *dia, char *file);

#endif
#endif
