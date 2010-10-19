Evas_Object *_els_smart_icon_add              (Evas *evas);
Eina_Bool    _els_smart_icon_file_key_set     (Evas_Object *obj, const char *file, const char *key);
Eina_Bool    _els_smart_icon_file_edje_set    (Evas_Object *obj, const char *file, const char *part);
void         _els_smart_icon_file_get         (const Evas_Object *obj, const char **file, const char **key);
void         _els_smart_icon_smooth_scale_set (Evas_Object *obj, Eina_Bool smooth);
Eina_Bool    _els_smart_icon_smooth_scale_get (const Evas_Object *obj);
Evas_Object *_els_smart_icon_object_get       (const Evas_Object *obj);
void         _els_smart_icon_size_get         (const Evas_Object *obj, int *w, int *h);
void         _els_smart_icon_fill_inside_set  (Evas_Object *obj, Eina_Bool fill_inside);
Eina_Bool    _els_smart_icon_fill_inside_get  (const Evas_Object *obj);
void         _els_smart_icon_scale_up_set     (Evas_Object *obj, Eina_Bool scale_up);
Eina_Bool    _els_smart_icon_scale_up_get     (const Evas_Object *obj);
void         _els_smart_icon_scale_down_set   (Evas_Object *obj, Eina_Bool scale_down);
Eina_Bool    _els_smart_icon_scale_down_get   (const Evas_Object *obj);
void         _els_smart_icon_scale_size_set   (Evas_Object *obj, int size);
int          _els_smart_icon_scale_size_get   (const Evas_Object *obj);
void         _els_smart_icon_scale_set        (Evas_Object *obj, double scale);
double       _els_smart_icon_scale_get        (const Evas_Object *obj);
void         _els_smart_icon_orient_set       (Evas_Object *obj, Elm_Image_Orient orient);
Elm_Image_Orient _els_smart_icon_orient_get   (const Evas_Object *obj);
void         _els_smart_icon_edit_set         (Evas_Object *obj, Eina_Bool, Evas_Object *parent);
Eina_Bool    _els_smart_icon_edit_get         (const Evas_Object *obj);
