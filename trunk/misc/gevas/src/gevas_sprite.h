/*
 * Playable sprite.
 *
 * Copyright (C) 2001 Ben Martin.
 *
 * Original author: Ben Martin
 *
 * See COPYING for full details of copying & use of this software.
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *
 */

#ifndef INC_GTK_GEVAS_SPRITE__H
#define INC_GTK_GEVAS_SPRITE__H

#include <gevasev_handler.h>
#include <gevas.h>
#include <gevasobj.h>

#include <gtk/gtkobject.h>
#include <gtk/gtk.h>


#ifdef __cplusplus
extern "C" {
#endif							/* __cplusplus */

    
    
#define GTK_GEVAS_SPRITE(obj) GTK_CHECK_CAST \
        (obj, gevas_sprite_get_type (), GtkgEvasSprite)
    
#define GTK_GEVAS_SPRITE(klass)  GTK_CHECK_CLASS_CAST \
        (klass, gevas_sprite_get_type (), GtkgEvasSpriteClass)
        
#define GTK_IS_GEVAS_SPRITE(obj)       GTK_CHECK_TYPE \
        (obj, gevas_sprite_get_type ())

    
    typedef struct _GtkgEvasSprite      GtkgEvasSprite;
    typedef struct _GtkgEvasSpriteClass GtkgEvasSpriteClass;
    typedef GtkgEvasObj*                GtkgEvasSprite_T;

    struct _GtkgEvasObjCollection
    {
        GtkObject gobj;


        GtkgEvasObjCollection* col;
        GtkgEvas* gevas;
    };

    struct _GtkgEvasSpriteClass {
        GtkObjectClass parent_class;
    };


    guint           gevas_sprite_get_type(void);
    GtkgEvasSprite* gevas_sprite_new( GtkgEvas* _gevas );



/* public */

    
    void gevas_sprite_add(           GtkgEvasSprite* ev, GtkgEvasSprite_T o );
    void gevas_sprite_add_all(       GtkgEvasSprite* ev, GtkgEvasSprite*s);
    void gevas_sprite_remove(        GtkgEvasSprite* ev, GtkgEvasSprite_T o );
    void gevas_sprite_remove_all(    GtkgEvasSprite* ev, GtkgEvasSprite*s);
    void gevas_sprite_clear(         GtkgEvasSprite* ev );
    void gevas_sprite_move(          GtkgEvasSprite* ev, gint32 x, gint32 y );
    void gevas_sprite_move_relative( GtkgEvasSprite* ev, gint32 dx, gint32 dy );
    void gevas_sprite_hide(          GtkgEvasSprite* ev );
    void gevas_sprite_show(          GtkgEvasSprite* ev );
    void gevas_sprite_set_visible(   GtkgEvasSprite* ev, gboolean v );
    gboolean gevas_sprite_contains(  GtkgEvasSprite* ev, GtkgEvasSprite_T o );
    gboolean gevas_sprite_contains_all( GtkgEvasSprite* ev, GtkgEvasSprite*s);

    void gevas_sprite_push_metadata_prefix( GtkgEvasSprite* ev, const char* p );
    void gevas_sprite_pop_metadata_prefix ( GtkgEvasSprite* ev, const char* p );
    gboolean gevas_sprite_load_from_metadata( GtkgEvasSprite* ev, const char* loc );

    void gevas_sprite_set_current_frame( GtkgEvasSprite* ev, gint f );
    void gevas_sprite_set_current_frame_by_name( GtkgEvasSprite* ev, const char* n );
    void gevas_sprite_advance_n( GtkgEvasSprite* ev, gint n );
    void gevas_sprite_retard_n ( GtkgEvasSprite* ev, gint n );
    void gevas_sprite_jumpto_start( GtkgEvasSprite* ev );
    void gevas_sprite_jumpto_end  ( GtkgEvasSprite* ev );

    void gevas_sprite_play( GtkgEvasSprite* ev );
    void gevas_sprite_play_n( GtkgEvasSprite* ev, gint n );
    void gevas_sprite_play_to_end( GtkgEvasSprite* ev );
    void gevas_sprite_play_one_loop( GtkgEvasSprite* ev );

    void gevas_sprite_set_play_backwards( GtkgEvasSprite* ev, gboolean v );
    
    
/* package */
    void gevas_sprite_dump( GtkgEvasSprite* ev, Evas_List li );
    


#ifdef __cplusplus
}
#endif							/* __cplusplus */
#endif							/*  */
