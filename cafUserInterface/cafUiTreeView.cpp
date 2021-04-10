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

#include "cafUiTreeView.h"

#include "cafObject.h"
#include "cafUiDefaultObjectEditor.h"

#include "cafUiTreeViewEditor.h"
#include <QHBoxLayout>
#include <QTreeView>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTreeView::UiTreeView( QWidget* parent, Qt::WindowFlags f )
    : QWidget( parent, f )
{
    m_layout = new QVBoxLayout( this );
    m_layout->setContentsMargins( 0, 0, 0, 0 );

    setLayout( m_layout );

    m_treeViewEditor = new UiTreeViewEditor();

    QWidget* widget = m_treeViewEditor->getOrCreateWidget( this );

    this->m_layout->insertWidget( 0, widget );

    connect( m_treeViewEditor, SIGNAL( selectionChanged() ), SLOT( slotOnSelectionChanged() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTreeView::~UiTreeView()
{
    if ( m_treeViewEditor ) delete m_treeViewEditor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeView::setItem( caf::UiItem* object )
{
    m_treeViewEditor->setItemRoot( object );
    m_treeViewEditor->updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTreeView* UiTreeView::treeView()
{
    return m_treeViewEditor->treeView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTreeView::isTreeItemEditWidgetActive() const
{
    return m_treeViewEditor->isTreeItemEditWidgetActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeView::selectedUiItems( std::vector<UiItem*>& objects )
{
    m_treeViewEditor->selectedUiItems( objects );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeView::slotOnSelectionChanged()
{
    emit selectionChanged();

    std::vector<UiItem*> objects;
    m_treeViewEditor->selectedUiItems( objects );
    ObjectHandle* objHandle = nullptr;

    if ( objects.size() )
    {
        ObjectUiCapability* uiObjH = dynamic_cast<ObjectUiCapability*>( objects[0] );
        if ( uiObjH )
        {
            objHandle = uiObjH->objectHandle();
        }
    }

    emit selectedObjectChanged( objHandle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeView::enableDefaultContextMenu( bool enable )
{
    m_treeViewEditor->enableDefaultContextMenu( enable );
}

//--------------------------------------------------------------------------------------------------
/// Enables or disables automatic updating of the SelectionManager selection state based on
/// the selections in this tree view
//--------------------------------------------------------------------------------------------------
void UiTreeView::enableSelectionManagerUpdating( bool enable )
{
    m_treeViewEditor->enableSelectionManagerUpdating( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeView::selectAsCurrentItem( const UiItem* uiItem )
{
    m_treeViewEditor->selectAsCurrentItem( uiItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeView::selectItems( const std::vector<const UiItem*>& uiItems )
{
    m_treeViewEditor->selectItems( uiItems );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeView::setExpanded( const UiItem* uiItem, bool doExpand ) const
{
    m_treeViewEditor->setExpanded( uiItem, doExpand );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem* UiTreeView::uiItemFromModelIndex( const QModelIndex& index ) const
{
    return m_treeViewEditor->uiItemFromModelIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex UiTreeView::findModelIndex( const UiItem* object ) const
{
    return m_treeViewEditor->findModelIndex( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeView::setDragDropInterface( UiDragDropInterface* dragDropInterface )
{
    m_treeViewEditor->setDragDropInterface( dragDropInterface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeView::enableAppendOfClassNameToUiItemText( bool enable )
{
    m_treeViewEditor->enableAppendOfClassNameToUiItemText( enable );
}

} // End of namespace caf
