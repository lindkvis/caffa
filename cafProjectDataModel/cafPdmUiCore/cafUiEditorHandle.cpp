//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafUiEditorHandle.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiEditorHandle::UiEditorHandle()
    : uiUpdated( this )
    , m_pdmItem( nullptr )
    , m_isConfiguringUi( false )
    , m_containingEditor( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiEditorHandle::~UiEditorHandle()
{
    if ( m_pdmItem ) m_pdmItem->removeFieldEditor( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiEditorHandle::updateUi()
{
    CAF_ASSERT( !m_isConfiguringUi );
    m_isConfiguringUi = true;
    this->configureAndUpdateUi();
    m_isConfiguringUi = false;

    uiUpdated.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiEditorHandle* UiEditorHandle::topMostContainingEditor()
{
    if ( m_containingEditor )
    {
        return m_containingEditor->topMostContainingEditor();
    }

    return this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiEditorHandle::bindToPdmItem( UiItem* item )
{
    if ( m_pdmItem ) m_pdmItem->removeFieldEditor( this );
    m_pdmItem = item;
    if ( m_pdmItem ) m_pdmItem->addFieldEditor( this );
}

} // End of namespace caf
