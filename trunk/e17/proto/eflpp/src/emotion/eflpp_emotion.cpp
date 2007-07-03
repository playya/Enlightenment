#include "eflpp_emotion.h"

extern "C" {
#include <Emotion.h>
}

#include <iostream>
#include <assert.h>
using namespace std;

namespace efl {

//===============================================================================================
// EvasEmotion
//===============================================================================================

EvasEmotion::EvasEmotion( EvasCanvas* canvas, const char* name )
    :EvasObject( canvas, "emotion", name )
{
}

EvasEmotion::EvasEmotion( const char* filename, EvasCanvas* canvas, const char* name )
         :EvasObject( canvas, "emotion", name ? name : filename )
{
    setFile( filename );
}

EvasEmotion::EvasEmotion( int x, int y, const char* filename, EvasCanvas* canvas, const char* name )
         :EvasObject( canvas, "emotion", name ? name : filename )
{
    setFile( filename );
    move( x, y );
}

EvasEmotion::EvasEmotion( int x, int y, int width, int height, const char* filename, EvasCanvas* canvas, const char* name )
    :EvasObject( canvas, "emotion", name ? name : filename )
{
    setFile( filename );
    move( x, y );
    resize( width, height );
}

void EvasEmotion::setFile( const char* filename )
{
    emotion_object_file_set( o, filename );
}

void EvasEmotion::setPlay( bool b )
{
    emotion_object_play_set( o, b );
}

void EvasEmotion::setSmoothScale( bool b )
{
    emotion_object_smooth_scale_set( o, b );
}

EvasEmotion::~EvasEmotion()
{
}

}
