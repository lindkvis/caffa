#include "cafColorTable.h"

#include "cafAssert.h"

#include <algorithm>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ColorTable::ColorTable( const std::vector<QColor>& colors )
    : m_colors( colors )
{
    CAF_ASSERT( m_colors.size() > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::ColorTable::ColorTable( const ColorTable& colors )
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
QColor ColorTable::cycledColor( size_t index ) const
{
    size_t cycledIndex = index % m_colors.size();

    return m_colors[cycledIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ColorTable ColorTable::reversed() const
{
    std::vector<QColor> invertedColors = m_colors;
    std::reverse( invertedColors.begin(), invertedColors.end() );
    return ColorTable( invertedColors );
}
