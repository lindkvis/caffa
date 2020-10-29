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

#include "cafPdmWebFieldEditorHandle.h"

#include "cafFieldUiCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectUiCapability.h"
#include "cafUiCommandSystemProxy.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WLabel.h>
#include <Wt/WWidget.h>

#include <QVariant>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebFieldEditorHandle::PdmWebFieldEditorHandle()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebFieldEditorHandle::~PdmWebFieldEditorHandle()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebFieldEditorHandle::setUiField( FieldUiCapability* field )
{
    this->bindToPdmItem( field );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmWebFieldEditorHandle::hasLabel() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebFieldEditorHandle::applyTextToLabel( Wt::WLabel* label, const QString& uiConfigName ) const
{
    if ( label )
    {
        const FieldUiCapability* fieldHandle = dynamic_cast<const FieldUiCapability*>( pdmItem() );
        if ( fieldHandle )
        {
            std::string labelText = fieldHandle->uiName( uiConfigName ).toStdString();
            label->setText( labelText );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Wt::WWidget>
    PdmWebFieldEditorHandle::findOrCreateCombinedWidget( std::list<std::unique_ptr<Wt::WWidget>>& existingWidgets )
{
    if ( m_combinedWidget )
    {
        for ( std::unique_ptr<Wt::WWidget>& widgetPtr : existingWidgets )
        {
            if ( widgetPtr.get() == m_combinedWidget.get() )
            {
                return std::move( widgetPtr );
            }
        }
        CAF_ASSERT( false && "Should never happen" );
        return nullptr;
    }
    else
    {
        m_combinedWidget = createCombinedWidget();
        return std::unique_ptr<Wt::WWidget>( m_combinedWidget.get() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Wt::WWidget>
    PdmWebFieldEditorHandle::findOrCreateEditorWidget( std::list<std::unique_ptr<Wt::WWidget>>& existingWidgets )
{
    if ( m_editorWidget )
    {
        for ( std::unique_ptr<Wt::WWidget>& widgetPtr : existingWidgets )
        {
            if ( widgetPtr.get() == m_editorWidget.get() )
            {
                return std::move( widgetPtr );
            }
        }
        CAF_ASSERT( false && "Should never happen" );
        return nullptr;
    }
    else
    {
        m_editorWidget = createEditorWidget();
        return std::unique_ptr<Wt::WWidget>( m_editorWidget.get() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Wt::WLabel>
    PdmWebFieldEditorHandle::findOrCreateLabelWidget( std::list<std::unique_ptr<Wt::WWidget>>& existingWidgets )
{
    if ( m_labelWidget )
    {
        for ( std::unique_ptr<Wt::WWidget>& widgetPtr : existingWidgets )
        {
            if ( widgetPtr.get() == m_labelWidget.get() )
            {
                return std::unique_ptr<Wt::WLabel>( static_cast<Wt::WLabel*>( widgetPtr.release() ) );
            }
        }
        CAF_ASSERT( false && "Should never happen" );
        return nullptr;
    }
    else
    {
        m_labelWidget = createLabelWidget();
        m_labelWidget->setMinimumSize( Wt::WLength( 10, Wt::LengthUnit::FontEx ), Wt::WLength::Auto );

        return std::unique_ptr<Wt::WLabel>( m_labelWidget.get() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmWebFieldEditorHandle::rowStretchFactor() const
{
    return isMultiRowEditor() ? 1 : 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebFieldEditorHandle::updateContextMenuPolicy()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldUiCapability* PdmWebFieldEditorHandle::uiField()
{
    return dynamic_cast<FieldUiCapability*>( pdmItem() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebFieldEditorHandle::setValueToField( const QVariant& newUiValue )
{
    UiCommandSystemProxy::instance()->setUiValueToField( uiField(), newUiValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmWebFieldEditorHandle::isMultiRowEditor() const
{
    return false;
}

} // End of namespace caf
