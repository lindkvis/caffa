//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#pragma once

#include "cafColor.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace caffa
{
//==================================================================================================
/// Utility class to provide Icons when required. Qt crashes if a non-empty QIcon is created
/// ... without a GUI Application running. So create the icon on demand instead.
//==================================================================================================
class IconProvider
{
public:
    IconProvider( const std::pair<int, int>& preferredSize = std::pair<int, int>( 16, 16 ) );
    IconProvider( const std::string&         iconResourceString,
                  const std::pair<int, int>& preferredSize = std::pair<int, int>( 16, 16 ) );
    IconProvider( const IconProvider& rhs );
    IconProvider& operator=( const IconProvider& rhs );

    void setActive( bool active );
    bool valid() const;
    const std::pair<int, int> preferredSize() const;
    void setPreferredSize( const std::pair<int, int>& size );

    const std::string& iconResourceString() const;
    const std::string& overlayResourceString() const;
    const Color&       backgroundColor() const;

    void setIconResourceString( const std::string& iconResourceString );
    void setOverlayResourceString( const std::string& overlayResourceString );
    void setBackgroundColor( const Color& backgroundColor );

private:
    bool m_active;

    std::string              m_iconResourceString;
    std::string              m_overlayResourceString;
    Color                    m_backgroundColor;
    std::pair<int, int>      m_preferredSize;
};
} // namespace caffa
