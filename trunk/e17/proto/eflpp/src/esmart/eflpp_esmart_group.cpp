#include "eflpp_esmart_group.h"

#include <iostream>
#include <assert.h>

using namespace std;

namespace efl {

//===========================================================================//
// EvasEsmartGroup
//===========================================================================//

EvasEsmartGroup::EvasEsmartGroup( EvasCanvas* canvas, const char* name )
    :EvasEsmart( canvas, "<attach>", name )
{
  o = newEsmart( "EsmartGroup" );
}

EvasEsmartGroup::EvasEsmartGroup( int x, int y, EvasCanvas* canvas, const char* name )
    :EvasEsmart( canvas, "<attach>", name )
{
  o = newEsmart( "EsmartGroup" );
  move( x, y );
}

EvasEsmartGroup::EvasEsmartGroup( int x, int y, int width, int height, EvasCanvas* canvas, const char* name )
    :EvasEsmart( canvas, "<attach>", name )
{
  o = newEsmart( "EsmartGroup" );
  move( x, y );
  resize( width, height );
}

EvasEsmartGroup::~EvasEsmartGroup()
{
}

void EvasEsmartGroup::add (EvasObject *object)
{
  evasObjectList.push_back (object);
}

void EvasEsmartGroup::remove (EvasObject* object)
{
  cerr << "EvasEsmartGroup::remove" << endl;
}

// Handler functions

void EvasEsmartGroup::addHandler()
{
  cerr << "EvasEsmartGroup::addHandler" << endl;
}

void EvasEsmartGroup::delHandler()
{
  cerr << "EvasEsmartGroup::delHandler" << endl;
}

void EvasEsmartGroup::moveHandler( Evas_Coord x, Evas_Coord y )
{
  cerr << "EvasEsmartGroup::moveHandler" << endl;
}

void EvasEsmartGroup::resizeHandler( Evas_Coord w, Evas_Coord h )
{
  cerr << "EvasEsmartGroup::resizeHandler" << endl;
}

void EvasEsmartGroup::showHandler()
{
  cerr << "EvasEsmartGroup::showHandler" << endl;
  for (list<EvasObject*>::iterator eol_it = evasObjectList.begin ();
       eol_it != evasObjectList.end ();
       eol_it++)
  {
    EvasObject *eo = (*eol_it);
  
    eo->show();
  }
}

void EvasEsmartGroup::hideHandler()
{
  cerr << "EvasEsmartGroup::hideHandler" << endl;
  for (list<EvasObject*>::iterator eol_it = evasObjectList.begin ();
       eol_it != evasObjectList.end ();
       eol_it++)
  {
    EvasObject *eo = (*eol_it);

    eo->hide();
  } 
}

void EvasEsmartGroup::colorSetHandler( int r, int g, int b, int a )
{
  cerr << "EvasEsmartGroup::colorSetHandler" << endl;
}

void EvasEsmartGroup::clipSetHandler( Evas_Object *clip )
{
  cerr << "EvasEsmartGroup::clipSetHandler" << endl;
}

void EvasEsmartGroup::clipUnsetHandler()
{
  cerr << "EvasEsmartGroup::clipUnsetHandler" << endl;
}

} // end namespace efl
