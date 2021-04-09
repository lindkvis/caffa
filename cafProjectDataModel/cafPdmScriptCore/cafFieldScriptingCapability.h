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

#include "cafFieldCapability.h"
#include "cafFieldHandle.h"

#include <string>

namespace caf
{
class FieldHandle;
class ObjectFactory;
class ObjectHandle;
class PdmScriptIOMessages;

class FieldScriptingCapability : public FieldCapability
{
public:
    FieldScriptingCapability( caf::FieldHandle* owner, const std::string& scriptFieldName, bool giveOwnership );
    virtual ~FieldScriptingCapability();

    const std::string scriptFieldName() const;

    bool        isIOWriteable() const;
    void        setIOWriteable( bool writeable );
    static void addToField( caf::FieldHandle* field, const std::string& fieldName );

protected:
    FieldHandle* m_owner;
    std::string  m_scriptFieldName;
    bool         m_IOWriteable;
};

} // namespace caf

#define CAF_InitScriptableField( field, keyword, default, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_InitField( field, keyword, default, uiName, iconResourceName, toolTip, whatsThis );              \
    caf::FieldScriptingCapability::addToField( field, keyword )

#define CAF_InitScriptableFieldNoDefault( field, keyword, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_InitFieldNoDefault( field, keyword, uiName, iconResourceName, toolTip, whatsThis );              \
    caf::FieldScriptingCapability::addToField( field, keyword )

#define CAF_InitScriptableFieldWithScriptKeyword( field, keyword, scriptKeyword, default, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_InitField( field, keyword, default, uiName, iconResourceName, toolTip, whatsThis );                                              \
    caf::FieldScriptingCapability::addToField( field, keyword )

#define CAF_InitScriptableFieldWithScriptKeywordNoDefault( field, keyword, scriptKeyword, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_InitFieldNoDefault( field, keyword, uiName, iconResourceName, toolTip, whatsThis );                                              \
    caf::FieldScriptingCapability::addToField( field, keyword )
