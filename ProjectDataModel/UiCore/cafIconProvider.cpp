//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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
#include "cafIconProvider.h"

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider( const std::pair<int, int>& preferredSize )
    : m_active( true )
    , m_preferredSize( preferredSize )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider( const std::string& iconResourceString, const std::pair<int, int>& preferredSize )
    : m_active( true )
    , m_iconResourceString( iconResourceString )
    , m_preferredSize( preferredSize )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider( const IconProvider& rhs )
    : m_active( rhs.m_active )
    , m_iconResourceString( rhs.m_iconResourceString )
    , m_overlayResourceString( rhs.m_overlayResourceString )
    , m_preferredSize( rhs.m_preferredSize )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider& IconProvider::operator=( const IconProvider& rhs )
{
    m_active                = rhs.m_active;
    m_iconResourceString    = rhs.m_iconResourceString;
    m_overlayResourceString = rhs.m_overlayResourceString;
    m_preferredSize         = rhs.m_preferredSize;
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setActive( bool active )
{
    m_active = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool IconProvider::valid() const
{
    return !m_iconResourceString.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::pair<int, int> IconProvider::preferredSize() const
{
    return m_preferredSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setPreferredSize( const std::pair<int, int>& size )
{
    m_preferredSize = size;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setIconResourceString( const std::string& iconResourceString )
{
    m_iconResourceString = iconResourceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setOverlayResourceString( const std::string& overlayResourceString )
{
    m_overlayResourceString = overlayResourceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& IconProvider::iconResourceString() const
{
    return m_iconResourceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& caffa::IconProvider::overlayResourceString() const
{
    return m_overlayResourceString;
}
