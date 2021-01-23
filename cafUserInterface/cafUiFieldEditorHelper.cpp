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

#include "cafFieldHandle.h"
#include "cafFieldUiCapability.h"
#include "cafUiComboBoxEditor.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiListEditor.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiFieldEditorHandle* caf::UiFieldEditorHelper::createFieldEditorForField( caf::FieldUiCapability* field )
{
    caf::UiFieldEditorHandle* fieldEditor = nullptr;

    // If editor type is specified, find in factory
    if ( !field->uiEditorTypeName().empty() )
    {
        fieldEditor = caf::Factory<UiFieldEditorHandle, std::string>::instance()->create( field->uiEditorTypeName() );
    }
    else
    {
        // Find the default field editor
        std::string fieldTypeName = typeid( *( field->fieldHandle() ) ).name();

        if ( fieldTypeName.find( "PtrField" ) != std::string::npos )
        {
            fieldTypeName = caf::PdmUiComboBoxEditor::uiEditorTypeName();
        }
        else if ( fieldTypeName.find( "PtrArrayField" ) != std::string::npos )
        {
            fieldTypeName = caf::PdmUiListEditor::uiEditorTypeName();
        }
        else if ( field->toUiBasedVariant().isVector() )
        {
            // Handle a single value field with valueOptions: Make a combobox

            bool                       useOptionsOnly = true;
            std::deque<OptionItemInfo> options        = field->valueOptions( &useOptionsOnly );
            CAF_ASSERT( useOptionsOnly ); // Not supported

            if ( !options.empty() )
            {
                fieldTypeName = caf::PdmUiComboBoxEditor::uiEditorTypeName();
            }
        }

        fieldEditor = caf::Factory<UiFieldEditorHandle, std::string>::instance()->create( fieldTypeName );
    }

    return fieldEditor;
}
