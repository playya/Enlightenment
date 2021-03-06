#include "calibrate.h"

#include <cstdio>
#include <cerrno>
using namespace std;

#include <eflpp_evas.h>
#include <eflpp_ecore.h>

#ifdef ENABLE_EFLPP_FB
#include <Ecore_Fb.h>
#endif

#include <Ecore.h>

const unsigned int offset = 32;

static Ecore_Event_Filter* filter = 0;

static void* calibration_event_filter_start( void* data )
{
    return 0;
}

static int calibration_event_filter_event( void* loop_data, void *data, int type, void* event )
{
#ifdef ENABLE_EFLPP_FB
    if ( type == ECORE_FB_EVENT_MOUSE_BUTTON_UP )
    {
        Ecore_Fb_Event_Mouse_Button_Up* ev = static_cast<Ecore_Fb_Event_Mouse_Button_Up*>( event );
        cout << " - data = " << data << endl;
        cout << " - mouse button up at position = " << ev->x << ", " << ev->y << endl;
        CalibrationRectangle* cr = reinterpret_cast<CalibrationRectangle*>( data );
        cr->nextPoint( ev->x, ev->y );
        return 0; // swallow event
    }
    else
#endif
    {
        return 1; // keep event
    }
}

static void calibration_event_filter_end( void* data, void *loop_data )
{
}

CalibrationRectangle::CalibrationRectangle( int x, int y, int width, int height, EvasCanvas* evas )
    :EvasRectangle( x, y, width, height, evas, "CalibrationRectangle" )
{
    setLayer( 255 );
    setColor( 255, 255, 255, 0 ); // white, fully transparent

    // setup the five calibration points
    Size s = evas->size();
    cd.canvasPoints[TopLeft].set( offset, offset );
    cd.canvasPoints[BottomLeft].set( offset, s.height()-offset );
    cd.canvasPoints[BottomRight].set( s.width()-offset, s.height()-offset );
    cd.canvasPoints[TopRight].set( s.width()-offset, offset );
    cd.canvasPoints[Center].set( s.width()/2, s.height()/2 );

    switch ( eApp->mainWindow()->rotation() )
    {
        case 0:
            cout << "ROT 0" << endl;
            cd.screenPoints[TopLeft] = cd.canvasPoints[TopLeft];
            cd.screenPoints[BottomLeft] = cd.canvasPoints[BottomLeft];
            cd.screenPoints[BottomRight] = cd.canvasPoints[BottomRight];
            cd.screenPoints[TopRight] = cd.canvasPoints[TopRight];
            cd.screenPoints[Center] = cd.canvasPoints[Center];
            break;
        case 90:
            cd.screenPoints[TopLeft] = cd.canvasPoints[TopLeft];
            cd.screenPoints[BottomLeft] = cd.canvasPoints[BottomLeft];
            cd.screenPoints[BottomRight] = cd.canvasPoints[BottomRight];
            cd.screenPoints[TopRight] = cd.canvasPoints[TopRight];
            cd.screenPoints[Center] = cd.canvasPoints[Center];
            break;
        case 180:
            cd.screenPoints[TopLeft] = cd.canvasPoints[TopLeft];
            cd.screenPoints[BottomLeft] = cd.canvasPoints[BottomLeft];
            cd.screenPoints[BottomRight] = cd.canvasPoints[BottomRight];
            cd.screenPoints[TopRight] = cd.canvasPoints[TopRight];
            cd.screenPoints[Center] = cd.canvasPoints[Center];
            break;
        case 270:
            cout << "ROT 270" << endl;
            cd.screenPoints[TopLeft].set( s.height()-offset, offset );
            cd.screenPoints[BottomLeft].set( offset, offset );
            cd.screenPoints[BottomRight].set( offset, s.width()-offset );
            cd.screenPoints[TopRight].set( s.height()-offset, s.width()-offset );
            cd.screenPoints[Center].set( s.height()/2, s.width()/2 );
            break;
    }

    // setup background
    background = new EvasGradient( 0, 0, s.width(), s.height(), evas );
    background->setLayer( 21 );
    background->setAngle( 15 );
    background->addColor( 255, 255, 255, 255, 10 );
    background->addColor( 200, 200, 200, 255, 10 );
    background->addColor( 255, 255, 255, 255, 10 );
    background->show();

    // setup text
    text = new EvasText( PACKAGE_DATA_DIR "/fonts/Vera.ttf", 12, 0, s.height()/2, "foo", evas );
    text->setColor( 0, 0, 0, 255 );
    text->setLayer( 22 );
    text->setText( "Click on crosshair to calibrate screen" );
    text->show();

    // setup crosshair
#if 0
    crosshair = new EvasImage( PACKAGE_DATA_DIR "/images/crosshair.png", evas );
    crosshair->setLayer( 23 );
#else
    crosshairhorz = new EvasLine( evas );
    crosshairvert = new EvasLine( evas );
    crosshairhorz->setLayer( 23 );
    crosshairvert->setLayer( 23 );
    crosshairhorz->setColor( 0, 0, 0, 255 );
    crosshairvert->setColor( 0, 0, 0, 255 );
    crosshairhorz->show();
    crosshairvert->show();
#endif
}

CalibrationRectangle::~CalibrationRectangle()
{
}

bool CalibrationRectangle::handleShow()
{
#ifdef ENABLE_EFLPP_FB
    ecore_fb_touch_screen_calibrate_set( 1, 0, 0, 0, 0 );
#endif
    cout << "this = " << this << endl;
    filter = ecore_event_filter_add( &calibration_event_filter_start,
                            &calibration_event_filter_event,
                            &calibration_event_filter_end, this );
    position = TopLeft;
    moveCrossHair();
}

void CalibrationRectangle::nextPoint( int x, int y )
{
    cd.devicePoints[position].set( x, y );

    if ( ++position <= LastPosition )
    {
        moveCrossHair();
    }
    else
    {
        //FIXME: Don't delete or hide, dissolve, and after that, hide :D
        //hide();
        hide();
        delete background;
#if 0
        delete crosshair;
#else
        delete crosshairhorz;
        delete crosshairvert;
#endif
        delete text;
        //crosshair->hide();
        //text->hide();
#ifdef ENABLE_EFLPP_FB
        ecore_fb_touch_screen_calibrate_set( 0, 0, 0, 0, 0 );
#endif
        ecore_event_filter_del( filter );
        calibrate();
        done.emit();
    }
}

void CalibrationRectangle::moveCrossHair()
{
#if 0
    crosshair->move( cd.canvasPoints[position] );
    crosshair->show();
#else
    int x = cd.canvasPoints[position].x();
    int y = cd.canvasPoints[position].y();
    crosshairhorz->setGeometry( x-offset/2, y, offset, 0 );
    crosshairvert->setGeometry( x, y-offset/2, 0, offset );
#endif
}

bool CalibrationRectangle::calibrate()
{
    // calibration code based on ts_calibrate.c (C) Russell King

    int j;
    float n, x, y, x2, y2, xy, z, zx, zy;
    float det, cal_a, cal_b, cal_c, cal_d, cal_e, cal_f, cal_i;
    float scaling = 65536.0;
    int cal_x[5], cal_xfb[5], cal_y[5], cal_yfb[5], cal_o[7];

    cal_x[0]=cd.devicePoints[ TopLeft ].x();
    cal_y[0]=cd.devicePoints[ TopLeft ].y();
    cal_x[1]=cd.devicePoints[ TopRight ].x();
    cal_y[1]=cd.devicePoints[ TopRight ].y();
    cal_x[2]=cd.devicePoints[ BottomLeft ].x();
    cal_y[2]=cd.devicePoints[ BottomLeft ].y();
    cal_x[3]=cd.devicePoints[ BottomRight ].x();
    cal_y[3]=cd.devicePoints[ BottomRight ].y();
    cal_x[4]=cd.devicePoints[ Center ].x();
    cal_y[4]=cd.devicePoints[ Center ].y();

    cal_xfb[0]=cd.screenPoints[ TopLeft ].x();
    cal_yfb[0]=cd.screenPoints[ TopLeft ].y();
    cal_xfb[1]=cd.screenPoints[ TopRight ].x();
    cal_yfb[1]=cd.screenPoints[ TopRight ].y();
    cal_xfb[2]=cd.screenPoints[ BottomLeft ].x();
    cal_yfb[2]=cd.screenPoints[ BottomLeft ].y();
    cal_xfb[3]=cd.screenPoints[ BottomRight ].x();
    cal_yfb[3]=cd.screenPoints[ BottomRight ].y();
    cal_xfb[4]=cd.screenPoints[ Center ].x();
    cal_yfb[4]=cd.screenPoints[ Center ].y();

    // Get sums for matrix
    n = x = y = x2 = y2 = xy = 0;
    for(j=0;j<5;j++) {
        n += 1.0;
        x += (float)cal_x[j];
        y += (float)cal_y[j];
        x2 += (float)(cal_x[j]*cal_x[j]);
        y2 += (float)(cal_y[j]*cal_y[j]);
        xy += (float)(cal_x[j]*cal_y[j]);
    }

    // Get determinant of matrix -- check if determinant is too small
    det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
    if ( det < 0.1 && det > -0.1 )
    {
        cout << "CalibrationRectangle::calibrate() - determinant " << det << " is too small - aborting." << endl;
        return false;
    }

    // Get elements of inverse matrix
    cal_a = (x2*y2 - xy*xy)/det;
    cal_b = (xy*y - x*y2)/det;
    cal_c = (x*xy - y*x2)/det;
    cal_e = (n*y2 - y*y)/det;
    cal_f = (x*y - n*xy)/det;
    cal_i = (n*x2 - x*x)/det;

    // Get sums for x calibration
    z = zx = zy = 0;
    for( j=0; j < 5; j++ ) {
        z += (float)cal_xfb[j];
        zx += (float)(cal_xfb[j]*cal_x[j]);
        zy += (float)(cal_xfb[j]*cal_y[j]);
    }

    // Now multiply out to get the calibration for framebuffer x coord
    cal_o[0] = (int)((cal_a*z + cal_b*zx + cal_c*zy)*(scaling));
    cal_o[1] = (int)((cal_b*z + cal_e*zx + cal_f*zy)*(scaling));
    cal_o[2] = (int)((cal_c*z + cal_f*zx + cal_i*zy)*(scaling));

    cout << "CAL: " << (cal_a*z + cal_b*zx + cal_c*zy) << " "
                    << (cal_b*z + cal_e*zx + cal_f*zy) << " "
                    << (cal_c*z + cal_f*zx + cal_i*zy) << endl;

    // Get sums for y calibration
    z = zx = zy = 0;
    for (j=0;j<5;j++) {
        z += (float)cal_yfb[j];
        zx += (float)(cal_yfb[j]*cal_x[j]);
        zy += (float)(cal_yfb[j]*cal_y[j]);
    }

    // Now multiply out to get the calibration for framebuffer y coord
    cal_o[3] = (int)((cal_a*z + cal_b*zx + cal_c*zy)*(scaling));
    cal_o[4] = (int)((cal_b*z + cal_e*zx + cal_f*zy)*(scaling));
    cal_o[5] = (int)((cal_c*z + cal_f*zx + cal_i*zy)*(scaling));

    cout << "CAL: " << (cal_a*z + cal_b*zx + cal_c*zy) << " "
                    << (cal_b*z + cal_e*zx + cal_f*zy) << " "
                    << (cal_c*z + cal_f*zx + cal_i*zy) << endl;


    // If we got here, we're OK, so assign scaling to a[6] and return
    cal_o[6] = (int) scaling;

    cout << "CAL constants: " << cal_o[0] << " " << cal_o[1] << " " << cal_o[2] << " "
                              << cal_o[3] << " " << cal_o[4] << " " << cal_o[5] << " " << cal_o[6] << endl;

    FILE* stream = fopen( "/etc/pointercal", "w" );
    if ( stream == NULL )
    {
        cout << "CalibrationRectangle::calibrate() - couldn't open /etc/pointercal (" << strerror( errno ) << ")" << endl;
        return false;
    }

    fprintf( stream, "%d %d %d %d %d %d %d\n", cal_o[1], cal_o[2], cal_o[0], cal_o[4], cal_o[5], cal_o[3], cal_o[6] );
    fclose( stream );

    return true;
}
