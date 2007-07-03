#include <eflpp_sys.h>

#include "calibrate.h"

#include <eflpp_ecore.h>
#include <eflpp_evas.h>
using namespace efl;

#define WIDTH 240
#define HEIGHT 320

int main( int argc, const char **argv )
{
    efl::EcoreApplication app( argc, argv, "Ecore Calibration Example" );
    efl::EcoreEvasWindow* mw = new efl::EcoreEvasWindow( WIDTH, HEIGHT );
    efl::EvasCanvas* evas = mw->canvas();
    Size s = evas->size();

    CalibrationRectangle* r = new CalibrationRectangle( 0, 0, s.width(), s.height(), evas );
    r->show(); // starts calibration

    app.exec();
    return 0;
}
