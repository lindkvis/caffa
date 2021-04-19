#include "cafColorTable.h"

#include "cafAssert.h"

#include <algorithm>

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ColorTable::ColorTable( const std::vector<Color>& colors )
    : m_colors( colors )
{
    CAFFA_ASSERT( m_colors.size() > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::ColorTable::ColorTable( const ColorTable& colors )
    : m_colors( colors.m_colors )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t ColorTable::size() const
{
    return m_colors.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Color ColorTable::cycledColor( size_t index ) const
{
    size_t cycledIndex = index % m_colors.size();

    return m_colors[cycledIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ColorTable ColorTable::reversed() const
{
    std::vector<Color> invertedColors = m_colors;
    std::reverse( invertedColors.begin(), invertedColors.end() );
    return ColorTable( invertedColors );
}
