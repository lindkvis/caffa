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
#include "cafPdmWebDateEditor.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafFactory.h"
#include "cafField.h"
#include "cafObject.h"
#include "cafPdmWebDefaultObjectEditor.h"
#include "cafPdmWebFieldEditorHandle.h"
#include "cafSelectionManager.h"
#include "cafUiOrdering.h"

#include <Wt/WCalendar.h>
#include <Wt/WDate.h>
#include <Wt/WLineEdit.h>

namespace caf
{
CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT( PdmWebDateEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebDateEditor::configureAndUpdateUi()
{
    CAF_ASSERT( m_dateEdit );

    applyTextToLabel( m_label.get() );

    m_dateEdit->setEnabled( !uiField()->isUiReadOnly() );

    caf::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &m_attributes );
    }

    if ( !m_attributes.dateFormat.empty() )
    {
        m_dateEdit->setFormat( m_attributes.dateFormat );
    }
    Variant   v         = uiField()->uiValue();
    int       julianDay = v.value<int>();
    Wt::WDate date      = Wt::WDate::fromJulianDay( julianDay );

    m_dateEdit->setDate( date );
    m_dateEdit->changed().connect( [=] { this->slotEditingFinished(); } );
    // m_dateEdit->setMinimumSize(Wt::WLength(10, Wt::LengthUnit::FontEm), Wt::WLength::Auto);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebDateEditor::createEditorWidget()
{
    m_dateEdit = new Wt::WDateEdit;
    return m_dateEdit.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WLabel* PdmWebDateEditor::createLabelWidget()
{
    m_label = new Wt::WLabel;
    return m_label.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebDateEditor::slotEditingFinished()
{
    if ( m_dateEdit->validate() == Wt::ValidationState::Valid )
    {
        auto    date      = m_dateEdit->date();
        int     julianDay = date.toJulianDay();
        Variant v( julianDay );
        this->setValueToField( v );
    }
}

} // end namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
