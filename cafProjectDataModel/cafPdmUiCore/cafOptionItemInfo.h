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
#pragma once

#include "cafIconProvider.h"
#include "cafVariant.h"

#include <deque>
#include <memory>
#include <string>

namespace caf
{
//==================================================================================================
/// Class to keep Ui information about an option /choice in a Combobox or similar.
//==================================================================================================

class OptionItemInfo
{
public:
    // Template pass-through for enum types, ensuring the T type gets cast to an int before storing in the Variant
    // Note the extra dummy parameter. This ensures compilation fails for non-enum types and these variants get removed
    // due to SFINAE (https://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error)
    template <typename T>
    OptionItemInfo( const std::string&            anOptionUiText,
                    T                             aValue,
                    bool                          isReadOnly               = false,
                    std::shared_ptr<IconProvider> anIcon                   = nullptr,
                    typename std::enable_if<std::is_enum<T>::value>::type* = 0 )
        : OptionItemInfo( anOptionUiText, Variant( static_cast<int>( aValue ) ), isReadOnly, anIcon )
    {
    }
    OptionItemInfo( const std::string&            anOptionUiText,
                    const Variant&                aValue,
                    bool                          isReadOnly = false,
                    std::shared_ptr<IconProvider> anIcon     = nullptr );
    OptionItemInfo( const std::string&            anOptionUiText,
                    caf::ObjectHandle*            obj,
                    bool                          isReadOnly = false,
                    std::shared_ptr<IconProvider> anIcon     = nullptr );

    static OptionItemInfo createHeader( const std::string&            anOptionUiText,
                                        bool                          isReadOnly = false,
                                        std::shared_ptr<IconProvider> anIcon     = nullptr );

    void setLevel( int level );

    std::shared_ptr<IconProvider> iconProvider() const;
    const std::string             optionUiText() const;
    const Variant                 value() const;
    bool                          isReadOnly() const;
    bool                          isHeading() const;
    int                           level() const;

    // Static utility methods to handle std::list of OptionItemInfo
    // Please regard as private to the PDM system

    static std::deque<std::string> extractUiTexts( const std::deque<OptionItemInfo>& optionList );
   

private:
    std::string                   m_optionUiText;
    Variant                       m_value;
    bool                          m_isReadOnly;
    std::shared_ptr<IconProvider> m_iconProvider;
    int                           m_level;
};

} // namespace caf