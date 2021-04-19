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
#include "cafUiTableViewEditor.h"

#include "cafChildArrayField.h"
#include "cafField.h"
#include "cafObject.h"
#include "cafSelectionManager.h"
#include "cafUiCheckBoxDelegate.h"
#include "cafUiEditorHandle.h"
#include "cafUiTableViewDelegate.h"
#include "cafUiTableViewQModel.h"

#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QTableView>
#include <QWidget>

namespace caffa
{
CAF_UI_FIELD_EDITOR_SOURCE_INIT( UiTableViewEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTableViewEditor::UiTableViewEditor()
{
    m_tableView           = nullptr;
    m_tableHeading        = nullptr;
    m_tableModel          = nullptr;
    m_tableHeadingIcon    = nullptr;
    m_delegate            = nullptr;
    m_previousFieldHandle = nullptr;

    m_useDefaultContextMenu = false;

    m_checkboxDelegate = new UiCheckBoxDelegate();

    m_tableSelectionLevel               = SelectionManager::BASE_LEVEL;
    m_rowSelectionLevel                 = SelectionManager::FIRST_LEVEL;
    m_isBlockingSelectionManagerChanged = false;
    m_isUpdatingSelectionQModel         = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTableViewEditor::~UiTableViewEditor()
{
    if ( m_checkboxDelegate ) m_checkboxDelegate->deleteLater();
    if ( m_delegate ) m_delegate->deleteLater();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiTableViewEditor::createEditorWidget( QWidget* parent )
{
    m_tableModel = new UiTableViewQModel( parent );

    m_delegate = new UiTableViewDelegate( nullptr, m_tableModel );

    m_tableView = new QTableView( parent );
    m_tableView->setShowGrid( true );
    m_tableView->setModel( m_tableModel );
    m_tableView->installEventFilter( this );

    connect( m_tableView->selectionModel(),
             SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             SLOT( slotSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );

    return m_tableView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiTableViewEditor::createLabelWidget( QWidget* parent )
{
    if ( m_tableHeading.isNull() )
    {
        m_tableHeading = new QLabel( parent );
    }

    if ( m_tableHeadingIcon.isNull() )
    {
        m_tableHeadingIcon = new QLabel( parent );
    }

    QHBoxLayout* layoutForIconLabel = new QHBoxLayout();
    layoutForIconLabel->setMargin( 0 );
    layoutForIconLabel->addWidget( m_tableHeadingIcon );
    layoutForIconLabel->addSpacing( 3 );
    layoutForIconLabel->addWidget( m_tableHeading );
    layoutForIconLabel->addStretch();

    QWidget* widget = new QWidget( parent );
    widget->setLayout( layoutForIconLabel );

    return widget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableViewEditor::configureAndUpdateUi()
{
    if ( !m_tableModel ) return;

    auto childArrayFH = childArrayFieldHandle();

    UiTableViewEditorAttribute editorAttrib;
    bool                       editorAttribLoaded = false;

    if ( childArrayFH && childArrayFH->ownerObject() && childArrayFH->ownerObject()->capability<FieldUiCapability>() )
    {
        childArrayFH->ownerObject()->capability<ObjectUiCapability>()->editorAttribute( childArrayFH, &editorAttrib );
        editorAttribLoaded = true;

        this->setTableSelectionLevel( editorAttrib.tableSelectionLevel );
        this->setRowSelectionLevel( editorAttrib.rowSelectionLevel );
        this->enableHeaderText( editorAttrib.enableHeaderText );

        QPalette myPalette( m_tableView->palette() );
        myPalette.setColor( QPalette::Base, editorAttrib.baseColor );
        m_tableView->setPalette( myPalette );
    }

    m_tableModel->setArrayFieldAndBuildEditors( childArrayFH );

    if ( m_tableModel->rowCount() > 0 )
    {
        for ( int i = 0; i < m_tableModel->columnCount(); i++ )
        {
            if ( m_tableModel->isRepresentingBoolean( m_tableModel->index( 0, i ) ) )
            {
                m_tableView->setItemDelegateForColumn( i, m_checkboxDelegate );
            }
            else
            {
                m_tableView->setItemDelegateForColumn( i, m_delegate );
            }
        }
    }

    if ( childArrayFH && childArrayFH->capability<FieldUiCapability>() )
    {
        QString text         = "";
        auto    iconProvider = childArrayFH->capability<FieldUiCapability>()->uiIconProvider();
        QIcon   icon( QString::fromStdString( iconProvider->iconResourceString() ) );
        if ( !icon.isNull() )
        {
            m_tableHeadingIcon->setPixmap( icon.pixmap( 16, 16 ) );
            m_tableHeading->setText( QString::fromStdString( childArrayFH->capability<FieldUiCapability>()->uiName() ) +
                                     QString( " (%1)" ).arg( childArrayFH->size() ) );
        }
        else
        {
            m_tableHeadingIcon->setText( QString::fromStdString( childArrayFH->capability<FieldUiCapability>()->uiName() ) +
                                         QString( " (%1)" ).arg( childArrayFH->size() ) );
            m_tableHeading->setText( "" );
        }
        m_tableModel->createPersistentPushButtonWidgets( m_tableView );
    }
    else
    {
        m_tableHeading->setText( "" );
        m_tableHeadingIcon->setPixmap( QPixmap() );
    }

    bool firstTimeConfiguringField = m_previousFieldHandle != childArrayFH;

    if ( firstTimeConfiguringField )
    {
        if ( editorAttrib.minimumHeight > 0 )
        {
            m_tableView->setMinimumHeight( editorAttrib.minimumHeight );
        }

        // Set specified widths (if any)
        if ( editorAttribLoaded )
        {
            int colCount = m_tableModel->columnCount();
            for ( int c = 0; c < colCount && c < static_cast<int>( editorAttrib.columnWidths.size() ); c++ )
            {
                auto w = editorAttrib.columnWidths[c];
                if ( w > 0 ) m_tableView->setColumnWidth( c, w );
            }
        }
    }

    if ( firstTimeConfiguringField || editorAttrib.alwaysEnforceResizePolicy )
    {
        if ( editorAttrib.resizePolicy == UiTableViewEditorAttribute::RESIZE_TO_FIT_CONTENT )
        {
            // Set default column widths
            m_tableView->resizeColumnsToContents();
        }
        else if ( editorAttrib.resizePolicy == UiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER )
        {
#if QT_VERSION >= 0x050000
            m_tableView->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
#else
            m_tableView->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
#endif
        }
    }

    m_previousFieldHandle = childArrayFH;

    // Set default row heights
    m_tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableViewEditor::selectedUiItems( const QModelIndexList& modelIndexList, std::vector<UiItem*>& objects )
{
    for ( const QModelIndex& mi : modelIndexList )
    {
        int row = mi.row();

        caffa::ObjectUiCapability* uiObject = uiObj( m_tableModel->objectForRow( row ) );
        if ( uiObject )
        {
            objects.push_back( uiObject );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTableView* UiTableViewEditor::tableView()
{
    return m_tableView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableViewEditor::enableHeaderText( bool enable )
{
    m_tableHeading->setVisible( enable );
    m_tableHeadingIcon->setVisible( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableViewEditor::setTableSelectionLevel( int selectionLevel )
{
    m_tableSelectionLevel = selectionLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableViewEditor::setRowSelectionLevel( int selectionLevel )
{
    m_rowSelectionLevel = selectionLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableViewEditor::onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels )
{
    if ( !m_tableView->isVisible() || m_isBlockingSelectionManagerChanged ) return;

    if ( isSelectionRoleDefined() && ( changedSelectionLevels.count( m_rowSelectionLevel ) ) )
    {
        std::vector<UiItem*> items;
        SelectionManager::instance()->selectedItems( items, m_rowSelectionLevel );

        QItemSelection totalSelection;
        for ( auto item : items )
        {
            Object*        obj           = dynamic_cast<Object*>( item );
            QItemSelection itemSelection = m_tableModel->modelIndexFromObject( obj );
            totalSelection.merge( itemSelection, QItemSelectionModel::Select );
        }

        m_isUpdatingSelectionQModel = true;
        m_tableView->selectionModel()->select( totalSelection, QItemSelectionModel::SelectCurrent );
        m_isUpdatingSelectionQModel = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTableViewEditor::isMultiRowEditor() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// NOTE: If no selection role is defined, the selection manager is not changed, the selection in the
/// editor is local to the editor
//--------------------------------------------------------------------------------------------------
void UiTableViewEditor::slotSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
    if ( !m_isUpdatingSelectionQModel ) updateSelectionManagerFromTableSelection();

    // Hack to circumvent missing redraw from Qt in certain situations, when selection has changed

    m_tableView->resize( m_tableView->width(), m_tableView->height() + 1 );
    m_tableView->resize( m_tableView->width(), m_tableView->height() - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTableViewEditor::isSelectionRoleDefined() const
{
    return m_rowSelectionLevel != SelectionManager::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* UiTableViewEditor::objectFromModelIndex( const QModelIndex& mi )
{
    if ( mi.isValid() )
    {
        return m_tableModel->objectForRow( mi.row() );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableViewEditor::updateSelectionManagerFromTableSelection()
{
    if ( isSelectionRoleDefined() )
    {
        std::set<UiItem*> selectedRowObjects;
        QModelIndexList   modelIndexList = m_tableView->selectionModel()->selectedIndexes();
        for ( const QModelIndex& mi : modelIndexList )
        {
            ObjectHandle* obj = m_tableModel->objectForRow( mi.row() );

            if ( obj && obj->capability<FieldUiCapability>() )
            {
                selectedRowObjects.insert( obj->capability<FieldUiCapability>() );
            }
        }

        std::vector<SelectionManager::SelectionItem> newCompleteSelection;

        for ( auto item : selectedRowObjects )
        {
            newCompleteSelection.push_back( { item, m_rowSelectionLevel } );
        }

        if ( childArrayFieldHandle() && childArrayFieldHandle()->ownerObject() )
        {
            newCompleteSelection.push_back(
                { childArrayFieldHandle()->ownerObject()->capability<FieldUiCapability>(), m_tableSelectionLevel } );
        }

        m_isBlockingSelectionManagerChanged = true;
        SelectionManager::instance()->setSelection( newCompleteSelection );
        m_isBlockingSelectionManagerChanged = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTableViewEditor::eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type() == QEvent::FocusIn )
    {
        this->updateSelectionManagerFromTableSelection();
    }
    // standard event processing
    return QObject::eventFilter( obj, event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::ChildArrayFieldHandle* UiTableViewEditor::childArrayFieldHandle()
{
    caffa::ChildArrayFieldHandle* fieldHandle = nullptr;
    if ( this->uiField() )
    {
        fieldHandle = dynamic_cast<ChildArrayFieldHandle*>( this->uiField()->fieldHandle() );
    }

    return fieldHandle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableViewPushButtonEditorAttribute::registerPushButtonTextForFieldKeyword( const std::string& keyword,
                                                                                  const std::string& text )
{
    m_fieldKeywordAndPushButtonText[keyword] = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTableViewPushButtonEditorAttribute::showPushButtonForFieldKeyword( const std::string& keyword ) const
{
    if ( m_fieldKeywordAndPushButtonText.count( keyword ) > 0 ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string UiTableViewPushButtonEditorAttribute::pushButtonText( const std::string& keyword ) const
{
    if ( showPushButtonForFieldKeyword( keyword ) )
    {
        return m_fieldKeywordAndPushButtonText.at( keyword );
    }

    return "";
}

} // end namespace caffa
