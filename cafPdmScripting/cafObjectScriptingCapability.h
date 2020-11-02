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

#include "cafChildArrayField.h"
#include "cafObjectCapability.h"
#include "cafObjectMethod.h"
#include "cafObjectScriptingCapabilityRegister.h"

#include <QString>

#include <map>
#include <memory>
#include <vector>

class QTextStream;

#define CAF_PDM_InitScriptableObject( uiName, iconResourceName, toolTip, whatsThis )              \
    CAF_InitObject( uiName, iconResourceName, toolTip, whatsThis );                           \
    caf::ObjectScriptingCapabilityRegister::registerScriptClassNameAndComment( classKeyword(), \
                                                                                  classKeyword(), \
                                                                                  whatsThis );

#define CAF_PDM_InitScriptableObjectWithNameAndComment( uiName, iconResourceName, toolTip, whatsThis, scriptClassName, scriptComment ) \
    CAF_InitObject( uiName, iconResourceName, toolTip, whatsThis );                                                                \
    caf::ObjectScriptingCapabilityRegister::registerScriptClassNameAndComment( classKeyword(),                                      \
                                                                                  scriptClassName,                                     \
                                                                                  scriptComment );

namespace caf
{
class Object;
class ObjectHandle;
class ObjectFactory;
class PdmScriptIOMessages;

//==================================================================================================
//
//
//
//==================================================================================================
class ObjectScriptingCapability : public ObjectCapability
{
public:
    ObjectScriptingCapability( ObjectHandle* owner, bool giveOwnership );

    ~ObjectScriptingCapability() override;

    void readFields( QTextStream& inputStream, ObjectFactory* objectFactory, PdmScriptIOMessages* errorMessageContainer );
    void writeFields( QTextStream& outputStream ) const;

private:
    ObjectHandle* m_owner;
};
} // namespace caf
