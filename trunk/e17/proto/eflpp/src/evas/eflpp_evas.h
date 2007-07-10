#ifndef EVAS_BASE_H
#define EVAS_BASE_H

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include "eflpp_evasevent.h"
#include "eflpp_evasfont.h"
#include "eflpp_evasutils.h"

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

/* Forward declarations */
class EvasCanvas;
class EvasGradient;
class EvasLine;
class EvasObject;
class EvasPolygon;
class EvasRectangle;
class EvasText;

static const char* EVAS_LOAD_ERROR[] =
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

    /* Focus Handling */
    EvasObject* focusObject() const;

  protected:
    Evas* o;
};

/**
 * An Evas Object wrapper
 *
 * This base class handles most of the common operation on Evas objects.
 */
class EvasObject
{
  friend class EvasCanvas;
  friend class EdjePart;

  protected:
    // construction/destruction
    EvasObject( EvasCanvas* canvas,
            const char* type = "unknown",
            const char* name = "(null)" );
    virtual ~EvasObject();

  public:
    bool operator==(const EvasObject& rhs) { return rhs.o == o; };

    /* don't use these */
    const char* type() const { return _type; };
    Evas_Object* obj() const { return o; };
    EvasCanvas* canvas() const { return _canvas; }; // FIXME: Rename to parent() ?

    /* Name */
    const char* name() const;
    void setName( const char* name );

    /* Geometry */
    virtual const Size size() const;
    virtual void move( const Point& );
    virtual void move( int x = 0, int y = 0 );
    virtual void resize( int width, int height, bool ff = true );    /* gets overloaded by Image */
    virtual void resize( const Size&, bool ff = true );
    virtual void rresize( double factor, bool ff = true );
    virtual Rect geometry() const;
    virtual void setGeometry( int x, int y, int width, int height );
    virtual void setGeometry( const Rect& rect );

    /* Clipping */
    virtual EvasObject* clip() const;
    virtual void setClip( EvasObject* object );

    /* Color */
    virtual Color color() const;
    virtual void setColor( int r, int g, int b, int alpha );
    virtual void setColor( const Color& color );

    /* Z */
    virtual int layer() const;
    virtual void setLayer( int layer );
    virtual void raise();
    virtual void lower();
    virtual void stackAbove( const EvasObject* obj );
    virtual void stackBelow( const EvasObject* obj );

    /* Visibility */
    virtual void show();
    virtual void hide();
    virtual bool isVisible() const;
    virtual void setVisible( bool visibility );

    /* Focus */
    virtual void setFocus( bool focus );
    virtual bool hasFocus() const;

    /* Events */
    virtual bool handleMouseIn( const EvasMouseInEvent& );
    virtual bool handleMouseOut( const EvasMouseOutEvent& );
    virtual bool handleMouseDown( const EvasMouseDownEvent& );
    virtual bool handleMouseUp( const EvasMouseUpEvent& );
    virtual bool handleMouseMove( const EvasMouseMoveEvent& );
    virtual bool handleMouseWheel( const EvasMouseWheelEvent& );
    virtual bool handleFree();
    virtual bool handleKeyDown( const EvasKeyDownEvent& );
    virtual bool handleKeyUp( const EvasKeyUpEvent& );
    virtual bool handleFocusIn();
    virtual bool handleFocusOut();
    virtual bool handleShow();
    virtual bool handleHide();
    virtual bool handleMove();
    virtual bool handleResize();
    virtual bool handleRestack();

  private:
    static EvasObject* objectLink( Evas_Object* evas_object = 0 );
    void registerCallbacks();
    static void dispatcher( void *data, Evas *evas, Evas_Object *evas_object, void *event_info);

  protected:
    const char* _type;
    Evas_Object* o;
    EvasCanvas* _canvas;

  private:
    EvasObject(); // disable default constructor
    EvasObject( const EvasObject& ); // disable copy constructor
    bool operator=(const EvasObject& ); // disable assignment operator
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
    EvasText( EvasText* ao );
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
class EvasTextBlock : public EvasObject
{
  public:
    EvasTextBlock( EvasCanvas* canvas, const char* name = 0 );
    EvasTextBlock( int x, int y, const char* text, EvasCanvas* canvas, const char* name = 0 );
    virtual ~EvasTextBlock();

    void insertText( const char* text );
    void insertFormat( const char* font, int size = 12, const char* color = "#ffffffff" );
    void insertFormattedText( const char* text, const char* font = "Vera", int size = 12, const char* color = "#ffffffff" );
};

/*
 * Wraps an Evas Image Object
 */
class EvasImage : public EvasObject
{
  public:
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

    /* Border */
    void setBorder( int left, int right, int top, int bottom );

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
    return s << "<" << obj.type() << " Object '" << obj.name() << "' @ " << obj.geometry() << ">";
}

}

#endif
