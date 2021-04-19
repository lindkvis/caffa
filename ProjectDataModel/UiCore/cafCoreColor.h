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
#include "cafCoreColorIo.h"
#include "cafValueFieldSpecializations.h"

namespace caffa
{
//==================================================================================================
/// Partial specialization for ValueFieldSpecialization< Color >
//==================================================================================================

template <>
class ValueFieldSpecialization<Color>
{
public:
    static Variant convert( const Color& value )
    {
        auto [red, green, blue, alpha]    = value.rgba();
        std::vector<unsigned char> colorV = { red, green, blue, alpha };
        return Variant( colorV );
    }

    static void setFromVariant( const Variant& variantValue, Color& value )
    {
        std::vector<unsigned char> colorV = variantValue.value<std::vector<unsigned char>>();
        CAFFA_ASSERT( colorV.size() == 4u );
        value = Color( colorV[0], colorV[1], colorV[2], colorV[3] );
    }

    static bool isEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return variantValue == variantValue2;
    }
};

} // end namespace caffa
