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
#include "cafFieldScriptingCapability.h"

#include "cafFieldHandle.h"

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldScriptingCapability::FieldScriptingCapability( caffa::FieldHandle*  owner,
                                                    const std::string& scriptFieldName,
                                                    bool               giveOwnership )
{
    m_IOWriteable     = true;
    m_owner           = owner;
    m_scriptFieldName = scriptFieldName;
    owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldScriptingCapability::~FieldScriptingCapability()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string FieldScriptingCapability::scriptFieldName() const
{
    return m_scriptFieldName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldScriptingCapability::isIOWriteable() const
{
    return m_IOWriteable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldScriptingCapability::setIOWriteable( bool writeable )
{
    m_IOWriteable = writeable;
}

void FieldScriptingCapability::addToField( caffa::FieldHandle* field, const std::string& fieldName )
{
    if ( field->capability<FieldScriptingCapability>() == nullptr )
    {
        new FieldScriptingCapability( field, fieldName, true );
    }
}
