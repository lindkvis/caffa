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

#include "cafPdmWebTreeViewEditor.h"

#include "cafChildArrayField.h"
#include "cafField.h"
#include "cafObject.h"
#include "cafUiCommandSystemProxy.h"
#include "cafUiDragDropInterface.h"
#include "cafUiEditorHandle.h"
#include "cafUiTreeOrdering.h"
#include "cafUiTreeViewAttribute.h"
#include "cafPdmWebTreeViewWModel.h"
#include "cafSelectionManager.h"

#include <Wt/WEvent.h>
#include <Wt/WFitLayout.h>

using namespace std::placeholders;

namespace caf
{
class PdmWebTreeWidget : public Wt::WTreeView
{
public:
    PdmWebTreeWidget()
        : Wt::WTreeView()
    {
        setLayoutSizeAware( true );
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebTreeViewEditor::PdmWebTreeViewEditor()
{
    m_useDefaultContextMenu       = false;
    m_updateSelectionManager      = false;
    m_appendClassNameToUiItemText = false;
    m_treeView                    = nullptr;
    m_treeViewModel               = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebTreeViewEditor::~PdmWebTreeViewEditor()
{
    m_treeViewModel->setPdmItemRoot( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebTreeViewEditor::createWidget()
{
    m_treeViewModel.reset( new PdmWebTreeViewWModel( this ) );
    m_treeView = new PdmWebTreeWidget;
    m_treeView->setModel( m_treeViewModel );
    m_treeView->setSortingEnabled( false );
    m_treeView->setAlternatingRowColors( true );
    m_treeView->setColumnResizeEnabled( false );
    m_treeView->selectionChanged().connect( std::bind( &PdmWebTreeViewEditor::slotOnSelectionChanged, this ) );
    updateContextMenuSignals();
    return m_treeView.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    PdmUiTreeViewEditorAttribute editorAttributes;

    {
        ObjectUiCapability* uiObjectHandle = dynamic_cast<ObjectUiCapability*>( this->pdmItemRoot() );
        if ( uiObjectHandle )
        {
            uiObjectHandle->objectEditorAttribute( uiConfigName, &editorAttributes );
        }
    }
    if ( editorAttributes.columnHeaders.empty() )
    {
        m_treeView->setHeaderHeight( 0 );
    }
    else
    {
        m_treeViewModel->setColumnHeaders( editorAttributes.columnHeaders );
    }
    m_treeViewModel->setUiConfigName( uiConfigName );
    m_treeViewModel->setPdmItemRoot( this->pdmItemRoot() );

    if ( editorAttributes.currentObject )
    {
        auto uiObjectHandle = editorAttributes.currentObject->capability<ObjectUiCapability>();
        if ( uiObjectHandle )
        {
            selectAsCurrentItem( uiObjectHandle );
        }
    }
    m_treeView->refresh();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::updateMySubTree( UiItem* uiItem )
{
    if ( m_treeViewModel )
    {
        m_treeViewModel->updateSubTree( uiItem );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WTreeView* PdmWebTreeViewEditor::treeView()
{
    return m_treeView.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::selectedUiItems( std::vector<UiItem*>& objects )
{
    if ( !this->treeView() ) return;

    Wt::WModelIndexSet idxList = this->treeView()->selectionModel()->selectedIndexes();

    for ( auto idx : idxList )
    {
        caf::UiItem* item = this->m_treeViewModel->uiItemFromModelIndex( idx );
        if ( item )
        {
            objects.push_back( item );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::enableDefaultContextMenu( bool enable )
{
    m_useDefaultContextMenu = enable;

    updateContextMenuSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::enableSelectionManagerUpdating( bool enable )
{
    m_updateSelectionManager = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::updateContextMenuSignals()
{
    if ( !m_treeView ) return;

    if ( !m_useDefaultContextMenu )
    {
        m_treeView->setAttributeValue( "oncontextmenu",
                                       "event.cancelBubble = true; event.returnValue = false; return false;" );
        // m_treeView->mouseWentUp().connect(std::bind(&PdmWebTreeViewEditor::slotCustomMenuRequested, this, _1, _2));
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::slotCustomMenuRequested( const Wt::WModelIndex& item, const Wt::WMouseEvent& event )
{
    if ( event.button() == Wt::MouseButton::Right )
    {
        // Select the item, it was not yet selected.
        SelectionManager::instance()->setActiveChildArrayFieldHandle( this->currentChildArrayFieldHandle() );
        // Select the item, it was not yet selected.
        if ( !m_treeView->isSelected( item ) )
        {
            m_treeView->select( item );
            //            SelectionManager::instance()->setSelectedItem(uiItemFromModelIndex(item));
        }

        m_popup = std::make_unique<WPopupMenuWrapper>();
        caf::UiCommandSystemProxy::instance()->populateMenuWithDefaultCommands( "PdmUiTreeViewEditor", m_popup.get() );
        m_popup->menu()->aboutToHide().connect( std::bind( &PdmWebTreeViewEditor::slotOnActionSelection, this ) );

        m_popup->refreshEnabledState();
        if ( m_popup->menu()->isHidden() )
            m_popup->menu()->popup( event );
        else
            m_popup->menu()->hide();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ChildArrayFieldHandle* PdmWebTreeViewEditor::currentChildArrayFieldHandle()
{
    UiItem* currentSelectedItem = SelectionManager::instance()->selectedItem( SelectionManager::FIRST_LEVEL );

    FieldUiCapability* uiFieldHandle = dynamic_cast<FieldUiCapability*>( currentSelectedItem );
    if ( uiFieldHandle )
    {
        FieldHandle* fieldHandle = uiFieldHandle->fieldHandle();

        if ( dynamic_cast<ChildArrayFieldHandle*>( fieldHandle ) )
        {
            return dynamic_cast<ChildArrayFieldHandle*>( fieldHandle );
        }
    }

    ObjectHandle* pdmObject = dynamic_cast<caf::ObjectHandle*>( currentSelectedItem );
    if ( pdmObject )
    {
        ChildArrayFieldHandle* parentChildArray = dynamic_cast<ChildArrayFieldHandle*>( pdmObject->parentField() );

        if ( parentChildArray )
        {
            return parentChildArray;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::selectAsCurrentItem( const UiItem* uiItem )
{
    Wt::WModelIndex index = m_treeViewModel->findModelIndex( uiItem );
    m_treeView->clearSelection();
    m_treeView->select( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::selectItems( std::vector<const UiItem*> uiItems )
{
    m_treeView->clearSelection();

    if ( uiItems.empty() )
    {
        return;
    }

    Wt::WModelIndexSet selectedIndices;

    for ( const UiItem* uiItem : uiItems )
    {
        Wt::WModelIndex itemIndex = findModelIndex( uiItem );
        selectedIndices.insert( itemIndex );
    }
    m_treeView->setSelectedIndexes( selectedIndices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::slotOnSelectionChanged()
{
    this->updateSelectionManager();

    selectionChanged().emit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::slotOnActionSelection()
{
    if ( m_popup->menu()->result() )
    {
        Wt::WString                    text   = m_popup->menu()->result()->text();
        std::shared_ptr<ActionWrapper> action = m_popup->findAction( QString::fromStdString( text.narrow() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::setExpanded( const UiItem* uiItem, bool doExpand ) const
{
    Wt::WModelIndex index = m_treeViewModel->findModelIndex( uiItem );
    m_treeView->setExpanded( index, doExpand );

    if ( doExpand )
    {
        m_treeView->scrollTo( index );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem* PdmWebTreeViewEditor::uiItemFromModelIndex( const Wt::WModelIndex& index ) const
{
    return m_treeViewModel->uiItemFromModelIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WModelIndex PdmWebTreeViewEditor::findModelIndex( const UiItem* object ) const
{
    return m_treeViewModel->findModelIndex( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::setDragDropInterface( PdmUiDragDropInterface* dragDropInterface )
{
    m_treeViewModel->setDragDropInterface( dragDropInterface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::Signal<>& PdmWebTreeViewEditor::selectionChanged()
{
    return m_selectionChanged;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmWebTreeViewEditor::eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type() == QEvent::FocusIn )
    {
        this->updateSelectionManager();
    }

    // standard event processing
    return QObject::eventFilter( obj, event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::updateSelectionManager()
{
    if ( m_updateSelectionManager )
    {
        std::vector<UiItem*> items;
        this->selectedUiItems( items );
        SelectionManager::instance()->setSelectedItems( items );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewEditor::enableAppendOfClassNameToUiItemText( bool enable )
{
    m_appendClassNameToUiItemText = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmWebTreeViewEditor::isAppendOfClassNameToUiItemTextEnabled()
{
    return m_appendClassNameToUiItemText;
}

} // end namespace caf
