#pragma once

#include <QColor>

#include <vector>

namespace caf
{
//==================================================================================================
//
//
//
//==================================================================================================
class ColorTable
{
public:
    ColorTable( const std::vector<QColor>& colors );
    ColorTable( const ColorTable& colors );

    size_t size() const;

    QColor cycledColor( size_t index ) const;

    ColorTable reversed() const;

private:
    std::vector<QColor> m_colors;
};

} // namespace caf
