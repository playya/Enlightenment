/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS
#else
#ifndef E_UTILS_H
#define E_UTILS_H

EAPI void e_util_container_fake_mouse_up_later(E_Container *con, int button);
EAPI void e_util_container_fake_mouse_up_all_later(E_Container *con);
EAPI void e_util_wakeup(void);
EAPI void e_util_env_set(const char *var, const char *val);
    
#endif
#endif
