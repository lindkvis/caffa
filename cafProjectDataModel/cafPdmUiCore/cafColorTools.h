#pragma once

#include <QColor>

namespace caf
{
//==================================================================================================
///
///
//==================================================================================================
class ColorTools
{
public:
    static float  luminance( QColor color );
    static bool   isColorBright( QColor color );
    static QColor blackOrWhiteContrastColor( QColor color );

private:
    static float w3NonLinearColorValue( float colorFraction );
};
} // namespace caf
