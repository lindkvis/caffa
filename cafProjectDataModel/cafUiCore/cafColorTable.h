#pragma once

#include "cafColor.h"

#include <vector>

namespace caffa
{
//==================================================================================================
//
//
//
//==================================================================================================
class ColorTable
{
public:
    ColorTable( const std::vector<Color>& colors );
    ColorTable( const ColorTable& colors );

    size_t size() const;

    Color cycledColor( size_t index ) const;

    ColorTable reversed() const;

private:
    std::vector<Color> m_colors;
};

} // namespace caffa
