#include <e_mod_volume.h>
#include <e_mod_main.h>
#include <e_mod_face.h>
#include <e_mod_cdialog.h>
#include <e.h>

typedef struct _elemdata Elem_CFData;

struct _E_Config_Dialog_Data
{
   Evas_List *elems;
   Config_Mixer *mixer_conf;
};

struct _elemdata
{
   Config_Mixer_Elem *elem;
   int active;
   int balance;
};

struct _cfg
{
   Volume_Face *face;
   Config_Mixer *mixer_conf;
};

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

void
e_volume_config_mixer(void *data, void *data2)
{
   Volume_Face *face;
   Config_Mixer *mixer_conf;
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   struct _cfg *c;

   face = data;
   mixer_conf = data2;

   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = NULL;
   v->advanced.create_widgets = NULL;

   c = malloc(sizeof(struct _cfg));
   c->mixer_conf = mixer_conf;
   c->face = face;
   cfd = e_config_dialog_new(face->con, _("Mixer Face Configuration"), NULL, 0, v, c);
}

static void
_fill_data(Config_Mixer *mixer_conf, E_Config_Dialog_Data *cfdata)
{
   Evas_List *l;

   cfdata->elems = NULL;
   cfdata->mixer_conf = mixer_conf;
   for (l = mixer_conf->elems; l; l = evas_list_next(l))
     {
        Elem_CFData *ecfd;

        ecfd = calloc(1, sizeof(Elem_CFData));
        ecfd->elem = evas_list_data(l);
        ecfd->active = ecfd->elem->active;
        ecfd->balance = ecfd->elem->balance;
        cfdata->elems = evas_list_append(cfdata->elems, ecfd);
     }
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);

   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Evas_List *l;

   for (l = cfdata->elems; l; l = evas_list_next(l))
     {
        Elem_CFData *d;

        d = evas_list_data(l);
        free(d);
     }
   evas_list_free(cfdata->elems);

   free(cfdata);
   free(cfd->data);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ot;
   int i;
   Evas_List *l;
   Config_Mixer *mixer_conf;
   struct _cfg *c;

   c = cfd->data;
   mixer_conf = c->mixer_conf;

   _fill_data(mixer_conf, cfdata);

   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, D_("Mixer Configuration"), 0);
   ot = e_widget_table_add(evas, 0);
   for (l = cfdata->elems, i = 0; l; l = l->next, i++)
     {
        Evas_Object *olabel, *oact, *obal;
        Elem_CFData *ecfd;

        ecfd = l->data;

        olabel = e_widget_label_add(evas, ecfd->elem->name);
        oact = e_widget_check_add(evas, D_("Active"), &(ecfd->active));
        obal = e_widget_check_add(evas, D_("Balance"), &(ecfd->balance));

        e_widget_table_object_append(ot, olabel, 0, i, 1, 1, 1, 1, 1, 1);
        e_widget_table_object_append(ot, oact, 2, i, 1, 1, 0, 1, 1, 1);
        e_widget_table_object_append(ot, obal, 3, i, 1, 1, 0, 1, 1, 1);
     }
   
   e_widget_framelist_object_append(of, ot);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Evas_List *l;
   struct _cfg *c;
   int u;

   c = cfd->data;
   for (l = cfdata->elems; l; l = evas_list_next(l))
     {
        Elem_CFData *ecfd;

        u = 0;
        ecfd = evas_list_data(l);
        if (ecfd->active != ecfd->elem->active || ecfd->balance != ecfd->elem->balance)
          {
             ecfd->elem->active = ecfd->active;
             ecfd->elem->balance = ecfd->balance;
             e_volume_face_mixer_elem_update(c->face, ecfd->elem, cfdata->mixer_conf);
          }
     }
   return 1;
}
