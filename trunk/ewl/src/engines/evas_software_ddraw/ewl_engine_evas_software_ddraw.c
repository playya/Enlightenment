/* vim: set sw=8 ts=8 sts=8 expandtab: */
#include "Ewl_Engine_Evas_Software_DDraw.h"
#include "ewl_private.h"
#include "ewl_debug.h"
#include "ewl_macros.h"

static void ee_canvas_setup(Ewl_Window *win, int debug);
static int ee_init(Ewl_Engine *engine);
static void ee_shutdown(Ewl_Engine *engine);

static void *canvas_funcs[EWL_ENGINE_CANVAS_MAX] =
        {
                ee_canvas_setup,
                NULL, NULL, NULL, NULL, NULL
        };

DSTATIC_ASSERT(EWL_ENGINE_CANVAS_MAX == ARRAY_COUNT(canvas_funcs));

Ecore_DList *
ewl_engine_dependancies(void)
{
        Ecore_DList *d;

        DENTER_FUNCTION(DLEVEL_STABLE);

        d = ecore_dlist_new();
        ecore_dlist_append(d, strdup("win32"));
        ecore_dlist_append(d, strdup("evas"));

        DRETURN_PTR(d, DLEVEL_STABLE);
}

Ewl_Engine *
ewl_engine_create(int *argc __UNUSED__, char ** argv __UNUSED__)
{
        Ewl_Engine_Evas_Software_DDraw *engine;

        DENTER_FUNCTION(DLEVEL_STABLE);

        engine = NEW(Ewl_Engine_Evas_Software_DDraw, 1);
        if (!engine)
                DRETURN_PTR(NULL, DLEVEL_STABLE);

        if (!ee_init(EWL_ENGINE(engine)))
        {
                FREE(engine);
                DRETURN_PTR(NULL, DLEVEL_STABLE);
        }

        DRETURN_PTR(EWL_ENGINE(engine), DLEVEL_STABLE);
}

static int
ee_init(Ewl_Engine *engine)
{
        Ewl_Engine_Info *info;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR_RET(engine, FALSE);

        info = NEW(Ewl_Engine_Info, 1);
        info->shutdown = ee_shutdown;
        info->hooks.canvas = canvas_funcs;

        engine->functions = info;

        DRETURN_INT(TRUE, DLEVEL_STABLE);
}

static void
ee_shutdown(Ewl_Engine *engine)
{
        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(engine);

        IF_FREE(engine->functions);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ee_canvas_setup(Ewl_Window *win, int debug)
{
        Evas *evas;
        Ewl_Object *o;
        Evas_Engine_Info *info = NULL;
        Evas_Engine_Info_Software_DDraw *sinfo;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(win);
        DCHECK_TYPE(win, EWL_WINDOW_TYPE);

        o = EWL_OBJECT(win);

        evas = evas_new();
        evas_output_method_set(evas,
                        evas_render_method_lookup("software_ddraw"));

        info = evas_engine_info_get(evas);
        if (!info)
        {
                fprintf(stderr, "Unable to use software_ddraw engine "
                                "for rendering, ");
                exit(-1);
        }

        sinfo = (Evas_Engine_Info_Software_DDraw *)info;

        sinfo->info.window = ecore_win32_window_hwnd_get((Ecore_Win32_Window *)win->window);
        sinfo->info.depth = ecore_win32_screen_depth_get();
        sinfo->info.rotation = 0;

        evas_engine_info_set(evas, info);

        evas_output_size_set(evas, ewl_object_current_w_get(o),
                                        ewl_object_current_h_get(o));
        evas_output_viewport_set(evas, ewl_object_current_x_get(o),
                                        ewl_object_current_y_get(o),
                                        ewl_object_current_w_get(o),
                                        ewl_object_current_h_get(o));
        ewl_embed_canvas_set(EWL_EMBED(win), evas, win->window);

        DLEAVE_FUNCTION(DLEVEL_STABLE);
}



