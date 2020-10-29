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

#include "cafUiPushButtonEditor.h"

#include "cafUiDefaultObjectEditor.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiOrdering.h"

#include "cafFactory.h"

#include <QBoxLayout>

#include <cmath>

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiPushButtonEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPushButtonEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    CAF_ASSERT( !m_pushButton.isNull() );
    CAF_ASSERT( !m_label.isNull() );

    UiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );

    m_pushButton->setCheckable( true );
    m_pushButton->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
    m_pushButton->setToolTip( uiField()->uiToolTip( uiConfigName ) );

    PdmUiPushButtonEditorAttribute attributes;
    caf::ObjectUiCapability*    uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &attributes );
    }

    QVariant variantFieldValue = uiField()->uiValue();

    if ( !attributes.m_buttonIcon.isNull() )
    {
        m_pushButton->setIcon( attributes.m_buttonIcon );
    }

    if ( !attributes.m_buttonText.isEmpty() )
    {
        m_pushButton->setText( attributes.m_buttonText );
    }
    else
    {
        if ( variantFieldValue.type() == QVariant::Bool )
        {
            m_pushButton->setText( variantFieldValue.toBool() ? "On" : "Off" );
        }
        else
        {
            m_pushButton->setText( variantFieldValue.toString() );
        }
    }

    if ( uiField()->uiLabelPosition( uiConfigName ) != UiItemInfo::HIDDEN )
    {
        QSize defaultSize = m_pushButton->sizeHint();
        m_pushButton->setMinimumWidth( 10 * std::round( 0.1 * ( defaultSize.width() + 10 ) ) );
        m_buttonLayout->setAlignment( m_pushButton, Qt::AlignRight );
    }

    if ( variantFieldValue.type() == QVariant::Bool )
    {
        m_pushButton->setChecked( uiField()->uiValue().toBool() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPushButtonEditor::configureEditorForField( FieldHandle* fieldHandle )
{
    if ( fieldHandle )
    {
        if ( fieldHandle->capability<FieldIoCapability>() )
        {
            fieldHandle->capability<FieldIoCapability>()->disableIO();
        }

        if ( fieldHandle->capability<FieldUiCapability>() )
        {
            fieldHandle->capability<FieldUiCapability>()->setUiEditorTypeName(
                caf::PdmUiPushButtonEditor::uiEditorTypeName() );
            fieldHandle->capability<FieldUiCapability>()->setUiLabelPosition( caf::UiItemInfo::LEFT );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiPushButtonEditor::createEditorWidget( QWidget* parent )
{
    QWidget* containerWidget = new QWidget( parent );

    m_pushButton = new QPushButton( "", containerWidget );
    connect( m_pushButton, SIGNAL( clicked( bool ) ), this, SLOT( slotClicked( bool ) ) );

    m_buttonLayout = new QHBoxLayout( containerWidget );
    m_buttonLayout->addWidget( m_pushButton );
    m_buttonLayout->setMargin( 0 );
    m_buttonLayout->setSpacing( 0 );

    containerWidget->setLayout( m_buttonLayout );

    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiPushButtonEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPushButtonEditor::slotClicked( bool checked )
{
    if ( uiField() && dynamic_cast<Field<bool>*>( uiField()->fieldHandle() ) )
    {
        QVariant v;
        v = checked;
        this->setValueToField( v );
    }
}

} // end namespace caf