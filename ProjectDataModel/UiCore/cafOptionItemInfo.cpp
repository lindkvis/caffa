//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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
#include "cafOptionItemInfo.h"

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
OptionItemInfo::OptionItemInfo( const std::string& anOptionUiText, const Variant& aValue, bool isReadOnly /* = false */ )
    : m_optionUiText( anOptionUiText )
    , m_value( aValue )
    , m_isReadOnly( isReadOnly )
    , m_level( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
OptionItemInfo::OptionItemInfo( const std::string& anOptionUiText, caffa::ObjectHandle* obj, bool isReadOnly /*= false*/ )
    : m_optionUiText( anOptionUiText )
    , m_isReadOnly( isReadOnly )
    , m_level( 0 )
{
    m_value = Variant( caffa::ObservingPointer<caffa::ObjectHandle>( obj ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
OptionItemInfo OptionItemInfo::createHeader( const std::string& anOptionUiText, bool isReadOnly /*= false*/ )
{
    OptionItemInfo header( anOptionUiText, Variant(), isReadOnly );

    return header;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void OptionItemInfo::setLevel( int level )
{
    m_level = level;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string OptionItemInfo::optionUiText() const
{
    return m_optionUiText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const Variant OptionItemInfo::value() const
{
    return m_value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool OptionItemInfo::isReadOnly() const
{
    return m_isReadOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool OptionItemInfo::isHeading() const
{
    return !m_value.isValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int OptionItemInfo::level() const
{
    return m_level;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::deque<std::string> OptionItemInfo::extractUiTexts( const std::deque<OptionItemInfo>& optionList )
{
    std::deque<std::string> texts;

    for ( const auto& option : optionList )
    {
        texts.push_back( option.optionUiText() );
    }

    return texts;
}
