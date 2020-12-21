#pragma once

#include "cafColor.h"

namespace caf
{
//==================================================================================================
///
///
//==================================================================================================
class ColorTools
{
public:
    static float luminance( Color color );
    static bool  isColorBright( Color color );
    static Color blackOrWhiteContrastColor( Color color );

private:
    static float w3NonLinearColorValue( float colorFraction );
};
} // namespace caf
