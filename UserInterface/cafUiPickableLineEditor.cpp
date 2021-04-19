//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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
#include "cafUiPickableLineEditor.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafPickEventHandler.h"
#include "cafUiDefaultObjectEditor.h"
#include "cafUiFieldEditorHandle.h"

using namespace caffa;

CAF_UI_FIELD_EDITOR_SOURCE_INIT( UiPickableLineEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::UiPickableLineEditor::~UiPickableLineEditor()
{
    if ( m_attribute.pickEventHandler )
    {
        m_attribute.pickEventHandler->unregisterAsPickEventHandler();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::UiPickableLineEditor::configureAndUpdateUi()
{
    UiLineEditor::configureAndUpdateUi();

    caffa::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &m_attribute );
    }

    if ( m_attribute.pickEventHandler )
    {
        if ( m_attribute.enablePicking )
        {
            m_attribute.pickEventHandler->registerAsPickEventHandler();
            m_lineEdit->setStyleSheet( "QLineEdit {"
                                       "color: #000000;"
                                       "background-color: #FFDCFF;}" );
        }
        else
        {
            m_attribute.pickEventHandler->unregisterAsPickEventHandler();
            m_lineEdit->setStyleSheet( "" );
        }
    }

    m_lineEdit->setToolTip( QString::fromStdString( uiField()->uiToolTip() ) );
}
