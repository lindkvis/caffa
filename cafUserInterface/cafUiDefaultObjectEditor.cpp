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

#include "cafUiDefaultObjectEditor.h"

#include "cafField.h"
#include "cafProxyValueField.h"
#include "cafUiCheckBoxEditor.h"
#include "cafUiDateEditor.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiFilePathEditor.h"
#include "cafUiLineEditor.h"
#include "cafUiListEditor.h"
#include "cafUiTimeEditor.h"

#include <QGridLayout>

namespace caf
{
// Register default field editor for selected types
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiCheckBoxEditor, bool );

CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiLineEditor, std::string );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiDateEditor, std::time_t );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiLineEditor, int );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiLineEditor, double );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiLineEditor, float );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiLineEditor, uint64_t );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiListEditor, std::vector<std::string> );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiListEditor, std::vector<int> );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiListEditor, std::vector<float> );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiDefaultObjectEditor::PdmUiDefaultObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiDefaultObjectEditor::~PdmUiDefaultObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDefaultObjectEditor::createWidget( QWidget* parent )
{
    QWidget* widget = new QWidget( parent );
    widget->setObjectName( "ObjectEditor" );
    return widget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDefaultObjectEditor::recursivelyConfigureAndUpdateTopLevelUiOrdering( const UiOrdering& topLevelUiOrdering )
{
    CAF_ASSERT( this->widget() );

    recursivelyConfigureAndUpdateUiOrderingInNewGridLayout( topLevelUiOrdering, this->widget() );
}

} // end namespace caf
