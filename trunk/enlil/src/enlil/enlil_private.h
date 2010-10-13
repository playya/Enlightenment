#ifndef  PHOTO_MANAGER_PRIVATE_INC


#define  PHOTO_MANAGER_PRIVATE_INC

#include "Enlil.h"
#include <pthread.h>
#include <errno.h>

#include "../define.h"

extern int Enlil_LOG_DOMAIN_99;
#define LOG_DOMAIN Enlil_LOG_DOMAIN_99

struct Enlil_Configuration
{
    void *data;
    struct
    {
        Enlil_Album_New_Cb album_new_cb;
        Enlil_Album_Delete_Cb album_delete_cb;
        Enlil_Delete_Cb enlil_delete_cb;
        Enlil_Photo_New_Cb photo_new_cb;
        Enlil_Photo_Delete_Cb photo_delete_cb;
        Enlil_Photo_Update_Cb photo_update_cb;
    } monitor;

    struct {
        Enlil_Collection_New_Cb new_cb;
        Enlil_Collection_Delete_Cb delete_cb;
        Enlil_Collection_Album_New_Cb album_new_cb;
        Enlil_Collection_Album_Delete_Cb album_delete_cb;
    } collection;

    struct {
        Enlil_Tag_New_Cb new_cb;
        Enlil_Tag_Delete_Cb delete_cb;
        Enlil_Tag_Photo_New_Cb photo_new_cb;
        Enlil_Tag_Photo_Delete_Cb photo_delete_cb;
    } tag;
};

Enlil_Configuration enlil_conf_get(Enlil_Root *root);

int enlil_file_manager_init();
int enlil_file_manager_shutdown();

int enlil_thumb_init();
int enlil_thumb_shutdown();


void _enlil_root_collection_album_add(Enlil_Root *root, Enlil_Album_Collection *album_col, Enlil_Album *album);
void _enlil_root_collection_album_remove(Enlil_Root *root, Enlil_Album_Collection *album_col, Enlil_Album *album);

void _enlil_root_tag_photo_add(Enlil_Root *root, Enlil_Photo_Tag *photo_tag, Enlil_Photo *photo);
void _enlil_root_tag_photo_remove(Enlil_Root *root, Enlil_Photo_Tag *photo_tag, Enlil_Photo *photo);
void _enlil_root_album_add_end(Enlil_Root *root, Enlil_Album *album);
void _enlil_root_collection_add_end(Enlil_Root *root, Enlil_Collection *collection, Eina_Bool notify);
void _enlil_root_tag_add_end(Enlil_Root *root, Enlil_Tag *tag, Eina_Bool notify);

void _enlil_album_photo_name_changed(Enlil_Album *album, Enlil_Photo *photo);
void _enlil_album_photo_datetimeoriginal_changed(Enlil_Album *album, Enlil_Photo *photo);
void _enlil_root_album_name_changed(Enlil_Root *root, Enlil_Album *album);

const char *_enlil_photo_exif_datetimeoriginal_get(const Enlil_Photo *photo);


Eet_Data_Descriptor *_enlil_album_file_name_edd_new();
Eet_Data_Descriptor *_enlil_photo_file_name_edd_new();

Eet_Data_Descriptor * enlil_photo_edd_new(Eet_Data_Descriptor *edd_tag);
Eet_Data_Descriptor * _enlil_album_header_edd_new(Eet_Data_Descriptor *edd_collection);
Eet_Data_Descriptor * _enlil_album_collection_edd_new();
Eet_Data_Descriptor * _enlil_collection_edd_new();
Eet_Data_Descriptor * _enlil_tag_edd_new();
Eet_Data_Descriptor * _enlil_photo_tag_edd_new();
Eet_Data_Descriptor * _enlil_exif_edd_new();
Eet_Data_Descriptor * _enlil_iptc_edd_new();

void _enlil_album_flickr_id_set(Enlil_Album *album, const char *id);

long long 	_enlil_photo_flickr_last_change_get(Enlil_Photo *photo);
void		_enlil_photo_flickr_last_change_set(Enlil_Photo *photo, long long last_change);
long long 	_enlil_photo_flickr_fs_time_get(Enlil_Photo *photo);
void		_enlil_photo_flickr_fs_time_calc(Enlil_Photo *photo);

int enlil_trans_init();
int enlil_trans_shutdown();

#endif   /* ----- #ifndef PHOTO_MANAGER_PRIVATE_INC  ----- */

