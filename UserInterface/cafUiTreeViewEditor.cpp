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

#include "cafUiTreeViewEditor.h"

#include "cafChildArrayField.h"
#include "cafField.h"
#include "cafObject.h"
#include "cafQActionWrapper.h"
#include "cafSelectionManager.h"
#include "cafUiCommandSystemProxy.h"
#include "cafUiDragDropInterface.h"
#include "cafUiEditorHandle.h"
#include "cafUiTreeOrdering.h"
#include "cafUiTreeViewQModel.h"

#include <QApplication>
#include <QDragMoveEvent>
#include <QEvent>
#include <QGridLayout>
#include <QMenu>
#include <QModelIndexList>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QStyleOptionViewItem>
#include <QTreeView>
#include <QWidget>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class UiTreeViewWidget : public QTreeView
{
public:
    explicit UiTreeViewWidget( QWidget* parent = nullptr )
        : QTreeView( parent ){};
    ~UiTreeViewWidget() override{};

    bool isTreeItemEditWidgetActive() const { return state() == QAbstractItemView::EditingState; }

protected:
    void dragMoveEvent( QDragMoveEvent* event ) override
    {
        caffa::UiTreeViewQModel* treeViewModel = dynamic_cast<caffa::UiTreeViewQModel*>( model() );
        if ( treeViewModel && treeViewModel->dragDropInterface() )
        {
            treeViewModel->dragDropInterface()->onProposedDropActionUpdated( event->proposedAction() );
        }

        QTreeView::dragMoveEvent( event );
    }

    void dragLeaveEvent( QDragLeaveEvent* event ) override
    {
        caffa::UiTreeViewQModel* treeViewModel = dynamic_cast<caffa::UiTreeViewQModel*>( model() );
        if ( treeViewModel && treeViewModel->dragDropInterface() )
        {
            treeViewModel->dragDropInterface()->onDragCanceled();
        }

        QTreeView::dragLeaveEvent( event );
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTreeViewEditor::UiTreeViewEditor()
{
    m_useDefaultContextMenu       = false;
    m_updateSelectionManager      = false;
    m_appendClassNameToUiItemText = false;
    m_layout                      = nullptr;
    m_treeView                    = nullptr;
    m_treeViewModel               = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTreeViewEditor::~UiTreeViewEditor()
{
    m_treeViewModel->setItemRoot( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiTreeViewEditor::createWidget( QWidget* parent )
{
    m_mainWidget = new QWidget( parent );
    m_layout     = new QVBoxLayout();
    m_layout->setContentsMargins( 0, 0, 0, 0 );
    m_mainWidget->setLayout( m_layout );

    m_treeViewModel = new caffa::UiTreeViewQModel( this );
    m_treeView      = new UiTreeViewWidget( m_mainWidget );
    m_treeView->setModel( m_treeViewModel );
    m_treeView->installEventFilter( this );
    m_treeView->setRootIsDecorated( true );
    m_treeView->setRootIndex( QModelIndex() );

    connect( treeView()->selectionModel(),
             SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             SLOT( slotOnSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );

    m_layout->addWidget( m_treeView );

    updateContextMenuSignals();

    return m_mainWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewEditor::configureAndUpdateUi()
{
    UiTreeViewEditorAttribute editorAttributes;

    {
        ObjectUiCapability* uiObjectHandle = dynamic_cast<ObjectUiCapability*>( this->itemRoot() );
        if ( uiObjectHandle )
        {
            uiObjectHandle->objectEditorAttribute( &editorAttributes );
        }
    }

    QStringList columnHeaders;
    for ( auto header : editorAttributes.columnHeaders )
    {
        columnHeaders.push_back( QString::fromStdString( header ) );
    }
    m_treeViewModel->setColumnHeaders( columnHeaders );
    if ( m_treeView )
    {
        m_treeView->setHeaderHidden( columnHeaders.empty() );
    }

    m_treeViewModel->setItemRoot( this->itemRoot() );

    if ( editorAttributes.currentObject )
    {
        ObjectUiCapability* uiObjectHandle = editorAttributes.currentObject->capability<ObjectUiCapability>();
        if ( uiObjectHandle )
        {
            selectAsCurrentItem( uiObjectHandle );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTreeView* UiTreeViewEditor::treeView()
{
    return m_treeView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTreeViewEditor::isTreeItemEditWidgetActive() const
{
    return m_treeView->isTreeItemEditWidgetActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewEditor::selectedUiItems( std::vector<UiItem*>& objects )
{
    if ( !this->treeView() ) return;

    QModelIndexList idxList = this->treeView()->selectionModel()->selectedIndexes();

    for ( int i = 0; i < idxList.size(); i++ )
    {
        caffa::UiItem* item = this->m_treeViewModel->uiItemFromModelIndex( idxList[i] );
        if ( item )
        {
            objects.push_back( item );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewEditor::updateMySubTree( UiItem* uiItem )
{
    if ( m_treeViewModel )
    {
        m_treeViewModel->updateSubTree( uiItem );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewEditor::enableDefaultContextMenu( bool enable )
{
    m_useDefaultContextMenu = enable;

    updateContextMenuSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewEditor::enableSelectionManagerUpdating( bool enable )
{
    m_updateSelectionManager = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewEditor::updateContextMenuSignals()
{
    if ( !m_treeView ) return;

    if ( m_useDefaultContextMenu )
    {
        m_treeView->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( m_treeView, SIGNAL( customContextMenuRequested( QPoint ) ), SLOT( customMenuRequested( QPoint ) ) );
    }
    else
    {
        m_treeView->setContextMenuPolicy( Qt::DefaultContextMenu );
        disconnect( m_treeView, nullptr, this, nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewEditor::customMenuRequested( QPoint pos )
{
    // This seems a bit strange. Why ?
    SelectionManager::instance()->setActiveChildArrayFieldHandle( this->currentChildArrayFieldHandle() );

    // TODO: abstract away the dependency on Qt and reintroduce the methods
    /* UiCommandSystemProxy::instance()->setCurrentContextMenuTargetWidget( m_mainWidget->parentWidget() );
    caffa::QMenuWrapper menuWrapper;

    caffa::UiCommandSystemProxy::instance()->populateMenuWithDefaultCommands( "UiTreeViewEditor", &menuWrapper );

    if ( menuWrapper.actions().size() > 0 )
    {
        // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the
    viewport(). QPoint globalPos = m_treeView->viewport()->mapToGlobal( pos );

        menuWrapper.menu()->exec( globalPos );
    }

    UiCommandSystemProxy::instance()->setCurrentContextMenuTargetWidget( nullptr ); */
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ChildArrayFieldHandle* UiTreeViewEditor::currentChildArrayFieldHandle()
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

    ObjectHandle* object = dynamic_cast<caffa::ObjectHandle*>( currentSelectedItem );
    if ( object )
    {
        ChildArrayFieldHandle* parentChildArray = dynamic_cast<ChildArrayFieldHandle*>( object->parentField() );

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
void UiTreeViewEditor::selectAsCurrentItem( const UiItem* uiItem )
{
    QModelIndex index        = m_treeViewModel->findModelIndex( uiItem );
    QModelIndex currentIndex = m_treeView->currentIndex();

    m_treeView->clearSelection();

    m_treeView->setCurrentIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewEditor::selectItems( std::vector<const UiItem*> uiItems )
{
    m_treeView->clearSelection();

    if ( uiItems.empty() )
    {
        return;
    }

    QModelIndex index = findModelIndex( uiItems.back() );
    m_treeView->setCurrentIndex( index );

    for ( const UiItem* uiItem : uiItems )
    {
        QModelIndex itemIndex = findModelIndex( uiItem );
        m_treeView->selectionModel()->select( itemIndex, QItemSelectionModel::Select );
    }
    m_treeView->setFocus( Qt::MouseFocusReason );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewEditor::slotOnSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
    this->updateSelectionManager();

    emit selectionChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewEditor::setExpanded( const UiItem* uiItem, bool doExpand ) const
{
    QModelIndex index = m_treeViewModel->findModelIndex( uiItem );
    m_treeView->setExpanded( index, doExpand );

    if ( doExpand )
    {
        m_treeView->scrollTo( index );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem* UiTreeViewEditor::uiItemFromModelIndex( const QModelIndex& index ) const
{
    return m_treeViewModel->uiItemFromModelIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex UiTreeViewEditor::findModelIndex( const UiItem* object ) const
{
    return m_treeViewModel->findModelIndex( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewEditor::setDragDropInterface( UiDragDropInterface* dragDropInterface )
{
    m_treeViewModel->setDragDropInterface( dragDropInterface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTreeViewEditor::eventFilter( QObject* obj, QEvent* event )
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
void UiTreeViewEditor::updateSelectionManager()
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
void UiTreeViewEditor::enableAppendOfClassNameToUiItemText( bool enable )
{
    m_appendClassNameToUiItemText = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTreeViewEditor::isAppendOfClassNameToUiItemTextEnabled()
{
    return m_appendClassNameToUiItemText;
}
} // end namespace caffa
