#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <Eet.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "Efreet.h"
#include "efreet_private.h"
#include "efreet_cache_private.h"

typedef struct _Efreet_Old_Cache Efreet_Old_Cache;

struct _Efreet_Old_Cache
{
    Eina_Hash *hash;
    Eet_File *ef;
};

#ifdef EFREET_MODULE_LOG_DOM
#undef EFREET_MODULE_LOG_DOM
#endif
#define EFREET_MODULE_LOG_DOM _efreet_cache_log_dom

static int _efreet_cache_log_dom = -1;

/**
 * Data for cache files
 */
#ifdef ICON_CACHE
static Eet_Data_Descriptor *directory_edd = NULL;
static Eet_Data_Descriptor *icon_theme_edd = NULL;
static Eet_Data_Descriptor *icon_theme_directory_edd = NULL;
static Eet_Data_Descriptor *icons_edd = NULL;
static Eet_Data_Descriptor *fallback_edd = NULL;

static Eet_Data_Descriptor *icon_fallback_edd = NULL;
static Eet_Data_Descriptor *icon_element_pointer_edd = NULL;
static Eet_Data_Descriptor *icon_element_edd = NULL;
static Eet_Data_Descriptor *icon_edd = NULL;

static Eet_File            *icon_cache = NULL;
static Eet_File            *icon_theme_cache = NULL;

static const char          *icon_theme_cache_file = NULL;

static const char          *theme_name = NULL;
static Efreet_Cache_Icons  *theme_cache = NULL;
static Efreet_Cache_Icons  *fallback_cache = NULL;
#endif

static Eet_Data_Descriptor *version_edd = NULL;
static Eet_Data_Descriptor *desktop_edd = NULL;

static Eet_File            *desktop_cache = NULL;
static const char          *desktop_cache_dirs = NULL;
static const char          *desktop_cache_file = NULL;

static Ecore_File_Monitor  *cache_monitor = NULL;

static Ecore_Event_Handler *cache_exe_handler = NULL;
#ifdef ICON_CACHE
static Ecore_Job           *icon_cache_job = NULL;
static Ecore_Exe           *icon_cache_exe = NULL;
static int                  icon_cache_exe_lock = -1;
#endif
static Ecore_Job           *desktop_cache_job = NULL;
static Ecore_Exe           *desktop_cache_exe = NULL;
static int                  desktop_cache_exe_lock = -1;

static Eina_List           *old_desktop_caches = NULL;

#ifdef ICON_CACHE
static Efreet_Cache_Icons *_efreet_cache_free(Efreet_Cache_Icons *cache);
static Efreet_Cache_Icons *_efreet_cache_fallback_free(Efreet_Cache_Icons *cache);
#endif
static void efreet_cache_edd_shutdown(void);

static Eina_Bool cache_exe_cb(void *data, int type, void *event);
static void cache_update_cb(void *data, Ecore_File_Monitor *em,
                            Ecore_File_Event event, const char *path);

static void desktop_cache_update_cache_job(void *data);
#ifdef ICON_CACHE
static void icon_cache_update_cache_job(void *data);
#endif
static void desktop_cache_update_free(void *data, void *ev);
#ifdef ICON_CACHE
static void icon_cache_update_free(void *data, void *ev);
#endif

EAPI int EFREET_EVENT_ICON_CACHE_UPDATE = 0;
EAPI int EFREET_EVENT_DESKTOP_CACHE_UPDATE = 0;

int
efreet_cache_init(void)
{
    char buf[PATH_MAX];

    _efreet_cache_log_dom = eina_log_domain_register("efreet_cache", EFREET_DEFAULT_LOG_COLOR);
    if (_efreet_cache_log_dom < 0)
        return 0;

#ifdef ICON_CACHE
    EFREET_EVENT_ICON_CACHE_UPDATE = ecore_event_type_new();
#endif
    EFREET_EVENT_DESKTOP_CACHE_UPDATE = ecore_event_type_new();

    snprintf(buf, sizeof(buf), "%s/efreet", efreet_cache_home_get());
    if (!ecore_file_mkpath(buf)) goto error;

    if (efreet_cache_update)
    {
        cache_exe_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                                    cache_exe_cb, NULL);
        if (!cache_exe_handler) goto error;

        cache_monitor = ecore_file_monitor_add(buf,
                                               cache_update_cb,
                                               NULL);
        if (!cache_monitor) goto error;

#ifdef ICON_CACHE
        efreet_cache_icon_update();
#endif
        efreet_cache_desktop_update();
    }

    return 1;
error:
    if (cache_exe_handler) ecore_event_handler_del(cache_exe_handler);
    cache_exe_handler = NULL;
    if (cache_monitor) ecore_file_monitor_del(cache_monitor);
    cache_monitor = NULL;
    efreet_cache_edd_shutdown();
    return 0;
}

void
efreet_cache_shutdown(void)
{
    Efreet_Old_Cache *d;

#ifdef ICON_CACHE
    theme_cache = _efreet_cache_free(theme_cache);
    fallback_cache = _efreet_cache_fallback_free(fallback_cache);

    IF_RELEASE(theme_name);

    icon_cache = efreet_cache_close(icon_cache);
    icon_theme_cache = efreet_cache_close(icon_theme_cache);
#endif

    desktop_cache = efreet_cache_close(desktop_cache);
    IF_RELEASE(desktop_cache_file);
    IF_RELEASE(desktop_cache_dirs);

    if (cache_exe_handler) ecore_event_handler_del(cache_exe_handler);
    cache_exe_handler = NULL;
    if (cache_monitor) ecore_file_monitor_del(cache_monitor);
    cache_monitor = NULL;

    efreet_cache_edd_shutdown();
    if (desktop_cache_job)
    {
        ecore_job_del(desktop_cache_job);
        desktop_cache_job = NULL;
    }
#ifdef ICON_CACHE
    IF_RELEASE(icon_theme_cache_file);
    if (icon_cache_exe_lock > 0)
    {
        close(icon_cache_exe_lock);
        icon_cache_exe_lock = -1;
    }
#endif

    if (desktop_cache_exe_lock > 0)
    {
        close(desktop_cache_exe_lock);
        desktop_cache_exe_lock = -1;
    }

    EINA_LIST_FREE(old_desktop_caches, d)
    {
        eina_hash_free(d->hash);
        free(d);
    }

    eina_log_domain_unregister(_efreet_cache_log_dom);
}

#ifdef ICON_CACHE
/*
 * Needs EAPI because of helper binaries
 */
EAPI const char *
efreet_icon_cache_file(void)
{
    static char cache_file[PATH_MAX] = { '\0' };
    const char *cache;

    cache = efreet_cache_home_get();

    snprintf(cache_file, sizeof(cache_file), "%s/efreet/icons_%s.eet", cache, efreet_hostname_get());

    return cache_file;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI const char *
efreet_icon_theme_cache_file(void)
{
    char tmp[PATH_MAX] = { '\0' };

    if (icon_theme_cache_file) return icon_theme_cache_file;

    snprintf(tmp, sizeof(tmp), "%s/efreet/icon_themes_%s.eet",
             efreet_cache_home_get(), efreet_hostname_get());
    icon_theme_cache_file = eina_stringshare_add(tmp);

    return icon_theme_cache_file;
}
#endif

/*
 * Needs EAPI because of helper binaries
 */
EAPI Eet_Data_Descriptor *
efreet_version_edd(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (version_edd) return version_edd;

    EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Cache_Version);
    version_edd = eet_data_descriptor_file_new(&eddc);
    if (!version_edd) return NULL;

    EET_DATA_DESCRIPTOR_ADD_BASIC(version_edd, Efreet_Cache_Version,
                                  "minor", minor, EET_T_UCHAR);
    EET_DATA_DESCRIPTOR_ADD_BASIC(version_edd, Efreet_Cache_Version,
                                  "major", major, EET_T_UCHAR);

    return version_edd;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI const char *
efreet_desktop_cache_file(void)
{
    char tmp[PATH_MAX] = { '\0' };
    const char *cache, *lang, *country, *modifier;

    if (desktop_cache_file) return desktop_cache_file;

    cache = efreet_cache_home_get();
    lang = efreet_lang_get();
    country = efreet_lang_country_get();
    modifier = efreet_lang_modifier_get();

    if (lang && country && modifier)
        snprintf(tmp, sizeof(tmp), "%s/efreet/desktop_%s_%s_%s@%s.eet", cache, efreet_hostname_get(), lang, country, modifier);
    else if (lang && country)
        snprintf(tmp, sizeof(tmp), "%s/efreet/desktop_%s_%s_%s.eet", cache, efreet_hostname_get(), lang, country);
    else if (lang)
        snprintf(tmp, sizeof(tmp), "%s/efreet/desktop_%s_%s.eet", cache, efreet_hostname_get(), lang);
    else
        snprintf(tmp, sizeof(tmp), "%s/efreet/desktop_%s.eet", cache, efreet_hostname_get());

    desktop_cache_file = eina_stringshare_add(tmp);
    return desktop_cache_file;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI const char *
efreet_desktop_cache_dirs(void)
{
    char tmp[PATH_MAX] = { '\0' };

    if (desktop_cache_dirs) return desktop_cache_dirs;

    snprintf(tmp, sizeof(tmp), "%s/efreet/desktop_dirs.cache", efreet_cache_home_get());

    desktop_cache_dirs = eina_stringshare_add(tmp);
    return desktop_cache_dirs;
}

#define EDD_SHUTDOWN(Edd)                       \
    if (Edd) eet_data_descriptor_free(Edd);       \
Edd = NULL;

static void
efreet_cache_edd_shutdown(void)
{
    EDD_SHUTDOWN(version_edd);
    EDD_SHUTDOWN(desktop_edd);
#ifdef ICON_CACHE
    EDD_SHUTDOWN(fallback_edd);
    EDD_SHUTDOWN(icon_theme_edd);
    EDD_SHUTDOWN(icon_theme_directory_edd);
    EDD_SHUTDOWN(icons_edd);
    EDD_SHUTDOWN(directory_edd);
    EDD_SHUTDOWN(icon_fallback_edd);
    EDD_SHUTDOWN(icon_element_pointer_edd);
    EDD_SHUTDOWN(icon_element_edd);
    EDD_SHUTDOWN(icon_edd);
#endif
}

#ifdef ICON_CACHE

#define EFREET_POINTER_TYPE(Edd_Dest, Edd_Source, Type)   \
{                                                                     \
    typedef struct _Efreet_##Type##_Pointer Efreet_##Type##_Pointer;   \
    struct _Efreet_##Type##_Pointer                                    \
    {                                                                  \
        Efreet_##Type *pointer;                                         \
    };                                                                 \
    \
    EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_##Type##_Pointer); \
    Edd_Dest = eet_data_descriptor_file_new(&eddc);                    \
    EET_DATA_DESCRIPTOR_ADD_SUB(Edd_Dest, Efreet_##Type##_Pointer,     \
                                "pointer", pointer, Edd_Source);       \
}

static Efreet_Cache_Icons *
_efreet_cache_free(Efreet_Cache_Icons *c)
{
    if (!c) return NULL;

    if (c->icons) efreet_hash_free(c->icons, EINA_FREE_CB(efreet_cache_icon_free));
    if (c->dirs) efreet_hash_free(c->dirs, free);
    free(c);

    return NULL;
}

static Efreet_Cache_Icons *
_efreet_cache_fallback_free(Efreet_Cache_Icons *c)
{
    if (!c) return NULL;

    if (c->icons) efreet_hash_free(c->icons, EINA_FREE_CB(efreet_cache_icon_fallback_free));
    if (c->dirs) efreet_hash_free(c->dirs, free);
    free(c);

    return NULL;
}

static Eet_Data_Descriptor *
efreet_icon_directory_edd(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (directory_edd) return directory_edd;

    EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Cache_Directory);
    directory_edd = eet_data_descriptor_file_new(&eddc);
    if (!directory_edd) return NULL;

    EET_DATA_DESCRIPTOR_ADD_BASIC(directory_edd, Efreet_Cache_Directory,
                                  "modified_time", modified_time, EET_T_LONG_LONG);

    return directory_edd;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI Eet_Data_Descriptor *
efreet_icons_edd(Eina_Bool include_dirs)
{
    Eet_Data_Descriptor_Class eddc;

    if (icons_edd) return icons_edd;

    EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Cache_Icon_Element);
    icon_element_edd = eet_data_descriptor_file_new(&eddc);
    if (!icon_element_edd) return NULL;

    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_element_edd, Efreet_Cache_Icon_Element,
                                  "type", type, EET_T_USHORT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_element_edd, Efreet_Cache_Icon_Element,
                                  "normal", normal, EET_T_USHORT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_element_edd, Efreet_Cache_Icon_Element,
                                  "normal", normal, EET_T_USHORT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_element_edd, Efreet_Cache_Icon_Element,
                                  "min", min, EET_T_USHORT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_element_edd, Efreet_Cache_Icon_Element,
                                  "max", max, EET_T_USHORT);
    EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY_STRING(icon_element_edd, Efreet_Cache_Icon_Element,
                                             "paths", paths);

    EFREET_POINTER_TYPE(icon_element_pointer_edd, icon_element_edd, Cache_Icon_Element);

    EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Cache_Icon);
    icon_edd = eet_data_descriptor_file_new(&eddc);
    if (!icon_edd) return NULL;

    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_edd, Efreet_Cache_Icon,
                                  "theme", theme, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(icon_edd, Efreet_Cache_Icon,
                                      "icons", icons, icon_element_pointer_edd);

    EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Cache_Icons);
    icons_edd = eet_data_descriptor_file_new(&eddc);
    if (!icons_edd) return NULL;

    EET_DATA_DESCRIPTOR_ADD_HASH(icons_edd, Efreet_Cache_Icons,
                                 "icons", icons, icon_edd);

    if (include_dirs)
      EET_DATA_DESCRIPTOR_ADD_HASH(icons_edd, Efreet_Cache_Icons,
                                   "dirs", dirs, efreet_icon_directory_edd());

    return icons_edd;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI Eet_Data_Descriptor *
efreet_icon_theme_edd(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (icon_theme_edd) return icon_theme_edd;

    EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Icon_Theme_Directory);
    icon_theme_directory_edd = eet_data_descriptor_file_new(&eddc);
    if (!icon_theme_directory_edd) return NULL;

    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_directory_edd, Efreet_Icon_Theme_Directory,
                                  "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_directory_edd, Efreet_Icon_Theme_Directory,
                                  "context", context, EET_T_UCHAR);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_directory_edd, Efreet_Icon_Theme_Directory,
                                  "type", type, EET_T_UCHAR);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_directory_edd, Efreet_Icon_Theme_Directory,
                                  "size.normal", size.normal, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_directory_edd, Efreet_Icon_Theme_Directory,
                                  "size.min", size.min, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_directory_edd, Efreet_Icon_Theme_Directory,
                                  "size.max", size.max, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_directory_edd, Efreet_Icon_Theme_Directory,
                                  "size.threshold", size.threshold, EET_T_UINT);

    EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Icon_Theme);
    icon_theme_edd = eet_data_descriptor_file_new(&eddc);
    if (!icon_theme_edd) return NULL;

    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_edd, Efreet_Icon_Theme,
                                  "name.internal", name.internal, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_edd, Efreet_Icon_Theme,
                                  "name.name", name.name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_edd, Efreet_Icon_Theme,
                                  "comment", comment, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_edd, Efreet_Icon_Theme,
                                  "example_icon", example_icon, EET_T_STRING);

    eet_data_descriptor_element_add(icon_theme_edd, "paths", EET_T_STRING, EET_G_LIST,
                                    offsetof(Efreet_Icon_Theme, paths), 0, NULL, NULL);
    eet_data_descriptor_element_add(icon_theme_edd, "inherits", EET_T_STRING, EET_G_LIST,
                                    offsetof(Efreet_Icon_Theme, inherits), 0, NULL, NULL);
    EET_DATA_DESCRIPTOR_ADD_LIST(icon_theme_edd, Efreet_Icon_Theme,
                                  "directories", directories, icon_theme_directory_edd);

    EET_DATA_DESCRIPTOR_ADD_BASIC(icon_theme_edd, Efreet_Icon_Theme,
                                  "last_cache_check", last_cache_check, EET_T_DOUBLE);

    return icon_theme_edd;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI Eet_Data_Descriptor *
efreet_icons_fallback_edd(Eina_Bool include_dirs)
{
    Eet_Data_Descriptor_Class eddc;

    if (fallback_edd) return fallback_edd;

    EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Cache_Fallback_Icon);
    icon_fallback_edd = eet_data_descriptor_file_new(&eddc);
    if (!icon_fallback_edd) return NULL;

    EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY_STRING(icon_fallback_edd,
                                             Efreet_Cache_Fallback_Icon, "icons", icons);

    EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Cache_Icons);
    fallback_edd = eet_data_descriptor_file_new(&eddc);
    if (!fallback_edd) return NULL;

    EET_DATA_DESCRIPTOR_ADD_HASH(fallback_edd, Efreet_Cache_Icons,
                                 "icons", icons, icon_fallback_edd);

    if (include_dirs)
      EET_DATA_DESCRIPTOR_ADD_HASH(fallback_edd, Efreet_Cache_Icons,
                                   "dirs", dirs, efreet_icon_directory_edd());

    return fallback_edd;
}
#endif

/*
 * Needs EAPI because of helper binaries
 */
EAPI Eet_Data_Descriptor *
efreet_desktop_edd(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (desktop_edd) return desktop_edd;

    EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Desktop);
    desktop_edd = eet_data_descriptor_file_new(&eddc);
    if (!desktop_edd) return NULL;

    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "type", type, EET_T_INT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "version", version, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "orig_path", orig_path, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "load_time", load_time, EET_T_LONG_LONG);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "generic_name", generic_name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "comment", comment, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "icon", icon, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "try_exec", try_exec, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "exec", exec, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "path", path, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "startup_wm_class", startup_wm_class, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "url", url, EET_T_STRING);
    eet_data_descriptor_element_add(desktop_edd, "only_show_in", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, only_show_in), 0, NULL, NULL);
    eet_data_descriptor_element_add(desktop_edd, "not_show_in", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, not_show_in), 0, NULL, NULL);
    eet_data_descriptor_element_add(desktop_edd, "categories", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, categories), 0, NULL, NULL);
    eet_data_descriptor_element_add(desktop_edd, "mime_types", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, mime_types), 0, NULL, NULL);
    eet_data_descriptor_element_add(desktop_edd, "x", EET_T_STRING, EET_G_HASH, offsetof(Efreet_Desktop, x), 0, NULL, NULL);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "no_display", no_display, EET_T_UCHAR);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "hidden", hidden, EET_T_UCHAR);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "terminal", terminal, EET_T_UCHAR);
    EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "startup_notify", startup_notify, EET_T_UCHAR);

    return desktop_edd;
}

#ifdef ICON_CACHE
/*
 * Needs EAPI because of helper binaries
 */
EAPI void
efreet_cache_icon_free(Efreet_Cache_Icon *icon)
{
    unsigned int i;

    if (!icon) return;

    for (i = 0; i < icon->icons_count; ++i)
    {
        free(icon->icons[i]->paths);
        free(icon->icons[i]);
    }

    free(icon->icons);
    free(icon);
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI void
efreet_cache_icon_fallback_free(Efreet_Cache_Fallback_Icon *icon)
{
    if (!icon) return;

    free(icon->icons);
    free(icon);
}

Efreet_Cache_Icon *
efreet_cache_icon_find(Efreet_Icon_Theme *theme, const char *icon)
{
    if (!efreet_cache_check(&icon_cache, efreet_icon_cache_file(), EFREET_ICON_CACHE_MAJOR)) return NULL;

    if (theme_name && strcmp(theme_name, theme->name.internal))
    {
        /* FIXME: this is bad if people have pointer to this cache, things will go wrong */
        INFO("theme_name change from `%s` to `%s`", theme_name, theme->name.internal);
        IF_RELEASE(theme_name);
        theme_cache = _efreet_cache_free(theme_cache);
    }

    if (!theme_name)
    {
        INFO("loading theme %s", theme->name.internal);
        theme_cache = eet_data_read(icon_cache, efreet_icons_edd(EINA_FALSE), theme->name.internal);
        if (theme_cache && !theme_cache->icons)
            theme_cache->icons = eina_hash_string_superfast_new(NULL);
        if (theme_cache)
            theme_name = eina_stringshare_add(theme->name.internal);
    }

    if (!theme_cache) return NULL;

    return eina_hash_find(theme_cache->icons, icon);
}

Efreet_Cache_Fallback_Icon *
efreet_cache_icon_fallback_find(const char *icon)
{
    if (!efreet_cache_check(&icon_cache, efreet_icon_cache_file(), EFREET_ICON_CACHE_MAJOR)) return NULL;

    if (!fallback_cache)
    {
        INFO("loading fallback cache");
        fallback_cache = eet_data_read(icon_cache, efreet_icons_fallback_edd(EINA_FALSE), EFREET_CACHE_ICON_FALLBACK);
        if (fallback_cache && !fallback_cache->icons)
            fallback_cache->icons = eina_hash_string_superfast_new(NULL);
    }

    if (!fallback_cache) return NULL;

    return eina_hash_find(fallback_cache->icons, icon);
}

Efreet_Icon_Theme *
efreet_cache_icon_theme_find(const char *theme)
{
    if (!efreet_cache_check(&icon_theme_cache, efreet_icon_theme_cache_file(), EFREET_ICON_CACHE_MAJOR)) return NULL;
    return eet_data_read(icon_theme_cache, efreet_icon_theme_edd(), theme);
}

void
efreet_cache_icon_theme_free(Efreet_Icon_Theme *theme)
{
    void *data;

    if (!theme) return;

    eina_list_free(theme->paths);
    eina_list_free(theme->inherits);
    EINA_LIST_FREE(theme->directories, data)
        free(data);

    free(theme);
}

char **
efreet_cache_icon_theme_name_list(int *num)
{
    char **keys;
    int i;

    if (!efreet_cache_check(&icon_theme_cache, efreet_icon_theme_cache_file(), EFREET_ICON_CACHE_MAJOR)) return NULL;
    keys = eet_list(icon_theme_cache, "*", num);
    for (i = 0; i < *num; i++)
    {
        if (!strcmp(keys[i], EFREET_CACHE_VERSION) && (i < (*num + 1)))
        {
            memmove(&keys[i], &keys[i + 1], (*num - i - 1) * sizeof(char *));
            (*num)--;
            break;
        }
    }
    return keys;
}

#endif

Efreet_Desktop *
efreet_cache_desktop_find(const char *file)
{
    Efreet_Desktop *desktop;
    char rp[PATH_MAX];

    if (!realpath(file, rp)) return NULL;

    if (!efreet_cache_check(&desktop_cache, efreet_desktop_cache_file(), EFREET_DESKTOP_CACHE_MAJOR)) return NULL;

    desktop = eet_data_read(desktop_cache, efreet_desktop_edd(), rp);
    if (!desktop) return NULL;
    desktop->ref = 1;
    desktop->eet = 1;
    return desktop;
}

void
efreet_cache_desktop_update(void)
{
    if (!efreet_cache_update) return;

    /* TODO: Make sure we don't create a lot of execs, maybe use a timer? */
    if (desktop_cache_job) ecore_job_del(desktop_cache_job);
    desktop_cache_job = ecore_job_add(desktop_cache_update_cache_job, NULL);
}

#ifdef ICON_CACHE
void
efreet_cache_icon_update(void)
{
    if (!efreet_cache_update) return;

    /* TODO: Make sure we don't create a lot of execs, maybe use a timer? */
    if (icon_cache_job) ecore_job_del(icon_cache_job);
    icon_cache_job = ecore_job_add(icon_cache_update_cache_job, NULL);
}
#endif

void
efreet_cache_desktop_free(Efreet_Desktop *desktop)
{
    Efreet_Old_Cache *d;
    Efreet_Desktop *curr;
    Eina_List *l;

    if (!old_desktop_caches) return;

    EINA_LIST_FOREACH(old_desktop_caches, l, d)
    {
        curr = eina_hash_find(d->hash, desktop->orig_path);
        if (curr && curr == desktop)
        {
            eina_hash_del_by_key(d->hash, desktop->orig_path);
            if (eina_hash_population(d->hash) == 0)
            {
                eina_hash_free(d->hash);
                d->hash = NULL;
            }
            break;
        }
    }
}

Eina_Bool
efreet_cache_check(Eet_File **ef, const char *path, int major)
{
    Efreet_Cache_Version *version;

    if (*ef == NON_EXISTING) return EINA_FALSE;
    if (*ef) return EINA_TRUE;
    if (!*ef)
        *ef = eet_open(path, EET_FILE_MODE_READ);
    if (!*ef)
    {
        *ef = NON_EXISTING;
        return EINA_FALSE;
    }

    version = eet_data_read(*ef, efreet_version_edd(), EFREET_CACHE_VERSION);
    if ((!version) || (version->major != major))
    {
        IF_FREE(version);
        eet_close(*ef);
        *ef = NON_EXISTING;
        return EINA_FALSE;
    }
    free(version);
    return EINA_TRUE;
}

void *
efreet_cache_close(Eet_File *ef)
{
    if (ef && ef != NON_EXISTING)
        eet_close(ef);
    return NULL;
}

static Eina_Bool
cache_exe_cb(void *data __UNUSED__, int type __UNUSED__, void *event)
{
    Ecore_Exe_Event_Del *ev;

    ev = event;
    if (ev->exe == desktop_cache_exe)
    {
        if (desktop_cache_exe_lock > 0)
        {
            close(desktop_cache_exe_lock);
            desktop_cache_exe_lock = -1;
        }
        desktop_cache_exe = NULL;
    }
#ifdef ICON_CACHE
    else if (ev->exe == icon_cache_exe)
    {
        if (icon_cache_exe_lock > 0)
        {
            close(icon_cache_exe_lock);
            icon_cache_exe_lock = -1;
        }
        icon_cache_exe = NULL;
    }
#endif
    return ECORE_CALLBACK_RENEW;
}

static void
cache_update_cb(void *data __UNUSED__, Ecore_File_Monitor *em __UNUSED__,
                Ecore_File_Event event, const char *path)
{
    const char *file;
    Efreet_Event_Cache_Update *ev = NULL;
    Efreet_Old_Cache *d = NULL;

    if (event != ECORE_FILE_EVENT_CREATED_FILE &&
        event != ECORE_FILE_EVENT_MODIFIED) return;

    file = ecore_file_file_get(path);
    if (!file) return;
    if (!strcmp(file, "desktop_data.update"))
    {
        ev = NEW(Efreet_Event_Cache_Update, 1);
        if (!ev) goto error;
        d = NEW(Efreet_Old_Cache, 1);
        if (!d) goto error;

        d->hash = efreet_desktop_cache;
        d->ef = desktop_cache;
        old_desktop_caches = eina_list_append(old_desktop_caches, d);

        efreet_desktop_cache = eina_hash_string_superfast_new(NULL);
        desktop_cache = NULL;

        efreet_util_desktop_cache_reload();
        ecore_event_add(EFREET_EVENT_DESKTOP_CACHE_UPDATE, ev, desktop_cache_update_free, d);
    }
#ifdef ICON_CACHE
    else if (!strcmp(file, "icon_data.update"))
    {
        ev = NEW(Efreet_Event_Cache_Update, 1);
        if (!ev) goto error;
        d = NEW(Efreet_Old_Cache, 1);
        if (!d) goto error;

        if (theme_cache)
        {
            INFO("Destorying theme cache due to cache change.");
            theme_cache = _efreet_cache_free(theme_cache);
        }

        IF_RELEASE(theme_name);

        if (fallback_cache)
        {
            INFO("Destroying fallback cache due to cache change.");
            fallback_cache = _efreet_cache_fallback_free(fallback_cache);
        }

        icon_cache = efreet_cache_close(icon_cache);

        d->hash = efreet_icon_themes;
        d->ef = icon_theme_cache;

        efreet_icon_themes = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_icon_theme_free));
        icon_theme_cache = NULL;

        ecore_event_add(EFREET_EVENT_ICON_CACHE_UPDATE, ev, icon_cache_update_free, d);
    }
#endif
    return;
error:
    IF_FREE(ev);
    IF_FREE(d);
}

static void
desktop_cache_update_cache_job(void *data __UNUSED__)
{
    char file[PATH_MAX];
    struct flock fl;
    int prio;

    desktop_cache_job = NULL;

    /* TODO: Retry update cache later */
    if (desktop_cache_exe_lock > 0) return;

    if (!efreet_desktop_write_cache_dirs_file()) return;

    snprintf(file, sizeof(file), "%s/efreet/desktop_exec.lock", efreet_cache_home_get());

    desktop_cache_exe_lock = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (desktop_cache_exe_lock < 0) return;
    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(desktop_cache_exe_lock, F_SETLK, &fl) < 0) goto error;
    prio = ecore_exe_run_priority_get();
    ecore_exe_run_priority_set(19);
    desktop_cache_exe = ecore_exe_run(PACKAGE_LIB_DIR "/efreet/efreet_desktop_cache_create", NULL);
    ecore_exe_run_priority_set(prio);
    if (!desktop_cache_exe) goto error;

    return;

error:
    if (desktop_cache_exe_lock > 0)
    {
        close(desktop_cache_exe_lock);
        desktop_cache_exe_lock = -1;
    }
}

#ifdef ICON_CACHE
static void
icon_cache_update_cache_job(void *data __UNUSED__)
{
    char file[PATH_MAX];
    struct flock fl;
    int prio;

    icon_cache_job = NULL;

    /* TODO: Retry update cache later */
    if (icon_cache_exe_lock > 0) return;

    snprintf(file, sizeof(file), "%s/efreet/icon_exec.lock", efreet_cache_home_get());

    icon_cache_exe_lock = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (icon_cache_exe_lock < 0) return;
    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(icon_cache_exe_lock, F_SETLK, &fl) < 0) goto error;
    prio = ecore_exe_run_priority_get();
    ecore_exe_run_priority_set(19);
    icon_cache_exe = ecore_exe_run(PACKAGE_LIB_DIR "/efreet/efreet_icon_cache_create", NULL);
    ecore_exe_run_priority_set(prio);
    if (!icon_cache_exe) goto error;

    return;

error:
    if (icon_cache_exe_lock > 0)
    {
        close(icon_cache_exe_lock);
        icon_cache_exe_lock = -1;
    }
}
#endif

static void
desktop_cache_update_free(void *data, void *ev)
{
    Efreet_Old_Cache *d;
    int dangling = 0;

    d = data;
    /*
     * All users should now had the chance to update their pointers, so we can now
     * free the old cache
     */
    if (d->hash)
    {
        Eina_Iterator *it;
        Eina_Hash_Tuple *tuple;

        it = eina_hash_iterator_tuple_new(d->hash);
        EINA_ITERATOR_FOREACH(it, tuple)
        {
            printf("Efreet: %d:%s still in cache on cache close!\n",
                   ((Efreet_Desktop *)tuple->data)->ref, (char *)tuple->key);
            dangling++;
        }
        eina_iterator_free(it);

        eina_hash_free(d->hash);
    }
    /*
     * If there are dangling references the eet file won't be closed - to
     * avoid crashes, but this will leak instead.
     */
    if (dangling == 0)
    {
        efreet_cache_close(d->ef);
    }
    else
    {
        printf("Efreet: ERROR. There are still %i desktop files with old\n"
               "dangling references to desktop files. This application\n"
               "has not handled the EFREET_EVENT_DESKTOP_CACHE_UPDATE\n"
               "fully and released its references. Please fix the application\n"
               "so it does this.\n",
               dangling);
    }
    old_desktop_caches = eina_list_remove(old_desktop_caches, d);
    free(d);
    free(ev);
}

#ifdef ICON_CACHE
static void
icon_cache_update_free(void *data, void *ev)
{
    Efreet_Old_Cache *d;

    d = data;
    if (d->hash)
        eina_hash_free(d->hash);
    efreet_cache_close(d->ef);
    free(d);
    free(ev);
}
#endif
