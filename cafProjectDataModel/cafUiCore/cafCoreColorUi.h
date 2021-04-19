//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
#include "cafCoreColor.h"
#include "cafUiFieldSpecialization.h"
#include "cafUiItem.h"
#include "cafValueFieldSpecializations.h"

namespace caffa
{
template <>
class UiFieldSpecialization<Color>
{
public:
    /// Convert the field value into a QVariant
    static Variant convert( const Color& value ) { return ValueFieldSpecialization<Color>::convert( value ); }

    /// Set the field value from a QVariant
    static void setFromVariant( const Variant& variantValue, Color& value )
    {
        ValueFieldSpecialization<Color>::setFromVariant( variantValue, value );
    }

    static bool isDataElementEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return ValueFieldSpecialization<Color>::isEqual( variantValue, variantValue2 );
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static std::deque<OptionItemInfo> valueOptions( bool* useOptionsOnly, const Color& )
    {
        return std::deque<OptionItemInfo>();
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<Color>&, std::vector<ObjectHandle*>* ) {}
};

} // end namespace caffa

//--------------------------------------------------------------------------------------------------
// If the macro for registering the editor is put as the single statement
// in a cpp file, a dummy static class must be used to make sure the compile unit
// is included
//--------------------------------------------------------------------------------------------------
class ColorInitializer
{
public:
    ColorInitializer();
};
static ColorInitializer colorInitializer;
