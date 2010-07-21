
#include "main.h"
#include "download.h"

static void _done_cb(void *data, Enlil_Photo *photo, int status);
static int _progress_cb(void *data, Enlil_Photo *photo, long int dltotal, long int dlnow);
static void _start_cb(void *data, Enlil_Photo *photo);


Download *download_new(Evas_Object *parent)
{
    Evas_Object *bx, *lbl, *pb, *notify;
    Download *dl;

    dl = calloc(1, sizeof(Download));

    notify = elm_notify_add(parent);
    elm_notify_orient_set(notify, ELM_NOTIFY_ORIENT_BOTTOM_RIGHT);
    elm_win_resize_object_add(parent, notify);
    evas_object_size_hint_weight_set(notify, -1.0, -1.0);
    evas_object_size_hint_align_set(notify, -1.0, -1.0);

    bx = elm_box_add(parent);
    evas_object_size_hint_weight_set(bx, -1.0, -1.0);
    evas_object_size_hint_align_set(bx, -1.0, -1.0);
    evas_object_show(bx);
    elm_notify_content_set(notify, bx);

    lbl = elm_label_add(bx);
    elm_label_label_set(lbl, D_("Downloading ..."));
    evas_object_size_hint_weight_set(lbl, 1.0, 1.0);
    evas_object_size_hint_align_set(lbl, -1.0, -1.0);
    elm_box_pack_end(bx, lbl);
    evas_object_show(lbl);


    pb = elm_progressbar_add(bx);
    elm_progressbar_label_set(pb, "Photo");
    evas_object_size_hint_weight_set(pb, 1.0, 1.0);
    evas_object_size_hint_align_set(pb, -1.0, -1.0);
    elm_box_pack_end(bx, pb);
    evas_object_show(pb);

    dl->main = notify;
    dl->lbl = lbl;
    dl->pb = pb;

    return dl;
}

void download_free(Download **_dl)
{
    ASSERT_RETURN_VOID(_dl != NULL);
    Download *dl = *_dl;

    ASSERT_RETURN_VOID(dl != NULL);
    evas_object_del(dl->main);
}

void download_add(Download *dl, const char *source, Enlil_Photo *photo)
{
    ASSERT_RETURN_VOID(dl != NULL);
    ASSERT_RETURN_VOID(source != NULL);
    ASSERT_RETURN_VOID(photo != NULL);

    enlil_download_add(photo, source, _start_cb, _progress_cb, _done_cb, dl);
}


static void _start_cb(void *data, Enlil_Photo *photo)
{
    Download *dl = data;

    evas_object_show(dl->main);

    elm_progressbar_label_set(dl->pb, enlil_photo_name_get(photo));
    elm_progressbar_pulse_set(dl->pb, 0);

    flickr_job_start_cb(NULL, NULL, enlil_photo_album_get(photo), photo);
}

static void _done_cb(void *data, Enlil_Photo *photo, int status)
{
    Enlil_Photo_Data *photo_data = enlil_photo_user_data_get(photo);
    Enlil_Album *album = enlil_photo_album_get(photo);
    Download *dl = data;

    evas_object_hide(dl->main);
    flickr_job_done_cb(NULL, NULL, enlil_photo_album_get(photo), photo);

    if(photo_data)
    {
        //if the photo is new, the data doesnt exists right now
        photo_data->flickr_sync.state = PHOTO_FLICKR_NONE;
        photos_list_object_item_update(photo_data->list_photo_item);
    }

    if(!enlil_download_photos_of_album_in_list(album, EINA_FALSE))
    {
        //set the album as uptodate and force to check if it is really uptodate
        Enlil_Album_Data *album_data = enlil_album_user_data_get(album);
        album_data->flickr_sync.album_flickr_notuptodate = EINA_FALSE;
	album_data->flickr_sync.album_notinflickr = EINA_FALSE;
	album_data->flickr_sync.album_notuptodate = EINA_FALSE;
	album_data->flickr_sync.photos_notinlocal = EINA_FALSE;

        photos_list_object_header_update(album_data->list_photo_item);

        album_data->flickr_sync.inwin.notinlocal.is_updating = EINA_FALSE;
        elm_pager_content_promote(album_data->flickr_sync.inwin.notinlocal.pager,
                album_data->flickr_sync.inwin.notinlocal.pb);
        elm_progressbar_pulse(album_data->flickr_sync.inwin.notinlocal.pb, EINA_FALSE);

        enlil_flickr_job_sync_album_photos_append(album,
                flickr_photo_new_cb,
                flickr_photo_notinflickr_cb,
                flickr_photo_known_cb,
                flickr_album_error_cb,
                enlil_data);

        if(album_data->flickr_sync.inwin.win)
            flickr_sync_update(album);
    }
}

static int _progress_cb(void *data, Enlil_Photo *photo, long int dltotal, long int dlnow)
{
    Download *dl = data;

    elm_progressbar_value_set(dl->pb, (double)dlnow / dltotal);

    return 0;
}


