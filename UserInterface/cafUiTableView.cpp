//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafUiTableView.h"

#include "cafActionWrapper.h"
#include "cafChildArrayField.h"
#include "cafObject.h"
#include "cafSelectionManager.h"
#include "cafUiTableViewEditor.h"

#include "cafUiCommandSystemProxy.h"
#include <QTableView>
#include <QVBoxLayout>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTableView::UiTableView( QWidget* parent, Qt::WindowFlags f )
    : QWidget( parent, f )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );

    setLayout( layout );

    m_listViewEditor = new UiTableViewEditor();

    m_listViewEditor->createWidgets( this );

    {
        QWidget* widget = m_listViewEditor->labelWidget();
        layout->addWidget( widget );
    }

    {
        QWidget* widget = m_listViewEditor->editorWidget();
        layout->addWidget( widget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTableView::~UiTableView()
{
    if ( m_listViewEditor ) delete m_listViewEditor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableView::setChildArrayField( ChildArrayFieldHandle* childArrayField )
{
    CAFFA_ASSERT( m_listViewEditor );

    if ( childArrayField )
    {
        // Keep the possible custom context menu setting from the user of the table view.
        // setUIField will set it based on the UiItem settings, but turning the custom menu off should not
        // be respected when using the field in a separate view.
        auto orgContextPolicy = m_listViewEditor->tableView()->contextMenuPolicy();

        m_listViewEditor->setUiField( childArrayField->capability<FieldUiCapability>() );

        auto newContextPolicy = m_listViewEditor->tableView()->contextMenuPolicy();
        if ( newContextPolicy == Qt::DefaultContextMenu )
        {
            m_listViewEditor->tableView()->setContextMenuPolicy( orgContextPolicy );
        }
    }
    else
    {
        m_listViewEditor->setUiField( nullptr );
    }

    // SIG_CAFFA_HACK
    m_listViewEditor->updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTableView* UiTableView::tableView()
{
    CAFFA_ASSERT( m_listViewEditor );

    return m_listViewEditor->tableView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableView::enableHeaderText( bool enable )
{
    m_listViewEditor->enableHeaderText( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableView::setTableSelectionLevel( int selectionLevel )
{
    m_listViewEditor->setTableSelectionLevel( selectionLevel );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableView::setRowSelectionLevel( int selectionLevel )
{
    m_listViewEditor->setRowSelectionLevel( selectionLevel );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* UiTableView::objectFromModelIndex( const QModelIndex& mi )
{
    return m_listViewEditor->objectFromModelIndex( mi );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableView::addActionsToMenu( MenuInterface* menu, ChildArrayFieldHandle* childArrayField )
{
    // This is function is required to execute before populating the menu
    // Several commands rely on the activeChildArrayFieldHandle in the selection manager
    SelectionManager::instance()->setActiveChildArrayFieldHandle( childArrayField );

    caffa::UiCommandSystemProxy::instance()->populateMenuWithDefaultCommands( menu );
}

} // End of namespace caffa
