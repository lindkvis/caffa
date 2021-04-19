#include "cafColorTools.h"

#include <algorithm>
#include <cmath>

using namespace caffa;

//--------------------------------------------------------------------------------------------------
/// Checks if W3 luminance is above halfway
//--------------------------------------------------------------------------------------------------
bool ColorTools::isColorBright( Color color )
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
Color ColorTools::blackOrWhiteContrastColor( Color color )
{
    if ( isColorBright( color ) )
    {
        return Color( 0, 0, 0, 0 );
    }
    return Color( 255, 255, 255, 255 );
}

//--------------------------------------------------------------------------------------------------
/// W3 relative luminance
/// https://www.w3.org/TR/WCAG20-TECHS/G18.html
//--------------------------------------------------------------------------------------------------
float ColorTools::luminance( Color color )
{
    float redValue   = w3NonLinearColorValue( color.getf<Color::RED>() );
    float greenValue = w3NonLinearColorValue( color.getf<Color::GREEN>() );
    float blueValue  = w3NonLinearColorValue( color.getf<Color::BLUE>() );

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
