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

#include "cafPdmWebTreeView.h"

#include "cafPdmObject.h"
#include "cafPdmWebDefaultObjectEditor.h"
#include "cafPdmWebTreeViewEditor.h"

#include <Wt/WFitLayout.h>
#include <Wt/WLabel.h>
#include <Wt/WTreeView.h>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebTreeView::PdmWebTreeView()
{
    this->setMargin( 0 );
    m_treeViewEditor.reset( new PdmWebTreeViewEditor() );
    this->setCentralWidget( std::unique_ptr<Wt::WWidget>( m_treeViewEditor->getOrCreateWidget() ) );
    m_treeViewEditor->selectionChanged().connect( this, &PdmWebTreeView::slotOnSelectionChanged );
    this->setMinimumSize( 150, 100 );
    this->setTitle( "Project Tree" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebTreeView::~PdmWebTreeView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeView::setUiConfigurationName( QString uiConfigName )
{
    // Reset everything, and possibly create widgets etc afresh
    if ( m_uiConfigName != uiConfigName )
    {
        m_uiConfigName = uiConfigName;

        m_treeViewEditor->updateUi( m_uiConfigName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeView::setPdmItem( caf::PdmUiItem* object )
{
    m_treeViewEditor->setPdmItemRoot( object );
    m_treeViewEditor->updateUi( m_uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WTreeView* PdmWebTreeView::treeView()
{
    return m_treeViewEditor->treeView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeView::selectedUiItems( std::vector<PdmUiItem*>& objects )
{
    m_treeViewEditor->selectedUiItems( objects );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeView::slotOnSelectionChanged()
{
    m_selectionChanged.emit();

    std::vector<PdmUiItem*> objects;
    m_treeViewEditor->selectedUiItems( objects );
    PdmObjectHandle* objHandle = nullptr;

    if ( objects.size() )
    {
        PdmObjectUiCapability* uiObjH = dynamic_cast<PdmObjectUiCapability*>( objects[0] );
        if ( uiObjH )
        {
            objHandle = uiObjH->objectHandle();
        }
    }

    m_selectedObjectChanged.emit( objHandle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeView::enableDefaultContextMenu( bool enable )
{
    m_treeViewEditor->enableDefaultContextMenu( enable );
}

//--------------------------------------------------------------------------------------------------
/// Enables or disables automatic updating of the SelectionManager selection state based on
/// the selections in this tree view
//--------------------------------------------------------------------------------------------------
void PdmWebTreeView::enableSelectionManagerUpdating( bool enable )
{
    m_treeViewEditor->enableSelectionManagerUpdating( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeView::selectAsCurrentItem( const PdmUiItem* uiItem )
{
    m_treeViewEditor->selectAsCurrentItem( uiItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeView::selectItems( const std::vector<const PdmUiItem*>& uiItems )
{
    m_treeViewEditor->selectItems( uiItems );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeView::setExpanded( const PdmUiItem* uiItem, bool doExpand ) const
{
    m_treeViewEditor->setExpanded( uiItem, doExpand );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItem* PdmWebTreeView::uiItemFromModelIndex( const Wt::WModelIndex& index ) const
{
    return m_treeViewEditor->uiItemFromModelIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WModelIndex PdmWebTreeView::findModelIndex( const PdmUiItem* object ) const
{
    return m_treeViewEditor->findModelIndex( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::Signal<>& PdmWebTreeView::selectionChanged()
{
    return m_selectionChanged;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::Signal<caf::PdmObjectHandle*>& PdmWebTreeView::selectedObjectChanged()
{
    return m_selectedObjectChanged;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
/*void PdmWebTreeView::setDragDropInterface(PdmUiDragDropInterface* dragDropInterface)
{
    m_treeViewEditor->setDragDropInterface(dragDropInterface);
} */

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeView::enableAppendOfClassNameToUiItemText( bool enable )
{
    m_treeViewEditor->enableAppendOfClassNameToUiItemText( enable );
}

} // End of namespace caf
