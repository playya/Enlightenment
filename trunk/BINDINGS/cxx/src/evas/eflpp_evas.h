#ifndef EVAS_BASE_H
#define EVAS_BASE_H

#include "eflpp_evasevent.h"
#include "eflpp_evasfont.h"
#include "eflpp_evasutils.h"
#include "eflpp_evasobject.h"

/* EFL++ */
#include <eflpp_common.h>

/* EFL */
#include <Evas.h>

/**
 * C++ Wrapper for the Enlightenment Canvas Library (EVAS)
 *
 * @author Michael 'Mickey' Lauer <mickey@Vanille.de>
 */

namespace efl {

static const char* const EVAS_LOAD_ERROR[] =
{
    "No error on load",
    "A non-specific error occured",
    "File (or file path) does not exist",
    "Permission denied to an existing file (or path)"
    "Allocation of resources failure prevented load",
    "File corrupt (but was detected as a known format)",
    "File contains an unknown format",
    "File contains an incompatible format",
};

/* Forward declarations */
class EvasCanvas;
class EvasGradient;
class EvasLine;
class EvasObject;
class EvasPolygon;
class EvasRectangle;
class EvasText;
class EvasTextblockStyle;

/**
 * An Evas Canvas Wrapper
 */
class EvasCanvas : public Trackable
{
  public:
    EvasCanvas( int width, int height );
    EvasCanvas( Evas* evas );
    EvasCanvas();
    ~EvasCanvas();

    Evas* obj() const { return o; };

    /* Output Methods */
    int lookupRenderMethod( const char* method );

    /*! \brief Set the Evas output method.
     *  This does include a call to lookupRenderMethod().
     *  @param method Name of the output method. (See C API docs).
     */
    bool setOutputMethod( const char* method );

    void resize( int width, int height );
    Size size() const;
    Rect geometry() const;
    void setViewport( int x, int y, int width, int height );
    Rect viewport() const;

    /* Font Path */
    void appendFontPath( const char* path );
    void prependFontPath( const char* path );
    void clearFontPath();
    const Evas_List* fontPath();

    /* Font Cache */
    void setFontCache( unsigned int size );
    void flushFontCache();
    int fontCache();

    /* Image Cache */
    void setImageCache( unsigned int size );
    void flushImageCache();
    void reloadImageCache();
    int imageCache();

    /* Object queries */
    EvasObject* focusedObject() const;
    EvasObject* objectAtTop() const;
    EvasObject* objectAtBottom() const;

  protected:
    Evas* o;
};

/**
 * Wraps an Evas Line Object
 */
class EvasLine : public EvasObject
{
  public:
    EvasLine( EvasCanvas* canvas, const char* name = 0 );
    EvasLine( int x1, int y1, int x2, int y2, EvasCanvas* canvas, const char* name = 0 );
    virtual ~EvasLine();

    virtual void setGeometry( int x, int y, int width, int height );
};

/*
 * Wraps an Evas Rectangle Object
 */
class EvasRectangle : public EvasObject
{
  public:
    EvasRectangle( EvasCanvas* canvas, const char* name = 0 );
    EvasRectangle( int x, int y, int width, int height, EvasCanvas* canvas, const char* name = 0 );
    EvasRectangle( const Rect& r, EvasCanvas* canvas, const char* name = 0 );
    virtual ~EvasRectangle();
};

/*
 * Wraps an Evas Polygon Object
 */
class EvasPolygon : public EvasObject
{
  public:
    EvasPolygon( EvasCanvas* canvas, const char* name = 0 );
    virtual ~EvasPolygon();

    void addPoint( int x, int y );
    void clearPoints();
};

/*
 * Wraps an Evas Text Object
 */
class EvasText : public EvasObject
{
  public:
    EvasText( EvasText* ao, EvasCanvas* canvas );
    EvasText( EvasCanvas* canvas, const char* name = 0 );
    EvasText( int x, int y, const char* text, EvasCanvas* canvas, const char* name = 0 );
    EvasText( const char* font, int size, const char* text, EvasCanvas* canvas, const char* name = 0 );
    EvasText( const char* font, int size, int x, int y, const char* text, EvasCanvas* canvas, const char* name = 0 );
    virtual ~EvasText();

    virtual const Size size() const;

    void setFontSource( const char* source );
    EvasFont font() const;
    void setFont( const char* font, Evas_Font_Size size );
    void setFont( const EvasFont& font );
    const char* text() const;
    void setText( const char* );

    int ascent() const;
    int descent() const;
    int maxAscent() const;
    int maxDescent() const;
    int horizontalAdvance() const;
    int verticalAdvance() const;
    int inset() const;

    /*
    //int characterPosition() const;
    //int characterCoordinates() const;
    EAPI int               evas_object_text_char_pos_get     (Evas_Object *obj, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);
    EAPI int               evas_object_text_char_coords_get  (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw,        Evas_Coord *ch);
    */

};

/*
 * Wraps an Evas Textblock Object
 */
class EvasTextblock : public EvasObject
{
  public:
    EvasTextblock( EvasCanvas* canvas, const char* name = 0 );
    EvasTextblock( int x, int y, const char* text, EvasCanvas* canvas, const char* name = 0 );
    virtual ~EvasTextblock();

    void setStyle( const EvasTextblockStyle* style );
    void setText( const char* text );
    void clear();
};

/*
 * Wraps an Evas Image Object
 */
class EvasImage : public EvasObject
{
  public:
    EvasImage( EvasCanvas* canvas, const char* name = 0 );
    EvasImage( const char* filename, EvasCanvas* canvas, const char* name = 0 );
    EvasImage( int x, int y, const char* filename, EvasCanvas* canvas, const char* name = 0 );
    EvasImage( Evas_Object* object, EvasCanvas* canvas, const char* name = 0 );
    virtual ~EvasImage();

   /**
    * Sets the @filename containing the image data.
    * If the image data resides in an .eet or .edb you probably
    * want to supply a @key specifying the image name.
    * @returns true, if the image could be loaded, false, otherwise.
    * @see evas_object_image_file_set
    */
    bool setFile( const char* path, const char* key = 0 );
    void setFill( int x, int y, int width, int height );
    void setFill( int x, int y, const Size& );

    /* Size */
    virtual void resize( int width, int height, bool ff = true );
    virtual void resize( const Size& size, bool ff = true );
    virtual const Size size() const;
    const Size& trueSize();

    /// Sets the size of the image to be display by the given image object.
    void setImageSize (int w, int h);

    /// Retrieves the size of the image displayed by the given image object.
    void getImageSize (int &w, int &h);

    /* Border */
    void setBorder( int left, int right, int top, int bottom );

    /// Sets the raw image data.
    void setData (void *data);

    /// Retrieves the raw image data.
    void *getData (bool for_writing);

    /// Replaces an image object's internal image data buffer.
    void setDataCopy (void *data);

    /// Update a rectangle after putting data into the image.
    void addDataUpdate (int x, int y, int w, int h);

    static void setResourcePath( const char* path ) { EvasImage::_resourcePath = (char*) path; };
    static const char* resourcePath()               { return EvasImage::_resourcePath; };

  private:
    static char* _resourcePath;
    Size _size; // true size
};

/*
 * Wraps an Evas Gradient Object
 */
class EvasGradient : public EvasObject
{
  public:
    EvasGradient( int x, int y, int width, int height, EvasCanvas* canvas, const char* name = 0 );
    EvasGradient( EvasCanvas* canvas, const char* name = 0 );
    virtual ~EvasGradient();

   void addColorStop( int r, int g, int b, int a, int distance );
   void clear();
   void setAngle( Evas_Angle angle );
   Evas_Angle angle();

};

inline ostream& operator<<( ostream& s, const EvasCanvas& canvas )
{
    return s << "(Canvas)";
}
inline ostream& operator<<( ostream& s, const EvasObject& obj )
{
    return s << "<" << " Object '" << obj.name() << "' @ " << obj.geometry() << ">";
}

}

#endif
