//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
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

#include "cafUiFieldEditorHelper.h"

#include "cafClassTypeName.h"

#include "cafFieldUiCapability.h"
#include "cafUiComboBoxEditor.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiListEditor.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiFieldEditorHandle* caf::UiFieldEditorHelper::createFieldEditorForField( caf::FieldUiCapability* field,
                                                                                     const QString& uiConfigName )
{
    caf::UiFieldEditorHandle* fieldEditor = nullptr;

    // If editor type is specified, find in factory
    if ( !field->uiEditorTypeName( uiConfigName ).isEmpty() )
    {
        fieldEditor =
            caf::Factory<UiFieldEditorHandle, QString>::instance()->create( field->uiEditorTypeName( uiConfigName ) );
    }
    else
    {
        // Find the default field editor
        QString fieldTypeName = qStringTypeName( *( field->fieldHandle() ) );

        if ( fieldTypeName.indexOf( "PtrField" ) != -1 )
        {
            fieldTypeName = caf::PdmUiComboBoxEditor::uiEditorTypeName();
        }
        else if ( fieldTypeName.indexOf( "PtrArrayField" ) != -1 )
        {
            fieldTypeName = caf::PdmUiListEditor::uiEditorTypeName();
        }
        else if ( field->toUiBasedQVariant().type() != QVariant::List )
        {
            // Handle a single value field with valueOptions: Make a combobox

            bool                     useOptionsOnly = true;
            QList<PdmOptionItemInfo> options        = field->valueOptions( &useOptionsOnly );
            CAF_ASSERT( useOptionsOnly ); // Not supported

            if ( !options.empty() )
            {
                fieldTypeName = caf::PdmUiComboBoxEditor::uiEditorTypeName();
            }
        }

        fieldEditor = caf::Factory<UiFieldEditorHandle, QString>::instance()->create( fieldTypeName );
    }

    return fieldEditor;
}