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

#include "cafUiFieldEditorHandle.h"

#include "cafFieldUiCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectUiCapability.h"
#include "cafQActionWrapper.h"
#include "cafUiCommandSystemProxy.h"

#include <QAbstractScrollArea>
#include <QLabel>
#include <QMenu>
#include <QVariant>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiFieldEditorHandle::UiFieldEditorHandle()
{
    m_combinedWidget = QPointer<QWidget>();
    m_editorWidget   = QPointer<QWidget>();
    m_labelWidget    = QPointer<QWidget>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiFieldEditorHandle::~UiFieldEditorHandle()
{
    // Note : deleteLater will not work unless you are actually inside an event loop.
    // See https://doc.qt.io/qt-5/qobject.html#deleteLater
    // Although it states that they will be deleted at startup of the event loop, it seems as that is not happening.

    if ( !m_combinedWidget.isNull() ) m_combinedWidget->deleteLater();
    if ( !m_editorWidget.isNull() ) m_editorWidget->deleteLater();
    if ( !m_labelWidget.isNull() ) m_labelWidget->deleteLater();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiFieldEditorHandle::setUiField( FieldUiCapability* field )
{
    this->bindToItem( field );

    if ( m_editorWidget )
    {
        // Required to be called here to be able to handle different context menu
        // policy when switching between objects of same type. In this case, the
        // UiFieldEditorHandle::createWidgets() will not be run, as the field
        // widgets are cached by the property editor

        updateContextMenuPolicy();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiFieldEditorHandle::updateContextMenuPolicy()
{
    if ( m_editorWidget.isNull() ) return;

    FieldUiCapability* field = uiField();
    if ( field && field->isCustomContextMenuEnabled() )
    {
        m_editorWidget->setContextMenuPolicy( Qt::CustomContextMenu );
    }
    else
    {
        m_editorWidget->setContextMenuPolicy( Qt::DefaultContextMenu );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldUiCapability* UiFieldEditorHandle::uiField()
{
    return dynamic_cast<FieldUiCapability*>( item() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiFieldEditorHandle::createWidgets( QWidget* parent )
{
    if ( m_combinedWidget.isNull() ) m_combinedWidget = createCombinedWidget( parent );
    if ( m_editorWidget.isNull() ) m_editorWidget = createEditorWidget( parent );
    if ( m_labelWidget.isNull() ) m_labelWidget = createLabelWidget( parent );

    if ( m_editorWidget )
    {
        updateContextMenuPolicy();

        connect( m_editorWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( customMenuRequested( QPoint ) ) );
    }
    if ( m_labelWidget )
    {
        m_labelWidget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMargins UiFieldEditorHandle::labelContentMargins() const
{
    return calculateLabelContentMargins();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiFieldEditorHandle::rowStretchFactor() const
{
    return isMultiRowEditor() ? 1 : 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiFieldEditorHandle::setValueToField( const Variant& newUiValue )
{
    UiCommandSystemProxy::instance()->setUiValueToField( uiField(), newUiValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiFieldEditorHandle::updateLabelFromField( QLabel* label /*= ""*/ ) const
{
    CAFFA_ASSERT( label );

    const FieldUiCapability* fieldHandle = dynamic_cast<const FieldUiCapability*>( item() );
    if ( fieldHandle )
    {
        auto  iconProvider = fieldHandle->uiIconProvider();
        QIcon ic( QString::fromStdString( iconProvider->iconResourceString() ) );
        if ( !ic.isNull() )
        {
            label->setPixmap( ic.pixmap( ic.actualSize( QSize( 64, 64 ) ) ) );
        }
        else
        {
            label->setText( QString::fromStdString( fieldHandle->uiName() ) );
        }

        label->setEnabled( !fieldHandle->isUiReadOnly() );
        label->setToolTip( QString::fromStdString( fieldHandle->uiToolTip() ) );
    }
}

//------------------------------------------------------------------------------------------------------------
/// Re-implement this virtual method if a custom UiField is misaligned with its label.
/// See cafUiLineEditor for an example.
//------------------------------------------------------------------------------------------------------------
QMargins UiFieldEditorHandle::calculateLabelContentMargins() const
{
    return m_labelWidget->contentsMargins();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiFieldEditorHandle::isMultiRowEditor() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiFieldEditorHandle::customMenuRequested( QPoint pos )
{
    ObjectHandle* objectHandle = nullptr;
    if ( uiField() && uiField()->fieldHandle() )
    {
        objectHandle = uiField()->fieldHandle()->ownerObject();
    }

    if ( !objectHandle )
    {
        return;
    }

    auto widget = editorWidget();
    if ( widget )
    {
        QPoint globalPos;

        auto abstractScrollAreaWidget = dynamic_cast<QAbstractScrollArea*>( widget );

        if ( abstractScrollAreaWidget )
        {
            // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the
            // viewport().
            globalPos = abstractScrollAreaWidget->viewport()->mapToGlobal( pos );
        }
        else
        {
            globalPos = widget->mapToGlobal( pos );
        }

        QMenuWrapper menuWrapper;
        // UiCommandSystemProxy::instance()->setCurrentContextMenuTargetWidget( widget );
        // objectHandle->capability<ObjectUiCapability>()->defineCustomContextMenu( uiField()->fieldHandle(),
        //&menuWrapper,
        // widget );

        if ( !menuWrapper.actions().empty() )
        {
            menuWrapper.menu()->exec( globalPos );
        }
        // UiCommandSystemProxy::instance()->setCurrentContextMenuTargetWidget( nullptr );
    }
}

} // End of namespace caffa
