/*
 * =====================================================================================
 *
 *       Filename:  eyelight_viewer_expose.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  08/14/2008 08:47:29 PM CEST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  (Watchwolf), Atton Jonathan <watchwolf@watchwolf.fr>
 *        Company:
 *
 * =====================================================================================
 */

#include "eyelight_viewer.h"
#include<time.h>
void _eyelight_viewer_expose_window_previous_cb(void *data, Evas_Object *o, const char *emission, const char *source);
void _eyelight_viewer_expose_window_next_cb(void *data, Evas_Object *o, const char *emission, const char *source);
void _eyelight_viewer_expose_slides_load(Eyelight_Viewer* pres);
void _eyelight_viewer_expose_slides_destroy(Eyelight_Viewer* pres);
void _eyelight_viewer_expose_window_next(Eyelight_Viewer* pres, int pos);
void _eyelight_viewer_expose_window_previous(Eyelight_Viewer* pres, int pos);
void _eyelight_viewer_expose_slide_select_cb(void *data, Evas_Object *o, const char *emission, const char *source);
void _eyelight_viewer_expose_next_cb(void *data, Evas_Object *o, const char *emission, const char *source);
void _eyelight_viewer_expose_previous_cb(void *data, Evas_Object *o, const char *emission, const char *source);
void _eyelight_viewer_expose_up_cb(void *data, Evas_Object *o, const char *emission, const char *source);
void _eyelight_viewer_expose_down_cb(void *data, Evas_Object *o, const char *emission, const char *source);

int _eyelight_viewer_expose_slides_load_idle(void *data);



void eyelight_viewer_expose_start(Eyelight_Viewer* pres,int select, int nb_lines, int nb_cols)
{
    Evas_Object *o;
    int nb_slides = nb_lines * nb_cols;


    int first_slide = select/nb_slides * nb_slides;
    pres->expose_current = select-first_slide;
    pres->state = EYELIGHT_VIEWER_STATE_EXPOSE;
    pres->expose_nb_lines = nb_lines;
    pres->expose_nb_cols = nb_cols;
    pres->expose_first_slide = first_slide;

    o = edje_object_add(pres->evas);
    pres->expose_background = o;
    if(edje_object_file_set(o, pres->theme, "eyelight/expose_main") ==  0)
        printf("eyelight_viewer_expose_start(), edje_object_file_set() erreur! 1\n");
    evas_object_show(o);
    evas_object_resize(o, pres->current_size_w, pres->current_size_h);
    edje_object_scale_set(o, pres->current_scale);
    evas_object_move(o, pres->current_pos_x, pres->current_pos_y);
    evas_object_smart_member_add(o, pres->smart_obj);
    evas_object_clip_set(o, pres->current_clip);

    edje_object_signal_callback_add(o,"expose,window,next","eyelight",_eyelight_viewer_expose_window_next_cb,pres);
    edje_object_signal_callback_add(o,"expose,window,previous","eyelight",_eyelight_viewer_expose_window_previous_cb,pres);
    edje_object_signal_callback_add(o,"expose,slide_next","eyelight",_eyelight_viewer_expose_next_cb,pres);
    edje_object_signal_callback_add(o,"expose,slide,previous","eyelight",_eyelight_viewer_expose_previous_cb,pres);
    edje_object_signal_callback_add(o,"expose,slide,up","eyelight",_eyelight_viewer_expose_up_cb,pres);
    edje_object_signal_callback_add(o,"expose,slide,down","eyelight",_eyelight_viewer_expose_down_cb,pres);

    _eyelight_viewer_expose_slides_load(pres);
}

void _eyelight_viewer_expose_slides_load(Eyelight_Viewer* pres)
{
    int i;

    int first_slide = pres->expose_first_slide;
    int nb_cols = pres->expose_nb_cols;
    int nb_lines = pres->expose_nb_lines;
    int nb_slides = nb_lines * nb_cols;

    pres->expose_slides = calloc(nb_slides,sizeof(Evas_Object*));
    pres->expose_image_thumbnails = calloc(nb_slides,sizeof(Evas_Object*));

    Evas_Object *o_image, *o_swallow;

    int w_body, h_body, x_body, y_body;
    int w_swallow, h_swallow;

    edje_object_part_geometry_get(pres->expose_background,"body",&x_body,&y_body,&w_body,&h_body);

    int w = (w_body-(nb_cols+1))/nb_cols;
    int h = (h_body-(nb_lines+1))/nb_lines;

    for(i=0;i<nb_slides && i+first_slide<pres->size;i++)
    {
        int pos_y = i/nb_cols;
        int pos_x = i%nb_cols;

        o_swallow = edje_object_add(pres->evas);
        evas_object_smart_member_add(o_swallow, pres->smart_obj);
        pres->expose_slides[i] = o_swallow;
        if(edje_object_file_set(o_swallow, pres->theme
                    , "eyelight/expose_slide") ==  0)
            printf("eyelight_viewer_expose_slides_load(), edje_object_file_set() erreur! 3 \n");
        if(i!=pres->expose_current)
            edje_object_signal_emit(o_swallow
                    ,"expose,slide,unselect","eyelight");
        else
            edje_object_signal_emit(o_swallow
                    ,"expose,slide,select","eyelight");
        edje_object_signal_callback_add(o_swallow
                ,"expose,slide,choose","eyelight"
                ,_eyelight_viewer_expose_slide_select_cb,pres);

        const Eyelight_Thumb* thumb = eyelight_viewer_thumbnails_get(pres,i+first_slide);

        o_image = evas_object_image_add(pres->evas);
        pres->expose_image_thumbnails[i] = o_image;
        evas_object_color_set(o_image,255,255,255,255);
        evas_object_image_size_set(o_image, thumb->w, thumb->h);
        evas_object_image_data_set(o_image,thumb->thumb);
        evas_object_image_filled_set(o_image,1);
        evas_object_show(o_image);
        edje_object_part_swallow(o_swallow,"object.swallow",o_image);

        evas_object_move(o_swallow,w*pos_x+x_body+pres->current_pos_x,h*pos_y+y_body+pres->current_pos_y);
        evas_object_clip_set(o_swallow,pres->current_clip);
        evas_object_resize(o_swallow,w,h);
        edje_object_signal_emit(o_swallow,"show","eyelight");
        evas_object_show(o_swallow);
    }

    eyelight_viewer_thumbnails_clean(pres, first_slide, first_slide+nb_slides);
}

void _eyelight_viewer_expose_slides_destroy(Eyelight_Viewer* pres)
{
    int i;
    int size = pres->expose_nb_lines * pres->expose_nb_cols;

    for(i=0;i<size;i++)
    {
        if(pres->expose_slides[i])
            evas_object_del(pres->expose_slides[i]);
        if(pres->expose_image_thumbnails[i])
        {
            evas_object_image_data_set(pres->expose_image_thumbnails[i],NULL);
            evas_object_del(pres->expose_image_thumbnails[i]);
        }
    }
    EYELIGHT_FREE(pres->expose_slides);
    EYELIGHT_FREE(pres->expose_image_thumbnails);
}

void eyelight_viewer_expose_stop(Eyelight_Viewer* pres)
{
    if(!pres->state == EYELIGHT_VIEWER_STATE_EXPOSE)
        return ;

    evas_object_del(pres->expose_background);
    _eyelight_viewer_expose_slides_destroy(pres);
    eyelight_viewer_thumbnails_clean(pres,-1,-1);
    pres->state = EYELIGHT_VIEWER_STATE_DEFAULT;
}

/*
 * @brief go to the next window
 * pos is the slide which will be selected, -1 to select the first slide
 */
void _eyelight_viewer_expose_window_next(Eyelight_Viewer* pres, int pos)
{
    int size = pres->expose_nb_cols * pres->expose_nb_lines;
    if(pos<0)
        pos = 0;
    if(size+pres->expose_first_slide+pos>pres->size-1)
        return ;

    pres->expose_current=pos;
    pres->expose_first_slide = pres->expose_first_slide + size;
    _eyelight_viewer_expose_slides_destroy(pres);
    _eyelight_viewer_expose_slides_load(pres);
}

/*
 * @brief go to the previous window
 * pos is the slide which will be selected, -1 to select the last slide
 */
void _eyelight_viewer_expose_window_previous(Eyelight_Viewer* pres, int pos)
{
    int size = pres->expose_nb_cols * pres->expose_nb_lines;
    if(pos<0)
        pos = size-1;

    if(pres->expose_first_slide-size<0)
        return ;

    pres->expose_current=pos;
    pres->expose_first_slide = pres->expose_first_slide - size;
    _eyelight_viewer_expose_slides_destroy(pres);
    _eyelight_viewer_expose_slides_load(pres);
}

void eyelight_viewer_expose_window_next(Eyelight_Viewer* pres)
{
    _eyelight_viewer_expose_window_next(pres,-1);
}

void eyelight_viewer_expose_window_previous(Eyelight_Viewer* pres)
{
    _eyelight_viewer_expose_window_previous(pres,-1);
}
void eyelight_viewer_expose_next(Eyelight_Viewer* pres)
{
    if(pres->expose_current+pres->expose_first_slide>=pres->size-1)
        return ;

    int size = pres->expose_nb_cols * pres->expose_nb_lines;

    if(pres->expose_current>=size-1)
    {
        eyelight_viewer_expose_window_next(pres);
    }
    else
    {
        edje_object_signal_emit(
                pres->expose_slides[pres->expose_current],
                "expose,slide,unselect","eyelight");

        pres->expose_current++;
        edje_object_signal_emit(
                pres->expose_slides[pres->expose_current],
                "expose,slide,select","eyelight");
    }
}

void eyelight_viewer_expose_down(Eyelight_Viewer* pres)
{
    int next = pres->expose_current + pres->expose_first_slide + pres->expose_nb_cols;
    if(next>pres->size-1)
        return ;

    int size = pres->expose_nb_cols * pres->expose_nb_lines;

    if(pres->expose_current+pres->expose_nb_cols>size-1)
    {
        _eyelight_viewer_expose_window_next(pres,pres->expose_current%pres->expose_nb_cols);
    }
    else
    {
        edje_object_signal_emit(pres->expose_slides[pres->expose_current],
                "expose,slide,unselect","eyelight");

        pres->expose_current+=pres->expose_nb_cols;
        edje_object_signal_emit(pres->expose_slides[pres->expose_current],
                "expose,slide,select","eyelight");
    }
}

void eyelight_viewer_expose_previous(Eyelight_Viewer* pres)
{
    if(pres->expose_current+pres->expose_first_slide<=0)
        return ;

    int size = pres->expose_nb_cols * pres->expose_nb_lines;

    if(pres->expose_current<=0)
    {
        eyelight_viewer_expose_window_previous(pres);
    }
    else
    {
        edje_object_signal_emit(pres->expose_slides[pres->expose_current],
                "expose,slide,unselect","eyelight");

        pres->expose_current--;
        edje_object_signal_emit(pres->expose_slides[pres->expose_current],
                "expose,slide,select","eyelight");
    }
}

void eyelight_viewer_expose_up(Eyelight_Viewer* pres)
{
    int next = pres->expose_first_slide + pres->expose_current - pres->expose_nb_cols;
    if(next<0)
        return ;

    int size = pres->expose_nb_cols * pres->expose_nb_lines;

    if(pres->expose_current-pres->expose_nb_cols<0)
    {
        _eyelight_viewer_expose_window_previous(pres,pres->expose_current+size-pres->expose_nb_cols);
    }
    else
    {
        edje_object_signal_emit(pres->expose_slides[pres->expose_current],
                "expose,slide,unselect","eyelight");

        pres->expose_current-=pres->expose_nb_cols;
        edje_object_signal_emit(pres->expose_slides[pres->expose_current],
                "expose,slide,select","eyelight");
    }
}

void eyelight_viewer_expose_select(Eyelight_Viewer* pres)
{
    int new_slide = pres->expose_first_slide + pres->expose_current;
    eyelight_viewer_expose_stop(pres);

    eyelight_viewer_slide_goto(pres,new_slide);
}

void eyelight_viewer_expose_smart_obj_set(Eyelight_Viewer* pres,Evas_Object *obj)
{
    evas_object_smart_member_add(pres->expose_background,obj);
}

void eyelight_viewer_expose_resize(Eyelight_Viewer* pres,int w, int h)
{
    evas_object_resize(pres->expose_background,w,h);
}

void eyelight_viewer_expose_scale_set(Eyelight_Viewer* pres,double ratio)
{
    edje_object_scale_set(pres->expose_background,ratio);
}

void eyelight_viewer_expose_move(Eyelight_Viewer* pres,int x, int y)
{
    int w_body, h_body, x_body, y_body;
    int w_swallow, h_swallow;
    int i;
    int nb_cols = pres->expose_nb_cols;
    int nb_lines = pres->expose_nb_lines;
    int nb_slides = nb_lines * nb_cols;
    int w,h;

    w = pres->current_size_w;
    h = pres->current_size_h;

    evas_object_move(pres->expose_background,x,y);

    edje_object_part_geometry_get(pres->expose_background,"expose/body",&x_body,&y_body,&w_body,&h_body);

    int w_slide = (w_body-(nb_cols+1))/nb_cols;
    int h_slide = (h_body-(nb_lines+1))/nb_lines;

    for(i=0;i<nb_slides;i++)
    {
        int pos_y = i/nb_cols;
        int pos_x = i%nb_cols;
        if(pres->expose_slides[i])
        {
            evas_object_resize(pres->expose_slides[i],w_slide,h_slide);
                evas_object_move(pres->expose_slides[i],
                        w_slide*pos_x+x_body+pres->current_pos_x,h_slide*pos_y+y_body+pres->current_pos_y);
        }
    }
}

void eyelight_viewer_expose_clip_set(Eyelight_Viewer* pres,Evas_Object *clip)
{
    int i;

    evas_object_clip_set(pres->expose_background,clip);

    int nb_slides = eyelight_viewer_size_get(pres);
    for(i=0;i<nb_slides;i++)
    {
        if(pres->expose_slides[i])
            evas_object_clip_set(pres->expose_slides[i],clip);
    }
}


void _eyelight_viewer_expose_slide_select_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
    Eyelight_Viewer*pres = (Eyelight_Viewer*)data;
    int i;

    for(i=0;i<pres->expose_nb_lines*pres->expose_nb_cols;i++)
    {
        if(o==pres->expose_slides[i])
        {
            pres->expose_current=i;
            eyelight_viewer_expose_select(pres);
            return ;
        }
    }
}

void _eyelight_viewer_expose_window_next_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
    Eyelight_Viewer*pres = (Eyelight_Viewer*)data;
    eyelight_viewer_expose_window_next(pres);
}


void _eyelight_viewer_expose_window_previous_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
    Eyelight_Viewer*pres = (Eyelight_Viewer*)data;
    eyelight_viewer_expose_window_previous(pres);
}

void _eyelight_viewer_expose_next_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
    Eyelight_Viewer*pres = (Eyelight_Viewer*)data;
    eyelight_viewer_expose_next(pres);
}


void _eyelight_viewer_expose_previous_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
    Eyelight_Viewer*pres = (Eyelight_Viewer*)data;
    eyelight_viewer_expose_previous(pres);
}

void _eyelight_viewer_expose_up_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
    Eyelight_Viewer*pres = (Eyelight_Viewer*)data;
    eyelight_viewer_expose_up(pres);
}


void _eyelight_viewer_expose_down_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
    Eyelight_Viewer*pres = (Eyelight_Viewer*)data;
    eyelight_viewer_expose_down(pres);
}



