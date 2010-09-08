Evas_Object *_els_smart_icon_add              (Evas *evas);
Eina_Bool    _els_smart_icon_file_key_set     (Evas_Object *obj, const char *file, const char *key);
Eina_Bool    _els_smart_icon_file_edje_set    (Evas_Object *obj, const char *file, const char *part);
void         _els_smart_icon_smooth_scale_set (Evas_Object *obj, int smooth);
Evas_Object *_els_smart_icon_object_get(Evas_Object *obj);
void         _els_smart_icon_size_get         (Evas_Object *obj, int *w, int *h);
void         _els_smart_icon_fill_inside_set  (Evas_Object *obj, int fill_inside);
void         _els_smart_icon_scale_up_set     (Evas_Object *obj, int scale_up);
void         _els_smart_icon_scale_down_set   (Evas_Object *obj, int scale_down);
void         _els_smart_icon_scale_size_set   (Evas_Object *obj, int size);
void         _els_smart_icon_scale_set        (Evas_Object *obj, double scale);
void         _els_smart_icon_orient_set       (Evas_Object *obj, Elm_Image_Orient orient);

void         _els_smart_icon_edit_set         (Evas_Object *obj, Eina_Bool, Evas_Object *parent);
