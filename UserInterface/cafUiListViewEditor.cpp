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

#include "cafUiListViewEditor.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafObjectGroup.h"
#include "cafQVariantConverter.h"
#include "cafUiEditorHandle.h"

#include <QGridLayout>
#include <QTableView>
#include <QWidget>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiListViewModel::UiListViewModel( QObject* parent )
    : QAbstractTableModel( parent )
{
    m_columnCount = 0;
    m_objectGroup = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiListViewModel::rowCount( const QModelIndex& parent /*= QModelIndex( ) */ ) const
{
    if ( !m_objectGroup )
    {
        return 0;
    }

    return static_cast<int>( m_objectGroup->objects.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiListViewModel::columnCount( const QModelIndex& parent /*= QModelIndex( ) */ ) const
{
    if ( !m_objectGroup )
    {
        return 0;
    }

    if ( m_columnCount < 0 )
    {
        return 0;
    }

    return m_columnCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiListViewModel::computeColumnCount()
{
    if ( m_editorAttribute.fieldNames.size() > 0 )
    {
        m_columnCount = static_cast<int>( m_editorAttribute.fieldNames.size() );
    }
    else if ( m_objectGroup )
    {
        m_columnCount = 0;

        // Loop over all objects and find the object with largest number of fields
        for ( auto object : m_objectGroup->objects )
        {
            m_columnCount = std::max( m_columnCount, (int)object->fields().size() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant UiListViewModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */ ) const
{
    return QVariant( QString( "Header %1" ).arg( section ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant caffa::UiListViewModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole */ ) const
{
    if ( m_objectGroup && ( role == Qt::DisplayRole || role == Qt::EditRole ) )
    {
        if ( index.row() < static_cast<int>( m_objectGroup->objects.size() ) )
        {
            ObjectHandle* object = m_objectGroup->objects[index.row()];
            if ( object )
            {
                std::vector<FieldHandle*> fields = object->fields();

                if ( index.column() < static_cast<int>( fields.size() ) )
                {
                    size_t fieldIndex = 0;

                    if ( m_editorAttribute.fieldNames.size() > 0 )
                    {
                        auto fieldName = m_editorAttribute.fieldNames[index.column()];
                        for ( size_t i = 0; i < fields.size(); i++ )
                        {
                            if ( fields[i]->keyword() == fieldName )
                            {
                                fieldIndex = i;
                                break;
                            }
                        }
                    }
                    else
                    {
                        fieldIndex = index.column();
                    }

                    FieldUiCapability* uiFieldHandle = fields[fieldIndex]->capability<FieldUiCapability>();
                    if ( uiFieldHandle )
                    {
                        return QVariant::fromValue( uiFieldHandle->uiValue() );
                    }
                    else
                    {
                        return QVariant();
                    }
                }
            }
        }
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::UiListViewModel::setData( ObjectCollection* objectGroup )
{
    m_objectGroup = objectGroup;

    if ( m_objectGroup )
    {
        caffa::ObjectUiCapability* uiObject = uiObj( m_objectGroup );
        if ( uiObject )
        {
            uiObject->objectEditorAttribute( &m_editorAttribute );
        }
    }

    computeColumnCount();
    beginResetModel();
    endResetModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiListViewEditor::UiListViewEditor()
    : m_tableView( nullptr )
    , m_tableModel( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiListViewEditor::~UiListViewEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiListViewEditor::createWidget( QWidget* parent )
{
    CAFFA_ASSERT( parent );

    QWidget*     mainWidget = new QWidget( parent );
    QVBoxLayout* layout     = new QVBoxLayout();
    mainWidget->setLayout( layout );

    m_tableModel = new UiListViewModel( mainWidget );

    m_tableView = new QTableView( mainWidget );
    m_tableView->setShowGrid( false );
    m_tableView->setModel( m_tableModel );

    layout->addWidget( m_tableView );

    return mainWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiListViewEditor::configureAndUpdateUi()
{
    ObjectCollection* objectGroup = dynamic_cast<ObjectCollection*>( object() );
    m_tableModel->setData( objectGroup );

    m_tableView->resizeColumnsToContents();
}

} // end namespace caffa
