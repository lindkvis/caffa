#include "cafColorTools.h"

#include <algorithm>
#include <cmath>

using namespace caf;

//--------------------------------------------------------------------------------------------------
/// Checks if W3 luminance is above halfway
//--------------------------------------------------------------------------------------------------
bool ColorTools::isColorBright( QColor color )
{
    if ( luminance( color ) >= 0.5 )
    {
        return true;
    }

    return false;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor ColorTools::blackOrWhiteContrastColor( QColor color )
{
    if ( isColorBright( color ) )
    {
        return QColor( Qt::black );
    }
    return QColor( Qt::white );
}

//--------------------------------------------------------------------------------------------------
/// W3 relative luminance
/// https://www.w3.org/TR/WCAG20-TECHS/G18.html
//--------------------------------------------------------------------------------------------------
float ColorTools::luminance( QColor color )
{
    float redValue   = w3NonLinearColorValue( color.redF() );
    float greenValue = w3NonLinearColorValue( color.greenF() );
    float blueValue  = w3NonLinearColorValue( color.blueF() );

    return 0.2126f * redValue + 0.7152f * greenValue + 0.0722f * blueValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float ColorTools::w3NonLinearColorValue( float colorFraction )
{
    if ( colorFraction <= 0.03928f )
    {
        return colorFraction / 12.92f;
    }
    return std::pow( ( colorFraction + 0.055f ) / 1.055f, 2.4f );
}
