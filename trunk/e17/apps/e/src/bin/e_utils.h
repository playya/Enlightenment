/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS
#else
#ifndef E_UTILS_H
#define E_UTILS_H

EAPI void         e_util_container_fake_mouse_up_later(E_Container *con, int button);
EAPI void         e_util_container_fake_mouse_up_all_later(E_Container *con);
EAPI void         e_util_wakeup(void);
EAPI void         e_util_env_set(const char *var, const char *val);
EAPI E_Zone      *e_util_zone_current_get(E_Manager *man);
EAPI int          e_util_utils_installed(void);
EAPI int          e_util_app_installed(char *app);
EAPI int          e_util_glob_match(const char *str, const char *glob);
EAPI E_Container *e_util_container_number_get(int num);
EAPI E_Zone      *e_util_container_zone_number_get(int con_num, int zone_num);
EAPI int          e_util_head_exec(int head, char *cmd);
EAPI int          e_util_strcmp(char *s1, char *s2);    
EAPI int          e_util_both_str_empty(char *s1, char *s2);
EAPI int          e_util_immortal_check(void);
EAPI int          e_util_edje_icon_list_set(Evas_Object *obj, char *list);
EAPI int          e_util_menu_item_edje_icon_list_set(E_Menu_Item *mi, char *list);
EAPI int          e_util_edje_icon_set(Evas_Object *obj, char *name);
EAPI int          e_util_menu_item_edje_icon_set(E_Menu_Item *mi, char *name);
EAPI E_Container *e_util_container_window_find(Ecore_X_Window win);
EAPI E_Border    *e_util_desk_border_above(E_Border *bd);
EAPI E_Border    *e_util_desk_border_below(E_Border *bd);
    
#endif
#endif
