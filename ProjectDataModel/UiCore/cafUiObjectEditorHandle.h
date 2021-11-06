//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafObservingPointer.h"
#include "cafUiEditorHandle.h"

namespace caffa
{
class ObjectHandle;

//==================================================================================================
/// Abstract class to handle editors for complete Objects
//==================================================================================================

class UiObjectEditorHandle : public UiEditorHandle
{
public:
    UiObjectEditorHandle();
    ~UiObjectEditorHandle() override;

    void                setObject( ObjectHandle* object );
    ObjectHandle*       object();
    const ObjectHandle* object() const;

    /// This function is intended to be called after a Object has been created or deleted
    static void updateUiAllObjectEditors();

protected:
    virtual void cleanupBeforeSettingObject(){};

private:
    static std::set<UiObjectEditorHandle*> m_sRegisteredObjectEditors;
};

} // End of namespace caffa
