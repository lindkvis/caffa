#pragma once

#include "cafColorTable.h"

namespace caf
{
class ColorTables
{
public:
    static caf::ColorTable kellyColors();
    static caf::ColorTable salmonContrast();
    static caf::ColorTable svgColors( int maxCount );
};
} // namespace caf
