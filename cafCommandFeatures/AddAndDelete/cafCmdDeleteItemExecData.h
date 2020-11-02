//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron AS
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

#include "cafField.h"
#include "cafObject.h"

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class CmdDeleteItemExecData : public Object
{
    CAF_HEADER_INIT;

public:
    CmdDeleteItemExecData()
    {
        CAF_InitObject( "CmdDeleteItemExecData uiName",
                            "",
                            "CmdDeleteItemExecData tooltip",
                            "CmdDeleteItemExecData whatsthis" );

        CAF_InitField( &m_pathToField, "PathToField", QString(), "PathToField", "", "PathToField tooltip", "PathToField whatsthis" );
        CAF_InitField( &m_indexToObject,
                           "indexToObject",
                           -1,
                           "indexToObject",
                           "",
                           "indexToObject tooltip",
                           "indexToObject whatsthis" );
        CAF_InitField( &m_deletedObjectAsXml,
                           "deletedObjectAsXml",
                           QString(),
                           "deletedObjectAsXml",
                           "",
                           "deletedObjectAsXml tooltip",
                           "deletedObjectAsXml whatsthis" );
    }

    caf::PdmPointer<ObjectHandle> m_rootObject;

    caf::Field<QString> m_pathToField;
    caf::Field<int>     m_indexToObject;
    caf::Field<QString> m_deletedObjectAsXml;
};

} // end namespace caf
