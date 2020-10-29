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


#include "cafPdmWebTreeEditorHandle.h"
#include "cafObject.h"

#include <Wt/WWidget.h>

namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebTreeEditorHandle::getOrCreateWidget()
{
    if (!m_widget) 
    {
        m_widget = this->createWidget();
    }
    return m_widget.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebTreeEditorHandle::widget()
{
    return m_widget.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmWebTreeEditorHandle::setPdmItemRoot(UiItem* root)
{
    this->bindToPdmItem(root);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UiItem* PdmWebTreeEditorHandle::pdmItemRoot()
{
    return this->pdmItem();
}

} //End of namespace caf

