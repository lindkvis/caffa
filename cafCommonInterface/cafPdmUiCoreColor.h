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

#include "cafPdmUiFieldSpecialization.h"
#include "cafPdmUiItem.h"
#include "cafValueFieldSpecializations.h"

#include "cafPdmCoreColor.h"

#include <QColor>

namespace caf
{
template <>
class PdmUiFieldSpecialization<QColor>
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert( const QColor& value ) { return ValueFieldSpecialization<QColor>::convert( value ); }

    /// Set the field value from a QVariant
    static void setFromVariant( const QVariant& variantValue, QColor& value )
    {
        ValueFieldSpecialization<QColor>::setFromVariant( variantValue, value );
    }

    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return ValueFieldSpecialization<QColor>::isEqual( variantValue, variantValue2 );
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const QColor& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<QColor>&, std::vector<ObjectHandle*>* ) {}
};

} // end namespace caf

//--------------------------------------------------------------------------------------------------
// If the macro for registering the editor is put as the single statement
// in a cpp file, a dummy static class must be used to make sure the compile unit
// is included
//--------------------------------------------------------------------------------------------------
class PdmColorInitializer
{
public:
    PdmColorInitializer();
};
static PdmColorInitializer pdmColorInitializer;
