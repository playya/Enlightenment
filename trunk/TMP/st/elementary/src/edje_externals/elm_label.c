#include "private.h"

typedef struct _Elm_Params_Label
{
   Elm_Params base;
} Elm_Params_Label;

static void
external_label_state_set(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Label *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   //if (p->base.label) elm_label_label_set(obj, p->base.label);
}

static Eina_Bool
external_label_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "label"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     elm_label_label_set(obj, param->s);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_label_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "label"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     param->s = elm_label_label_get(obj);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_label_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params __UNUSED__)
{
   Elm_Params_Label *mem;

   mem = ELM_NEW(Elm_Params_Label);
   if (!mem)
     return NULL;

   return mem;
}

static Evas_Object *external_label_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
	ERR("no content");
	return NULL;
}

static void
external_label_params_free(void *params)
{
   external_common_params_free(params);
}

static Edje_External_Param_Info external_label_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(label, "label");
DEFINE_EXTERNAL_TYPE_SIMPLE(label, "label");
