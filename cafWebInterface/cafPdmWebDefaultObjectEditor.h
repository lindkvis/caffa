//##################################################################################################
//
//   Caffa Web Interface
//   Copyright (C) Ceetron AS
//   Copyright (C) Gaute Lindkvist
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

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4275 4564 )
#endif

#include "cafPdmWebFormLayoutObjectEditor.h"

namespace caf
{
class UiFieldEditorHandle;
class UiItem;
class UiGroup;

//==================================================================================================
/// The default editor for Objects. Manages the field editors in a grid layout vertically
//==================================================================================================
class PdmWebDefaultObjectEditor : public PdmWebFormLayoutObjectEditor
{
public:
    PdmWebDefaultObjectEditor();
    ~PdmWebDefaultObjectEditor() override;

private:
    Wt::WContainerWidget* createWidget() override;
    void                  recursivelyConfigureAndUpdateTopLevelUiOrdering( const UiOrdering& topLevelUiItems ) override;

protected:
    void         resetWidget( Wt::WContainerWidget* widget );
    virtual void cleanupBeforeSettingObject() override;
};

} // end namespace caf
#ifdef _MSC_VER
#pragma warning( pop )
#endif
