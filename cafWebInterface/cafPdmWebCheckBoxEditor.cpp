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

#include "cafPdmWebCheckBoxEditor.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafFactory.h"
#include "cafField.h"
#include "cafObject.h"
#include "cafPdmWebDefaultObjectEditor.h"
#include "cafUiOrdering.h"
#include "cafVariant.h"

namespace caf
{
CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT( PdmWebCheckBoxEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebCheckBoxEditor::configureAndUpdateUi()
{
    CAF_ASSERT( m_checkBox );
    CAF_ASSERT( m_label );

    applyTextToLabel( m_label.get() );

    m_checkBox->setEnabled( !uiField()->isUiReadOnly() );
    m_checkBox->setToolTip( uiField()->uiToolTip() );

    m_checkBox->setChecked( uiField()->uiValue().value<bool>() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebCheckBoxEditor::createEditorWidget()
{
    m_checkBox = new Wt::WCheckBox;
    m_checkBox->changed().connect( std::bind( &PdmWebCheckBoxEditor::slotClicked, this ) );

    return m_checkBox.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WLabel* PdmWebCheckBoxEditor::createLabelWidget()
{
    m_label = new Wt::WLabel;
    return m_label.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebCheckBoxEditor::slotClicked()
{
    Variant v;
    v = m_checkBox->isChecked();
    this->setValueToField( v );
}

} // end namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
